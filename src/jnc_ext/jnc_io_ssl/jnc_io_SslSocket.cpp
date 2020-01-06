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

	JNC_MAP_CONST_PROPERTY("m_address",     &SslSocket::getAddress)
	JNC_MAP_CONST_PROPERTY("m_peerAddress", &SslSocket::getPeerAddress)

	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize",   &SslSocket::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize",  &SslSocket::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &SslSocket::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options",         &SslSocket::setOptions)

	JNC_MAP_FUNCTION("open",         &SslSocket::open_0)
	JNC_MAP_OVERLOAD(&SslSocket::open_1)
	JNC_MAP_FUNCTION("close",        &SslSocket::close)
	JNC_MAP_FUNCTION("connect",      &SslSocket::connect)
	JNC_MAP_FUNCTION("listen",       &SslSocket::listen)
	JNC_MAP_FUNCTION("accept",       &SslSocket::accept)
	JNC_MAP_FUNCTION("unsuspend",    &SslSocket::unsuspend)
	JNC_MAP_FUNCTION("read",         &SslSocket::read)
	JNC_MAP_FUNCTION("write",        &SslSocket::write)
	JNC_MAP_FUNCTION("wait",         &SslSocket::wait)
	JNC_MAP_FUNCTION("cancelWait",   &SslSocket::cancelWait)
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

bool
JNC_CDECL
SslSocket::open_0(uint16_t family)
{
	close();

	bool result = SocketBase::open(family, IPPROTO_TCP, NULL);
	if (!result)
		return false;

	m_ioThread.start();
	return true;
}

bool
JNC_CDECL
SslSocket::open_1(DataPtr addressPtr)
{
	close();

	SocketAddress* address = (SocketAddress*)addressPtr.m_p;

	bool result = SocketBase::open(address ? address->m_family : AF_INET, IPPROTO_TCP, address);
	if (!result)
		return false;

	m_ioThread.start();
	return true;
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
	err::setError(err::SystemErrorCode_NotImplemented);
	return NULL;
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
		bool result = tcpConnect(SslSocketEvent_TcpConnected);
		if (result)
			sendRecvLoop(SslSocketEvent_TcpConnected);
	}
	else if (m_ioThreadFlags & IoThreadFlag_Listening)
	{
		m_lock.unlock();
		acceptLoop();
	}
	else if (m_ioThreadFlags & IoThreadFlag_IncomingConnection)
	{
		m_lock.unlock();
		sendRecvLoop(SslSocketEvent_TcpConnected);
	}
	else
	{
		m_lock.unlock();
		ASSERT(false); // shouldn't normally happen
	}
}

void
SslSocket::acceptLoop()
{
}

void
SslSocket::sendRecvLoop(uint_t baseEvents)
{
}

#if (0)

