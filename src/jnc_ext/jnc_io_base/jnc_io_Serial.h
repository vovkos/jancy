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

#pragma once

#include "jnc_io_AsyncIoDevice.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE (Serial)
JNC_DECLARE_TYPE (SerialPortDesc)

//..............................................................................

enum SerialCompatibilityFlag
{
	SerialCompatibilityFlag_WinReadCheckComstat  = 0x04,
	SerialCompatibilityFlag_WinReadWaitFirstChar = 0x08,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum SerialEvent
{
	SerialEvent_CtsOn          = 0x0010,
	SerialEvent_CtsOff         = 0x0020,
	SerialEvent_DsrOn          = 0x0040,
	SerialEvent_DsrOff         = 0x0080,
	SerialEvent_RingOn         = 0x0100,
	SerialEvent_RingOff        = 0x0200,
	SerialEvent_DcdOn          = 0x0400,
	SerialEvent_DcdOff         = 0x0800,
	SerialEvent_StatusLineMask = 0x0ff0,
};

//..............................................................................

struct SerialHdr: IfaceHdr
{
	uint_t m_baudRate;
	axl::io::SerialFlowControl m_flowControl;
	uint_t m_dataBits;
	axl::io::SerialStopBits m_stopBits;
	axl::io::SerialParity m_parity;

	bool m_dtr;
	bool m_rts;

	uint_t m_readInterval;
	uint_t m_readParallelism;
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_writeBufferSize;
	uint_t m_compatibilityFlags;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Serial: 
	public SerialHdr,
	public AsyncIoDevice
{
	friend class IoThread;

protected:
	enum Def
	{
		Def_ReadInterval       = 10,
		Def_ReadParallelism    = 4,
		Def_ReadBlockSize      = 1 * 1024,
		Def_ReadBufferSize     = 16 * 1024,
		Def_WriteBufferSize    = 16 * 1024,
		Def_BaudRate           = 38400,
		Def_CompatibilityFlags =
			AsyncIoCompatibilityFlag_MaintainReadBlockSize |
			AsyncIoCompatibilityFlag_MaintainWriteBlockSize |
			SerialCompatibilityFlag_WinReadWaitFirstChar,
	};

	enum IoFlag
	{
		IoFlag_Writing = 0x0010,
	};

	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			containerof (this, Serial, m_ioThread)->ioThreadFunc ();
		}
	};

protected:
	IoThread m_ioThread;

	axl::io::Serial m_serial;

#if (_AXL_OS_WIN)
	dword_t m_serialEvents;
#endif

public:
	Serial ();

	~Serial ()
	{
		close ();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots (jnc::GcHeap* gcHeap)
	{
		AsyncIoDevice::markOpaqueGcRoots (gcHeap);
	}

	bool
	JNC_CDECL
	open (DataPtr namePtr);

	void
	JNC_CDECL
	close ();

	bool
	JNC_CDECL
	setReadInterval (uint_t count);

	bool
	JNC_CDECL
	setReadParallelism (uint_t count)
	{
		return setReadParallelismImpl (&m_readParallelism, count ? count : Def_ReadParallelism);
	}

	bool
	JNC_CDECL
	setReadBlockSize (size_t size)
	{
		return setReadBlockSizeImpl (&m_readBlockSize, size ? size : Def_ReadBlockSize);
	}

	bool
	JNC_CDECL
	setReadBufferSize (size_t size)
	{
		return setReadBufferSizeImpl (&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	bool
	JNC_CDECL
	setWriteBufferSize (size_t size)
	{
		return setWriteBufferSizeImpl (&m_writeBufferSize, size ? size : Def_WriteBufferSize);
	}

	bool
	JNC_CDECL
	setCompatibilityFlags (uint_t flags);

	bool
	JNC_CDECL
	setBaudRate (uint_t baudRate);

	bool
	JNC_CDECL
	setFlowControl (axl::io::SerialFlowControl flowControl);

	bool
	JNC_CDECL
	setDataBits (uint_t dataBits);

	bool
	JNC_CDECL
	setStopBits (axl::io::SerialStopBits stopBits);

	bool
	JNC_CDECL
	setParity (axl::io::SerialParity parity);

	uint_t
	JNC_CDECL
	getStatusLines ()
	{
		return m_serial.getStatusLines ();
	}

	bool
	JNC_CDECL
	setDtr (bool dtr);

	bool
	JNC_CDECL
	setRts (bool rts);

	size_t
	JNC_CDECL
	read (
		DataPtr ptr,
		size_t size
		)
	{
		return bufferedRead (ptr, size);
	}

	size_t
	JNC_CDECL
	write (
		DataPtr ptr,
		size_t size
		)
	{
		return bufferedWrite (ptr, size, &m_compatibilityFlags);
	}

	handle_t 
	JNC_CDECL
	wait (
		uint_t eventMask,
		FunctionPtr handlerPtr
		)
	{
		return AsyncIoDevice::wait (eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait (handle_t handle)
	{
		return AsyncIoDevice::cancelWait (handle);
	}

	uint_t
	JNC_CDECL
	blockingWait (
		uint_t eventMask,
		uint_t timeout
		)
	{
		return AsyncIoDevice::blockingWait (eventMask, timeout);
	}

protected:
	void
	ioThreadFunc ();

#if (_AXL_OS_WIN)
	bool
	setReadWaitFirstChar ();
#endif
};

//..............................................................................

struct SerialPortDesc
{
	JNC_DECLARE_TYPE_STATIC_METHODS (SerialPortDesc)

	DataPtr m_nextPtr;
	DataPtr m_deviceNamePtr;
	DataPtr m_descriptionPtr;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createSerialPortDescList (DataPtr countPtr);

//..............................................................................

} // namespace io
} // namespace jnc
