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
#include "jnc_Error.h"

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
	JNC_MAP_CONST_PROPERTY("m_stateString", &SslSocket::getStateString)
	JNC_MAP_CONST_PROPERTY("m_stateStringLong", &SslSocket::getStateStringLong)
	JNC_MAP_CONST_PROPERTY("m_peerCertificateChainLength", &SslSocket::getPeerCertificateChainLength)
	JNC_MAP_CONST_PROPERTY("m_peerCertificateChainEntry", &SslSocket::getPeerCertificateChainEntry)
	JNC_MAP_CONST_PROPERTY("m_peerCertificate", &SslSocket::getPeerCertificate)
	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize", &SslSocket::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize", &SslSocket::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &SslSocket::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options", &SslSocket::setOptions)

	JNC_MAP_FUNCTION("open", &SslSocket::open_0)
	JNC_MAP_OVERLOAD(&SslSocket::open_1)
	JNC_MAP_FUNCTION("close", &SslSocket::close)
	JNC_MAP_FUNCTION("loadCertificate", &SslSocket::loadCertificate)
	JNC_MAP_FUNCTION("loadPrivateKey", &SslSocket::loadPrivateKey)
	JNC_MAP_FUNCTION("loadCaCertificate", &SslSocket::loadCaCertificate)
	JNC_MAP_FUNCTION("setCaCertificateDir", &SslSocket::setCaCertificateDir)
	JNC_MAP_FUNCTION("connect", &SslSocket::connect)
	JNC_MAP_FUNCTION("listen", &SslSocket::listen)
	JNC_MAP_FUNCTION("accept", &SslSocket::accept)
	JNC_MAP_FUNCTION("unsuspend", &SslSocket::unsuspend)
	JNC_MAP_FUNCTION("read", &SslSocket::read)
	JNC_MAP_FUNCTION("write", &SslSocket::write)
	JNC_MAP_FUNCTION("wait", &SslSocket::wait)
	JNC_MAP_FUNCTION("cancelWait", &SslSocket::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &SslSocket::blockingWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

SslSocket::SslSocket()
{
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options,

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
	m_writeBuffer.setBufferSize(Def_WriteBufferSize);
}

size_t
JNC_CDECL
SslSocket::getPeerCertificateChainLength()
{
	STACK_OF(X509)* chain = SSL_get_peer_cert_chain(m_ssl);
	return chain ? sk_X509_num(chain) : 0;
}

SslCertificate*
JNC_CDECL
SslSocket::getPeerCertificateChainEntry(size_t i)
{
	STACK_OF(X509)* chain = SSL_get_peer_cert_chain(m_ssl);
	X509* cert = sk_X509_value(chain, i);
	return cert ? SslCertificate::create(cert) : NULL;
}

SslCertificate*
JNC_CDECL
SslSocket::getPeerCertificate()
{
	X509* cert = SSL_get_peer_certificate(m_ssl);
	return cert ? SslCertificate::create(cert) : NULL;
}

bool
JNC_CDECL
SslSocket::open_0(uint16_t family)
{
	close();

	return
		SocketBase::open(family, IPPROTO_TCP, NULL) &&
		m_sslCtx.create() &&
		m_ioThread.start();
}

bool
JNC_CDECL
SslSocket::open_1(DataPtr addressPtr)
{
	close();

	SocketAddress* address = (SocketAddress*)addressPtr.m_p;

	return
		SocketBase::open(address ? address->m_family : AF_INET, IPPROTO_TCP, address) &&
		m_sslCtx.create() &&
		m_ioThread.start();
}

void
JNC_CDECL
SslSocket::close()
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

	m_localAddress.m_family = jnc::io::AddressFamily_Undefined;

	m_ssl.close();
	m_sslBio.close();
	m_sslCtx.close();
	SocketBase::close();
}

bool
JNC_CDECL
SslSocket::connect(DataPtr addressPtr)
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
SslSocket::listen(size_t backLogLimit)
{
	bool result = m_socket.listen(backLogLimit);
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
	)
{
	SocketAddress* address = ((SocketAddress*)addressPtr.m_p);
	SslSocket* connectionSocket = createClass<SslSocket> (m_runtime);

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
		m_activeEvents &= ~SslSocketEvent_IncomingConnection;

	m_lock.unlock();

	bool result =
		connectionSocket->m_socket.setBlockingMode(false) && // not guaranteed to be propagated across 'accept' calls
		connectionSocket->m_sslCtx.create() &&
		connectionSocket->m_ioThread.start();

	if (!result)
		return NULL;

	connectionSocket->wakeIoThread();
	return connectionSocket;
}

void
SslSocket::ioThreadFunc()
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

		bool result =
			connectLoop(SslSocketEvent_TcpConnected) &&
			sslHandshakeLoop(true);

		if (result)
		{
			wakeIoThread();
			sslReadWriteLoop();
		}
	}
	else if (m_ioThreadFlags & IoThreadFlag_Listening)
	{
		m_lock.unlock();
		acceptLoop(SslSocketEvent_IncomingConnection);
	}
	else if (m_ioThreadFlags & IoThreadFlag_IncomingConnection)
	{
		m_lock.unlock();

		bool result = sslHandshakeLoop(false);
		if (result)
		{
			wakeIoThread();
			sslReadWriteLoop();
		}
	}
	else
	{
		m_lock.unlock();
		ASSERT(false); // shouldn't normally happen
	}
}

