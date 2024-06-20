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
#include "jnc_io_WebSocket.h"
#include "jnc_io_WebSocketLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE_REQ(
	WebSocket,
	"io.WebSocket",
	g_webSocketLibGuid,
	WebSocketLibCacheSlot_WebSocket,
	WebSocket,
	&WebSocket::markOpaqueGcRoots
)

JNC_BEGIN_OPAQUE_CLASS_REQUIRE_TABLE(WebSocket)
	JNC_OPAQUE_CLASS_REQUIRE_TYPE(jnc::TypeKind_Class, "io.WebSocketHandshake")
	JNC_OPAQUE_CLASS_REQUIRE_TYPE(jnc::TypeKind_Class, "io.SslState")
JNC_END_OPAQUE_CLASS_REQUIRE_TABLE()

JNC_BEGIN_TYPE_FUNCTION_MAP(WebSocket)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<WebSocket>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<WebSocket>)

	JNC_MAP_CONST_PROPERTY("m_address", &WebSocket::getAddress)
	JNC_MAP_CONST_PROPERTY("m_peerAddress", &WebSocket::getPeerAddress)
	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize", &WebSocket::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize", &WebSocket::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &WebSocket::setWriteBufferSize)
 	JNC_MAP_AUTOGET_PROPERTY("m_options", &WebSocket::setOptions)
 	JNC_MAP_AUTOGET_PROPERTY("m_extraHeaders", &WebSocket::setExtraHeaders)

	JNC_MAP_FUNCTION("open", &WebSocket::open_0)
	JNC_MAP_OVERLOAD(&WebSocket::open_1)
	JNC_MAP_FUNCTION("close", &WebSocket::close)
	JNC_MAP_FUNCTION("connect", &WebSocket::connect)
	JNC_MAP_FUNCTION("listen", &WebSocket::listen)
	JNC_MAP_FUNCTION("accept", &WebSocket::accept)
	JNC_MAP_FUNCTION("unsuspend", &WebSocket::unsuspend)
	JNC_MAP_FUNCTION("serverHandshake", &WebSocket::serverHandshake)
	JNC_MAP_FUNCTION("read", &WebSocket::read)
	JNC_MAP_FUNCTION("write", &WebSocket::write)
	JNC_MAP_FUNCTION("wait", &WebSocket::wait)
	JNC_MAP_FUNCTION("cancelWait", &WebSocket::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &WebSocket::blockingWait)
	JNC_MAP_FUNCTION("asyncWait", &WebSocket::asyncWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

WebSocket::WebSocket() {
	m_sslState = NULL;
	m_extraHeaders = NULL;
	m_publicHandshakeRequest = NULL;
	m_publicHandshakeResponse = NULL;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options;

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
	m_writeBuffer.setBufferSize(Def_WriteBufferSize);

#if (_AXL_OS_WIN)
	m_overlappedIo = NULL;
#endif

	// need to prime opaque class fields

	Module* module = m_box->m_type->getModule();
	jnc::primeClass(module, &m_handshakeRequest);
	jnc::primeClass(module, &m_handshakeResponse);
	sl::construct(m_handshakeRequest.p());
	sl::construct(m_handshakeResponse.p());
}

WebSocket::~WebSocket() {
	close();

	sl::destruct(m_handshakeRequest.p());
	sl::destruct(m_handshakeResponse.p());
}

void
JNC_CDECL
WebSocket::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	AsyncIoDevice::markOpaqueGcRoots(gcHeap);
}

bool
JNC_CDECL
WebSocket::setExtraHeaders(WebSocketHandshakeHeaders* headers) {
	m_lock.lock();
	m_extraHeaders = headers;
	m_lock.unlock();
	return true;
}

bool
WebSocket::openImpl(
	uint16_t family,
	const SocketAddress* address,
	bool isSecure
) {
	close();

	bool result =
		requireWebSocketCapability() &&
		SocketBase::open(family, IPPROTO_TCP, address) &&
		(!isSecure || openSsl());

	if (!result)
		return false;

#if (_AXL_OS_WIN)
	ASSERT(!m_overlappedIo);
	m_overlappedIo = new OverlappedIo;
#endif

	m_ioThreadFlags |= IoThreadFlag_Datagram;
	m_ioThread.start();
	return true;
}

