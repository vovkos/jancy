#pragma once

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (SerialEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (Serial)
JNC_DECLARE_TYPE (SerialPortDesc)

//.............................................................................

enum SerialEventCode
{
	SerialEventCode_IncomingData = 0,
	SerialEventCode_IoError,
	SerialEventCode_TransmitBufferReady,
	SerialEventCode_StatusLineChanged,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SerialEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (SerialEventParams)

	SerialEventCode m_eventCode;
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
			containerof (this, Serial, m_ioThread)->ioThreadFunc ();
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
	
#if (_JNC_OS_WIN)
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
	JNC_CDECL
	open (DataPtr namePtr);

	void
	JNC_CDECL
	close ();

	size_t
	JNC_CDECL
	read (
		DataPtr ptr,
		size_t size
		);

	size_t
	JNC_CDECL
	write (
		DataPtr ptr,
		size_t size
		);

	bool
	JNC_CDECL
	setBaudRate (uint_t baudRate);

	bool
	JNC_CDECL
	setDataBits (uint_t dataBits);

	bool
	JNC_CDECL
	setStopBits (axl::io::SerialStopBits stopBits);

	bool
	JNC_CDECL
	setParity (axl::io::SerialParity parity);

	bool
	JNC_CDECL
	setFlowControl (axl::io::SerialFlowControl flowControl);

	bool
	JNC_CDECL
	setDtr (bool dtr);

	bool
	JNC_CDECL
	setRts (bool rts);

	uint_t
	JNC_CDECL
	getStatusLines ()
	{
		return m_serial.getStatusLines ();
	}

protected:
	void
	fireSerialEvent (
		SerialEventCode eventCode,
		uint_t lines = 0,
		uint_t mask = 0
		);

	void
	fireSerialEvent (
		SerialEventCode eventCode,
		const err::ErrorRef& error
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
