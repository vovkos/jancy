#include "pch.h"
#include "jnc_io_DeviceMonitor.h"
#include "jnc_io_DevMonLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	DeviceMonitor,
	"io.DeviceMonitor",
	g_devMonLibGuid,
	DevMonLibTypeCacheSlot_DeviceMonitor,
	DeviceMonitor,
	&DeviceMonitor::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (DeviceMonitor)
	JNC_MAP_CONSTRUCTOR (&sl::construct <DeviceMonitor>)
	JNC_MAP_DESTRUCTOR (&sl::destruct <DeviceMonitor>)

	JNC_MAP_AUTOGET_PROPERTY ("m_readParallelism", &DeviceMonitor::setReadParallelism)
	JNC_MAP_AUTOGET_PROPERTY ("m_readBlockSize",   &DeviceMonitor::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_readBufferSize",  &DeviceMonitor::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_pendingNotifySizeLimit", &DeviceMonitor::setPendingNotifySizeLimit)
	JNC_MAP_AUTOGET_PROPERTY ("m_isEnabled",       &DeviceMonitor::setEnabled)
	JNC_MAP_AUTOGET_PROPERTY ("m_fileNameFilter",  &DeviceMonitor::setFileNameFilter)

	JNC_MAP_FUNCTION ("open",              &DeviceMonitor::open)
	JNC_MAP_FUNCTION ("close",             &DeviceMonitor::close)
	JNC_MAP_FUNCTION ("connect",           &DeviceMonitor::connect)
	JNC_MAP_FUNCTION ("setIoctlDescTable", &DeviceMonitor::setIoctlDescTable)
	JNC_MAP_FUNCTION ("read",              &DeviceMonitor::read)
	JNC_MAP_FUNCTION ("wait",              &DeviceMonitor::wait)
	JNC_MAP_FUNCTION ("cancelWait",        &DeviceMonitor::cancelWait)
	JNC_MAP_FUNCTION ("blockingWait",      &DeviceMonitor::blockingWait)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

DeviceMonitor::DeviceMonitor ()
{
	m_readParallelism = Def_ReadParallelism;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_pendingNotifySizeLimit = Def_PendingNotifySizeLimit;

	m_readBuffer.setBufferSize (Def_ReadBufferSize);
}

bool
JNC_CDECL
DeviceMonitor::setPendingNotifySizeLimit (size_t limit)
{
	if (!m_isConnected)
	{
		m_pendingNotifySizeLimit = limit;
		return true;
	}

	bool result = m_monitor.setPendingNotifySizeLimit (limit);
	if (!result)
		return false;

	m_pendingNotifySizeLimit = limit;
	return true;
}

bool
JNC_CDECL
DeviceMonitor::setFileNameFilter (DataPtr filterPtr)
{
	if (!m_isConnected)
	{
		setError (err::SystemErrorCode_InvalidDeviceState);
		return true;
	}

	const char* filter = (const char*) filterPtr.m_p;
	bool result = m_monitor.setFileNameFilter (filter);
	if (!result)
		return false;

	m_fileNameFilterPtr = strDup (filter);
	return true;
}

bool
JNC_CDECL
DeviceMonitor::setEnabled (bool isEnabled)
{
	if (!m_isConnected)
	{
		setError (err::SystemErrorCode_InvalidDeviceState);
		return true;
	}

	bool result = isEnabled ? m_monitor.enable () : m_monitor.disable ();
	if (!result)
		return false;

	m_isEnabled = isEnabled;
	return true;
}

bool
JNC_CDECL
DeviceMonitor::open ()
{
	close ();

	bool result = m_monitor.open ();
	if (!result)
		return false;

#if (_AXL_OS_WIN)
	ASSERT (!m_overlappedIo);
	m_overlappedIo = AXL_MEM_NEW (OverlappedIo);
#endif

	AsyncIoDevice::open ();
	m_ioThread.start ();
	return true;
}

void
JNC_CDECL
DeviceMonitor::close ()
{
	if (!m_monitor.isOpen ())
		return;

	m_lock.lock ();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread ();
	m_lock.unlock ();

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	gcHeap->enterWaitRegion ();
	m_ioThread.waitAndClose ();
	gcHeap->leaveWaitRegion ();

	m_monitor.close ();

	AsyncIoDevice::close ();
	m_isConnected = false;
	m_isEnabled = false;
	m_deviceNamePtr = g_nullPtr;
	m_fileNameFilterPtr = g_nullPtr;

#if (_AXL_OS_WIN)
	if (m_overlappedIo)
	{
		AXL_MEM_DELETE (m_overlappedIo);
		m_overlappedIo = NULL;
	}
#endif
}

bool
JNC_CDECL
DeviceMonitor::connect (DataPtr deviceNamePtr)
{
	bool result =
		m_monitor.connect ((const char*) deviceNamePtr.m_p) &&
		m_monitor.setPendingNotifySizeLimit (m_pendingNotifySizeLimit);

	if (!result)
		return false;

#if (_JNC_OS_WIN)
	dm::DeviceInfo deviceInfo;
	m_monitor.getTargetDeviceInfo (&deviceInfo);
	m_deviceNamePtr = strDup (deviceInfo.m_deviceName);
#elif (_JNC_OS_LINUX)
	dm::HookInfo hookInfo;
	m_monitor.getTargetHookInfo (&hookInfo);
	m_deviceNamePtr = strDup (hookInfo.m_fileName);
#endif

	m_lock.lock ();
	m_ioThreadFlags |= IoThreadFlag_Connected;
	wakeIoThread ();
	m_lock.unlock ();

	m_isConnected = true;
	return true;
}

