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
#include "jnc_io_WebSockLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	WebSocket,
	"io.WebSocket",
	g_webSockLibGuid,
	WebSockLibCacheSlot_WebSocket,
	WebSocket,
	&WebSocket::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(WebSocket)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<WebSocket>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<WebSocket>)

	JNC_MAP_CONST_PROPERTY("m_address", &WebSocket::getAddress)
	JNC_MAP_CONST_PROPERTY("m_peerAddress", &WebSocket::getPeerAddress)
	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize", &WebSocket::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize", &WebSocket::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &WebSocket::setWriteBufferSize)
 	JNC_MAP_AUTOGET_PROPERTY("m_options", &WebSocket::setOptions)

	JNC_MAP_FUNCTION("open", &WebSocket::open_0)
	JNC_MAP_OVERLOAD(&WebSocket::open_1)
	JNC_MAP_FUNCTION("close", &WebSocket::close)
	JNC_MAP_FUNCTION("connect", &WebSocket::connect)
	JNC_MAP_FUNCTION("listen", &WebSocket::listen)
	JNC_MAP_FUNCTION("accept", &WebSocket::accept)
	JNC_MAP_FUNCTION("unsuspend", &WebSocket::unsuspend)
	JNC_MAP_FUNCTION("read", &WebSocket::read)
	JNC_MAP_FUNCTION("write", &WebSocket::write)
	JNC_MAP_FUNCTION("wait", &WebSocket::wait)
	JNC_MAP_FUNCTION("cancelWait", &WebSocket::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &WebSocket::blockingWait)
	JNC_MAP_FUNCTION("asyncWait", &WebSocket::asyncWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

WebSocket::WebSocket()
{
	m_sslState = NULL;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options,

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
	m_writeBuffer.setBufferSize(Def_WriteBufferSize);
}

bool
JNC_CDECL
WebSocket::open_0(
	uint16_t family,
	bool isSecure
	)
{
	close();

	return
		requireWebSockCapability() &&
		SocketBase::open(family, IPPROTO_TCP, NULL) &&
		(!isSecure || openSsl());
}

bool
JNC_CDECL
WebSocket::open_1(
	DataPtr addressPtr,
	bool isSecure
	)
{
	close();

	SocketAddress* address = (SocketAddress*)addressPtr.m_p;

	return
		requireWebSockCapability() &&
		SocketBase::open(address ? address->m_family : AF_INET, IPPROTO_TCP, address) &&
		(!isSecure || openSsl());
}

void
JNC_CDECL
WebSocket::close()
{
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

	if (m_sslState)
	{
		m_sslState->closeSsl();
		m_sslState = NULL;
	}

	SocketBase::close();
}

bool
JNC_CDECL
WebSocket::connect(DataPtr addressPtr)
{
	SocketAddress* address = (SocketAddress*)addressPtr.m_p;
	bool result = m_socket.connect(address->getSockAddr());
	if (!result)
		return false;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Connecting;
	wakeIoThread();
	m_lock.unlock();
	return result;
}

bool
JNC_CDECL
WebSocket::listen(size_t backLogLimit)
{
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
	)
{
	SocketAddress* address = ((SocketAddress*)addressPtr.m_p);
	WebSocket* connectionSocket = createClass<WebSocket> (m_runtime);

	m_lock.lock();
	if (m_pendingIncomingConnectionList.isEmpty())
	{
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

	if (address)
		address->setSockAddr(incomingConnection->m_sockAddr);

	m_incomingConnectionPool.put(incomingConnection);

	if (m_pendingIncomingConnectionList.isEmpty())
		m_activeEvents &= ~SocketEvent_IncomingConnection;

	m_lock.unlock();

	bool result =
		connectionSocket->m_socket.setBlockingMode(false) && // not guaranteed to be propagated across 'accept' calls
		(!m_sslState || connectionSocket->openSsl());

	if (!result)
		return NULL;

	connectionSocket->wakeIoThread();
	return connectionSocket;
}

bool
WebSocket::openSsl()
{
	//m_sslState = ;
	return true;
}

void
WebSocket::ioThreadFunc()
{
	ASSERT(m_socket.isOpen());

	sleepIoThread();

	m_lock.lock();
	if (m_ioThreadFlags & IoThreadFlag_Closing)
	{
		m_lock.unlock();
	}
	else if (m_ioThreadFlags & IoThreadFlag_Connecting)
	{
		m_lock.unlock();

		bool result = connectLoop(SocketEvent_Connected);
		if (result)
			transportLoop(true);
	}
	else if (m_ioThreadFlags & IoThreadFlag_Listening)
	{
		m_lock.unlock();
		acceptLoop(SocketEvent_IncomingConnection);
	}
	else if (m_ioThreadFlags & IoThreadFlag_IncomingConnection)
	{
		m_lock.unlock();

		bool result = suspendLoop();
		if (result)
			transportLoop(false);
	}
	else
	{
		m_lock.unlock();
		ASSERT(false); // shouldn't normally happen
	}
}

void
WebSocket::transportLoop(bool isClient)
{
	if (!m_sslState)
	{
		wakeIoThread();
		tcpSendRecvLoop();
		return;
	}

	bool result = sslHandshakeLoop(m_sslState, isClient);
	if (!result)
		return;

	wakeIoThread();
	sslReadWriteLoop();
}

#if (_JNC_OS_WIN)

void
WebSocket::sslReadWriteLoop()
{
	bool result;

	sys::NotificationEvent socketEvent;
	HANDLE waitTable[] =
	{
		m_ioThreadEvent.m_event,
		socketEvent.m_event,
	};

	sl::Array<char> readBlock;
	sl::Array<char> writeBlock;

	bool canReadSocket = true;
	bool canWriteSocket = true;

	uint_t prevSocketEventMask = 0;

	for (;;)
	{
		uint_t socketEventMask = FD_CLOSE;

		if (!canReadSocket)
			socketEventMask |= FD_READ;

		if (!canWriteSocket)
			socketEventMask |= FD_WRITE;

		if (socketEventMask != prevSocketEventMask)
		{
			result = m_socket.m_socket.wsaEventSelect(socketEvent.m_event, socketEventMask);
			if (!result)
			{
				setIoErrorEvent(err::getLastError());
				return;
			}

			prevSocketEventMask = socketEventMask;
		}

		DWORD waitResult = ::WaitForMultipleObjects(countof(waitTable), waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent(err::getLastSystemErrorCode());
			return;
		}

		if (socketEvent.wait(0))
		{
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents(&networkEvents);
			if (!result)
			{
				setIoErrorEvent();
				return;
			}

			if (networkEvents.lNetworkEvents & FD_READ)
			{
				int error = networkEvents.iErrorCode[FD_READ_BIT];
				if (error)
				{
					setIoErrorEvent(error);
					return;
				}

				canReadSocket = true;
			}

			if (networkEvents.lNetworkEvents & FD_WRITE)
			{
				int error = networkEvents.iErrorCode[FD_WRITE_BIT];
				if (error)
				{
					setIoErrorEvent(error);
					return;
				}

				canWriteSocket = true;
			}

			if (!canReadSocket && (networkEvents.lNetworkEvents & FD_CLOSE))
			{
				processFdClose(networkEvents.iErrorCode[FD_CLOSE_BIT]);
				return;
			}

			socketEvent.reset();
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = SocketEvent_Connected | SslSocketEvent_SslHandshakeCompleted;

		readBlock.setCount(m_readBlockSize); // update read block size

		while (canReadSocket && !m_readBuffer.isFull())
		{
			m_lock.unlock();

			size_t actualSize = m_sslState->m_ssl.read(readBlock, readBlock.getCount());
			if (actualSize == -1)
			{
				uint_t error = err::getLastError()->m_code;
				switch (error)
				{
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

				m_lock.lock();
			}
			else if (actualSize == 0) // disconnect by remote node
			{
				setEvents(SocketEvent_Disconnected);
				return;
			}
			else
			{
				m_lock.lock();
				addToReadBuffer(readBlock, actualSize);
			}
		}

		while (canWriteSocket)
		{
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			m_lock.unlock();

			size_t blockSize = writeBlock.getCount();
			size_t actualSize = m_sslState->m_ssl.write(writeBlock, blockSize);
			if (actualSize == -1)
			{
				uint_t error = err::getLastError()->m_code;
				switch (error)
				{
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
			}
			else if ((size_t)actualSize < blockSize)
			{
				writeBlock.remove(0, actualSize);
			}
			else
			{
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
WebSocket::tcpSendRecvLoop()
{
	ASSERT(m_socket.isOpen() && m_overlappedIo);

	bool result;

	HANDLE waitTable[3] =
	{
		m_ioThreadEvent.m_event,
		m_overlappedIo->m_sendOverlapped.m_completionEvent.m_event,
		NULL, // placeholder for recv completion event
	};

	size_t waitCount = 2; // always 2 or 3

	bool isSendingSocket = false;

	m_ioThreadEvent.signal(); // do initial update of active events

	for (;;)
	{
		dword_t waitResult = ::WaitForMultipleObjects(waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent(err::getLastSystemErrorCode());
			break;
		}

		// do as much as we can without lock

		while (!m_overlappedIo->m_activeOverlappedRecvList.isEmpty())
		{
			OverlappedRecv* recv = *m_overlappedIo->m_activeOverlappedRecvList.getHead();
			result = recv->m_overlapped.m_completionEvent.wait(0);
			if (!result)
				break;

			dword_t actualSize;
			result = m_socket.m_socket.wsaGetOverlappedResult(&recv->m_overlapped, &actualSize);
			if (!result)
			{
				processSendRecvError();
				return;
			}

			m_overlappedIo->m_activeOverlappedRecvList.remove(recv);

			if (actualSize == 0)
			{
				setEvents(SocketEvent_Disconnected);
				return;
			}

			// only the main read buffer must be lock-protected

			m_lock.lock();
			addToReadBuffer(recv->m_buffer, actualSize);
			m_lock.unlock();

			recv->m_overlapped.m_completionEvent.reset();
			m_overlappedIo->m_overlappedRecvPool.put(recv);
		}

		if (isSendingSocket && m_overlappedIo->m_sendOverlapped.m_completionEvent.wait(0))
		{
			dword_t actualSize;
			result = m_socket.m_socket.wsaGetOverlappedResult(&m_overlappedIo->m_sendOverlapped, &actualSize);
			if (!result)
			{
				processSendRecvError();
				break;
			}

			if (actualSize < m_overlappedIo->m_sendBlock.getCount()) // shouldn't happen, actually (unless with a non-standard WSP)
				m_overlappedIo->m_sendBlock.remove(0, actualSize);

			m_overlappedIo->m_sendOverlapped.m_completionEvent.reset();
			isSendingSocket = false;
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			break;
		}

		if (m_ioThreadFlags & IoThreadFlag_Suspended)
		{
			m_lock.unlock();
			continue;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = SocketEvent_Connected;

		getNextWriteBlock(&m_overlappedIo->m_sendBlock);
		updateReadWriteBufferEvents();

		// take snapshots before releasing the lock

		bool isReadBufferFull = m_readBuffer.isFull();
		size_t readBlockSize = m_readBlockSize;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();

		if (!isSendingSocket && !m_overlappedIo->m_sendBlock.isEmpty())
		{
			result = m_socket.m_socket.wsaSend(
				m_overlappedIo->m_sendBlock,
				m_overlappedIo->m_sendBlock.getCount(),
				NULL,
				0,
				&m_overlappedIo->m_sendOverlapped
				);

			if (!result)
			{
				processSendRecvError();
				break;
			}

			isSendingSocket = true;
		}

		size_t activeReadCount = m_overlappedIo->m_activeOverlappedRecvList.getCount();
		if (!isReadBufferFull && activeReadCount < Def_ReadParallelism)
		{
			size_t newReadCount = Def_ReadParallelism - activeReadCount;
			dword_t flags = 0; // if WSARecv doesn't complete immediately, the 'flags' arg is not touched

			for (size_t i = 0; i < newReadCount; i++)
			{
				OverlappedRecv* recv = m_overlappedIo->m_overlappedRecvPool.get();
				result =
					recv->m_buffer.setCount(readBlockSize) &&
					m_socket.m_socket.wsaRecv(
						recv->m_buffer,
						readBlockSize,
						NULL,
						&flags,
						&recv->m_overlapped
						);

				if (!result)
				{
					m_overlappedIo->m_overlappedRecvPool.put(recv);
					processSendRecvError();
					return;
				}

				m_overlappedIo->m_activeOverlappedRecvList.insertTail(recv);
			}
		}

		if (m_overlappedIo->m_activeOverlappedRecvList.isEmpty())
		{
			waitCount = 2;
		}
		else
		{
			OverlappedRecv* recv = *m_overlappedIo->m_activeOverlappedRecvList.getHead();
			waitTable[2] = recv->m_overlapped.m_completionEvent.m_event;
			waitCount = 3;
		}
	}
}

#elif (_JNC_OS_POSIX)

bool
WebSocket::sslHandshakeLoop(bool isClient)
{
	if (isClient)
		m_ssl.setConnectState();
	else
		m_ssl.setAcceptState();

	int selectFd = AXL_MAX(m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	for (;;)
	{
		int result = m_ssl.doHandshake();
		if (result)
			break;

		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET(m_ioThreadSelfPipe.m_readFile, &readSet);

		uint_t error = err::getLastError()->m_code;
		switch (error)
		{
		case SSL_ERROR_WANT_READ:
			FD_SET(m_socket.m_socket, &readSet);
			break;

		case SSL_ERROR_WANT_WRITE:
			FD_SET(m_socket.m_socket, &writeSet);
			break;

		default:
			setIoErrorEvent();
			return false;
		}

		result = ::select(selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET(m_ioThreadSelfPipe.m_readFile, &readSet))
		{
			char buffer[256];
			m_ioThreadSelfPipe.read(buffer, sizeof(buffer));
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			return false;
		}

		m_lock.unlock();
	}

	m_lock.lock();
	m_activeEvents = SocketEvent_Connected | SslSocketEvent_SslHandshakeCompleted;
	processWaitLists_l();

	return true;
}

void
WebSocket::sslReadWriteLoop()
{
	int result;
	int selectFd = AXL_MAX(m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	sl::Array<char> readBlock;
	sl::Array<char> writeBlock;
	readBlock.setCount(Def_ReadBlockSize);

	char buffer[256];
	sl::Array<char> writeParams(ref::BufKind_Stack, buffer, sizeof(buffer));
	writeParams.setCount(sizeof(SocketAddress));

	bool canReadSocket = true;
	bool canWriteSocket = true;

	for (;;)
	{
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

		if (FD_ISSET(m_ioThreadSelfPipe.m_readFile, &readSet))
		{
			char buffer[256];
			m_ioThreadSelfPipe.read(buffer, sizeof(buffer));
		}

		if (FD_ISSET(m_socket.m_socket, &readSet))
			canReadSocket = true;

		if (FD_ISSET(m_socket.m_socket, &writeSet))
			canWriteSocket = true;

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = SocketEvent_Connected | SslSocketEvent_SslHandshakeCompleted;

		readBlock.setCount(m_readBlockSize); // update read block size

		while (canReadSocket && !m_readBuffer.isFull())
		{
			m_lock.unlock();

			ssize_t actualSize = m_ssl.read(readBlock, readBlock.getCount());
			if (actualSize == -1)
			{
				uint_t error = err::getLastError()->m_code;
				switch (error)
				{
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

				m_lock.lock();
			}
			else if (actualSize == 0) // disconnect by remote node
			{
				setEvents(SocketEvent_Disconnected);
				return;
			}
			else
			{
				m_lock.lock();
				addToReadBuffer(readBlock, actualSize);
			}
		}

		while (canWriteSocket)
		{
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			m_lock.unlock();

			size_t blockSize = writeBlock.getCount();
			ssize_t actualSize = m_ssl.write(writeBlock, blockSize);
			if (actualSize == -1)
			{
				uint_t error = err::getLastError()->m_code;
				switch (error)
				{
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
			}
			else if ((size_t)actualSize < blockSize)
			{
				writeBlock.remove(0, actualSize);
			}
			else
			{
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

#endif

//..............................................................................

} // namespace io
} // namespace jnc