void
JNC_CDECL
WebSocket::close() {
	if (!m_socket.isOpen())
		return;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread();
	m_lock.unlock();

	GcHeap* gcHeap = m_runtime->getGcHeap();
	gcHeap->enterWaitRegion();
	m_ioThread.waitAndClose();
	gcHeap->leaveWaitRegion();

	if (m_sslState) {
		m_sslState->closeSsl();
		m_sslState = NULL;
	}

	m_handshakeRequest->clear();
	m_handshakeResponse->clear();
	m_publicHandshakeRequest = NULL;
	m_publicHandshakeResponse = NULL;

	SocketBase::close();

#if (_AXL_OS_WIN)
	delete m_overlappedIo;
	m_overlappedIo = NULL;
#endif
}

bool
JNC_CDECL
WebSocket::connect(
	DataPtr addressPtr,
	String resource,
	String host
) {
	SocketAddress* address = (SocketAddress*)addressPtr.m_p;
	bool result = m_socket.connect(address->getSockAddr());
	if (!result)
		return false;

	m_resource = resource >> toAxl;
	m_host = host >> toAxl;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Connecting;
	wakeIoThread();
	m_lock.unlock();
	return result;
}

bool
JNC_CDECL
WebSocket::listen(size_t backLogLimit) {
	bool result =
		requireSocketCapability(SocketCapability_Server) &&
		m_socket.listen(backLogLimit);

	if (!result)
		return false;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Listening;
	wakeIoThread();
	m_lock.unlock();
	return true;
}

WebSocket*
JNC_CDECL
WebSocket::accept(
	DataPtr addressPtr,
	bool isSuspended
) {
	SocketAddress* address = ((SocketAddress*)addressPtr.m_p);
	WebSocket* connectionSocket = createClass<WebSocket>(m_runtime);

	m_lock.lock();
	if (m_pendingIncomingConnectionList.isEmpty()) {
		m_lock.unlock();
		setError(err::Error(err::SystemErrorCode_InvalidDeviceState));
		return NULL;
	}

	IncomingConnection* incomingConnection = m_pendingIncomingConnectionList.removeHead();
	sl::takeOver(&connectionSocket->m_socket.m_socket, &incomingConnection->m_socket.m_socket);
	connectionSocket->setOptions(m_options);
	connectionSocket->AsyncIoDevice::open();
	connectionSocket->m_ioThreadFlags = IoThreadFlag_IncomingConnection;

	if (isSuspended)
		connectionSocket->m_ioThreadFlags |= IoThreadFlag_Suspended;

#if (_AXL_OS_WIN)
	connectionSocket->m_overlappedIo = new OverlappedIo;
#endif

	if (address)
		address->setSockAddr(incomingConnection->m_sockAddr);

	m_incomingConnectionPool.put(incomingConnection);

	if (m_pendingIncomingConnectionList.isEmpty())
		m_activeEvents &= ~SocketEvent_IncomingConnection;

	m_lock.unlock();

	bool result =
		connectionSocket->m_socket.setBlockingMode(false) && // not guaranteed to be propagated across 'accept' calls
		(!m_sslState || connectionSocket->openSsl()) &&
		connectionSocket->m_ioThread.start();

	if (!result)
		return NULL;

	connectionSocket->wakeIoThread();
	return connectionSocket;
}

