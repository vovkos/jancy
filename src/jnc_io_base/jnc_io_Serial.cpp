#include "pch.h"
#include "jnc_io_Serial.h"

namespace jnc {
namespace io {

//.............................................................................

Serial::Serial ()
{
	m_runtime = rt::getCurrentThreadRuntime ();
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
#if (_AXL_ENV == AXL_ENV_WIN)
	m_ioThreadEvent.signal ();
#else
	m_selfPipe.write (" ", 1);
#endif
}

bool
AXL_CDECL
Serial::open (rt::DataPtr namePtr)
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
		ext::propagateLastError ();
		return false;
	}

	m_isOpen = true;

#if (_AXL_ENV == AXL_ENV_WIN)
	m_ioThreadEvent.reset ();
#elif (_AXL_ENV == AXL_ENV_POSIX)
	m_selfPipe.create ();
#endif

	m_ioThread.start ();
	return true;
}

void
AXL_CDECL
Serial::close ()
{
	if (!m_serial.isOpen ())
		return;

	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	wakeIoThread ();
	m_ioLock.unlock ();

	rt::enterWaitRegion (m_runtime);
	m_ioThread.waitAndClose ();
	rt::leaveWaitRegion (m_runtime);

	m_ioFlags = 0;
	m_serial.close ();

#if (_AXL_ENV == AXL_ENV_POSIX)
	m_selfPipe.close ();
#endif

	m_isOpen = false;
	m_syncId++;
}

void
Serial::fireSerialEvent (
	SerialEventKind eventKind,
	uint_t lines,
	uint_t mask
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	rt::DataPtr paramsPtr = rt::createData <SerialEventParams> (m_runtime);
	SerialEventParams* params = (SerialEventParams*) paramsPtr.m_p;
	params->m_eventKind = eventKind;
	params->m_syncId = m_syncId;
	params->m_lines = lines;
	params->m_mask = mask;

	rt::callMulticast (m_onSerialEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}


void
Serial::fireSerialEvent (
	SerialEventKind eventKind,
	err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	rt::DataPtr paramsPtr = rt::createData <SerialEventParams> (m_runtime);
	SerialEventParams* params = (SerialEventParams*) paramsPtr.m_p;
	params->m_eventKind = eventKind;
	params->m_syncId = m_syncId;
	params->m_errorPtr = rt::memDup (error, error->m_size);

	rt::callMulticast (m_onSerialEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

bool
AXL_CDECL
Serial::setBaudRate (uint_t baudRate)
{
	axl::io::SerialSettings settings;
	settings.m_baudRate = baudRate;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_BaudRate);
	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	m_baudRate = baudRate;
	return true;
}

bool
AXL_CDECL
Serial::setDataBits (uint_t dataBits)
{
	axl::io::SerialSettings settings;
	settings.m_dataBits = dataBits;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_DataBits);
	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	m_dataBits = dataBits;
	return true;
}

bool
AXL_CDECL
Serial::setStopBits (axl::io::SerialStopBits stopBits)
{
	axl::io::SerialSettings settings;
	settings.m_stopBits = stopBits;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_StopBits);
	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	m_stopBits = stopBits;
	return true;
}

bool
AXL_CDECL
Serial::setParity (axl::io::SerialParity parity)
{
	axl::io::SerialSettings settings;
	settings.m_parity = parity;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_Parity);
	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	m_parity = parity;
	return true;
}

bool
AXL_CDECL
Serial::setFlowControl (axl::io::SerialFlowControl flowControl)
{
	axl::io::SerialSettings settings;
	settings.m_flowControl = flowControl;
	bool result = m_serial.setSettings (&settings, axl::io::SerialSettingId_FlowControl);
	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	m_flowControl = flowControl;
	return true;
}

size_t
AXL_CDECL
Serial::read (
	rt::DataPtr ptr,
	size_t size
	)
{
	size_t result = m_serial.read (ptr.m_p, size);

	m_ioLock.lock ();
	m_ioFlags &= ~IoFlag_IncomingData;
	wakeIoThread ();
	m_ioLock.unlock ();

	if (result == -1)
		ext::propagateLastError ();

	return result;
}

size_t
AXL_CDECL
Serial::write (
	rt::DataPtr ptr,
	size_t size
	)
{
	size_t result = m_serial.write (ptr.m_p, size);

	if (result == -1)
		ext::propagateLastError ();

	return result;
}

#if (_AXL_ENV == AXL_ENV_WIN)

void
Serial::ioThreadFunc ()
{
	ASSERT (m_serial.isOpen ());

	mt::Event serialEvent;

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
			fireSerialEvent (SerialEventKind_IoError, err::getLastError ());
			break;
		}

		DWORD waitResult = ::WaitForMultipleObjects (countof (waitTable), waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			err::Error error = err::getLastSystemErrorCode ();
			fireSerialEvent (SerialEventKind_IoError, error);
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

			fireSerialEvent (SerialEventKind_IncomingData);
		}

		if (event & (EV_CTS | EV_DSR | EV_RING | EV_RLSD))
		{
			uint_t lines = m_serial.getStatusLines ();
			uint_t mask = lines ^ prevLines;
			prevLines = lines;

			if (mask)
				fireSerialEvent (SerialEventKind_StatusLineChanged, lines, mask);
		}
	}
}
#elif (_AXL_ENV == AXL_ENV_POSIX)
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
			m_ioLock.lock ();
			ASSERT (!(m_ioFlags & IoFlag_IncomingData));
			m_ioFlags |= IoFlag_IncomingData;
			m_ioLock.unlock ();

			fireSerialEvent (SerialEventKind_IncomingData);
		}
	}
}
#endif

//.............................................................................

rt::DataPtr
createSerialPortDesc (
	rt::Runtime* runtime,
	axl::io::SerialPortDesc* portDesc
	)
{
	rt::DataPtr portPtr = rt::createData <SerialPortDesc> (runtime);
	SerialPortDesc* port = (SerialPortDesc*) portPtr.m_p;
	port->m_deviceNamePtr = rt::strDup (portDesc->getDeviceName ());
	port->m_descriptionPtr = rt::strDup (portDesc->getDescription ());

	return portPtr;
}

rt::DataPtr
createSerialPortDescList (rt::DataPtr countPtr)
{
	sl::StdList <axl::io::SerialPortDesc> portList;
	axl::io::createSerialPortDescList (&portList);

	if (portList.isEmpty ())
	{
		if (countPtr.m_p)
			*(size_t*) countPtr.m_p = 0;

		return rt::g_nullPtr;
	}

	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	rt::ScopedNoCollectRegion noCollectRegion (runtime, false);

	sl::Iterator <axl::io::SerialPortDesc> it = portList.getHead ();

	rt::DataPtr portPtr = createSerialPortDesc (runtime, *it);
	
	rt::DataPtr resultPtr = portPtr;
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

//.............................................................................

} // namespace io
} // namespace jnc
