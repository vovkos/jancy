//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_io_Serial.h"
#include "jnc_io_IoLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	Serial,
	"io.Serial",
	g_ioLibGuid,
	IoLibCacheSlot_Serial,
	Serial,
	&Serial::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Serial)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <Serial>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <Serial>)

	JNC_MAP_AUTOGET_PROPERTY ("m_baudRate",    &Serial::setBaudRate)
	JNC_MAP_AUTOGET_PROPERTY ("m_flowControl", &Serial::setFlowControl)
	JNC_MAP_AUTOGET_PROPERTY ("m_dataBits",    &Serial::setDataBits)
	JNC_MAP_AUTOGET_PROPERTY ("m_stopBits",    &Serial::setStopBits)
	JNC_MAP_AUTOGET_PROPERTY ("m_parity",      &Serial::setParity)
	JNC_MAP_AUTOGET_PROPERTY ("m_dtr",         &Serial::setDtr)
	JNC_MAP_AUTOGET_PROPERTY ("m_rts",         &Serial::setRts)
	JNC_MAP_CONST_PROPERTY   ("m_statusLines", &Serial::getStatusLines)

	JNC_MAP_AUTOGET_PROPERTY ("m_readInterval",    &Serial::setReadInterval)
	JNC_MAP_AUTOGET_PROPERTY ("m_readParallelism", &Serial::setReadParallelism)
	JNC_MAP_AUTOGET_PROPERTY ("m_readBlockSize",   &Serial::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_readBufferSize",  &Serial::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_writeBufferSize", &Serial::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_options",         &Serial::setOptions)

	JNC_MAP_FUNCTION ("open",         &Serial::open)
	JNC_MAP_FUNCTION ("close",        &Serial::close)
	JNC_MAP_FUNCTION ("read",         &Serial::read)
	JNC_MAP_FUNCTION ("write",        &Serial::write)
	JNC_MAP_FUNCTION ("wait",         &Serial::wait)
	JNC_MAP_FUNCTION ("cancelWait",   &Serial::cancelWait)
	JNC_MAP_FUNCTION ("blockingWait", &Serial::blockingWait)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	SerialPortDesc,
	"io.SerialPortDesc",
	g_ioLibGuid,
	IoLibCacheSlot_SerialPortDesc
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (SerialPortDesc)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

Serial::Serial ()
{
	m_readInterval = Def_ReadInterval;
	m_readParallelism = Def_ReadParallelism;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options;

	m_baudRate = Def_BaudRate;
	m_flowControl = axl::io::SerialFlowControl_None,
	m_dataBits = 8;
	m_stopBits = axl::io::SerialStopBits_1;
	m_parity = axl::io::SerialParity_None;

	m_dtr = true;
	m_rts = false;

	m_readBuffer.setBufferSize (Def_ReadBufferSize);
	m_writeBuffer.setBufferSize (Def_WriteBufferSize);
}

#if (_AXL_OS_WIN)
bool
Serial::setReadWaitFirstChar ()
{
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
	timeouts.ReadTotalTimeoutConstant = MAXDWORD - 1;  // even when this (insanely) long wait completes,
	return m_serial.m_serial.setTimeouts (&timeouts);  // still no problem -- read will just return 0 bytes
}
#endif

bool
JNC_CDECL
Serial::open (DataPtr namePtr)
{
	close ();

	axl::io::SerialSettings serialSettings (
		m_baudRate,
		m_flowControl,
		m_dataBits,
		m_stopBits,
		m_parity,
		m_readInterval
		);

	bool result =
		m_serial.open ((const char*) namePtr.m_p) &&
		m_serial.setSettings (&serialSettings) &&
		m_serial.setDtr (m_dtr) &&
		m_serial.setRts (m_rts);

	if (!result)
	{
		propagateLastError ();
		return false;
	}

#if (_AXL_OS_WIN)
	if (m_options & SerialOption_WinReadWaitFirstChar)
	{
		result = setReadWaitFirstChar ();
		if (!result)
		{
			propagateLastError ();
			return false;
		}
	}
#endif

	AsyncIoDevice::open ();
	m_ioThread.start ();
	return true;
}

void
JNC_CDECL
Serial::close ()
{
	if (!m_serial.isOpen ())
		return;

	m_lock.lock ();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread ();
	m_lock.unlock ();

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	gcHeap->enterWaitRegion ();
	m_ioThread.waitAndClose ();
	gcHeap->leaveWaitRegion ();

	m_serial.close ();

	AsyncIoDevice::close ();
}

bool
JNC_CDECL
Serial::setReadInterval (uint_t interval)
{
	if (!m_isOpen)
	{
		m_readInterval = interval;
		return true;
	}

#if (_AXL_OS_WIN)
	if (m_options & SerialOption_WinReadWaitFirstChar) // interval is ignored
	{
		m_readInterval = interval;
		return true;
	}
#endif

	axl::io::SerialSettings settings;
	settings.m_readInterval = interval;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_ReadInterval);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_readInterval = interval;
	return true;
}