#if (_JNC_OS_WIN)

void
SslSocket::processFdClose(int error)
{
	if (!error)
		setEvents(SslSocketEvent_TcpDisconnected);
	else if (error == WSAECONNRESET)
		setEvents(SslSocketEvent_TcpDisconnected | SslSocketEvent_TcpReset);
	else
		setIoErrorEvent(error);
}

bool
SslSocket::sslHandshakeLoop(bool isClient)
{
	bool result =
		m_sslBio.createSocket(m_socket.m_socket) &&
		m_ssl.create(m_sslCtx);

	if (!result)
	{
		setIoErrorEvent();
		return false;
	}

	m_ssl.setBio(m_sslBio.detach());
	m_ssl.setExtraData(g_sslSocketSelfIdx, this);
	m_ssl.setInfoCallback(sslInfoCallback);

	if (isClient)
		m_ssl.setConnectState();
	else
		m_ssl.setAcceptState();

	sys::NotificationEvent socketEvent;
	HANDLE waitTable[] =
	{
		m_ioThreadEvent.m_event,
		socketEvent.m_event,
	};

	for (;;)
	{
		result = m_ssl.doHandshake();
		if (result)
			break;

		uint_t socketEventMask = FD_CLOSE;
		uint_t error = err::getLastError()->m_code;
		switch (error)
		{
		case SSL_ERROR_WANT_READ:
			socketEventMask |= FD_READ;
			break;

		case SSL_ERROR_WANT_WRITE:
			socketEventMask |= FD_WRITE;
			break;

		default:
			setIoErrorEvent();
			return false;
		}

		result = m_socket.m_socket.wsaEventSelect(socketEvent.m_event, socketEventMask);
		if (!result)
			return false;

		DWORD waitResult = ::WaitForMultipleObjects(countof(waitTable), waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent(err::getLastSystemErrorCode());
			return false;
		}

		WSANETWORKEVENTS networkEvents;
		result = m_socket.m_socket.wsaEnumEvents(&networkEvents);
		if (!result)
		{
			setIoErrorEvent();
			return false;
		}

		if ((networkEvents.lNetworkEvents & FD_CLOSE) && !(networkEvents.lNetworkEvents & FD_READ))
		{
			processFdClose(networkEvents.iErrorCode[FD_CLOSE_BIT]);
			return false;
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
	m_activeEvents = SslSocketEvent_TcpConnected | SslSocketEvent_SslHandshakeCompleted;
	processWaitLists_l();

	return true;
}

void
SslSocket::sslReadWriteLoop()
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
		m_activeEvents = SslSocketEvent_TcpConnected | SslSocketEvent_SslHandshakeCompleted;

		readBlock.setCount(m_readBlockSize); // update read block size

		while (canReadSocket && !m_readBuffer.isFull())
		{
			m_lock.unlock();

			size_t actualSize = m_ssl.read(readBlock, readBlock.getCount());
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
				setEvents(SslSocketEvent_TcpDisconnected);
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
			size_t actualSize = m_ssl.write(writeBlock, blockSize);
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

#elif (_JNC_OS_POSIX)

bool
SslSocket::sslHandshakeLoop(bool isClient)
{
	bool result =
		m_sslBio.createSocket(m_socket.m_socket) &&
		m_ssl.create(m_sslCtx);

	if (!result)
	{
		setIoErrorEvent();
		return false;
	}

	m_ssl.setBio(m_sslBio.detach());
	m_ssl.setExtraData(g_sslSocketSelfIdx, this);
	m_ssl.setInfoCallback(sslInfoCallback);

	if (isClient)
		m_ssl.setConnectState();
	else
		m_ssl.setAcceptState();

	int selectFd = AXL_MAX(m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	for (;;)
	{
		result = m_ssl.doHandshake();
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
	m_activeEvents = SslSocketEvent_TcpConnected | SslSocketEvent_SslHandshakeCompleted;
	processWaitLists_l();

	return true;
}

void
SslSocket::sslReadWriteLoop()
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
		m_activeEvents = SslSocketEvent_TcpConnected | SslSocketEvent_SslHandshakeCompleted;

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
				setEvents(SslSocketEvent_TcpDisconnected);
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

void
SslSocket::sslInfoCallback(
	const SSL* ssl,
	int where,
	int ret
	)
{
	SslSocket* self = (SslSocket*)::SSL_get_ex_data(ssl, g_sslSocketSelfIdx);
	if (!self)
		return;

	ASSERT(self->m_ssl == ssl);
	callMulticast(self->m_runtime, self->m_onStateChanged, where, ret);
}

//..............................................................................

} // namespace io
} // namespace jnc
