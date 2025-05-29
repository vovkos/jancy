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
	UsbMonLibCacheSlot_UsbMonitor,
	UsbMonitor,
	&UsbMonitor::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbMonitor)
	JNC_MAP_CONSTRUCTOR(&sl::construct<UsbMonitor>)
	JNC_MAP_DESTRUCTOR(&sl::destruct<UsbMonitor>)

	JNC_MAP_AUTOGET_PROPERTY("m_kernelBufferSize", &UsbMonitor::setKernelBufferSize)
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
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_kernelBufferSize = Def_KernelBufferSize;
	m_addressFilter = 0;

	m_readBuffer.setBufferSize(Def_ReadBufferSize);

#if (_AXL_OS_WIN)
	m_overlappedIo = NULL;
#endif
	m_transferTracker = NULL;
}

bool
JNC_CDECL
UsbMonitor::setKernelBufferSize(size_t size) {
	if (m_isOpen) {
#if (_AXL_OS_WIN)
		err::setError("USBPcap has a bug that leads to BSOD on changing kernel buffer size");
		return false;
#else
		if (!m_monitor.setKernelBufferSize(size))
			return false;
#endif
	}

	m_kernelBufferSize = size;
	return true;
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
	String captureDeviceName,
	size_t snapshotLength
) {
	close();

	bool result = requireUsbCapability();
	if (!result)
		return false;

	if (snapshotLength == 0)
		snapshotLength = Def_SnapshotLength;

#if (_AXL_OS_WIN)
	result =
		m_monitor.open(captureDeviceName >> toAxl) &&
		m_monitor.setSnapshotLength(snapshotLength) &&
		m_monitor.setKernelBufferSize(m_kernelBufferSize) &&
		m_monitor.setFilter(m_addressFilter) &&
		m_monitor.readPcapHdr();

	if (!result)
		return false;

	ASSERT(!m_overlappedIo);
	m_overlappedIo = new OverlappedIo;
#elif (_AXL_OS_LINUX)
	result =
		m_monitor.open(captureDeviceName >> toAxl, O_RDWR | O_NONBLOCK) &&
		m_monitor.setKernelBufferSize(m_kernelBufferSize);

	if (!result)
		return false;
#endif

	ASSERT(!m_transferTracker);
	if (m_options & UsbMonOption_CompletedTransfersOnly)
		m_transferTracker = new TransferTracker;

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
	delete m_overlappedIo;
	m_overlappedIo = NULL;
#endif

	delete m_transferTracker;
	m_transferTracker = NULL;
}

bool
JNC_CDECL
UsbMonitor::setAddressFilter(uint_t address) {
#if (_AXL_OS_WIN)
	if (m_monitor.isOpen() && !m_monitor.setFilter(address))
		return false;
#endif
	m_addressFilter = address;
	return true;
}

bool
UsbMonitor::parseAllTransfers_l(
	TransferParser* parser,
	TransferParserContext* context,
	const char* p,
	const char* end
) {
	while (p < end) {
		size_t size = parser->parse(p, end - p);
		if (size == -1)
			return false;

		const axl::io::UsbMonTransferHdr* hdr;
		axl::io::UsbMonTransferParserState state = parser->getState();
		switch (state) {
		case axl::io::UsbMonTransferParserState_CompleteHeader:
			hdr = parser->getTransferHdr();
#if (_AXL_OS_LINUX)
			context->m_dataMode = !m_addressFilter || hdr->m_address == m_addressFilter ?
				TransferDataMode_AddToBuffer :
				TransferDataMode_Ignore;

			if (!context->m_dataMode)
				break;
#endif

			addToReadBuffer(hdr, sizeof(axl::io::UsbMonTransferHdr));

			if (hdr->m_transferType == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)
				addToReadBuffer(
					parser->getIsoPacketArray(),
					hdr->m_isoHdr.m_packetCount * sizeof(axl::io::UsbMonIsoPacket)
				);

			break;

		case axl::io::UsbMonTransferParserState_IncompleteData:
		case axl::io::UsbMonTransferParserState_CompleteData:
		case axl::io::UsbMonTransferParserState_TruncatedData:
#if (_AXL_OS_LINUX)
			if (!context->m_dataMode)
				break;
#endif
			addToReadBuffer(p, size);
			break;
		}

		p += size;
	}

	return true;
}

