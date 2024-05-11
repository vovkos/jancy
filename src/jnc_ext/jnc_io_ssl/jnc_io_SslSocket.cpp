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
#include "jnc_io_SslSocket.h"
#include "jnc_io_SslLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	SslSocket,
	"io.SslSocket",
	g_sslLibGuid,
	SslLibCacheSlot_SslSocket,
	SslSocket,
	&SslSocket::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(SslSocket)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<SslSocket>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<SslSocket>)

	JNC_MAP_CONST_PROPERTY("m_address", &SslSocket::getAddress)
	JNC_MAP_CONST_PROPERTY("m_peerAddress", &SslSocket::getPeerAddress)
	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize", &SslSocket::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize", &SslSocket::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &SslSocket::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options", &SslSocket::setOptions)
	JNC_MAP_PROPERTY("m_hostname", &SslSocket::getHostname, &SslSocket::setHostname)

	JNC_MAP_FUNCTION("open", &SslSocket::open_0)
	JNC_MAP_OVERLOAD(&SslSocket::open_1)
	JNC_MAP_FUNCTION("close", &SslSocket::close)
	JNC_MAP_FUNCTION("connect", &SslSocket::connect)
	JNC_MAP_FUNCTION("listen", &SslSocket::listen)
	JNC_MAP_FUNCTION("accept", &SslSocket::accept)
	JNC_MAP_FUNCTION("unsuspend", &SslSocket::unsuspend)
	JNC_MAP_FUNCTION("read", &SslSocket::read)
	JNC_MAP_FUNCTION("write", &SslSocket::write)
	JNC_MAP_FUNCTION("wait", &SslSocket::wait)
	JNC_MAP_FUNCTION("cancelWait", &SslSocket::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &SslSocket::blockingWait)
	JNC_MAP_FUNCTION("asyncWait", &SslSocket::asyncWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

SslSocket::SslSocket() {
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options,

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
	m_writeBuffer.setBufferSize(Def_WriteBufferSize);
}

bool
JNC_CDECL
SslSocket::open_0(uint16_t family) {
	close();

	return
		requireSslCapability() &&
		SocketBase::open(family, IPPROTO_TCP, NULL) &&
		openSsl(m_runtime, &m_socket) &&
		m_ioThread.start();
}

bool
JNC_CDECL
SslSocket::open_1(DataPtr addressPtr) {
	close();

	SocketAddress* address = (SocketAddress*)addressPtr.m_p;

	return
		requireSslCapability() &&
		SocketBase::open(address ? address->m_family : AF_INET, IPPROTO_TCP, address) &&
		openSsl(m_runtime, &m_socket) &&
		m_ioThread.start();
}

void
JNC_CDECL
SslSocket::close() {
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

	closeSsl();
	SocketBase::close();
}

bool
JNC_CDECL
SslSocket::connect(DataPtr addressPtr) {
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
SslSocket::listen(size_t backLogLimit) {
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

SslSocket*
JNC_CDECL
SslSocket::accept(
	DataPtr addressPtr,
	bool isSuspended
) {
	SocketAddress* address = ((SocketAddress*)addressPtr.m_p);
	SslSocket* connectionSocket = createClass<SslSocket>(m_runtime);

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

	if (address)
		address->setSockAddr(incomingConnection->m_sockAddr);

	m_incomingConnectionPool.put(incomingConnection);

	if (m_pendingIncomingConnectionList.isEmpty())
		m_activeEvents &= ~SocketEvent_IncomingConnection;

	m_lock.unlock();

	bool result =
		connectionSocket->m_socket.setBlockingMode(false) && // not guaranteed to be propagated across 'accept' calls
		connectionSocket->openSsl(m_runtime, &connectionSocket->m_socket) &&
		connectionSocket->m_ioThread.start();

	if (!result)
		return NULL;

	connectionSocket->wakeIoThread();
	return connectionSocket;
}

void
SslSocket::ioThreadFunc() {
	ASSERT(m_socket.isOpen());

	sleepIoThread();

	m_lock.lock();
	if (m_ioThreadFlags & IoThreadFlag_Closing) {
		m_lock.unlock();
	} else if (m_ioThreadFlags & IoThreadFlag_Connecting) {
		m_lock.unlock();

		bool result =
			connectLoop(SocketEvent_TcpConnected) &&
			sslHandshakeLoop(this, true);

		if (result) {
			wakeIoThread();
			sslReadWriteLoop();
		}
	} else if (m_ioThreadFlags & IoThreadFlag_Listening) {
		m_lock.unlock();
		acceptLoop(SocketEvent_IncomingConnection);
	} else if (m_ioThreadFlags & IoThreadFlag_IncomingConnection) {
		m_lock.unlock();

		bool result =
			suspendLoop() &&
			sslHandshakeLoop(this, false);

		if (result) {
			wakeIoThread();
			sslReadWriteLoop();
		}
	} else {
		m_lock.unlock();
		ASSERT(false); // shouldn't normally happen
	}
}

#if (_JNC_OS_WIN)

void
SslSocket::sslReadWriteLoop() {
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

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = SocketEvent_TcpConnected | SslSocketEvent_SslHandshakeCompleted;

		readBlock.setCount(m_readBlockSize); // update read block size
		char* p = readBlock.p();

		while (canReadSocket && !m_readBuffer.isFull()) {
			m_lock.unlock();

			size_t actualSize = m_ssl.read(p, readBlock.getCount());
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

				m_lock.lock();
			} else if (actualSize == 0) { // disconnect by remote node
				setEvents(SocketEvent_TcpDisconnected);
				return;
			} else {
				m_lock.lock();
				addToReadBuffer(p, actualSize);
			}
		}

		while (canWriteSocket) {
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			m_lock.unlock();

			size_t blockSize = writeBlock.getCount();
			size_t actualSize = m_ssl.write(writeBlock, blockSize);
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

#elif (_JNC_OS_POSIX)

void
SslSocket::sslReadWriteLoop() {
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

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = SocketEvent_TcpConnected | SslSocketEvent_SslHandshakeCompleted;

		readBlock.setCount(m_readBlockSize); // update read block size
		char* p = readBlock.p();

		while (canReadSocket && !m_readBuffer.isFull()) {
			m_lock.unlock();

			ssize_t actualSize = m_ssl.read(p, readBlock.getCount());
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

				m_lock.lock();
			} else if (actualSize == 0) { // disconnect by remote node
				setEvents(SocketEvent_TcpDisconnected);
				return;
			} else {
				m_lock.lock();
				addToReadBuffer(p, actualSize);
			}
		}

		while (canWriteSocket) {
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			m_lock.unlock();

			size_t blockSize = writeBlock.getCount();
			ssize_t actualSize = m_ssl.write(writeBlock, blockSize);
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

#endif

//..............................................................................

} // namespace io
} // namespace jnc
