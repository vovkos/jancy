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

	JNC_MAP_AUTOGET_PROPERTY ("m_readInterval",       &Serial::setReadInterval)
	JNC_MAP_AUTOGET_PROPERTY ("m_readParallelism",    &Serial::setReadParallelism)
	JNC_MAP_AUTOGET_PROPERTY ("m_readBlockSize",      &Serial::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_readBufferSize",     &Serial::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_writeBufferSize",    &Serial::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_compatibilityFlags", &Serial::setCompatibilityFlags)

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
	m_compatibilityFlags = Def_CompatibilityFlags;

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
	if (m_compatibilityFlags & SerialCompatibilityFlag_WinReadWaitFirstChar)
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
	if (!m_isOpen)
	{
		m_serial.close (); // take care of failed open ()
		return;
	}

	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	wakeIoThread ();
	m_ioLock.unlock ();

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
	if (m_compatibilityFlags & SerialCompatibilityFlag_WinReadWaitFirstChar) // interval is ignored
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
Serial::setCompatibilityFlags (uint_t flags)
{
	if (!m_isOpen)
	{
		m_compatibilityFlags = flags;
		return true;
	}

#if (_AXL_OS_WIN)
	if ((flags & SerialCompatibilityFlag_WinReadWaitFirstChar) ^
		(m_compatibilityFlags & SerialCompatibilityFlag_WinReadWaitFirstChar))
	{
		bool result;

		if (flags & SerialCompatibilityFlag_WinReadWaitFirstChar)
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

	m_ioLock.lock ();
	m_compatibilityFlags = flags;
	wakeIoThread ();
	m_ioLock.unlock ();
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

	axl::io::win::StdOverlapped*
	overlappedTable [3] =
	{
		&serialWaitOverlapped,
		&writeOverlapped,
		NULL,  // placeholder for read overlapped
	};

	HANDLE waitTable [4] =
	{
		m_ioThreadEvent.m_event,
		serialWaitOverlapped.m_completionEvent.m_event,
		writeOverlapped.m_completionEvent.m_event,
		NULL, // placeholder for read completion event
	};

	size_t waitCount = 3; // always 3 or 4

	enum
	{
		EventMask = EV_RXCHAR | EV_CTS | EV_DSR | EV_RING | EV_RLSD,
	};

	enum WaitResult
	{
		WaitResult_Failed   = WAIT_FAILED,
		WaitResult_IoThread = WAIT_OBJECT_0,
		WaitResult_SerialWait,
		WaitResult_Write,
		WaitResult_Read,
	};

	result =
		m_serial.m_serial.setWaitMask (EventMask) &&
		m_serial.m_serial.overlappedWait (&m_serialEvents, &serialWaitOverlapped);

	if (!result)
	{
		setIoErrorEvent ();
		return;
	}

	m_ioThreadEvent.signal (); // do initial update of active events

	for (;;)
	{
		DWORD waitResult = ::WaitForMultipleObjects (waitCount, waitTable, false, INFINITE);

		dword_t actualSize = 0;
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent (err::getLastSystemErrorCode ());
			return;
		}

		if (waitResult >= WaitResult_SerialWait && waitResult <= WaitResult_Read)
		{
			size_t i = waitResult - WaitResult_SerialWait;
			axl::io::win::StdOverlapped* overlapped = overlappedTable [i];
			ASSERT (overlapped);

			result = m_serial.m_serial.getOverlappedResult (overlapped, &actualSize);
			if (!result)
			{
				setIoErrorEvent ();
				break;
			}

			overlapped->m_completionEvent.reset ();
		}

		bool canWrite = false;
		bool canWait = false;
		bool canReadMaybe = false;

		size_t readParallelism;
		size_t readBlockSize;
		uint_t compatibilityFlags;

		uint_t statusLines = m_serial.getStatusLines ();

		m_ioLock.lock ();
		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			break;
		}

		OverlappedRead* read = NULL;

		switch (waitResult)
		{
		case WaitResult_Read:
			ASSERT (!m_activeOverlappedReadList.isEmpty ());
			read = m_activeOverlappedReadList.removeHead ();
			addToReadBuffer (read->m_buffer, actualSize, &m_compatibilityFlags);
			m_freeOverlappedReadList.insertHead (read);
			break;

		case WaitResult_Write:
			ASSERT (m_ioFlags & IoFlag_Writing);
			m_ioFlags &= ~IoFlag_Writing;

			if (actualSize < m_writeBlock.getCount ()) // shouldn't happen, actually (unless with a non-standard driver)
				m_writeBlock.remove (0, actualSize);
			else
				m_writeBlock.clear ();

			break;

		case WaitResult_SerialWait:
			canWait = true; // always restart
			break;
		}

		while (!m_activeOverlappedReadList.isEmpty ())
		{
			read = *m_activeOverlappedReadList.getHead ();

			result =
				read->m_overlapped.m_completionEvent.wait (0) && // don't wait
				m_serial.m_serial.getOverlappedResult (&read->m_overlapped, &actualSize);

			if (!result) // not yet
				break;

			m_activeOverlappedReadList.remove (read);
			addToReadBuffer (read->m_buffer, actualSize, &m_compatibilityFlags);
			m_freeOverlappedReadList.insertHead (read);
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		if (!m_readBuffer.isEmpty ())
			m_activeEvents |= AsyncIoEvent_IncomingData;

		if (m_readBuffer.isFull ())
		{
			m_activeEvents |= AsyncIoEvent_ReceiveBufferFull;
		}
		else
		{
			canReadMaybe = true;
			readParallelism = m_readParallelism;
			readBlockSize = m_readBlockSize;
			compatibilityFlags = m_compatibilityFlags;
		}

		if (m_writeBlock.isEmpty ())
			m_writeBuffer.readAll (&m_writeBlock);

		if (!(m_ioFlags & IoFlag_Writing) && !m_writeBlock.isEmpty ())
		{
			m_ioFlags |= IoFlag_Writing;
			canWrite = true;
		}

		if (!m_writeBuffer.isFull ())
			m_activeEvents |= AsyncIoEvent_TransmitBufferReady;

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

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_ioLock.unlock ();

		result =
			(!canWrite || m_serial.m_serial.overlappedWrite (m_writeBlock, m_writeBlock.getCount (), &writeOverlapped)) &&
			(!canWait || m_serial.m_serial.overlappedWait (&m_serialEvents, &serialWaitOverlapped));

		if (!result)
		{
			setIoErrorEvent ();
			break;
		}

		// the rest is read handling

		if (canReadMaybe)
		{
			size_t newReadCount = 0;

			if (!(compatibilityFlags & SerialCompatibilityFlag_WinReadCheckComstat))
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
				read = createOverlappedRead ();

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
			// tables may already hold correct value -- but there's no harm in writing it over

			axl::io::win::StdOverlapped* overlapped = &m_activeOverlappedReadList.getHead ()->m_overlapped;
			waitTable [3] = overlapped->m_completionEvent.m_event;
			overlappedTable [2] = overlapped;
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
	int selectFd = AXL_MAX (m_serial.m_serial, m_selfPipe.m_readFile) + 1;

	sl::Array <char> readBlock;
	sl::Array <char> writeBlock;

	readBlock.setCount (Def_ReadBlockSize);

	bool canReadSerial = false;
	bool canAddToReadBuffer = false;
	bool canWriteSerial = false;

	// read/write loop

	for (;;)
	{
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		if (!canReadSerial)
			FD_SET (m_selfPipe.m_readFile, &readSet);

		if (!canWriteSerial)
			FD_SET (m_selfPipe.m_readFile, &writeSet);

		result = select (selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET (m_selfPipe.m_readFile, &readSet))
		{
			char buffer [256];
			m_selfPipe.read (buffer, sizeof (buffer));
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

		size_t readActualSize = 0;
		if (canReadSerial && canAddToReadBuffer)
		{
			readActualSize = m_serial.read (readBlock, readBlock.getCount ());
			if (readActualSize == -1)
			{
				setIoErrorEvent ();
				return;
			}
		}

		m_ioLock.lock ();
		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		if (canAddToReadBuffer)
		{
			addToReadBuffer (readBlock, readActualSize, &m_compatibilityFlags);
			readBlock.setCount (m_readBlockSize);
		}

		if (m_readBuffer.isFull ())
		{
			m_activeEvents |= AsyncIoEvent_ReceiveBufferFull;
			canAddToReadBuffer = false;
		}
		else
		{
			canAddToReadBuffer = true;
		}

		if (!m_readBuffer.isEmpty ())
			m_activeEvents |= AsyncIoEvent_IncomingData;

		if (writeBlock.isEmpty ())
		{
			if (m_writeMetaDataList.isEmpty ())
			{
				m_writeBuffer.readAll (&writeBlock);
			}
			else
			{
				size_t writeBlockSize = m_writeBuffer.getDataSize ();
				ReadWriteMetaData* meta = *m_readMetaDataList.getHead ();
				if (writeBlockSize < meta->m_blockSize)
				{
					meta->m_blockSize -= writeBlockSize;
				}
				else
				{
					writeBlockSize = meta->m_blockSize;
					m_readMetaDataList.remove (meta);
					m_freeReadWriteMetaDataList.insertHead (meta);
				}

				writeBlock.setCount (writeBlockSize);
				m_writeBuffer.read (writeBlock, writeBlockSize);
			}
		}

		if (!m_writeBuffer.isFull ())
			m_activeEvents |= AsyncIoEvent_TransmitBufferReady;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_ioLock.unlock ();

		if (canWriteSerial && !writeBlock.isEmpty ())
		{
			size_t writeBlockSize = writeBlock.getCount ();
			size_t writeActualSize = m_serial.write (writeBlock, writeBlockSize);
			if (writeActualSize == -1)
			{
				setIoErrorEvent ();
				return;
			}

			if (writeActualSize < writeBlockSize)
			{
				writeBlock.remove (0, writeActualSize);
				canWriteSerial = false;
			}
			else
			{
				writeBlock.clear ();
			}
		}
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