bool
JNC_CDECL
DeviceMonitor::setIoctlDescTable (
	DataPtr ioctlDescPtr,
	size_t count
	)
{
#if (_JNC_OS_POSIX)
	const dm_IoctlDesc* ioctlDesc = (const dm_IoctlDesc*) ioctlDescPtr.m_p;
	bool result = m_monitor.setIoctlDescTable (ioctlDesc, count);
	if (!result)
		return false;
#endif

	return true;
}

bool
DeviceMonitor::connectLoop ()
{
	for (;;)
	{
		sleepIoThread ();

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			return false;
		}

		if (m_ioThreadFlags & IoThreadFlag_Connected)
		{
			m_lock.unlock ();
			return true;
		}

		m_lock.unlock ();
	}

}

#if (_JNC_OS_WIN)

void
DeviceMonitor::ioThreadFunc ()
{
	ASSERT (m_monitor.isOpen () && m_overlappedIo);

	bool result = connectLoop ();
	if (!result)
		return;

	HANDLE waitTable [2] =
	{
		m_ioThreadEvent.m_event,
		NULL, // placeholder for read completion event
	};

	size_t waitCount = 1; // always 1 or 2

	m_ioThreadEvent.signal (); // do initial update of active events

	// read loop

	for (;;)
	{
		DWORD waitResult = ::WaitForMultipleObjects (waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent (err::getLastSystemErrorCode ());
			return;
		}

		// do as much as we can without lock

		while (!m_overlappedIo->m_activeOverlappedReadList.isEmpty ())
		{
			OverlappedRead* read = *m_overlappedIo->m_activeOverlappedReadList.getHead ();
			result = read->m_overlapped.m_completionEvent.wait (0);
			if (!result)
				break;

			dword_t actualSize;
			result = m_monitor.getOverlappedResult (&read->m_overlapped, &actualSize);
			if (!result)
			{
				setIoErrorEvent ();
				return;
			}

			read->m_overlapped.m_completionEvent.reset ();
			m_overlappedIo->m_activeOverlappedReadList.remove (read);

			// only the main read buffer must be lock-protected

			m_lock.lock ();
			addToReadBuffer (read->m_buffer, actualSize);
			m_lock.unlock ();

			read->m_overlapped.m_completionEvent.reset ();
			m_overlappedIo->m_overlappedReadPool.put (read);
		}

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			break;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		updateReadWriteBufferEvents ();

		// take snapshots before releasing the lock

		bool isReadBufferFull = m_readBuffer.isFull ();
		size_t readParallelism = m_readParallelism;
		size_t readBlockSize = m_readBlockSize;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();

		size_t activeReadCount = m_overlappedIo->m_activeOverlappedReadList.getCount ();
		if (!isReadBufferFull && activeReadCount < readParallelism)
		{
			size_t newReadCount = readParallelism - activeReadCount;

			for (size_t i = 0; i < newReadCount; i++)
			{
				OverlappedRead* read = m_overlappedIo->m_overlappedReadPool.get ();

				result =
					read->m_buffer.setCount (readBlockSize) &&
					m_monitor.overlappedRead (read->m_buffer, readBlockSize, &read->m_overlapped);

				if (!result)
				{
					setIoErrorEvent ();
					return;
				}

				m_overlappedIo->m_activeOverlappedReadList.insertTail (read);
			}
		}

		if (m_overlappedIo->m_activeOverlappedReadList.isEmpty ())
		{
			waitCount = 1;
		}
		else
		{
			// wait-table may already hold correct value -- but there's no harm in writing it over

			OverlappedRead* read = *m_overlappedIo->m_activeOverlappedReadList.getHead ();
			waitTable [1] = read->m_overlapped.m_completionEvent.m_event;
			waitCount = 2;
		}
	}
}

#elif (_JNC_OS_POSIX)

void
DeviceMonitor::ioThreadFunc ()
{
	ASSERT (m_monitor.isOpen ());

	int result = connectLoop ();
	if (!result)
		return;

	int selectFd = AXL_MAX (m_monitor.m_device, m_ioThreadSelfPipe.m_readFile) + 1;

	sl::Array <char> readBlock;
	readBlock.setCount (Def_ReadBlockSize);

	bool canReadMonitor = false;

	// read loop

	for (;;)
	{
		fd_set readSet = { 0 };
		FD_SET (m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canReadMonitor)
			FD_SET (m_monitor.m_device, &readSet);

		result = ::select (selectFd, &readSet, NULL, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET (m_ioThreadSelfPipe.m_readFile, &readSet))
		{
			char buffer [256];
			m_ioThreadSelfPipe.read (buffer, sizeof (buffer));
		}

		if (FD_ISSET (m_monitor.m_device, &readSet))
			canReadMonitor = true;

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		readBlock.setCount (m_readBlockSize); // update read block size

		while (canReadMonitor && !m_readBuffer.isFull ())
		{
			ssize_t actualSize = ::read (m_monitor.m_device, readBlock, readBlock.getCount ());
			if (actualSize == -1)
			{
				if (errno == EAGAIN)
				{
					canReadMonitor = false;
				}
				else
				{
					setIoErrorEvent_l (err::Errno (errno));
					return;
				}
			}
			else
			{
				addToReadBuffer (readBlock, actualSize);
			}
		}

		updateReadWriteBufferEvents ();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