bool
JNC_CDECL
SslSocket::connect(DataPtr addressPtr)
{
	const SocketAddress* address = (const SocketAddress*) addressPtr.m_p;
	bool result;

	m_lock.lock();
	if (m_ioThreadFlags)
	{
		m_lock.unlock();

		setError(err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	ASSERT(!m_connectParams);
	m_connectParams = AXL_MEM_NEW(ConnectParams);
	m_connectParams->m_userName = userName ? userName : "anonymous";

	if (privateKey && privateKeySize)
		m_connectParams->m_privateKey.copy((char*)privateKey, privateKeySize);

	m_connectParams->m_password = password;
	m_connectParams->m_channelType = channelType ? channelType: "session";

	if (channelExtra && channelExtraSize)
		m_connectParams->m_channelExtra.copy((char*)channelExtra, channelExtraSize);

	m_connectParams->m_processType = processType;

	if (processExtra && processExtraSize)
		m_connectParams->m_processExtra.copy((char*)processExtra, processExtraSize);

	m_connectParams->m_ptyType = ptyType;
	m_ptyWidth = ptyWidth;
	m_ptyHeight = ptyHeight;
	m_lock.unlock();

	m_remoteAddress = *address;
	result = m_socket.connect(m_remoteAddress.getSockAddr());
	if (!result)
		return false;

	wakeIoThread();
	return true;
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
		return;
	}

	m_lock.unlock();

	bool result = tcpConnect(SslSocketEvent_TcpConnectCompleted) && SslConnect();
	if (result)
	{
		wakeIoThread();
		SslReadWriteLoop();
	}

	m_SslSocket.close();
	m_SslSession.close();
}

err::Error
SslSocket::getLastSslError()
{
	char* errorString = NULL;
	int length = 0;
	::libSsl2_session_last_error(m_SslSession, &errorString, &length, false);
	return err::Error(sl::StringRef(errorString, length));
}

int
SslSocket::SslAsyncLoop(int result)
{
	if (result >= 0)
		return result;

	if (result != LIBSsl2_ERROR_EAGAIN)
	{
		setError(getLastSslError());
		return result;
	}

	if (m_ioThreadFlags & IoThreadFlag_Closing)
	{
		setError(err::Error(err::SystemErrorCode_Cancelled));
		return LIBSsl2_ERROR_CHANNEL_CLOSED;
	}

	sys::sleep(10);
	return LIBSsl2_ERROR_EAGAIN;
}

err::Error
getSslLastError(LIBSsl2_SESSION* SslSession)
{
	char* string;
	int length;
	::libSsl2_session_last_error(SslSession, &string, &length, false);
	return err::Error(sl::StringRef(string, length));
}

bool
SslSocket::SslConnect()
{
	int result;

	LIBSsl2_SESSION* SslSession = ::libSsl2_session_init();

	m_SslSession.attach(SslSession);
	::libSsl2_session_set_blocking(m_SslSession, false);

	do
	{
		result = ::libSsl2_session_handshake(m_SslSession, m_socket.m_socket);
		result = SslAsyncLoop(result);
	} while (result == LIBSsl2_ERROR_EAGAIN);

	if (result)
	{
		setIoErrorEvent(getSslLastError(m_SslSession));
		return false;
	}

	setEvents(SslSocketEvent_SslHandshakeCompleted);

	// loop to give user a chance to re-authenticate

	sl::String prevUserName = m_connectParams->m_userName;

	for (;;)
	{
		do
		{
			result = m_connectParams->m_privateKey.isEmpty() ?
				::libSsl2_userauth_password(
					m_SslSession,
					m_connectParams->m_userName,
					m_connectParams->m_password
					) :
				::libSsl2_userauth_publickey_frommemory(
					m_SslSession,
					m_connectParams->m_userName.cp(),
					m_connectParams->m_userName.getLength(),
					NULL,
					0,
					m_connectParams->m_privateKey.cp(),
					m_connectParams->m_privateKey.getCount(),
					m_connectParams->m_password.sz()
					);

			result = SslAsyncLoop(result);
		} while (result == LIBSsl2_ERROR_EAGAIN);

		if (!result)
			break;

		if (result != LIBSsl2_ERROR_AUTHENTICATION_FAILED &&
			result != LIBSsl2_ERROR_PUBLICKEY_UNVERIFIED)
		{
			setIoErrorEvent(getSslLastError(m_SslSession));
			return false;
		}

		setErrorEvent(SslSocketEvent_SslAuthenticateError, getSslLastError(m_SslSession));
		sleepIoThread();

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			return false;
		}

		if (m_connectParams->m_userName == prevUserName)
		{
			m_lock.unlock();
		}
		else // need to re-connect
		{
			prevUserName = m_connectParams->m_userName;
			m_lock.unlock();

			m_socket.close();

			result = m_socket.open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (!result)
			{
				setIoErrorEvent(err::getLastError());
				return false;
			}

			if (m_localAddress.m_family)
			{
				result = m_socket.bind(m_localAddress.getSockAddr());
				if (!result)
				{
					setIoErrorEvent(err::getLastError());
					return false;
				}
			}

			result = m_socket.connect(m_remoteAddress.getSockAddr());
			if (!result)
			{
				setIoErrorEvent(err::getLastError());
				return false;
			}

			result = tcpConnect(SslSocketEvent_TcpConnectCompleted);
			if (!result)
				return false;

			SslSession = ::libSsl2_session_init();

			m_SslSession.attach(SslSession);
			::libSsl2_session_set_blocking(m_SslSession, false);

			do
			{
				result = ::libSsl2_session_handshake(m_SslSession, m_socket.m_socket);
				result = SslAsyncLoop(result);
			} while (result == LIBSsl2_ERROR_EAGAIN);

			if (result)
			{
				setIoErrorEvent(getSslLastError(m_SslSession));
				return false;
			}

			setEvents(SslSocketEvent_SslHandshakeCompleted);
		}
	}

	setEvents(SslSocketEvent_SslAuthenticateCompleted);

	LIBSsl2_CHANNEL* channel;

	do
	{
		channel = ::libSsl2_channel_open_ex(
			m_SslSession,
			m_connectParams->m_channelType,
			m_connectParams->m_channelType.getLength(),
			LIBSsl2_CHANNEL_WINDOW_DEFAULT,
			LIBSsl2_CHANNEL_PACKET_DEFAULT,
			m_connectParams->m_channelExtra,
			m_connectParams->m_channelExtra.getCount()
			);

		result = channel ? 0 : SslAsyncLoop(::libSsl2_session_last_errno(m_SslSession));
	} while (result == LIBSsl2_ERROR_EAGAIN);

	if (!channel)
	{
		setIoErrorEvent(getSslLastError(m_SslSession));
		return false;
	}

	m_SslSocket.attach(channel);
	::libSsl2_channel_set_blocking(m_SslSocket, false);

	setEvents(SslSocketEvent_SslSocketOpened);

	if (!m_connectParams->m_ptyType.isEmpty())
	{
		do
		{
			result = ::libSsl2_channel_request_pty_ex(
				m_SslSocket,
				m_connectParams->m_ptyType,
				m_connectParams->m_ptyType.getLength(),
				NULL, 0,
				m_ptyWidth,
				m_ptyHeight,
				0, 0
				);

			result = SslAsyncLoop(result);
		} while (result == LIBSsl2_ERROR_EAGAIN);

		if (result)
		{
			setIoErrorEvent(getSslLastError(m_SslSession));
			return false;
		}
	}

	setEvents(SslSocketEvent_SslPtyRequestCompleted);

	if (!m_connectParams->m_processType.isEmpty())
	{
		do
		{
			result = ::libSsl2_channel_process_startup(
				m_SslSocket,
				m_connectParams->m_processType,
				m_connectParams->m_processType.getLength(),
				m_connectParams->m_processExtra,
				m_connectParams->m_processExtra.getCount()
				);

			result = SslAsyncLoop(result);
		} while (result == LIBSsl2_ERROR_EAGAIN);

		if (result)
		{
			setIoErrorEvent(getSslLastError(m_SslSession));
			return false;
		}
	}

	setEvents(SslSocketEvent_SslConnectCompleted);

	AXL_MEM_DELETE(m_connectParams);
	m_connectParams = NULL;
	return true;
}

