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

#define _JNC_IO_SERIAL_POLL   1
#define _JNC_IO_SERIAL_KQUEUE 0

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Serial)

//..............................................................................

enum SerialOption {
	SerialOption_WinReadCheckComstat  = 0x04,
	SerialOption_WinReadWaitFirstChar = 0x08,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum SerialEvent {
	SerialEvent_CtsOn          = 0x0010,
	SerialEvent_CtsOff         = 0x0020,
	SerialEvent_DsrOn          = 0x0040,
	SerialEvent_DsrOff         = 0x0080,
	SerialEvent_RingOn         = 0x0100,
	SerialEvent_RingOff        = 0x0200,
	SerialEvent_DcdOn          = 0x0400,
	SerialEvent_DcdOff         = 0x0800,
	SerialEvent_StatusLineMask = 0x0ff0,
	SerialEvent_LineError      = 0x1000,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum SerialLineError {
	SerialLineError_FramingError = 0x01,
	SerialLineError_ParityError  = 0x02,
	SerialLineError_BreakError   = 0x04,
};

//..............................................................................

struct SerialHdr: IfaceHdr {
	uint_t m_baudRate;
	axl::io::SerialFlowControl m_flowControl;
	uint_t m_dataBits;
	axl::io::SerialStopBits m_stopBits;
	axl::io::SerialParity m_parity;

	bool m_dtr;
	bool m_rts;
	bool m_breakCondition;

#if (_JNC_IO_SERIAL_POLL)
	uint_t m_updateInterval;
#endif
	uint_t m_readInterval;
	uint_t m_readParallelism;
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_writeBufferSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Serial:
	public SerialHdr,
	public AsyncIoDevice {
	friend class IoThread;

protected:
	enum Def {
#if (_JNC_OS_DARWIN)
		// doesn't work well with some drivers, so better disable by default
		Def_UpdateInterval  = -1,
#else
		Def_UpdateInterval  = -1,
#endif
		Def_ReadInterval    = 0,
		Def_ReadParallelism = 4,
		Def_ReadBlockSize   = 1 * 1024,
		Def_ReadBufferSize  = 16 * 1024,
		Def_WriteBufferSize = 16 * 1024,
		Def_BaudRate        = 38400,
		Def_Options         = SerialOption_WinReadWaitFirstChar,
	};

	enum IoThreadFlag {
		IoThreadFlag_Waiting = 0x0100,
	};

	class IoThread: public sys::ThreadImpl<IoThread> {
	public:
		void
		threadFunc() {
			containerof(this, Serial, m_ioThread)->ioThreadFunc();
		}
	};

#if (_AXL_OS_WIN)
	struct OverlappedIo {
		mem::Pool<OverlappedRead> m_overlappedReadPool;
		sl::List<OverlappedRead> m_activeOverlappedReadList;
		axl::io::win::StdOverlapped m_serialWaitOverlapped;
		axl::io::win::StdOverlapped m_writeOverlapped;
		sl::Array<char> m_writeBlock;
		dword_t m_serialEvents;

		OverlappedIo() {
			m_serialEvents = 0;
		}
	};
#elif (_AXL_OS_LINUX)
	class WaitThread: public sys::ThreadImpl<WaitThread> {
	public:
		void
		threadFunc() {
			containerof(this, Serial, m_waitThread)->waitThreadFunc();
		}
	};
#endif

protected:
	axl::io::Serial m_serial;
	IoThread m_ioThread;

#if (_AXL_OS_WIN)
	OverlappedIo* m_overlappedIo;
#elif (_AXL_OS_LINUX)
	WaitThread m_waitThread;
	sys::Event m_waitThreadTerminateEvent;
#endif

	uint_t m_lineErrors;

public:
	Serial();

	~Serial() {
		close();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
		AsyncIoDevice::markOpaqueGcRoots(gcHeap);
	}

	uintptr_t
	JNC_CDECL
	getOsHandle() {
#if (_JNC_OS_WIN)
		return (uintptr_t)(handle_t)m_serial.m_serial;
#else
		return m_serial.m_serial;
#endif
	}

	bool
	JNC_CDECL
	open(String name);

	void
	JNC_CDECL
	close();

#if (_JNC_IO_SERIAL_POLL)
	void
	JNC_CDECL
	setUpdateInterval(uint_t interval);
#endif

	bool
	JNC_CDECL
	setReadInterval(uint_t interval);

	void
	JNC_CDECL
	setReadParallelism(uint_t count) {
		AsyncIoDevice::setSetting(&m_readParallelism, count ? count : Def_ReadParallelism);
	}

	void
	JNC_CDECL
	setReadBlockSize(size_t size) {
		AsyncIoDevice::setSetting(&m_readBlockSize, size ? size : Def_ReadBlockSize);
	}

	bool
	JNC_CDECL
	setReadBufferSize(size_t size) {
		return AsyncIoDevice::setReadBufferSize(&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	bool
	JNC_CDECL
	setWriteBufferSize(size_t size) {
		return AsyncIoDevice::setWriteBufferSize(&m_writeBufferSize, size ? size : Def_WriteBufferSize);
	}

	bool
	JNC_CDECL
	setOptions(uint_t options);

	bool
	JNC_CDECL
	setBaudRate(uint_t baudRate);

	bool
	JNC_CDECL
	setFlowControl(axl::io::SerialFlowControl flowControl);

	bool
	JNC_CDECL
	setDataBits(uint_t dataBits);

	bool
	JNC_CDECL
	setStopBits(axl::io::SerialStopBits stopBits);

	bool
	JNC_CDECL
	setParity(axl::io::SerialParity parity);

	uint_t
	JNC_CDECL
	getStatusLines() {
		return m_serial.getStatusLines();
	}

	bool
	JNC_CDECL
	setDtr(bool dtr);

	bool
	JNC_CDECL
	setRts(bool rts);

	bool
	JNC_CDECL
	setBreakCondition(bool breakCondition);

	bool
	JNC_CDECL
	setupDevice(
		uint_t baudRate,
		uint_t dataBits,
		axl::io::SerialStopBits stopBits,
		axl::io::SerialParity parity,
		axl::io::SerialFlowControl flowControl,
		uint_t readInterval,
		bool dtr,
		bool rts
	);

	uint_t
	JNC_CDECL
	clearLineErrors();

	size_t
	JNC_CDECL
	read(
		DataPtr ptr,
		size_t size
	) {
		return bufferedRead(ptr, size);
	}

	size_t
	JNC_CDECL
	write(
		DataPtr ptr,
		size_t size
	) {
		return bufferedWrite(ptr, size);
	}

	handle_t
	JNC_CDECL
	wait(
		uint_t eventMask,
		FunctionPtr handlerPtr
	) {
		return AsyncIoDevice::wait(eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait(handle_t handle) {
		return AsyncIoDevice::cancelWait(handle);
	}

	uint_t
	JNC_CDECL
	blockingWait(
		uint_t eventMask,
		uint_t timeout
	) {
		return AsyncIoDevice::blockingWait(eventMask, timeout);
	}

	Promise*
	JNC_CDECL
	asyncWait(uint_t eventMask) {
		return AsyncIoDevice::asyncWait(eventMask);
	}

protected:
	void
	ioThreadFunc();

	bool
	applyFlowControl(axl::io::SerialFlowControl flowControl);

	void
	updateStatusLineEvents(uint_t statusLines);

#if (_AXL_OS_WIN)
	bool
	setReadWaitFirstChar();
#elif (_AXL_OS_LINUX)
	void
	waitThreadFunc();
#endif
};

//..............................................................................

} // namespace io
} // namespace jnc