bool
JNC_CDECL
Serial::setOptions (uint_t options)
{
	if (!m_isOpen)
	{
		m_options = options;
		return true;
	}

#if (_AXL_OS_WIN)
	if ((options & SerialOption_WinReadWaitFirstChar) ^
		(m_options & SerialOption_WinReadWaitFirstChar))
	{
		bool result;

		if (options & SerialOption_WinReadWaitFirstChar)
		{
			result = setReadWaitFirstChar ();
		}
		else // re-apply the original timeout
		{
			axl::io::SerialSettings settings;
			settings.m_readInterval = m_readInterval;
			result = m_serial.setSettings (&settings, axl::io::SerialSettingId_ReadInterval);
		}

		if (!result)
		{
			propagateLastError ();
			return false;
		}
	}
#endif

	m_lock.lock ();
	m_options = options;
	wakeIoThread ();
	m_lock.unlock ();
	return true;
}

bool
JNC_CDECL
Serial::setBaudRate (uint_t baudRate)
{
	if (!m_isOpen)
	{
		m_baudRate = baudRate;
		return true;
	}

	axl::io::SerialSettings settings;
	settings.m_baudRate = baudRate;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_BaudRate);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_baudRate = baudRate;
	return true;
}

bool
JNC_CDECL
Serial::setFlowControl (axl::io::SerialFlowControl flowControl)
{
	if (!m_isOpen)
	{
		m_flowControl = flowControl;
		return true;
	}

	axl::io::SerialSettings settings;
	settings.m_flowControl = flowControl;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_FlowControl);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_flowControl = flowControl;
	return true;
}

bool
JNC_CDECL
Serial::setDataBits (uint_t dataBits)
{
	if (!m_isOpen)
	{
		m_dataBits = dataBits;
		return true;
	}

	axl::io::SerialSettings settings;
	settings.m_dataBits = dataBits;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_DataBits);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_dataBits = dataBits;
	return true;
}

bool
JNC_CDECL
Serial::setStopBits (axl::io::SerialStopBits stopBits)
{
	if (!m_isOpen)
	{
		m_stopBits = stopBits;
		return true;
	}

	axl::io::SerialSettings settings;
	settings.m_stopBits = stopBits;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_StopBits);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_stopBits = stopBits;
	return true;
}

bool
JNC_CDECL
Serial::setParity (axl::io::SerialParity parity)
{
	if (!m_isOpen)
	{
		m_parity = parity;
		return true;
	}

	axl::io::SerialSettings settings;
	settings.m_parity = parity;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_Parity);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_parity = parity;
	return true;
}

bool
JNC_CDECL
Serial::setDtr (bool dtr)
{
	if (!m_isOpen)
	{
		m_dtr = dtr;
		return true;
	}

	bool result = m_serial.setDtr (dtr);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_dtr = dtr;
	return true;
}

bool
JNC_CDECL
Serial::setRts (bool rts)
{
	if (!m_isOpen)
	{
		m_rts = rts;
		return true;
	}

	bool result = m_serial.setRts (rts);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_rts = rts;
	return true;
}

#if (_JNC_OS_WIN)

