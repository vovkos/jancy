#pragma once

#include "jnc_io_IoLibGlobals.h"

namespace jnc {
namespace io {

//.............................................................................

enum SerialEventKind
{
	SerialEventKind_IncomingData = 0,
	SerialEventKind_IoError,
	SerialEventKind_TransmitBufferReady,
	SerialEventKind_StatusLineChanged,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SerialEventParams
{
	JNC_BEGIN_TYPE_MAP ("io.SerialEventParams", g_ioLibCacheSlot, IoLibTypeCacheSlot_SerialEventParams)
	JNC_END_TYPE_MAP ()

	SerialEventKind m_eventKind;
	uint_t m_syncId;
	uint_t m_lines;
	uint_t m_mask;
	rt::DataPtr m_errorPtr;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Serial: public rt::IfaceHdr
{
	friend class IoThread;

public:
	JNC_OPAQUE_CLASS_TYPE_INFO (Serial, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("io.Serial", g_ioLibCacheSlot, IoLibTypeCacheSlot_Serial)
		JNC_MAP_CONSTRUCTOR (&sl::construct <Serial>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <Serial>)
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
	JNC_END_CLASS_TYPE_MAP ()

protected:
	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			AXL_CONTAINING_RECORD (this, Serial, m_ioThread)->ioThreadFunc ();
		}
	};

	enum IoFlag
	{
		IoFlag_Closing      = 0x0001,
		IoFlag_IncomingData = 0x0010,
	};

protected:
	uint_t m_baudRate;
	axl::io::SerialFlowControl m_flowControl;
	uint_t m_dataBits;
	axl::io::SerialStopBits m_stopBits;
	axl::io::SerialParity m_parity;

	bool m_dtr;
	bool m_rts;
	bool m_isOpen;
	uint_t m_syncId;

	rt::ClassBox <rt::Multicast> m_onSerialEvent;

protected:
	rt::Runtime* m_runtime;
	axl::io::Serial m_serial;
	sys::Lock m_ioLock;
	volatile uint_t m_ioFlags;
	IoThread m_ioThread;
	
#if (_AXL_ENV == AXL_ENV_WIN)
	sys::Event m_ioThreadEvent;
#else
	axl::io::psx::Pipe m_selfPipe; // for self-pipe trick
#endif

public:
	Serial ();

	~Serial ()
	{
		close ();
	}

	bool
	AXL_CDECL
	open (rt::DataPtr namePtr);

	void
	AXL_CDECL
	close ();

	size_t
	AXL_CDECL
	read (
		rt::DataPtr ptr,
		size_t size
		);

	size_t
	AXL_CDECL
	write (
		rt::DataPtr ptr,
		size_t size
		);

	bool
	AXL_CDECL
	setBaudRate (uint_t baudRate);

	bool
	AXL_CDECL
	setDataBits (uint_t dataBits);

	bool
	AXL_CDECL
	setStopBits (axl::io::SerialStopBits stopBits);

	bool
	AXL_CDECL
	setParity (axl::io::SerialParity parity);

	bool
	AXL_CDECL
	setFlowControl (axl::io::SerialFlowControl flowControl);

	bool
	AXL_CDECL
	setDtr (bool dtr)
	{
		return m_serial.setDtr (dtr);
	}

	bool
	AXL_CDECL
	setRts (bool rts)
	{
		return m_serial.setRts (rts);
	}

	uint_t
	AXL_CDECL
	getStatusLines ()
	{
		return m_serial.getStatusLines ();
	}

protected:
	void
	fireSerialEvent (
		SerialEventKind eventKind,
		uint_t lines = 0,
		uint_t mask = 0
		);

	void
	fireSerialEvent (
		SerialEventKind eventKind,
		err::ErrorHdr* error
		);

	void
	ioThreadFunc ();

	void
	wakeIoThread ();
};

//.............................................................................

struct SerialPortDesc
{
	JNC_BEGIN_TYPE_MAP ("io.SerialPortDesc", g_ioLibCacheSlot, IoLibTypeCacheSlot_SerialPortDesc)
	JNC_END_TYPE_MAP ()

	rt::DataPtr m_nextPtr;
	rt::DataPtr m_deviceNamePtr;
	rt::DataPtr m_descriptionPtr;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

rt::DataPtr
createSerialPortDescList (rt::DataPtr countPtr);

//.............................................................................

} // namespace io
} // namespace jnc
