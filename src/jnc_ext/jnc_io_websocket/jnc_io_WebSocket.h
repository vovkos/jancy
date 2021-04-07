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
#include "jnc_io_SslStateBase.h"
#include "jnc_io_WebSocketState.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(WebSocket)

//..............................................................................

enum WebSocketOption
{
	WebSocketOption_IncludeControlFrames       = 0x04,
	WebSocketOption_DisableAutoPong            = 0x08,
	WebSocketOption_DisableAutoServerHandshake = 0x10,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum WebSocketEvent
{
	WebSocketEvent_WebSocketHandshakeCompleted = 0x0200,
	WebSocketEvent_IncomingControlFrame        = 0x0400,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum WebSocketAcceptFlag
{
	WebSocketAcceptFlag_Suspended            = 0x01,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct WebSocketHdr: IfaceHdr
{
	SslStateBase* m_sslState;
	WebSocketHandshake* m_publicHandshakeRequest;
	WebSocketHandshake* m_publicHandshakeResponse;

	size_t m_incomingFrameSizeLimit;
	size_t m_incomingMessageSizeLimit;
	size_t m_outgoingFragmentationThreshold;

	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_writeBufferSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class WebSocket:
	public WebSocketHdr,
	public SslSocketBase
{
	friend class IoThread;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(WebSocket)

protected:
	enum Def
	{
		Def_ReadParallelism = 2,
		Def_ReadBlockSize   = 4 * 1024,
		Def_ReadBufferSize  = 16 * 1024,
		Def_WriteBufferSize = 16 * 1024,
		Def_Options         =
			AsyncIoDeviceOption_KeepReadBlockSize |
			AsyncIoDeviceOption_KeepWriteBlockSize,
	};

	enum IoThreadFlag
	{
		IoThreadFlag_Connecting         = 0x0100,
		IoThreadFlag_Listening          = 0x0200,
		IoThreadFlag_IncomingConnection = 0x0400,
	};

	class IoThread: public sys::ThreadImpl<IoThread>
	{
	public:
		void
		threadFunc()
		{
			containerof(this, WebSocket, m_ioThread)->ioThreadFunc();
		}
	};

#if (_AXL_OS_WIN)
	typedef OverlappedRead OverlappedRecv;

	struct OverlappedIo
	{
		mem::Pool<OverlappedRecv> m_overlappedRecvPool;
		sl::List<OverlappedRecv> m_activeOverlappedRecvList;
		axl::io::win::StdOverlapped m_sendOverlapped;
		sl::Array<char> m_sendBlock;
	};
#endif

protected:
	IoThread m_ioThread;

#if (_AXL_OS_WIN)
	OverlappedIo* m_overlappedIo;
#endif

	WebSocketStateMachine m_stateMachine;
	ClassBox<WebSocketHandshake> m_handshakeRequest;
	ClassBox<WebSocketHandshake> m_handshakeResponse;
	sl::String m_resource;
	sl::String m_host;

public:
	WebSocket();

	~WebSocket()
	{
		close();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	static
	SocketAddress
	JNC_CDECL
	getAddress(WebSocket* self)
	{
		return self->SocketBase::getAddress();
	}

	static
	SocketAddress
	JNC_CDECL
	getPeerAddress(WebSocket* self)
	{
		return self->SocketBase::getPeerAddress();
	}

	void
	JNC_CDECL
	setReadBlockSize(size_t size)
	{
		AsyncIoDevice::setSetting(&m_readBlockSize, size ? size : Def_ReadBlockSize);
	}

	bool
	JNC_CDECL
	setReadBufferSize(size_t size)
	{
		AsyncIoDevice::setSetting(&m_readBufferSize, size ? size : Def_ReadBufferSize);
		return true;
	}

	bool
	JNC_CDECL
	setWriteBufferSize(size_t size)
	{
		AsyncIoDevice::setSetting(&m_writeBufferSize, size ? size : Def_WriteBufferSize);
		return true;
	}

	bool
	JNC_CDECL
	setOptions(uint_t options)
	{
		options |= AsyncIoDeviceOption_KeepReadBlockSize | AsyncIoDeviceOption_KeepWriteBlockSize;
		return SocketBase::setOptions(options);
	}

	bool
	JNC_CDECL
	open_0(
		uint16_t family,
		bool isSecure
		)
	{
		return openImpl(family, NULL, isSecure);
	}

	bool
	JNC_CDECL
	open_1(
		DataPtr addressPtr,
		bool isSecure
		)
	{
		SocketAddress* address = (SocketAddress*)addressPtr.m_p;
		return openImpl(address ? address->m_family : AddressFamily_Ip4, address, isSecure);
	}

	void
	JNC_CDECL
	close();

	bool
	JNC_CDECL
	connect(
		DataPtr addressPtr,
		DataPtr resourcePtr,
		DataPtr hostPtr,
		WebSocketHandshakeHeaders* extraHeaders
		);

	bool
	JNC_CDECL
	listen(size_t backLogLimit);

	WebSocket*
	JNC_CDECL
	accept(
		DataPtr addressPtr,
		bool isSuspended
		);

	void
	JNC_CDECL
	unsuspend()
	{
		unsuspendIoThread();
	}

	size_t
	JNC_CDECL
	serverHandshake(
		uint_t statusCode,
		DataPtr statusTextPtr,
		WebSocketHandshakeHeaders* extraHeaders
		);

	size_t
	JNC_CDECL
	read(
		DataPtr opcodePtr,
		DataPtr dataPtr,
		size_t size
		);

	size_t
	JNC_CDECL
	write(
		uint_t opcode,
		DataPtr ptr,
		size_t size
		);

	handle_t
	JNC_CDECL
	wait(
		uint_t eventMask,
		FunctionPtr handlerPtr
		)
	{
		return AsyncIoDevice::wait(eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait(handle_t handle)
	{
		return AsyncIoDevice::cancelWait(handle);
	}

	uint_t
	JNC_CDECL
	blockingWait(
		uint_t eventMask,
		uint_t timeout
		)
	{
		return AsyncIoDevice::blockingWait(eventMask, timeout);
	}

	Promise*
	JNC_CDECL
	asyncWait(uint_t eventMask)
	{
		return AsyncIoDevice::asyncWait(eventMask);
	}

protected:
	void
	ioThreadFunc();

	bool
	openImpl(
		uint16_t family,
		const SocketAddress* address,
		bool isSecure
		);

	bool
	openSsl();

	void
	transportLoop(bool isClient);

	void
	sslReadWriteLoop();

	void
	tcpSendRecvLoop();

	bool
	processIncomingData(
		const void* p,
		size_t size
		);
};

//..............................................................................

} // namespace io
} // namespace jnc
