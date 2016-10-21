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

JNC_DEFINE_TYPE (
	SerialEventParams,
	"io.SerialEventParams",
	g_ioLibGuid,
	IoLibCacheSlot_SerialEventParams
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (SerialEventParams)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	Serial,
	"io.Serial",
	g_ioLibGuid,
	IoLibCacheSlot_Serial,
	Serial,
	NULL
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
	JNC_MAP_FUNCTION ("open",  &Serial::open)
	JNC_MAP_FUNCTION ("close", &Serial::close)
	JNC_MAP_FUNCTION ("read",  &Serial::read)
	JNC_MAP_FUNCTION ("write", &Serial::write)
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
	m_runtime = getCurrentThreadRuntime ();
	m_ioFlags = 0;
	m_isOpen = false;
	m_syncId = 0;

	m_baudRate = 38400;
	m_flowControl = axl::io::SerialFlowControl_None;
	m_dataBits = 8;
	m_stopBits = axl::io::SerialStopBits_1;
	m_parity   = axl::io::SerialParity_None;

	m_dtr = true;
	m_rts = false;
}

void
Serial::wakeIoThread ()
{
#if (_JNC_OS_WIN)
	m_ioThreadEvent.signal ();
#else
	m_selfPipe.write (" ", 1);
#endif
}

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
		m_parity
		);

	bool result =
		m_serial.open ((const char*) namePtr.m_p) &&
		m_serial.setSettings (&serialSettings);

	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_isOpen = true;

#if (_JNC_OS_WIN)
	m_ioThreadEvent.reset ();
#elif (_JNC_OS_POSIX)
	m_selfPipe.create ();
#endif

	m_ioThread.start ();
	return true;
}

void
JNC_CDECL
Serial::close ()
{
	if (!m_serial.isOpen ())
		return;

	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	wakeIoThread ();
	m_ioLock.unlock ();

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	gcHeap->enterWaitRegion ();
	m_ioThread.waitAndClose ();
	gcHeap->leaveWaitRegion ();

	m_ioFlags = 0;
	m_serial.close ();

#if (_JNC_OS_POSIX)
	m_selfPipe.close ();
#endif

	m_isOpen = false;
	m_syncId++;
}

void
Serial::fireSerialEvent (
	SerialEventCode eventCode,
	uint_t lines,
	uint_t mask
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	DataPtr paramsPtr = createData <SerialEventParams> (m_runtime);
	SerialEventParams* params = (SerialEventParams*) paramsPtr.m_p;
	params->m_eventCode = eventCode;
	params->m_syncId = m_syncId;
	params->m_lines = lines;
	params->m_mask = mask;

	callMulticast (m_onSerialEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}


void
Serial::fireSerialEvent (
	SerialEventCode eventCode,
	const err::ErrorRef& error
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	DataPtr paramsPtr = createData <SerialEventParams> (m_runtime);
	SerialEventParams* params = (SerialEventParams*) paramsPtr.m_p;
	params->m_eventCode = eventCode;
	params->m_syncId = m_syncId;
	params->m_errorPtr = memDup (error, error->m_size);

	callMulticast (m_onSerialEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

bool
JNC_CDECL
Serial::setBaudRate (uint_t baudRate)
{
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
Serial::setDataBits (uint_t dataBits)
{
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
Serial::setFlowControl (axl::io::SerialFlowControl flowControl)
{
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
Serial::setDtr (bool dtr)
{
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
	bool result = m_serial.setRts (rts);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	m_rts = rts;
	return true;
}

size_t
JNC_CDECL
Serial::read (
	DataPtr ptr,
	size_t size
	)
{
	size_t result = m_serial.read (ptr.m_p, size);

	m_ioLock.lock ();
	m_ioFlags &= ~IoFlag_IncomingData;
	wakeIoThread ();
	m_ioLock.unlock ();

	if (result == -1)
		propagateLastError ();

	return result;
}

size_t
JNC_CDECL
Serial::write (
	DataPtr ptr,
	size_t size
	)
{
	size_t result = m_serial.write (ptr.m_p, size);

	if (result == -1)
		propagateLastError ();

	return result;
}

#if (_JNC_OS_WIN)

void
Serial::ioThreadFunc ()
{
	ASSERT (m_serial.isOpen ());

	sys::Event serialEvent;

	HANDLE waitTable [] =
	{
		m_ioThreadEvent.m_event,
		serialEvent.m_event,
	};

	uint_t prevLines = m_serial.getStatusLines ();

	for (;;)
	{
		dword_t mask = EV_CTS | EV_DSR | EV_RING | EV_RLSD;

		m_ioLock.lock ();

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			break;
		}

		if (!(m_ioFlags & IoFlag_IncomingData)) // don't re-issue select if not handled yet
			mask |= EV_RXCHAR;

		m_ioLock.unlock ();

		OVERLAPPED overlapped = { 0 };
		overlapped.hEvent = serialEvent.m_event;

		dword_t event = 0;
		bool result =
			m_serial.m_serial.setWaitMask (mask) &&
			m_serial.m_serial.wait (&event, &overlapped);

		if (!result)
		{
			fireSerialEvent (SerialEventCode_IoError, err::getLastError ());
			break;
		}

		DWORD waitResult = ::WaitForMultipleObjects (countof (waitTable), waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			err::Error error (err::getLastSystemErrorCode ());
			fireSerialEvent (SerialEventCode_IoError, error);
			return;
		}

		if (waitResult != WAIT_OBJECT_0 + 1)
			continue;

		if (event & EV_RXCHAR)
		{
			m_ioLock.lock ();
			ASSERT (!(m_ioFlags & IoFlag_IncomingData));
			m_ioFlags |= IoFlag_IncomingData;
			m_ioLock.unlock ();

			fireSerialEvent (SerialEventCode_IncomingData);
		}

		if (event & (EV_CTS | EV_DSR | EV_RING | EV_RLSD))
		{
			uint_t lines = m_serial.getStatusLines ();
			uint_t mask = lines ^ prevLines;
			prevLines = lines;

			if (mask)
				fireSerialEvent (SerialEventCode_StatusLineChanged, lines, mask);
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

	// read/write loop

	for (;;)
	{
		fd_set readSet = { 0 };

		FD_SET (m_selfPipe.m_readFile, &readSet);

		m_ioLock.lock ();

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			break;
		}

		if (!(m_ioFlags & IoFlag_IncomingData)) // don't re-issue select if not handled yet
			FD_SET (m_serial.m_serial, &readSet);

		m_ioLock.unlock ();

		result = select (selectFd, &readSet, NULL, NULL, NULL);
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
				err::Error error = err::getLastSystemErrorCode ();
				fireSerialEvent (SerialEventCode_IoError, error);
				return;
			}

			if (incomingDataSize == 0)
			{
				// shouldn't actually be here -- if USB serial is disconnected,
				// Fd::getIncomingDataSize should fail with some kind of IO error

				err::Error error (ENXIO);
				fireSerialEvent (SerialEventCode_IoError, error);
				return;
			}

			m_ioLock.lock ();
			ASSERT (!(m_ioFlags & IoFlag_IncomingData));
			m_ioFlags |= IoFlag_IncomingData;
			m_ioLock.unlock ();

			fireSerialEvent (SerialEventCode_IncomingData);
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
