#include "pch.h"
#include "jnc_io_UsbMonitor.h"
#include "jnc_io_UsbMonLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	UsbMonitor,
	"io.UsbMonitor",
	g_usbMonLibGuid,
	UsbMonLibTypeCacheSlot_UsbMonitor,
	UsbMonitor,
	&UsbMonitor::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbMonitor)
	JNC_MAP_CONSTRUCTOR(&sl::construct<UsbMonitor>)
	JNC_MAP_DESTRUCTOR(&sl::destruct<UsbMonitor>)

	JNC_MAP_AUTOGET_PROPERTY("m_kernelBufferSize", &UsbMonitor::setKernelBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readParallelism",  &UsbMonitor::setReadParallelism)
	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize",    &UsbMonitor::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize",   &UsbMonitor::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_addressFilter",    &UsbMonitor::setAddressFilter)
	JNC_MAP_AUTOGET_PROPERTY("m_options",          &UsbMonitor::setOptions)

	JNC_MAP_FUNCTION("open",         &UsbMonitor::open)
	JNC_MAP_FUNCTION("close",        &UsbMonitor::close)
	JNC_MAP_FUNCTION("read",         &UsbMonitor::read)
	JNC_MAP_FUNCTION("wait",         &UsbMonitor::wait)
	JNC_MAP_FUNCTION("cancelWait",   &UsbMonitor::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &UsbMonitor::blockingWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

UsbMonitor::UsbMonitor() {
	m_readParallelism = 1; // Def_ReadParallelism;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_kernelBufferSize = Def_KernelBufferSize;
	m_addressFilter = 0;

	m_readBuffer.setBufferSize(Def_ReadBufferSize);

#if (_AXL_OS_WIN)
	m_overlappedIo = NULL;
#endif
}

bool
JNC_CDECL
UsbMonitor::setOptions(uint_t options) {
	if (m_isOpen)
		return err::fail(err::SystemErrorCode_InvalidDeviceState);

	m_options = options & ~AsyncIoDeviceOption_KeepReadBlockSize;
	return true;
}

bool
JNC_CDECL
UsbMonitor::open(
	DataPtr captureDeviceNamePtr,
	size_t snapshotLength
) {
	const char* captureDeviceName = (const char*)captureDeviceNamePtr.m_p;

	close();

	bool result = requireDevMonCapability();
	if (!result)
		return false;

	if (snapshotLength == 0)
		snapshotLength = Def_SnapshotLength;

#if (_AXL_OS_WIN)
	result =
		m_monitor.open(captureDeviceName) &&
		m_monitor.setSnapshotLength(snapshotLength) &&
		m_monitor.setKernelBufferSize(m_kernelBufferSize) &&
		m_monitor.setFilter(m_addressFilter) &&
		m_monitor.readPcapHdr();

	ASSERT(!m_overlappedIo);
	m_overlappedIo = AXL_MEM_NEW(OverlappedIo);
#elif (_AXL_OS_LINUX
	bool result =
		m_monitor.open(captureDeviceName, O_RDWR | O_NONBLOCK) &&
		m_monitor.setKernelBufferSize(m_kernelBufferSize) &&

	m_monitor.m_addressFilter = m_addressFilter;
#endif

	if (!result)
		return false;

	AsyncIoDevice::open();
	m_ioThread.start();
	return true;
}

void
JNC_CDECL
UsbMonitor::close() {
	if (!m_monitor.isOpen())
		return;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread();
	m_lock.unlock();

	GcHeap* gcHeap = m_runtime->getGcHeap();
	gcHeap->enterWaitRegion();
	m_ioThread.waitAndClose();
	gcHeap->leaveWaitRegion();

	m_monitor.close();
	AsyncIoDevice::close();

#if (_AXL_OS_WIN)
	if (m_overlappedIo) {
		AXL_MEM_DELETE(m_overlappedIo);
		m_overlappedIo = NULL;
	}
#endif
}

bool
JNC_CDECL
UsbMonitor::setAddressFilter(uint_t address) {
#if (_AXL_OS_WIN)
	if (m_monitor.isOpen() && !m_monitor.setFilter(address))
		return false;
#elif (_AXL_OS_LINUX)
	m_monitor.m_addressFilter = address;
#endif
	m_addressFilter = address;
	return true;
}

#if (_JNC_OS_WIN)

void
UsbMonitor::ioThreadFunc() {
	ASSERT(m_monitor.isOpen() && m_overlappedIo);

	bool result;
	axl::io::win::UsbPcapTransferParser parser;

	HANDLE waitTable[2] = {
		m_ioThreadEvent.m_event,
		NULL, // placeholder for read completion event
	};

	size_t waitCount = 1; // always 1 or 2

	m_ioThreadEvent.signal(); // do initial update of active events

	// read loop

	for (;;) {
		DWORD waitResult = ::WaitForMultipleObjects(waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED) {
			setIoErrorEvent(err::getLastSystemErrorCode());
			return;
		}

		// do as much as we can without lock

		while (!m_overlappedIo->m_activeOverlappedReadList.isEmpty()) {
			OverlappedRead* read = *m_overlappedIo->m_activeOverlappedReadList.getHead();
			result = read->m_overlapped.m_completionEvent.wait(0);
			if (!result)
				break;

			size_t actualSize = m_monitor.getOverlappedResult(&read->m_overlapped);
			if (actualSize == -1) {
				setIoErrorEvent();
				return;
			}

			m_overlappedIo->m_activeOverlappedReadList.remove(read);
			m_overlappedIo->m_overlappedReadPool.put(read);
			read->m_overlapped.m_completionEvent.reset();

			const char* p = read->m_buffer;
			const char* end = p + actualSize;

			m_lock.lock(); // the main read buffer must be lock-protected

			while (p < end) {
				size_t size = parser.parse(p, end - p);
				if (size == -1) {
					m_lock.unlock();
					setIoErrorEvent();
					return;
				}

				axl::io::UsbMonTransferParseState state = parser.getState();
				switch (state) {
				case axl::io::UsbMonTransferParseState_CompleteHeader:
					addToReadBuffer(parser.getTransferHdr(), sizeof(axl::io::UsbMonTransferHdr));
					break;

				case axl::io::UsbMonTransferParseState_IncompleteData:
				case axl::io::UsbMonTransferParseState_CompleteData:
					addToReadBuffer(p, size);
					break;
				}

				p += size;
			}

			m_lock.unlock();
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			break;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		updateReadWriteBufferEvents();

		// take snapshots before releasing the lock

		bool isReadBufferFull = m_readBuffer.isFull();
		size_t readParallelism = m_readParallelism;
		size_t readBlockSize = m_readBlockSize;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();

		size_t activeReadCount = m_overlappedIo->m_activeOverlappedReadList.getCount();
		if (!isReadBufferFull && activeReadCount < readParallelism) {
			size_t newReadCount = readParallelism - activeReadCount;

			for (size_t i = 0; i < newReadCount; i++) {
				OverlappedRead* read = m_overlappedIo->m_overlappedReadPool.get();

				result =
					read->m_buffer.setCount(readBlockSize) &&
					m_monitor.overlappedRead(read->m_buffer, readBlockSize, &read->m_overlapped);

				if (!result) {
					m_overlappedIo->m_overlappedReadPool.put(read);
					setIoErrorEvent();
					return;
				}

				m_overlappedIo->m_activeOverlappedReadList.insertTail(read);
			}
		}

		if (m_overlappedIo->m_activeOverlappedReadList.isEmpty()) {
			waitCount = 1;
		} else {
			OverlappedRead* read = *m_overlappedIo->m_activeOverlappedReadList.getHead();
			waitTable[1] = read->m_overlapped.m_completionEvent.m_event;
			waitCount = 2;
		}
	}
}

#elif (_JNC_OS_POSIX)

void
UsbMonitor::ioThreadFunc() {
	ASSERT(m_monitor.isOpen());

	int result = connectLoop();
	if (!result)
		return;

	int selectFd = AXL_MAX(m_monitor.m_device, m_ioThreadSelfPipe.m_readFile) + 1;

	sl::Array<char> readBlock;
	readBlock.setCount(Def_ReadBlockSize);

	bool canReadMonitor = false;

	// read loop

	for (;;) {
		fd_set readSet = { 0 };
		FD_SET(m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canReadMonitor)
			FD_SET(m_monitor.m_device, &readSet);

		result = ::select(selectFd, &readSet, NULL, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET(m_ioThreadSelfPipe.m_readFile, &readSet)) {
			char buffer[256];
			m_ioThreadSelfPipe.read(buffer, sizeof(buffer));
		}

		if (FD_ISSET(m_monitor.m_device, &readSet))
			canReadMonitor = true;

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		readBlock.setCount(m_readBlockSize); // update read block size

		while (canReadMonitor && !m_readBuffer.isFull()) {
			ssize_t actualSize = ::read(m_monitor.m_device, readBlock, readBlock.getCount());
			if (actualSize == -1) {
				if (errno == EAGAIN) {
					canReadMonitor = false;
				} else {
					setIoErrorEvent_l(err::Errno(errno));
					return;
				}
			} else {
				addToReadBuffer(readBlock, actualSize);
			}
		}

		updateReadWriteBufferEvents();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
