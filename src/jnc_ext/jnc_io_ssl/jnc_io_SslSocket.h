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

#include "jnc_io_SslSocketBase.h"
#include "jnc_io_SslState.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(SslSocket)

//..............................................................................

struct SslSocketHdr: SslState {
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_writeBufferSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class SslSocket:
	public SslSocketHdr,
	public SslSocketBase {
	friend class IoThread;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(SslSocket)

protected:
	enum Def {
		Def_ReadBlockSize   = 4 * 1024,
		Def_ReadBufferSize  = 16 * 1024,
		Def_WriteBufferSize = 16 * 1024,
		Def_Options         = 0,
	};

	enum IoThreadFlag {
		IoThreadFlag_Connecting         = 0x0100,
		IoThreadFlag_Listening          = 0x0200,
		IoThreadFlag_IncomingConnection = 0x0400,
	};

	class IoThread: public sys::ThreadImpl<IoThread> {
	public:
		void
		threadFunc() {
			containerof(this, SslSocket, m_ioThread)->ioThreadFunc();
		}
	};

protected:
	IoThread m_ioThread;

public:
	SslSocket();

	~SslSocket() {
		close();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
		AsyncIoDevice::markOpaqueGcRoots(gcHeap);
	}

	static
	SocketAddress
	JNC_CDECL
	getAddress(SslSocket* self) {
		return self->SocketBase::getAddress();
	}

	static
	SocketAddress
	JNC_CDECL
	getPeerAddress(SslSocket* self) {
		return self->SocketBase::getPeerAddress();
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
	setOptions(uint_t flags) {
		return SocketBase::setOptions(flags);
	}

	bool
	JNC_CDECL
	open_0(uint16_t family);

	bool
	JNC_CDECL
	open_1(DataPtr addressPtr);

	void
	JNC_CDECL
	close();

	bool
	JNC_CDECL
	connect(DataPtr addressPtr);

	bool
	JNC_CDECL
	listen(size_t backLogLimit);

	SslSocket*
	JNC_CDECL
	accept(
		DataPtr addressPtr,
		bool isSuspended
	);

	void
	JNC_CDECL
	unsuspend() {
		unsuspendIoThread();
	}

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

	void
	sslReadWriteLoop();
};

//..............................................................................

} // namespace io
} // namespace jnc