void
Serial::ioThreadFunc ()
{
	ASSERT (m_serial.isOpen ());

	bool result;

	axl::io::win::StdOverlapped serialWaitOverlapped;
	axl::io::win::StdOverlapped writeOverlapped;

	HANDLE waitTable [4] =
	{
		m_ioThreadEvent.m_event,
		serialWaitOverlapped.m_completionEvent.m_event,
		writeOverlapped.m_completionEvent.m_event,
		NULL, // placeholder for read completion event
	};

	size_t waitCount = 3; // always 3 or 4

	bool isWritingSerial = false;
	bool isWaitingSerial = false;

	m_ioThreadEvent.signal (); // do initial update of active events

	uint_t prevSerialEventMask = 0;

	for (;;)
	{
		DWORD waitResult = ::WaitForMultipleObjects (waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent (err::getLastSystemErrorCode ());
			return;
		}

		// do as much as we can without lock

		while (!m_activeOverlappedReadList.isEmpty ())
		{
			OverlappedRead* read = *m_activeOverlappedReadList.getHead ();
			result = read->m_overlapped.m_completionEvent.wait (0);
			if (!result)
				break;

			dword_t actualSize;
			result = m_serial.m_serial.getOverlappedResult (&read->m_overlapped, &actualSize);
			if (!result)
			{
				setIoErrorEvent ();
				return;
			}

			read->m_overlapped.m_completionEvent.reset ();
			m_activeOverlappedReadList.remove (read);

			// only the main read buffer must be lock-protected

			m_lock.lock ();
			addToReadBuffer (read->m_buffer, actualSize);
			m_lock.unlock ();

			m_freeOverlappedReadList.insertHead (read);
		}

		if (writeOverlapped.m_completionEvent.wait (0))
		{
			ASSERT (isWritingSerial);

			dword_t actualSize;
			result = m_serial.m_serial.getOverlappedResult (&writeOverlapped, &actualSize);
			if (!result)
			{
				setIoErrorEvent ();
				return;
			}

			if (actualSize < m_overlappedWriteBlock.getCount ()) // shouldn't happen, actually (unless with a non-standard driver)
				m_overlappedWriteBlock.remove (0, actualSize);
			else
				m_overlappedWriteBlock.clear ();

			writeOverlapped.m_completionEvent.reset ();
			isWritingSerial = false;
		}

		if (serialWaitOverlapped.m_completionEvent.wait (0))
		{
			serialWaitOverlapped.m_completionEvent.reset ();
			isWaitingSerial = false;
		}

		uint_t statusLines = m_serial.getStatusLines ();

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			break;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		getNextWriteBlock (&m_overlappedWriteBlock);
		updateReadWriteBufferEvents ();

		if (statusLines & axl::io::SerialStatusLine_Cts)
			m_activeEvents |= SerialEvent_CtsOn;
		else
			m_activeEvents |= SerialEvent_CtsOff;

		if (statusLines & axl::io::SerialStatusLine_Dsr)
			m_activeEvents |= SerialEvent_DsrOn;
		else
			m_activeEvents |= SerialEvent_DsrOff;

		if (statusLines & axl::io::SerialStatusLine_Dcd)
			m_activeEvents |= SerialEvent_DcdOn;
		else
			m_activeEvents |= SerialEvent_DcdOff;

		if (statusLines & axl::io::SerialStatusLine_Ring)
			m_activeEvents |= SerialEvent_RingOn;
		else
			m_activeEvents |= SerialEvent_RingOff;

		// take snapshots before releasing the lock

		bool isReadBufferFull = m_readBuffer.isFull ();
		size_t readParallelism = m_readParallelism;
		size_t readBlockSize = m_readBlockSize;
		uint_t compatibilityFlags = m_options;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();

		if (!isWritingSerial && !m_overlappedWriteBlock.isEmpty ())
		{
			result = m_serial.m_serial.overlappedWrite (
				m_overlappedWriteBlock,
				m_overlappedWriteBlock.getCount (),
				&writeOverlapped
				);

			if (!result)
			{
				setIoErrorEvent ();
				break;
			}

			isWritingSerial = true;
		}

		if (!isWaitingSerial)
		{
			uint_t serialEventMask = EV_CTS | EV_DSR | EV_RING | EV_RLSD;

			if (compatibilityFlags & SerialOption_WinReadCheckComstat)
				serialEventMask |= EV_RXCHAR;

			if (prevSerialEventMask != serialEventMask)
			{
				result = m_serial.m_serial.setWaitMask (serialEventMask);
				if (!result)
				{
					setIoErrorEvent ();
					break;
				}

				prevSerialEventMask = serialEventMask;
			}

			result = m_serial.m_serial.overlappedWait (&m_serialEvents, &serialWaitOverlapped);
			if (!result)
			{
				setIoErrorEvent ();
				break;
			}

			isWaitingSerial = true;
		}

		if (!isReadBufferFull)
		{
			size_t newReadCount = 0;

			if (!(compatibilityFlags & SerialOption_WinReadCheckComstat))
			{
				size_t activeReadCount = m_activeOverlappedReadList.getCount ();
				if (activeReadCount < readParallelism)
					newReadCount = readParallelism - activeReadCount;
			}
			else if (m_activeOverlappedReadList.isEmpty ())
			{
				COMSTAT stat = { 0 };
				result = m_serial.m_serial.clearError (NULL, &stat);
				if (!result)
				{
					setIoErrorEvent ();
					break;
				}

				if (stat.cbInQue)
					newReadCount = 1;
			}

			for (size_t i = 0; i < newReadCount; i++)
			{
				OverlappedRead* read = createOverlappedRead ();

				result =
					read->m_buffer.setCount (readBlockSize) &&
					m_serial.m_serial.overlappedRead (read->m_buffer, readBlockSize, &read->m_overlapped);

				if (!result)
				{
					setIoErrorEvent ();
					return;
				}

				m_activeOverlappedReadList.insertTail (read);
			}
		}

		if (m_activeOverlappedReadList.isEmpty ())
		{
			waitCount = 3;
		}
		else
		{
			// wait-table may already hold correct value -- but there's no harm in writing it over

			axl::io::win::StdOverlapped* overlapped = &m_activeOverlappedReadList.getHead ()->m_overlapped;
			waitTable [3] = overlapped->m_completionEvent.m_event;
			waitCount = 4;
		}
	}
}

