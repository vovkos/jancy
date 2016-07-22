#pragma once

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (SerialEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (Serial)
JNC_DECLARE_TYPE (SerialPortDesc)

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
	JNC_DECLARE_TYPE_STATIC_METHODS (SerialEventParams)

	SerialEventKind m_eventKind;
	uint_t m_syncId;
	uint_t m_lines;
	uint_t m_mask;
	DataPtr m_errorPtr;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Serial: public IfaceHdr
{
	friend class IoThread;

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

	ClassBox <Multicast> m_onSerialEvent;

protected:
	Runtime* m_runtime;
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
	open (DataPtr namePtr);

	void
	AXL_CDECL
	close ();

	size_t
	AXL_CDECL
	read (
		DataPtr ptr,
		size_t size
		);

	size_t
	AXL_CDECL
	write (
		DataPtr ptr,
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
	JNC_DECLARE_TYPE_STATIC_METHODS (SerialPortDesc)

	DataPtr m_nextPtr;
	DataPtr m_deviceNamePtr;
	DataPtr m_descriptionPtr;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createSerialPortDescList (DataPtr countPtr);

//.............................................................................

} // namespace io
} // namespace jnc
