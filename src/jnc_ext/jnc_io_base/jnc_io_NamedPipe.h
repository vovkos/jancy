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

#if (!_AXL_OS_WIN)
#	error This file is Windows-only
#endif

#include "jnc_io_FileStream.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(NamedPipe)

//..............................................................................

enum NamedPipeEvent {
	NamedPipeEvent_IncomingConnection = 0x0002,
};

//..............................................................................

struct NamedPipeHdr: IfaceHdr {
	uint_t m_backLogLimit;
	uint_t m_readParallelism;
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_writeBufferSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class NamedPipe:
	public NamedPipeHdr,
	public AsyncIoDevice {
	friend class IoThread;

protected:
	enum Def {
		Def_BackLogLimit    = 4,
		Def_ReadParallelism = 4,
		Def_ReadBlockSize   = 4 * 1024,
		Def_ReadBufferSize  = 16 * 1024,
		Def_WriteBufferSize = 16 * 1024,
		Def_Options         = 0,
		Def_Timeout         = 0,
	};

	class IoThread: public sys::ThreadImpl<IoThread> {
	public:
		void
		threadFunc() {
			containerof(this, NamedPipe, m_ioThread)->ioThreadFunc();
		}
	};

	struct IncomingConnection: sl::ListLink {
		axl::io::win::NamedPipe m_pipe;
	};

	struct OverlappedConnect: sl::ListLink {
		axl::io::win::NamedPipe m_pipe;
		axl::io::win::StdOverlapped m_overlapped;
	};

	struct OverlappedIo {
		mem::Pool<OverlappedConnect> m_overlappedConnectPool;
		sl::List<OverlappedConnect> m_pipeList;
		sl::List<OverlappedConnect> m_activeOverlappedConnectList;
	};

protected:
	IoThread m_ioThread;

	sl::String_w m_pipeName;
	mem::Pool<IncomingConnection> m_incomingConnectionPool;
	sl::List<IncomingConnection> m_pendingIncomingConnectionList;
	OverlappedIo* m_overlappedIo;

public:
	NamedPipe();

	~NamedPipe() {
		close();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
		AsyncIoDevice::markOpaqueGcRoots(gcHeap);
	}

	bool
	JNC_CDECL
	open(String name);

	void
	JNC_CDECL
	close();

	void
	JNC_CDECL
	setBackLogLimit(uint_t limit) {
		AsyncIoDevice::setSetting(&m_backLogLimit, limit ? limit : Def_BackLogLimit);
	}

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

	FileStream*
	JNC_CDECL
	accept(bool isSuspended);

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
};

//..............................................................................

} // namespace io
} // namespace jnc