#elif (_JNC_OS_POSIX)

void
Serial::ioThreadFunc ()
{
	ASSERT (m_serial.isOpen ());

	int result;
	int selectFd = AXL_MAX (m_serial.m_serial, m_ioThreadSelfPipe.m_readFile) + 1;

	sl::Array <char> readBlock;
	sl::Array <char> writeBlock;

	readBlock.setCount (Def_ReadBlockSize);

	bool canReadSerial = false;
	bool canWriteSerial = false;

	// read/write loop

	for (;;)
	{
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET (m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canReadSerial)
			FD_SET (m_serial.m_serial, &readSet);

		if (!canWriteSerial)
			FD_SET (m_serial.m_serial, &writeSet);

		result = ::select (selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET (m_ioThreadSelfPipe.m_readFile, &readSet))
		{
			char buffer [256];
			m_ioThreadSelfPipe.read (buffer, sizeof (buffer));
		}

		if (FD_ISSET (m_serial.m_serial, &readSet))
		{
			size_t incomingDataSize = m_serial.m_serial.getIncomingDataSize ();
			if (incomingDataSize == -1)
			{
				setIoErrorEvent ();
				return;
			}

			canReadSerial = incomingDataSize > 0;
		}

		if (FD_ISSET (m_serial.m_serial, &writeSet))
			canWriteSerial = true;

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		readBlock.setCount (m_readBlockSize); // update read block size

		while (canReadSerial && !m_readBuffer.isFull ())
		{
			ssize_t actualSize = ::read (m_serial.m_serial, readBlock, readBlock.getCount ());
			if (actualSize != -1)
			{
				if (errno == EAGAIN)
				{
					canReadSerial = false;
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

		while (canWriteSerial)
		{
			getNextWriteBlock (&writeBlock);
			if (writeBlock.isEmpty ())
				break;

			size_t blockSize = writeBlock.getCount ();
			ssize_t actualSize = ::write (m_serial.m_serial, writeBlock, blockSize);
			if (actualSize == -1)
			{
				if (errno == EAGAIN)
				{
					canWriteSerial = false;
				}
				else if (actualSize < 0)
				{
					setIoErrorEvent_l (err::Errno ((int) actualSize));
					return;
				}
			}
			else if ((size_t) actualSize < blockSize)
			{
				writeBlock.remove (0, actualSize);
			}
			else
			{
				writeBlock.clear ();
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

DataPtr
createSerialPortDesc (
	Runtime* runtime,
	axl::io::SerialPortDesc* portDesc
	)
{
	DataPtr portPtr = createData <SerialPortDesc> (runtime);
	SerialPortDesc* port = (SerialPortDesc*) portPtr.m_p;
	port->m_deviceNamePtr = strDup (portDesc->getDeviceName ());
	port->m_descriptionPtr = strDup (portDesc->getDescription ());

	return portPtr;
}

DataPtr
createSerialPortDescList (DataPtr countPtr)
{
	sl::StdList <axl::io::SerialPortDesc> portList;
	axl::io::createSerialPortDescList (&portList);

	if (portList.isEmpty ())
	{
		if (countPtr.m_p)
			*(size_t*) countPtr.m_p = 0;

		return g_nullPtr;
	}

	Runtime* runtime = getCurrentThreadRuntime ();
	ScopedNoCollectRegion noCollectRegion (runtime, false);

	sl::Iterator <axl::io::SerialPortDesc> it = portList.getHead ();

	DataPtr portPtr = createSerialPortDesc (runtime, *it);

	DataPtr resultPtr = portPtr;
	size_t count = 1;

	SerialPortDesc* prevPort = (SerialPortDesc*) portPtr.m_p;
	for (it++; it; it++)
	{
		portPtr = createSerialPortDesc (runtime, *it);
		prevPort->m_nextPtr = portPtr;
		prevPort = (SerialPortDesc*) portPtr.m_p;
		count++;
	}

	if (countPtr.m_p)
		*(size_t*) countPtr.m_p = count;

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