bool
JNC_CDECL
WebSocket::serverHandshake(
	uint_t statusCode,
	String reasonPhrase
) {
	sl::String handshakeResponse;

	m_lock.lock();
	if (!(m_activeEvents & WebSocketEvent_WebSocketHandshakeRequested)) {
		m_lock.unlock();
		err::setError(err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	m_handshakeResponse->buildResponse(
		&handshakeResponse,
		statusCode,
		reasonPhrase >> toAxl,
		m_handshakeRequest,
		m_extraHeaders
	);

	m_publicHandshakeRequest = m_handshakeRequest;
	m_publicHandshakeResponse = m_handshakeResponse;
	size_t result = addToWriteBuffer(handshakeResponse.cp(), handshakeResponse.getLength());
	if (!result) {
		m_lock.unlock();
		err::setError(err::SystemErrorCode_BufferOverflow);
		return false;
	}

	wakeIoThread();
	setEvents_l(WebSocketEvent_WebSocketHandshakeRequested | WebSocketEvent_WebSocketHandshakeCompleted);
	return true;
}

size_t
JNC_CDECL
WebSocket::read(
	DataPtr opcodePtr,
	DataPtr dataPtr,
	size_t size
) {
	if (!opcodePtr.m_p)
		return bufferedRead(dataPtr, size);

	char buffer[256];
	sl::Array<char> params(rc::BufKind_Stack, buffer, sizeof(buffer));
	size_t result = bufferedRead(dataPtr, size, &params);

	*(uint_t*)opcodePtr.m_p = !params.isEmpty() ?
		(WebSocketFrameOpcode)params[0] :
		WebSocketFrameOpcode_Binary;

	return result;
}

size_t
JNC_CDECL
WebSocket::write(
	uint_t opcode,
	DataPtr ptr,
	size_t size
) {
	char buffer[256];
	sl::Array<char> frame(rc::BufKind_Stack, buffer, sizeof(buffer));

	size_t frameSize = buildWebSocketFrame(
		&frame,
		(WebSocketFrameOpcode)opcode,
		true,
		(m_ioThreadFlags & IoThreadFlag_Connecting) != 0, // client's frames are masked, server's are not
		ptr.m_p,
		size
	);

	m_lock.lock();
	size_t result = addToWriteBuffer(frame, frameSize);
	if (result != frameSize) {
		m_lock.unlock();
		return result;
	}

	if (opcode == WebSocketFrameOpcode_Close)
		m_ioThreadFlags |= IoThreadFlag_WebSocketCloseAdded;

	wakeIoThread();
	m_lock.unlock();

	return
		result != frameSize ? result :
		size ? size : 1; // to distinguish between success and buffer-full for zero-frames
}

bool
WebSocket::openSsl() {
	m_sslState = SslStateBase::createExternal(
		m_runtime,
		g_webSocketLibGuid,
		WebSocketLibCacheSlot_CreateSslState,
		&m_socket
	);

	return m_sslState != NULL;
}

void
WebSocket::ioThreadFunc() {
	ASSERT(m_socket.isOpen());

	sleepIoThread();

	m_lock.lock();
	if (m_ioThreadFlags & IoThreadFlag_Closing) {
		m_lock.unlock();
	} else if (m_ioThreadFlags & IoThreadFlag_Connecting) {
		m_lock.unlock();

		bool result = connectLoop(SocketEvent_TcpConnected);
		if (result)
			transportLoop(true);
	} else if (m_ioThreadFlags & IoThreadFlag_Listening) {
		m_lock.unlock();
		acceptLoop(SocketEvent_IncomingConnection);
	} else if (m_ioThreadFlags & IoThreadFlag_IncomingConnection) {
		m_lock.unlock();

		bool result = suspendLoop();
		if (result)
			transportLoop(false);
	} else {
		m_lock.unlock();
		ASSERT(false); // shouldn't normally happen
	}
}

void
WebSocket::transportLoop(bool isClient) {
	if (m_sslState) {
		bool result = sslHandshakeLoop(m_sslState, isClient);
		if (!result)
			return;
	}

	if (!isClient) {
		m_stateMachine.waitHandshake(m_handshakeRequest);
	} else {
		sl::String key = generateWebSocketHandshakeKey();

		m_lock.lock();
		sl::String handshake = m_handshakeRequest->buildRequest(m_resource, m_host, key, m_extraHeaders);
		m_publicHandshakeRequest = m_handshakeRequest;
		addToWriteBuffer(handshake.cp(), handshake.getLength());
		setEvents_l(WebSocketEvent_WebSocketHandshakeRequested);
		m_stateMachine.waitHandshakeResponse(m_handshakeResponse, key);
	}

	wakeIoThread();

	if (m_sslState)
		sslReadWriteLoop();
	else
		tcpSendRecvLoop();
}

bool
WebSocket::processIncomingData(
	const void* p0,
	size_t size
) {
	const char* p = (char*)p0;
	const char* end = p + size;

	sl::String handshakeResponse;
	sl::Array<char> replyFrame;

	while (p < end) {
		WebSocketState prevState = m_stateMachine.getState();
		size_t result = m_stateMachine.parse(p, end - p);
		if (result == -1) {
			setIoErrorEvent();
			return false;
		}

		p += result;

		WebSocketState state = m_stateMachine.getState();
		if (state == prevState)
			continue;

		switch (state) {
		case WebSocketState_HandshakeReady:
			m_lock.lock();
			if (m_options & WebSocketOption_DisableServerHandshakeResponse) {
				setEvents_l(WebSocketEvent_WebSocketHandshakeRequested);
			} else {
				m_handshakeResponse->buildResponse(&handshakeResponse, m_handshakeRequest, m_extraHeaders);
				m_publicHandshakeRequest = m_handshakeRequest;
				m_publicHandshakeResponse = m_handshakeResponse;
				addToWriteBuffer(handshakeResponse.cp(), handshakeResponse.getLength());
				setEvents_l(WebSocketEvent_WebSocketHandshakeRequested | WebSocketEvent_WebSocketHandshakeCompleted);
			}

			m_stateMachine.setConnectedState();
			break;

		case WebSocketState_HandshakeResponseReady:
			m_publicHandshakeResponse = m_handshakeResponse;
			setEvents(WebSocketEvent_WebSocketHandshakeCompleted);
			m_stateMachine.setConnectedState();
			break;

		case WebSocketState_Connected:
			break;

		case WebSocketState_ControlFrameReady: {
			const WebSocketFrame& frame = m_stateMachine.getFrame();
			WebSocketFrameOpcode opcode = (WebSocketFrameOpcode)frame.m_opcode;

			uint_t events = 0;

			m_lock.lock();
			if (m_options & WebSocketOption_IncludeControlFrames) {
				addToReadBuffer(
					frame.m_payload,
					frame.m_payload.getCount(),
					&opcode,
					sizeof(&opcode)
				);

				events |= AsyncIoDeviceEvent_IncomingData;
			}

			replyFrame.clear();

			switch (opcode) {
			case WebSocketFrameOpcode_Close:
				if (m_ioThreadFlags & IoThreadFlag_WebSocketCloseAdded) {
					events |= WebSocketEvent_WebSocketCloseCompleted; // even if we didn't transmit our own CLOSE frame yet
					break;
				}

				events |= WebSocketEvent_WebSocketCloseRequested;
				if (m_options & WebSocketOption_DisableCloseResponse)
					break;

				buildWebSocketFrame(&replyFrame, WebSocketFrameOpcode_Close, true, false, NULL, 0);
				result = addToWriteBuffer(replyFrame.cp(), replyFrame.getCount());
				if (result)
					m_ioThreadFlags |= IoThreadFlag_WebSocketCloseAdded;

				break;

			case WebSocketFrameOpcode_Ping:
				if (m_options & WebSocketOption_DisablePongResponse)
					break;

				buildWebSocketFrame(
					&replyFrame,
					WebSocketFrameOpcode_Pong,
					true,
					(m_ioThreadFlags & IoThreadFlag_Connecting) != 0, // client's frames are masked, server's are not
					frame.m_payload,
					frame.m_payloadLength
				);

				addToWriteBuffer(replyFrame.cp(), replyFrame.getCount());
				break;
			}

			if (events)
				setEvents_l(events);
			else
				m_lock.unlock();

			m_stateMachine.setConnectedState();
			break;
			}

		case WebSocketState_MessageReady: {
			const WebSocketMessage& message = m_stateMachine.getMessage();

			m_lock.lock();

			addToReadBuffer(
				message.m_payload,
				message.m_payload.getCount(),
				&message.m_opcode,
				sizeof(&message.m_opcode)
			);

			setEvents_l(AsyncIoDeviceEvent_IncomingData);
			m_stateMachine.setConnectedState();
			break;
			}

		default:
			ASSERT(false);
		}
	}

	return true;
}

inline
void
WebSocket::updateCloseEvents(
	const void* p,
	size_t size
) {
	if (size > sizeof(const WebSocketFrameHdr) &&
		((const WebSocketFrameHdr*)p)->m_opcode == WebSocketFrameOpcode_Close)
		m_activeEvents |= (m_activeEvents & WebSocketEvent_WebSocketCloseRequested) ?
			WebSocketEvent_WebSocketCloseCompleted :
			WebSocketEvent_WebSocketCloseRequested;
}

uint_t
WebSocket::resetActiveEvents(uint_t baseEvents) {
	uint_t prevEvents = m_activeEvents;

	m_activeEvents = baseEvents | (prevEvents & (
		WebSocketEvent_WebSocketHandshakeRequested |
		WebSocketEvent_WebSocketHandshakeCompleted |
		WebSocketEvent_WebSocketCloseRequested |
		WebSocketEvent_WebSocketCloseCompleted
	));

	return prevEvents;
}

#if (_JNC_OS_WIN)

void
WebSocket::sslReadWriteLoop() {
	bool result;

	sys::NotificationEvent socketEvent;
	HANDLE waitTable[] = {
		m_ioThreadEvent.m_event,
		socketEvent.m_event,
	};

	sl::Array<char> readBlock;
	sl::Array<char> writeBlock;

	bool canReadSocket = true;
	bool canWriteSocket = true;

	uint_t prevSocketEventMask = 0;

	for (;;) {
		uint_t socketEventMask = FD_CLOSE;

		if (!canReadSocket)
			socketEventMask |= FD_READ;

		if (!canWriteSocket)
			socketEventMask |= FD_WRITE;

		if (socketEventMask != prevSocketEventMask) {
			result = m_socket.m_socket.wsaEventSelect(socketEvent.m_event, socketEventMask);
			if (!result) {
				setIoErrorEvent(err::getLastError());
				return;
			}

			prevSocketEventMask = socketEventMask;
		}

		DWORD waitResult = ::WaitForMultipleObjects(countof(waitTable), waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED) {
			setIoErrorEvent(err::getLastSystemErrorCode());
			return;
		}

		if (socketEvent.wait(0)) {
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents(&networkEvents);
			if (!result) {
				setIoErrorEvent();
				return;
			}

			if (networkEvents.lNetworkEvents & FD_READ) {
				int error = networkEvents.iErrorCode[FD_READ_BIT];
				if (error) {
					setIoErrorEvent(error);
					return;
				}

				canReadSocket = true;
			}

			if (networkEvents.lNetworkEvents & FD_WRITE) {
				int error = networkEvents.iErrorCode[FD_WRITE_BIT];
				if (error) {
					setIoErrorEvent(error);
					return;
				}

				canWriteSocket = true;
			}

			if (!canReadSocket && (networkEvents.lNetworkEvents & FD_CLOSE)) {
				processFdClose(networkEvents.iErrorCode[FD_CLOSE_BIT]);
				return;
			}

			socketEvent.reset();
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			return;
		}

		uint_t prevActiveEvents = resetActiveEvents(
			SocketEvent_TcpConnected |
			SslSocketEvent_SslHandshakeCompleted
		);

		readBlock.setCount(m_readBlockSize); // update read block size
		char* p = readBlock.p();

		while (canReadSocket && !m_readBuffer.isFull()) {
			m_lock.unlock();

			size_t actualSize = m_sslState->m_ssl.read(p, readBlock.getCount());
			if (actualSize == -1) {
				uint_t error = err::getLastError()->m_code;
				switch (error) {
				case SSL_ERROR_WANT_READ:
					canReadSocket = false;
					break;

				case SSL_ERROR_WANT_WRITE:
					canWriteSocket = false;
					break;

				default:
					setIoErrorEvent();
					return;
				}
			} else if (actualSize == 0) { // disconnect by remote node
				setEvents(SocketEvent_TcpDisconnected);
				return;
			} else {
				result = processIncomingData(p, actualSize);
				if (!result)
					return;
			}

			m_lock.lock();
		}

		while (canWriteSocket) {
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			size_t blockSize = writeBlock.getCount();
			updateCloseEvents(writeBlock, blockSize);
			m_lock.unlock();

			size_t actualSize = m_sslState->m_ssl.write(writeBlock, blockSize);
			if (actualSize == -1) {
				uint_t error = err::getLastError()->m_code;
				switch (error) {
				case SSL_ERROR_WANT_READ:
					canReadSocket = false;
					break;

				case SSL_ERROR_WANT_WRITE:
					canWriteSocket = false;
					break;

				default:
					setIoErrorEvent();
					return;
				}
			} else if ((size_t)actualSize < blockSize) {
				writeBlock.remove(0, actualSize);
			} else {
				writeBlock.clear();
			}

			m_lock.lock();
		}

		updateReadWriteBufferEvents();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();
	}
}

void
WebSocket::tcpSendRecvLoop() {
	ASSERT(m_socket.isOpen() && m_overlappedIo);

	bool result;

	HANDLE waitTable[3] = {
		m_ioThreadEvent.m_event,
		m_overlappedIo->m_sendOverlapped.m_completionEvent.m_event,
		NULL, // placeholder for recv completion event
	};

	size_t waitCount = 2; // always 2 or 3

	bool isSendingSocket = false;

	m_ioThreadEvent.signal(); // do initial update of active events

	for (;;) {
		dword_t waitResult = ::WaitForMultipleObjects(waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED) {
			setIoErrorEvent(err::getLastSystemErrorCode());
			break;
		}

		// do as much as we can without lock

		while (!m_overlappedIo->m_activeOverlappedRecvList.isEmpty()) {
			OverlappedRecv* recv = *m_overlappedIo->m_activeOverlappedRecvList.getHead();
			result = recv->m_overlapped.m_completionEvent.wait(0);
			if (!result)
				break;

			dword_t actualSize;
			result = m_socket.m_socket.wsaGetOverlappedResult(&recv->m_overlapped, &actualSize);
			if (!result) {
				processSendRecvError();
				return;
			}

			m_overlappedIo->m_activeOverlappedRecvList.remove(recv);
			m_overlappedIo->m_overlappedRecvPool.put(recv);
			recv->m_overlapped.m_completionEvent.reset();

			if (actualSize == 0) {
				setEvents(SocketEvent_TcpDisconnected);
				return;
			}

			result = processIncomingData(recv->m_buffer, actualSize);
			if (!result)
				return;
		}

		if (isSendingSocket && m_overlappedIo->m_sendOverlapped.m_completionEvent.wait(0)) {
			dword_t actualSize;
			result = m_socket.m_socket.wsaGetOverlappedResult(&m_overlappedIo->m_sendOverlapped, &actualSize);
			if (!result) {
				processSendRecvError();
				break;
			}

			if (actualSize < m_overlappedIo->m_sendBlock.getCount()) // shouldn't happen, actually (unless with a non-standard WSP)
				m_overlappedIo->m_sendBlock.remove(0, actualSize);
			else
				m_overlappedIo->m_sendBlock.clear();

			m_overlappedIo->m_sendOverlapped.m_completionEvent.reset();
			isSendingSocket = false;
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			break;
		}

		if (m_ioThreadFlags & IoThreadFlag_Suspended) {
			m_lock.unlock();
			continue;
		}

		uint_t prevActiveEvents = resetActiveEvents(SocketEvent_TcpConnected);

		getNextWriteBlock(&m_overlappedIo->m_sendBlock);
		size_t blockSize = m_overlappedIo->m_sendBlock.getCount();
		updateCloseEvents(m_overlappedIo->m_sendBlock, blockSize);
		updateReadWriteBufferEvents();

		// take snapshots before releasing the lock

		bool isReadBufferFull = m_readBuffer.isFull();
		size_t readBlockSize = m_readBlockSize;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();

		if (!isSendingSocket && blockSize) {
			result = m_socket.m_socket.wsaSend(
				m_overlappedIo->m_sendBlock,
				blockSize,
				NULL,
				0,
				&m_overlappedIo->m_sendOverlapped
			);

			if (!result) {
				processSendRecvError();
				break;
			}

			isSendingSocket = true;
		}

		size_t activeReadCount = m_overlappedIo->m_activeOverlappedRecvList.getCount();
		if (!isReadBufferFull && activeReadCount < Def_ReadParallelism) {
			size_t newReadCount = Def_ReadParallelism - activeReadCount;
			dword_t flags = 0; // if WSARecv doesn't complete immediately, the 'flags' arg is not touched

			for (size_t i = 0; i < newReadCount; i++) {
				OverlappedRecv* recv = m_overlappedIo->m_overlappedRecvPool.get();

				result =
					recv->m_buffer.setCount(readBlockSize) &&
					m_socket.m_socket.wsaRecv(
						recv->m_buffer.p(),
						readBlockSize,
						NULL,
						&flags,
						&recv->m_overlapped
					);

				if (!result) {
					m_overlappedIo->m_overlappedRecvPool.put(recv);
					processSendRecvError();
					return;
				}

				m_overlappedIo->m_activeOverlappedRecvList.insertTail(recv);
			}
		}

		if (m_overlappedIo->m_activeOverlappedRecvList.isEmpty()) {
			waitCount = 2;
		} else {
			OverlappedRecv* recv = *m_overlappedIo->m_activeOverlappedRecvList.getHead();
			waitTable[2] = recv->m_overlapped.m_completionEvent.m_event;
			waitCount = 3;
		}
	}
}

#elif (_JNC_OS_POSIX)

void
WebSocket::sslReadWriteLoop() {
	int result;
	int selectFd = AXL_MAX(m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	sl::Array<char> readBlock;
	sl::Array<char> writeBlock;
	readBlock.setCount(Def_ReadBlockSize);

	char buffer[256];
	sl::Array<char> writeParams(rc::BufKind_Stack, buffer, sizeof(buffer));
	writeParams.setCount(sizeof(SocketAddress));

	bool canReadSocket = true;
	bool canWriteSocket = true;

	for (;;) {
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET(m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canReadSocket)
			FD_SET(m_socket.m_socket, &readSet);

		if (!canWriteSocket)
			FD_SET(m_socket.m_socket, &writeSet);

		result = ::select(selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET(m_ioThreadSelfPipe.m_readFile, &readSet)) {
			char buffer[256];
			m_ioThreadSelfPipe.read(buffer, sizeof(buffer));
		}

		if (FD_ISSET(m_socket.m_socket, &readSet))
			canReadSocket = true;

		if (FD_ISSET(m_socket.m_socket, &writeSet))
			canWriteSocket = true;

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			return;
		}

		uint_t prevActiveEvents = resetActiveEvents(
			SocketEvent_TcpConnected |
			SslSocketEvent_SslHandshakeCompleted
		);

		readBlock.setCount(m_readBlockSize); // update read block size
		char* p = readBlock.p();

		while (canReadSocket && !m_readBuffer.isFull()) {
			m_lock.unlock();

			ssize_t actualSize = m_sslState->m_ssl.read(p, readBlock.getCount());
			if (actualSize == -1) {
				uint_t error = err::getLastError()->m_code;
				switch (error) {
				case SSL_ERROR_WANT_READ:
					canReadSocket = false;
					break;

				case SSL_ERROR_WANT_WRITE:
					canWriteSocket = false;
					break;

				default:
					setIoErrorEvent();
					return;
				}
			} else if (actualSize == 0) { // disconnect by remote node
				setEvents(SocketEvent_TcpDisconnected);
				return;
			} else {
				result = processIncomingData(p, actualSize);
				if (!result)
					return;
			}

			m_lock.lock();
		}

		while (canWriteSocket) {
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			size_t blockSize = writeBlock.getCount();
			updateCloseEvents(writeBlock, blockSize);
			m_lock.unlock();

			ssize_t actualSize = m_sslState->m_ssl.write(writeBlock, blockSize);
			if (actualSize == -1) {
				uint_t error = err::getLastError()->m_code;
				switch (error) {
				case SSL_ERROR_WANT_READ:
					canReadSocket = false;
					break;

				case SSL_ERROR_WANT_WRITE:
					canWriteSocket = false;
					break;

				default:
					setIoErrorEvent();
					return;
				}
			} else if ((size_t)actualSize < blockSize) {
				writeBlock.remove(0, actualSize);
			} else {
				writeBlock.clear();
			}

			m_lock.lock();
		}

		updateReadWriteBufferEvents();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();
	}
}

void
WebSocket::tcpSendRecvLoop() {
	int result;
	int selectFd = AXL_MAX(m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	sl::Array<char> readBlock;
	sl::Array<char> writeBlock;
	readBlock.setCount(Def_ReadBlockSize);

	char buffer[256];
	sl::Array<char> writeParams(rc::BufKind_Stack, buffer, sizeof(buffer));
	writeParams.setCount(sizeof(SocketAddress));

	bool canReadSocket = false;
	bool canWriteSocket = false;

	for (;;) {
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET(m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canReadSocket)
			FD_SET(m_socket.m_socket, &readSet);

		if (!canWriteSocket)
			FD_SET(m_socket.m_socket, &writeSet);

		result = ::select(selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET(m_ioThreadSelfPipe.m_readFile, &readSet)) {
			char buffer[256];
			m_ioThreadSelfPipe.read(buffer, sizeof(buffer));
		}

		if (FD_ISSET(m_socket.m_socket, &readSet))
			canReadSocket = true;

		if (FD_ISSET(m_socket.m_socket, &writeSet))
			canWriteSocket = true;

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			return;
		}

		if (m_ioThreadFlags & IoThreadFlag_Suspended) {
			m_lock.unlock();
			continue;
		}

		uint_t prevActiveEvents = resetActiveEvents(SocketEvent_TcpConnected);

		readBlock.setCount(m_readBlockSize); // update read block size
		char* p = readBlock.p();

		while (canReadSocket && !m_readBuffer.isFull()) {
			ssize_t actualSize = ::recv(m_socket.m_socket, p, readBlock.getCount(), 0);
			if (actualSize == -1) {
				if (errno == EAGAIN) {
					canReadSocket = false;
				} else {
					setIoErrorEvent_l(err::Errno(errno));
					return;
				}
			} else if (actualSize == 0) {
				setEvents_l(m_socket.getError() ?
					SocketEvent_TcpDisconnected | SocketEvent_TcpReset :
					SocketEvent_TcpDisconnected
				);
				return;
			} else {
				result = processIncomingData(p, actualSize);
				if (!result)
					return;
			}
		}

		while (canWriteSocket) {
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			size_t blockSize = writeBlock.getCount();
			updateCloseEvents(writeBlock, blockSize);
			ssize_t actualSize = ::send(m_socket.m_socket, writeBlock, blockSize, 0);
			if (actualSize == -1) {
				if (errno == EAGAIN) {
					canWriteSocket = false;
				} else {
					setIoErrorEvent_l(err::Errno(errno));
					return;
				}
			} else if ((size_t)actualSize < blockSize) {
				writeBlock.remove(0, actualSize);
			} else {
				writeBlock.clear();
			}
		}

		updateReadWriteBufferEvents();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
