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
#include "jnc_io_Ssh.h"
#include "jnc_io_SshLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	SshChannel,
	"io.SshChannel",
	g_sshLibGuid,
	SshLibCacheSlot_SshChannel,
	SshChannel,
	&SshChannel::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(SshChannel)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<SshChannel>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<SshChannel>)

	JNC_MAP_CONST_PROPERTY("m_address",     &SshChannel::getAddress)
	JNC_MAP_CONST_PROPERTY("m_peerAddress", &SshChannel::getPeerAddress)

	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize",   &SshChannel::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize",  &SshChannel::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &SshChannel::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options",         &SshChannel::setOptions)

	JNC_MAP_FUNCTION("open",         &SshChannel::open_0)
	JNC_MAP_OVERLOAD(&SshChannel::open_1)
	JNC_MAP_FUNCTION("close",        &SshChannel::close)
	JNC_MAP_FUNCTION("connect",      &SshChannel::connect_0)
	JNC_MAP_OVERLOAD(&SshChannel::connect_1)
	JNC_MAP_FUNCTION("authenticate", &SshChannel::authenticate_0)
	JNC_MAP_OVERLOAD(&SshChannel::authenticate_1)
	JNC_MAP_OVERLOAD(&SshChannel::authenticate_2)
	JNC_MAP_FUNCTION("resizePty",    &SshChannel::resizePty)
	JNC_MAP_FUNCTION("read",         &SshChannel::read)
	JNC_MAP_FUNCTION("write",        &SshChannel::write)
	JNC_MAP_FUNCTION("wait",         &SshChannel::wait)
	JNC_MAP_FUNCTION("cancelWait",   &SshChannel::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &SshChannel::blockingWait)
	JNC_MAP_FUNCTION("asyncWait",    &SshChannel::asyncWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

SshChannel::SshChannel() {
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options,

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
	m_writeBuffer.setBufferSize(Def_WriteBufferSize);

	m_connectParams = NULL;
	m_ptyWidth = 0;
	m_ptyHeight = 0;
}

bool
JNC_CDECL
SshChannel::open_0(uint16_t family) {
	close();

	bool result =
		requireSshCapability() &&
		SocketBase::open(family, IPPROTO_TCP, NULL);

	if (!result)
		return false;

	m_ioThread.start();
	return true;
}

bool
JNC_CDECL
SshChannel::open_1(DataPtr addressPtr) {
	close();

	SocketAddress* address = (SocketAddress*)addressPtr.m_p;

	bool result =
		requireSshCapability() &&
		SocketBase::open(address ? address->m_family : AF_INET, IPPROTO_TCP, address);

	if (!result)
		return false;

	m_ioThread.start();
	return true;
}

void
JNC_CDECL
SshChannel::close() {
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

	delete m_connectParams;
	m_connectParams = NULL;
	SocketBase::close();
}

bool
JNC_CDECL
SshChannel::connect_0(
	DataPtr addressPtr,
	String userName,
	String password,
	String channelType,
	String processType,
	String ptyType,
	uint_t ptyWidth,
	uint_t ptyHeight
) {
	return connectImpl(
		(const SocketAddress*)addressPtr.m_p,
		userName >> toAxl,
		sl::StringRef(),
		NULL,
		0,
		password >> toAxl,
		channelType >> toAxl,
		NULL,
		0,
		processType >> toAxl,
		NULL,
		0,
		ptyType >> toAxl,
		ptyWidth,
		ptyHeight
	);
}

bool
JNC_CDECL
SshChannel::connect_1(DataPtr paramPtr) {
	SshConnectParams* params = (SshConnectParams*)paramPtr.m_p;

	return connectImpl(
		&params->m_address,
		params->m_userName >> toAxl,
		params->m_privateKeyFileName >> toAxl,
		params->m_privateKeyPtr.m_p,
		params->m_privateKeySize,
		params->m_password >> toAxl,
		params->m_channelType >> toAxl,
		params->m_channelExtraPtr.m_p,
		params->m_channelExtraSize,
		params->m_processType >> toAxl,
		params->m_processExtraPtr.m_p,
		params->m_processExtraSize,
		params->m_ptyType >> toAxl,
		params->m_ptyWidth,
		params->m_ptyHeight
	);
}

bool
SshChannel::connectImpl(
	const SocketAddress* address,
	const sl::StringRef& userName,
	const sl::StringRef& privateKeyFileName,
	const void* privateKey,
	size_t privateKeySize,
	const sl::StringRef& password,
	const sl::StringRef& channelType,
	const void* channelExtra,
	size_t channelExtraSize,
	const sl::StringRef& processType,
	const void* processExtra,
	size_t processExtraSize,
	const sl::StringRef& ptyType,
	uint_t ptyWidth,
	uint_t ptyHeight
) {
	bool result;

	m_lock.lock();
	if (m_ioThreadFlags) {
		m_lock.unlock();

		setError(err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	ASSERT(!m_connectParams);
	m_connectParams = new ConnectParams;
	m_connectParams->m_userName = !userName.isEmpty() ? userName : "anonymous";
	m_connectParams->m_privateKeyFileName = privateKeyFileName;

	if (privateKey && privateKeySize)
		m_connectParams->m_privateKey.copy((char*)privateKey, privateKeySize);

	m_connectParams->m_password = password;
	m_connectParams->m_channelType = !channelType.isEmpty() ? channelType: "session";

	if (channelExtra && channelExtraSize)
		m_connectParams->m_channelExtra.copy((char*)channelExtra, channelExtraSize);

	m_connectParams->m_processType = processType;

	if (processExtra && processExtraSize)
		m_connectParams->m_processExtra.copy((char*)processExtra, processExtraSize);

	m_connectParams->m_ptyType = ptyType;
	m_ptyWidth = ptyWidth;
	m_ptyHeight = ptyHeight;

	if (!m_connectParams->m_ptyType.isEmpty())
		m_ioThreadFlags |= IoFlag_Pty;

	m_lock.unlock();

	m_remoteAddress = *address;
	result = m_socket.connect(m_remoteAddress.getSockAddr());
	if (!result)
		return false;

	wakeIoThread();
	return true;
}

bool
JNC_CDECL
SshChannel::authenticate_0(
	String userName,
	String password
) {
	AuthenticateParams params = { 0 };
	params.m_userName = userName;
	params.m_password = password;
	return authenticateImpl(params);
}

bool
JNC_CDECL
SshChannel::authenticate_1(
	String userName,
	String privateKeyFileName,
	String password
) {
	AuthenticateParams params = { 0 };
	params.m_userName = userName;
	params.m_privateKeyFileName = privateKeyFileName;
	params.m_password = password;
	return authenticateImpl(params);
}

bool
JNC_CDECL
SshChannel::authenticate_2(
	String userName,
	DataPtr privateKeyPtr,
	size_t privateKeySize,
	String password
) {
	AuthenticateParams params = { 0 };
	params.m_userName = userName;
	params.m_privateKeyPtr = privateKeyPtr;
	params.m_privateKeySize = privateKeySize;
	params.m_password = password;
	return authenticateImpl(params);
}

bool
SshChannel::authenticateImpl(const AuthenticateParams& params) {
	m_lock.lock();
	if (!(m_activeEvents & SshEvent_SshAuthenticateError)) {
		m_lock.unlock();

		setError(err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	m_activeEvents &= ~SshEvent_SshAuthenticateError;

	sl::StringRef userName = params.m_userName >> toAxl;

	ASSERT(m_connectParams);
	m_connectParams->m_userName = !userName.isEmpty() ? userName : "anonymous";
	m_connectParams->m_privateKeyFileName = params.m_privateKeyFileName >> toAxl;
	m_connectParams->m_privateKey.copy((char*)params.m_privateKeyPtr.m_p, params.m_privateKeySize);
	m_connectParams->m_password = params.m_password >> toAxl;

	wakeIoThread();
	m_lock.unlock();

	return true;
}

bool
JNC_CDECL
SshChannel::resizePty(
	uint_t width,
	uint_t height
) {
	m_lock.lock();
	m_ptyWidth = width;
	m_ptyHeight = height;
	m_ioThreadFlags |= IoFlag_ResizePty;

	if (m_activeEvents & SshEvent_SshPtyRequestCompleted)
		wakeIoThread();

	m_lock.unlock();
	return true;
}

void
SshChannel::ioThreadFunc() {
	ASSERT(m_socket.isOpen());

	sleepIoThread();

	m_lock.lock();
	if (m_ioThreadFlags & IoThreadFlag_Closing) {
		m_lock.unlock();
		return;
	}

	m_lock.unlock();

	bool result =
		connectLoop(SocketEvent_TcpConnected) &&
		sshConnectLoop();

	if (result) {
		wakeIoThread();
		sshReadWriteLoop();
	}

	m_sshChannel.close();
	m_sshSession.close();
}

err::Error
SshChannel::getLastSshError() {
	char* errorString = NULL;
	int length = 0;
	::libssh2_session_last_error(m_sshSession, &errorString, &length, false);
	return err::Error(sl::StringRef(errorString, length));
}

int
SshChannel::sshAsyncLoop(int result) {
	if (result >= 0)
		return result;

	if (result != LIBSSH2_ERROR_EAGAIN) {
		setError(getLastSshError());
		return result;
	}

	if (m_ioThreadFlags & IoThreadFlag_Closing) {
		setError(err::Error(err::SystemErrorCode_Cancelled));
		return LIBSSH2_ERROR_CHANNEL_CLOSED;
	}

	sys::sleep(10);
	return LIBSSH2_ERROR_EAGAIN;
}

err::Error
getSshLastError(LIBSSH2_SESSION* sshSession) {
	char* string;
	int length;
	::libssh2_session_last_error(sshSession, &string, &length, false);
	return err::Error(sl::StringRef(string, length));
}

bool
SshChannel::sshConnectLoop() {
	int result;

	LIBSSH2_SESSION* sshSession = ::libssh2_session_init();

	m_sshSession.attach(sshSession);
	::libssh2_session_set_blocking(m_sshSession, false);

	do {
		result = ::libssh2_session_handshake(m_sshSession, m_socket.m_socket);
		result = sshAsyncLoop(result);
	} while (result == LIBSSH2_ERROR_EAGAIN);

	if (result) {
		setIoErrorEvent(getSshLastError(m_sshSession));
		return false;
	}

	setEvents(SshEvent_SshHandshakeCompleted);

	// loop to give user a chance to re-authenticate

	sl::String prevUserName = m_connectParams->m_userName;

	for (;;) {
		do {
			result =
				!m_connectParams->m_privateKey.isEmpty() ? ::libssh2_userauth_publickey_frommemory(
					m_sshSession,
					m_connectParams->m_userName.cp(),
					m_connectParams->m_userName.getLength(),
					NULL,
					0,
					m_connectParams->m_privateKey.cp(),
					m_connectParams->m_privateKey.getCount(),
					m_connectParams->m_password.sz()
				) :
				!m_connectParams->m_privateKeyFileName.isEmpty() ? ::libssh2_userauth_publickey_fromfile_ex(
					m_sshSession,
					m_connectParams->m_userName.cp(),
					m_connectParams->m_userName.getLength(),
					NULL,
					m_connectParams->m_privateKeyFileName.sz(),
					m_connectParams->m_password.sz()
				) :
				::libssh2_userauth_password(
					m_sshSession,
					m_connectParams->m_userName,
					m_connectParams->m_password
				);

			result = sshAsyncLoop(result);
		} while (result == LIBSSH2_ERROR_EAGAIN);

		if (!result)
			break;

		setIoErrorEvent(SshEvent_SshAuthenticateError, getSshLastError(m_sshSession));
		sleepIoThread();

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			return false;
		}

		if (m_connectParams->m_userName == prevUserName) {
			m_lock.unlock();
		} else { // need to re-connect
			prevUserName = m_connectParams->m_userName;
			m_lock.unlock();

			result = m_localAddress.m_family ?
				SocketBase::openSocket(m_family, IPPROTO_TCP, &m_localAddress) :
				SocketBase::openSocket(m_family, IPPROTO_TCP, NULL);

			if (!result) {
				setIoErrorEvent(err::getLastError());
				return false;
			}

			result = m_socket.connect(m_remoteAddress.getSockAddr());
			if (!result) {
				setIoErrorEvent(err::getLastError());
				return false;
			}

			result = connectLoop(SocketEvent_TcpConnected);
			if (!result)
				return false;

			sshSession = ::libssh2_session_init();

			m_sshSession.attach(sshSession);
			::libssh2_session_set_blocking(m_sshSession, false);

			do {
				result = ::libssh2_session_handshake(m_sshSession, m_socket.m_socket);
				result = sshAsyncLoop(result);
			} while (result == LIBSSH2_ERROR_EAGAIN);

			if (result) {
				setIoErrorEvent(getSshLastError(m_sshSession));
				return false;
			}

			setEvents(SshEvent_SshHandshakeCompleted);
		}
	}

	setEvents(SshEvent_SshAuthenticateCompleted);

	LIBSSH2_CHANNEL* channel;

	do {
		channel = ::libssh2_channel_open_ex(
			m_sshSession,
			m_connectParams->m_channelType,
			m_connectParams->m_channelType.getLength(),
			LIBSSH2_CHANNEL_WINDOW_DEFAULT,
			LIBSSH2_CHANNEL_PACKET_DEFAULT,
			m_connectParams->m_channelExtra,
			m_connectParams->m_channelExtra.getCount()
		);

		result = channel ? 0 : sshAsyncLoop(::libssh2_session_last_errno(m_sshSession));
	} while (result == LIBSSH2_ERROR_EAGAIN);

	if (!channel) {
		setIoErrorEvent(getSshLastError(m_sshSession));
		return false;
	}

	m_sshChannel.attach(channel);
	::libssh2_channel_set_blocking(m_sshChannel, false);

	setEvents(SshEvent_SshChannelOpened);

	if (!m_connectParams->m_ptyType.isEmpty()) {
		do {
			result = ::libssh2_channel_request_pty_ex(
				m_sshChannel,
				m_connectParams->m_ptyType,
				m_connectParams->m_ptyType.getLength(),
				NULL, 0,
				m_ptyWidth,
				m_ptyHeight,
				0, 0
			);

			result = sshAsyncLoop(result);
		} while (result == LIBSSH2_ERROR_EAGAIN);

		if (result) {
			setIoErrorEvent(getSshLastError(m_sshSession));
			return false;
		}
	}

	setEvents(SshEvent_SshPtyRequestCompleted);

	if (!m_connectParams->m_processType.isEmpty()) {
		do {
			result = ::libssh2_channel_process_startup(
				m_sshChannel,
				m_connectParams->m_processType,
				m_connectParams->m_processType.getLength(),
				m_connectParams->m_processExtra,
				m_connectParams->m_processExtra.getCount()
			);

			result = sshAsyncLoop(result);
		} while (result == LIBSSH2_ERROR_EAGAIN);

		if (result) {
			setIoErrorEvent(getSshLastError(m_sshSession));
			return false;
		}
	}

	setEvents(SshEvent_SshConnectCompleted);

	delete m_connectParams;
	m_connectParams = NULL;
	return true;
}

#if (_JNC_OS_WIN)

void
SshChannel::sshReadWriteLoop() {
	bool result;

	sys::NotificationEvent socketEvent;
	HANDLE waitTable[] = {
		m_ioThreadEvent.m_event,
		socketEvent.m_event,
	};

	sl::Array<char> readBlock;
	sl::Array<char> writeBlock;

	m_lock.lock();
	bool hasPty = (m_ioThreadFlags & IoFlag_Pty) != 0;
	m_lock.unlock();

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
				int error = networkEvents.iErrorCode[FD_CLOSE_BIT];
				if (!error)
					setEvents(SocketEvent_TcpDisconnected);
				else if (error == WSAECONNRESET)
					setEvents(SocketEvent_TcpDisconnected | SocketEvent_TcpReset);
				else
					setIoErrorEvent(error);

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
		m_activeEvents = SshEvent_FullyConnected;

		readBlock.setCount(m_readBlockSize); // update read block size
		char* p = readBlock.p();

		while (canReadSocket && !m_readBuffer.isFull()) {
			ssize_t actualSize;

			if (hasPty) {
				actualSize = ::libssh2_channel_read(m_sshChannel, p, readBlock.getCount());
			} else { // need to read stderr, too
				actualSize = ::libssh2_channel_read_stderr(m_sshChannel, p, readBlock.getCount());
				if (actualSize == LIBSSH2_ERROR_EAGAIN)
					actualSize = ::libssh2_channel_read(m_sshChannel, p, readBlock.getCount());
			}

			if (actualSize == LIBSSH2_ERROR_EAGAIN) {
				canReadSocket = false;
			} else if (actualSize < 0) {
				setIoErrorEvent_l(err::Errno((int)actualSize));
				return;
			} else if (actualSize == 0) { // disconnect by remote node
				setEvents_l(SocketEvent_TcpDisconnected);
				return;
			} else {
				addToReadBuffer(readBlock, actualSize);
			}
		}

		while (canWriteSocket) {
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			size_t blockSize = writeBlock.getCount();
			ssize_t actualSize = ::libssh2_channel_write(m_sshChannel, writeBlock, blockSize);
			if (actualSize == LIBSSH2_ERROR_EAGAIN) {
				canWriteSocket = false;
			} else if (actualSize < 0) {
				setIoErrorEvent_l(err::Errno((int)actualSize));
				return;
			} else if ((size_t)actualSize < blockSize) {
				writeBlock.remove(0, actualSize);
			} else {
				writeBlock.clear();
			}
		}

		if (canWriteSocket && (m_ioThreadFlags & IoFlag_ResizePty)) {
			ssize_t sshResult = ::libssh2_channel_request_pty_size(m_sshChannel, m_ptyWidth, m_ptyHeight);
			if (sshResult == LIBSSH2_ERROR_EAGAIN) {
				canWriteSocket = false;
			} else if (sshResult < 0) {
				setIoErrorEvent_l(err::Errno((int)sshResult));
				return;
			} else {
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
SshChannel::sshReadWriteLoop() {
	int result;
	int selectFd = AXL_MAX(m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	sl::Array<char> readBlock;
	sl::Array<char> writeBlock;

	m_lock.lock();
	bool hasPty = (m_ioThreadFlags & IoFlag_Pty) != 0;
	m_lock.unlock();

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
		m_activeEvents = SshEvent_FullyConnected;

		readBlock.setCount(m_readBlockSize); // update read block size

		while (canReadSocket && !m_readBuffer.isFull()) {
			ssize_t actualSize;

			if (hasPty) {
				actualSize = ::libssh2_channel_read(m_sshChannel, readBlock, readBlock.getCount());
			} else { // need to read stderr, too
				actualSize = ::libssh2_channel_read_stderr(m_sshChannel, readBlock, readBlock.getCount());
				if (actualSize == LIBSSH2_ERROR_EAGAIN)
					actualSize = ::libssh2_channel_read(m_sshChannel, readBlock, readBlock.getCount());
			}

			if (actualSize == LIBSSH2_ERROR_EAGAIN) {
				canReadSocket = false;
			} else if (actualSize < 0) {
				setIoErrorEvent_l(err::Errno((int)actualSize));
				return;
			} else if (actualSize == 0) { // disconnect by remote node
				setEvents_l(SocketEvent_TcpDisconnected);
				return;
			} else {
				addToReadBuffer(readBlock, actualSize);
			}
		}

		while (canWriteSocket) {
			getNextWriteBlock(&writeBlock);
			if (writeBlock.isEmpty())
				break;

			size_t blockSize = writeBlock.getCount();
			ssize_t actualSize = ::libssh2_channel_write(m_sshChannel, writeBlock, blockSize);
			if (actualSize == LIBSSH2_ERROR_EAGAIN) {
				canWriteSocket = false;
			} else if (actualSize < 0) {
				setIoErrorEvent_l(err::Errno((int)actualSize));
				return;
			} else if ((size_t)actualSize < blockSize) {
				writeBlock.remove(0, actualSize);
			} else {
				writeBlock.clear();
			}
		}

		if (canWriteSocket && (m_ioThreadFlags & IoFlag_ResizePty)) {
			ssize_t sshResult = ::libssh2_channel_request_pty_size(m_sshChannel, m_ptyWidth, m_ptyHeight);
			if (sshResult == LIBSSH2_ERROR_EAGAIN) {
				canWriteSocket = false;
			} else if (sshResult < 0) {
				setIoErrorEvent_l(err::Errno((int)sshResult));
				return;
			} else {
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

} // namespace io
} // namespace jnc