#if (_JNC_OS_WIN)

void
SslSocket::SslReadWriteLoop()
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
		uint_t socketEventMask = 0;

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

			socketEvent.reset();
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = SslSocketEvent_FullyConnected;

		readBlock.setCount(m_readBlockSize); // update read block size

		while (canReadSocket && !m_readBuffer.isFull())
		{
			ssize_t actualSize = ::libSsl2_channel_read(m_SslSocket, readBlock, readBlock.getCount());
			if (actualSize == LIBSsl2_ERROR_EAGAIN)
			{
				canReadSocket = false;
			}
			else if (actualSize < 0)
			{
				setIoErrorEvent_l(err::Errno((int)actualSize));
				return;
			}
			else if (actualSize == 0) // disconnect by remote node
			{
				setEvents_l(SslSocketEvent_TcpDisconnected);
				return;
			}
			else
			{
				addToReadBuffer(readBlock, actualSize);
			}
		}

		while (canWriteSocket)
		{
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			size_t blockSize = writeBlock.getCount();
			ssize_t actualSize = ::libSsl2_channel_write(m_SslSocket, writeBlock, blockSize);
			if (actualSize == LIBSsl2_ERROR_EAGAIN)
			{
				canWriteSocket = false;
			}
			else if (actualSize < 0)
			{
				setIoErrorEvent_l(err::Errno((int)actualSize));
				return;
			}
			else if ((size_t)actualSize < blockSize)
			{
				writeBlock.remove(0, actualSize);
			}
			else
			{
				writeBlock.clear();
			}
		}

		if (canWriteSocket && (m_ioThreadFlags & IoFlag_ResizePty))
		{
			ssize_t SslResult = ::libSsl2_channel_request_pty_size(m_SslSocket, m_ptyWidth, m_ptyHeight);
			if (SslResult == LIBSsl2_ERROR_EAGAIN)
			{
				canWriteSocket = false;
			}
			else if (SslResult < 0)
			{
				setIoErrorEvent_l(err::Errno((int)SslResult));
				return;
			}
			else
			{
				m_ioThreadFlags &= ~IoFlag_ResizePty;
			}
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
SslSocket::SslReadWriteLoop()
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
		m_activeEvents = SslSocketEvent_FullyConnected;

		readBlock.setCount(m_readBlockSize); // update read block size

		while (canReadSocket && !m_readBuffer.isFull())
		{
			ssize_t actualSize = ::libSsl2_channel_read(m_SslSocket, readBlock, readBlock.getCount());
			if (actualSize == LIBSsl2_ERROR_EAGAIN)
			{
				canReadSocket = false;
			}
			else if (actualSize < 0)
			{
				setIoErrorEvent_l(err::Errno((int)actualSize));
				return;
			}
			else if (actualSize == 0) // disconnect by remote node
			{
				setEvents_l(SslSocketEvent_TcpDisconnected);
				return;
			}
			else
			{
				addToReadBuffer(readBlock, actualSize);
			}
		}

		while (canWriteSocket)
		{
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			size_t blockSize = writeBlock.getCount();
			ssize_t actualSize = ::libSsl2_channel_write(m_SslSocket, writeBlock, blockSize);
			if (actualSize == LIBSsl2_ERROR_EAGAIN)
			{
				canWriteSocket = false;
			}
			else if (actualSize < 0)
			{
				setIoErrorEvent_l(err::Errno((int)actualSize));
				return;
			}
			else if ((size_t)actualSize < blockSize)
			{
				writeBlock.remove(0, actualSize);
			}
			else
			{
				writeBlock.clear();
			}
		}

		if (canWriteSocket && (m_ioThreadFlags & IoFlag_ResizePty))
		{
			ssize_t SslResult = ::libSsl2_channel_request_pty_size(m_SslSocket, m_ptyWidth, m_ptyHeight);
			if (SslResult == LIBSsl2_ERROR_EAGAIN)
			{
				canWriteSocket = false;
			}
			else if (SslResult < 0)
			{
				setIoErrorEvent_l(err::Errno((int)SslResult));
				return;
			}
			else
			{
				m_ioThreadFlags &= ~IoFlag_ResizePty;
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

#endif

} // namespace io
} // namespace jnc