bool
UsbMonitor::parseCompletedTransfersOnly_l(
	TransferParser* parser,
	TransferParserContext* context,
	const char* p,
	const char* end
) {
	ASSERT(m_transferTracker);

	const axl::io::UsbMonTransferHdr* hdr;
	sl::HashTableIterator<uint64_t, Transfer*> it;

	while (p < end) {
		size_t size = parser->parse(p, end - p);
		if (size == -1)
			return false;

		axl::io::UsbMonTransferParserState state = parser->getState();
		switch (state) {
		case axl::io::UsbMonTransferParserState_CompleteHeader:
			hdr = parser->getTransferHdr();
#if (_AXL_OS_LINUX)
			if (m_addressFilter && hdr->m_address != m_addressFilter) {
				context->m_dataMode = TransferDataMode_Ignore;
				break;
			}
#endif

			if (!(hdr->m_flags & axl::io::UsbMonTransferFlag_Completed)) {
				if (hdr->m_endpoint & 0x80) { // incomplete in-transfers
					context->m_dataMode = TransferDataMode_Ignore;
					break;
				}

				Transfer* transfer = m_transferTracker->m_transferPool.get();
				transfer->m_hdr = *hdr;
				transfer->m_data.clear();

				if (hdr->m_transferType == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)
					transfer->m_isoPacketArray = parser->getIsoPacketArray();

				m_transferTracker->m_activeTransferMap.add(hdr->m_id, transfer);
				m_transferTracker->m_activeTransferList.insertTail(transfer);
				context->m_transfer = transfer;
				context->m_dataMode = TransferDataMode_AddToTransfer;
			} else {
				if (hdr->m_endpoint & 0x80) { // completed in-transfer
					addToReadBuffer(hdr, sizeof(axl::io::UsbMonTransferHdr));

					if (hdr->m_transferType == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)
						addToReadBuffer(parser->getIsoPacketArray(), hdr->m_isoHdr.m_packetCount * sizeof(axl::io::UsbMonIsoPacket));

					context->m_dataMode = TransferDataMode_AddToBuffer;
					break;
				}

				context->m_dataMode = TransferDataMode_Ignore;
				it = m_transferTracker->m_activeTransferMap.find(hdr->m_id);
				if (!it) // ignore completed but untracked out-transfers
					break;

				Transfer* transfer = it->m_value;
				transfer->m_hdr.m_status = hdr->m_status;
				ASSERT(transfer->m_hdr.m_capturedDataSize == transfer->m_data.getCount());

				addToReadBuffer(&transfer->m_hdr, sizeof(axl::io::UsbMonTransferHdr));

				if (hdr->m_transferType == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)
					addToReadBuffer(parser->getIsoPacketArray(), hdr->m_isoHdr.m_packetCount * sizeof(axl::io::UsbMonIsoPacket));

				addToReadBuffer(transfer->m_data, transfer->m_hdr.m_capturedDataSize);
				m_transferTracker->m_activeTransferMap.erase(it);
				m_transferTracker->m_activeTransferList.remove(transfer);
				m_transferTracker->m_transferPool.put(transfer);
			}

			break;

		case axl::io::UsbMonTransferParserState_IncompleteData:
		case axl::io::UsbMonTransferParserState_CompleteData:
		case axl::io::UsbMonTransferParserState_TruncatedData:
			switch (context->m_dataMode) {
			case TransferDataMode_AddToTransfer:
				ASSERT(context->m_transfer);
				context->m_transfer->m_data.append(p, size);
				break;

			case TransferDataMode_AddToBuffer:
				addToReadBuffer(p, size);
				break;
			}

			break;
		}

		p += size;
	}

	return true;
}

#if (_JNC_OS_WIN)

void
UsbMonitor::ioThreadFunc() {
	ASSERT(m_monitor.isOpen() && m_overlappedIo);

	bool result;
	axl::io::win::UsbPcapTransferParser parser;
	TransferParserContext parserContext;

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

			m_lock.lock(); // the main read buffer must be lock-protected
			result = processIncomingData_l(&parser, &parserContext, read->m_buffer, actualSize);
			if (!result) {
				setIoErrorEvent_l();
				return;
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
		size_t readBlockSize = m_readBlockSize;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();

		if (!isReadBufferFull && m_overlappedIo->m_activeOverlappedReadList.isEmpty()) {
			OverlappedRead* read = m_overlappedIo->m_overlappedReadPool.get();

			result =
				read->m_buffer.setCount(readBlockSize) &&
				m_monitor.overlappedRead(read->m_buffer.p(), readBlockSize, &read->m_overlapped);

			if (!result) {
				m_overlappedIo->m_overlappedReadPool.put(read);
				setIoErrorEvent();
				return;
			}

			m_overlappedIo->m_activeOverlappedReadList.insertTail(read);
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

	axl::io::lnx::UsbMonTransferParser parser;
	TransferParserContext parserContext;

	int selectFd = AXL_MAX(m_monitor, m_ioThreadSelfPipe.m_readFile) + 1;
	bool canReadMonitor = false;

	sl::Array<char> readBlock;
	readBlock.setCount(Def_ReadBlockSize);

	// read loop

	for (;;) {
		fd_set readSet = { 0 };
		FD_SET(m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canReadMonitor)
			FD_SET(m_monitor, &readSet);

		int result = ::select(selectFd, &readSet, NULL, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET(m_ioThreadSelfPipe.m_readFile, &readSet)) {
			char buffer[256];
			m_ioThreadSelfPipe.read(buffer, sizeof(buffer));
		}

		if (FD_ISSET(m_monitor, &readSet))
			canReadMonitor = true;

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		readBlock.setCount(m_readBlockSize); // update read block size
		char* p = readBlock.p();

		while (canReadMonitor && !m_readBuffer.isFull()) {
			ssize_t actualSize = ::read(m_monitor, p, readBlock.getCount());
			if (actualSize == -1) {
				if (errno == EAGAIN)
					canReadMonitor = false;
				else {
					setIoErrorEvent_l(err::Errno(errno));
					return;
				}
			} else {
				result = processIncomingData_l(&parser, &parserContext, p, actualSize);
				if (!result) {
					setIoErrorEvent_l();
					return;
				}
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
