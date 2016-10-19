#include "pch.h"
#include "jnc_io_Ssh.h"
#include "jnc_io_SshLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE (
	SshEventParams,
	"io.SshEventParams",
	g_sshLibGuid,
	SshLibCacheSlot_SshEventParams
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (SshEventParams)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	SshChannel,
	"io.SshChannel",
	g_sshLibGuid,
	SshLibCacheSlot_SshChannel,
	SshChannel,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (SshChannel)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <SshChannel>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <SshChannel>)
	JNC_MAP_CONST_PROPERTY ("m_address",     &SshChannel::getAddress)
	JNC_MAP_CONST_PROPERTY ("m_peerAddress", &SshChannel::getPeerAddress)
	JNC_MAP_FUNCTION ("open",         &SshChannel::open)
	JNC_MAP_FUNCTION ("close",        &SshChannel::close)
	JNC_MAP_FUNCTION ("connect",      &SshChannel::connect)
	JNC_MAP_FUNCTION ("authenticate", &SshChannel::authenticate)
	JNC_MAP_FUNCTION ("resizePty",    &SshChannel::resizePty)
	JNC_MAP_FUNCTION ("read",         &SshChannel::read)
	JNC_MAP_FUNCTION ("write",        &SshChannel::write)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

SshChannel::SshChannel ()
{
	m_runtime = getCurrentThreadRuntime ();
	m_connectParams = NULL;
	m_ioFlags = 0;
	m_isOpen = false;
	m_syncId = 0;
}

void
SshChannel::wakeIoThread ()
{
#if (_JNC_OS_WIN)
	m_ioThreadEvent.signal ();
#else
	m_selfPipe.write (" ", 1);
#endif
}

void
SshChannel::sleepIoThread ()
{
#if (_JNC_OS_WIN)
	m_ioThreadEvent.wait ();
#elif (_JNC_OS_POSIX)
	char buffer [256];
	m_selfPipe.read (buffer, sizeof (buffer));
#endif
}

void
SshChannel::fireSshEvent (
	SshEventCode eventCode,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	DataPtr paramsPtr = createData <SshEventParams> (m_runtime);
	SshEventParams* params = (SshEventParams*) paramsPtr.m_p;
	params->m_eventCode = eventCode;
	params->m_syncId = m_syncId;

	if (error)
		params->m_errorPtr = memDup (error, error->m_size);

	callMulticast (m_onSshChannelEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

axl::io::SockAddr
JNC_CDECL
SshChannel::getAddress ()
{
	axl::io::SockAddr sockAddr;
	m_socket.getAddress (&sockAddr);
	return sockAddr;
}

axl::io::SockAddr
JNC_CDECL
SshChannel::getPeerAddress ()
{
	axl::io::SockAddr sockAddr;
	m_socket.getPeerAddress (&sockAddr);
	return sockAddr;
}

bool
JNC_CDECL
SshChannel::open (DataPtr addressPtr)
{
	close ();

	bool result = m_socket.open (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	if (addressPtr.m_p)
	{
		m_localAddress = *(jnc::io::SocketAddress*) addressPtr.m_p;

		result = m_socket.bind (m_localAddress.getSockAddr ());
		if (!result)
		{
			propagateLastError ();
			return false;
		}
	}

	m_isOpen = true;
	m_readBuffer.setCount (4 * 1024); // 4K buffer for reading

#if (_JNC_OS_WIN)
	m_ioThreadEvent.reset ();
#elif (_JNC_OS_POSIX)
	m_selfPipe.create ();
#endif

	m_ioThread.start ();
	return true;
}

void
JNC_CDECL
SshChannel::close ()
{
	if (!m_socket.isOpen ())
		return;

	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	wakeIoThread ();
	m_ioLock.unlock ();

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	gcHeap->enterWaitRegion ();
	m_ioThread.waitAndClose ();
	gcHeap->leaveWaitRegion ();

	m_ioFlags = 0;
	m_localAddress.m_family = jnc::io::AddressFamily_Undefined;
	m_socket.close ();
	m_sshChannel.close ();
	m_sshSession.close ();

	if (m_connectParams)
	{
		AXL_MEM_DELETE (m_connectParams);
		m_connectParams = NULL;
	}

#if (_JNC_OS_POSIX)
	m_selfPipe.close ();
#endif

	m_isOpen = false;
	m_syncId++;
}

bool
JNC_CDECL
SshChannel::connect (
	DataPtr addressPtr,
	DataPtr userNamePtr,
	DataPtr passwordPtr,
	DataPtr channelTypePtr,
	DataPtr processTypePtr,
	DataPtr ptyTypePtr,
	uint_t ptyWidth,
	uint_t ptyHeight,
	bool isSync
	)
{
	bool result;

	m_ioLock.lock ();
	if (m_ioFlags)
	{
		m_ioLock.unlock ();

		setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	ASSERT (!m_connectParams);
	m_connectParams = AXL_MEM_NEW (ConnectParams);
	m_connectParams->m_userName    = userNamePtr.m_p ? (const char*) userNamePtr.m_p : "anonymous";
	m_connectParams->m_password    = (const char*) passwordPtr.m_p;
	m_connectParams->m_channelType = channelTypePtr.m_p ? (const char*) channelTypePtr.m_p : "session";
	m_connectParams->m_processType = processTypePtr.m_p ? (const char*) processTypePtr.m_p : "shell";
	m_connectParams->m_ptyType     = ptyTypePtr.m_p ? (const char*) ptyTypePtr.m_p : "ansi";
	m_connectParams->m_ptyWidth    = ptyWidth;
	m_connectParams->m_ptyHeight   = ptyHeight;

	m_ioFlags |= IoFlag_Connecting;
	wakeIoThread ();
	m_ioLock.unlock ();

	if (!isSync)
	{
		result = m_socket.setBlockingMode (false); // temporarily turn on non-blocking mode
		if (!result)
		{
			propagateLastError ();
			return false;
		}
	}

	m_remoteAddress = *(jnc::io::SocketAddress*) addressPtr.m_p;
	result = m_socket.connect (m_remoteAddress.getSockAddr ());
	if (!result)
		propagateLastError ();

	return result;
}

bool
JNC_CDECL
SshChannel::authenticate (
	DataPtr userNamePtr,
	DataPtr passwordPtr
	)
{
	m_ioLock.lock ();
	if (!(m_ioFlags & IoFlag_AuthError))
	{
		m_ioLock.unlock ();

		setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	ASSERT (m_connectParams);
	m_connectParams->m_userName = userNamePtr.m_p ? (const char*) userNamePtr.m_p : "anonymous";
	m_connectParams->m_password = (const char*) passwordPtr.m_p;
	wakeIoThread ();
	m_ioLock.unlock ();

	return true;
}

size_t
JNC_CDECL
SshChannel::read (
	DataPtr ptr,
	size_t size
	)
{
	if (!m_sshChannel)
	{
		setError (err::SystemErrorCode_InvalidDeviceState);
		return -1;
	}

	m_ioLock.lock ();
	if (!(m_ioFlags & IoFlag_IncomingData))
	{
		Read read;
		read.m_buffer = ptr.m_p;
		read.m_size = size;
		m_readList.insertTail (&read);
		wakeIoThread ();
		m_ioLock.unlock ();

		GcHeap* gcHeap = m_runtime->getGcHeap ();
		gcHeap->enterWaitRegion ();
		read.m_completionEvent.wait ();
		gcHeap->leaveWaitRegion ();

		if (read.m_result == -1)
			setError (read.m_error);

		return read.m_result;
	}
	else if (m_incomingDataSize <= size)
	{
		size_t copySize = m_incomingDataSize;
		memcpy (ptr.m_p, m_readBuffer, copySize);
		m_ioFlags &= ~IoFlag_IncomingData;
		m_incomingDataSize = 0;
		wakeIoThread ();
		m_ioLock.unlock ();

		return copySize;
	}
	else
	{
		memcpy (ptr.m_p, m_readBuffer, size);
		m_readBuffer.remove (0, size);
		m_incomingDataSize -= size;
		m_ioLock.unlock ();

		fireSshEvent (SshEventCode_IncomingData);
		return size;
	}
}

size_t
JNC_CDECL
SshChannel::write (
	DataPtr ptr,
	size_t size
	)
{
	if (!m_sshChannel)
	{
		setError (err::SystemErrorCode_InvalidDeviceState);
		return -1;
	}

	int result;

	do
	{
		result = (int) libssh2_channel_write (m_sshChannel, (const char*) ptr.m_p, size);
		result = sshAsyncLoop (result);
	} while (result == LIBSSH2_ERROR_EAGAIN);

	return result >= 0 ? result : -1;
}

bool
JNC_CDECL
SshChannel::resizePty (
	uint_t width,
	uint_t height,
	bool isSync
	)
{
	if (!m_sshChannel)
	{
		setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	int result;

	if (!isSync)
	{
		result = libssh2_channel_request_pty_size (m_sshChannel, width, height);
		if (result && result != LIBSSH2_ERROR_EAGAIN)
		{
			setError (getLastSshError ());
			return false;
		}
	}
	else
	{
		do
		{
			result = libssh2_channel_request_pty_size (m_sshChannel, width, height);
			result = sshAsyncLoop (result);
		} while (result == LIBSSH2_ERROR_EAGAIN);

		if (result)
			return false;
	}

	return true;
}

void
SshChannel::ioThreadFunc ()
{
	ASSERT (m_socket.isOpen ());

	sleepIoThread ();

	if (m_ioFlags & IoFlag_Closing)
		return;

	if (tcpConnect () && sshConnect ())
	{
		sshReadLoop ();
		fireSshEvent (SshEventCode_Disconnected);
	}
}

err::Error
SshChannel::getLastSshError ()
{
	char* errorString = NULL;
	int length = 0;

	libssh2_session_last_error (m_sshSession, &errorString, &length, false);

	err::Error error;
	error.createStringError (sl::StringRef (errorString, length));
	return error;
}

int
SshChannel::sshAsyncLoop (int result)
{
	if (result >= 0)
		return 0;

	if (result != LIBSSH2_ERROR_EAGAIN)
	{
		setError (getLastSshError ());
		return result;
	}

	if (m_ioFlags & IoFlag_Closing)
	{
		setError (err::Error (err::SystemErrorCode_Cancelled));
		return LIBSSH2_ERROR_CHANNEL_CLOSED;
	}

	sys::sleep (10);
	return LIBSSH2_ERROR_EAGAIN;
}

err::Error
getSshLastError (LIBSSH2_SESSION* sshSession)
{
	char* string;
	int length;
	libssh2_session_last_error (sshSession, &string, &length, false);
	return err::Error (sl::StringRef (string, length));
}

bool
SshChannel::sshConnect ()
{
	int result;

	LIBSSH2_SESSION* sshSession = libssh2_session_init ();

	m_sshSession.attach (sshSession);
	libssh2_session_set_blocking (m_sshSession, false);

	do
	{
		result = libssh2_session_handshake (m_sshSession, m_socket.m_socket);
		result = sshAsyncLoop (result);
	} while (result == LIBSSH2_ERROR_EAGAIN);

	if (result)
	{
		fireSshEvent (SshEventCode_ConnectError, getSshLastError (m_sshSession));
		return false;
	}

	fireSshEvent (SshEventCode_SshHandshakeCompleted);

	// loop to give user a chance to re-authenticate

	sl::String prevUserName = m_connectParams->m_userName;

	for (;;)
	{
		do
		{
			result = libssh2_userauth_password (
				m_sshSession,
				m_connectParams->m_userName,
				m_connectParams->m_password
				);

			result = sshAsyncLoop (result);
		} while (result == LIBSSH2_ERROR_EAGAIN);

		if (!result)
			break;

		if (result != LIBSSH2_ERROR_AUTHENTICATION_FAILED)
		{
			fireSshEvent (SshEventCode_ConnectError, getSshLastError (m_sshSession));
			return false;
		}

		fireSshEvent (SshEventCode_SshAuthError, getSshLastError (m_sshSession));

		m_ioLock.lock ();
		m_ioFlags |= IoFlag_AuthError;
		m_ioLock.unlock ();

		sleepIoThread ();

		m_ioLock.lock ();
		m_ioFlags &= ~IoFlag_AuthError;
		m_ioLock.unlock ();

		if (m_ioFlags & IoFlag_Closing)
			return false;

		if (m_connectParams->m_userName == prevUserName)
		{
			fireSshEvent (SshEventCode_ReauthenticateInitiated);
		}
		else
		{
			// need to re-connect

			prevUserName = m_connectParams->m_userName;
			fireSshEvent (SshEventCode_ReconnectInitiated);

			m_socket.close ();

			result = m_socket.open (AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (!result)
			{
				fireSshEvent (SshEventCode_ConnectError, err::getLastError ());
				return false;
			}

			if (m_localAddress.m_family)
			{
				result = m_socket.bind (m_localAddress.getSockAddr ());
				if (!result)
				{
					fireSshEvent (SshEventCode_ConnectError, err::getLastError ());
					return false;
				}
			}

			result = m_socket.connect (m_remoteAddress.getSockAddr ());
			if (!result)
			{
				fireSshEvent (SshEventCode_ConnectError, err::getLastError ());
				return false;
			}

			result = tcpConnect ();
			if (!result)
				return false;

			sshSession = libssh2_session_init ();

			m_sshSession.attach (sshSession);
			libssh2_session_set_blocking (m_sshSession, false);

			do
			{
				result = libssh2_session_handshake (m_sshSession, m_socket.m_socket);
				result = sshAsyncLoop (result);
			} while (result == LIBSSH2_ERROR_EAGAIN);

			if (result)
			{
				fireSshEvent (SshEventCode_ConnectError, getSshLastError (m_sshSession));
				return false;
			}

			fireSshEvent (SshEventCode_SshHandshakeCompleted);
		}
	}

	fireSshEvent (SshEventCode_SshAuthCompleted);

	LIBSSH2_CHANNEL* channel;

	do
	{
		channel = libssh2_channel_open_ex (
			m_sshSession,
			m_connectParams->m_channelType,
			m_connectParams->m_channelType.getLength (),
			LIBSSH2_CHANNEL_WINDOW_DEFAULT,
			LIBSSH2_CHANNEL_PACKET_DEFAULT,
			NULL, 0
			);

		result = channel ? 0 : sshAsyncLoop (libssh2_session_last_errno (m_sshSession));
	} while (result == LIBSSH2_ERROR_EAGAIN);

	if (!channel)
	{
		fireSshEvent (SshEventCode_ConnectError, getSshLastError (m_sshSession));
		return false;
	}

	m_sshChannel.attach (channel);
	libssh2_channel_set_blocking (m_sshChannel, false);

	fireSshEvent (SshEventCode_SshChannelOpened);

	do
	{
		result = libssh2_channel_request_pty_ex (
			m_sshChannel,
			m_connectParams->m_ptyType,
			m_connectParams->m_ptyType.getLength (),
			NULL, 0,
			m_connectParams->m_ptyWidth,
			m_connectParams->m_ptyHeight,
			0, 0
			);

		result = sshAsyncLoop (result);
	} while (result == LIBSSH2_ERROR_EAGAIN);

	if (result)
	{
		fireSshEvent (SshEventCode_ConnectError, getSshLastError (m_sshSession));
		return false;
	}

	fireSshEvent (SshEventCode_SshPtyRequested);

	do
	{
		result = libssh2_channel_process_startup (
			m_sshChannel,
			m_connectParams->m_processType,
			m_connectParams->m_processType.getLength (),
			NULL, 0
			);

		result = sshAsyncLoop (result);
	} while (result == LIBSSH2_ERROR_EAGAIN);

	if (result)
	{
		fireSshEvent (SshEventCode_ConnectError, getSshLastError (m_sshSession));
		return false;
	}

	fireSshEvent (SshEventCode_SshProcessStarted);
	fireSshEvent (SshEventCode_ConnectCompleted);

	AXL_MEM_DELETE (m_connectParams);
	m_connectParams = NULL;
	return true;
}

void
SshChannel::sshReadLoop ()
{
	for (;;)
	{
		Read* read = NULL;
		char* readBuffer;
		size_t readBufferSize;

		m_ioLock.lock ();

		for (;;)
		{
			if (m_ioFlags & IoFlag_Closing)
			{
				m_ioLock.unlock ();
				return;
			}

			if (!m_readList.isEmpty ())
			{
				read = m_readList.removeHead ();
				readBuffer = (char*) read->m_buffer;
				readBufferSize = read->m_size;
				break;
			}
			else if (!(m_ioFlags & IoFlag_IncomingData))
			{
				ASSERT (m_incomingDataSize == 0);
				readBuffer = m_readBuffer;
				readBufferSize = m_readBuffer.getCount ();
				break;
			}

			m_ioLock.unlock ();
			sleepIoThread ();
			m_ioLock.lock ();
		}

		m_ioLock.unlock ();

		// read

		intptr_t result;

		for (;;)
		{
			if (m_ioFlags & IoFlag_Closing)
				return;

			result = libssh2_channel_read (m_sshChannel, readBuffer, readBufferSize);
			if (result != LIBSSH2_ERROR_EAGAIN)
				break;

			sys::sleep (10);
		}

		if (result < 0)
		{
			if (read)
			{
				read->m_result = -1;
				read->m_error = getSshLastError (m_sshSession);
				read->m_completionEvent.signal ();
			}

			return;
		}

		if (read)
		{
			read->m_result = result;
			read->m_completionEvent.signal ();
		}
		else
		{
			m_ioLock.lock ();
			ASSERT (!(m_ioFlags & IoFlag_IncomingData));
			m_ioFlags |= IoFlag_IncomingData;
			m_incomingDataSize = result;
			m_ioLock.unlock ();

			fireSshEvent (SshEventCode_IncomingData);

			if (result == 0) // EOF
				return;
		}
	}
}

#if (_JNC_OS_WIN)

bool
SshChannel::tcpConnect ()
{
	sys::Event sshChannelEvent;

	bool result = m_socket.m_socket.wsaEventSelect (sshChannelEvent.m_event, FD_CONNECT);
	if (!result)
	{
		fireSshEvent (SshEventCode_ConnectError, err::getLastError ());
		return false;
	}

	HANDLE waitTable [] =
	{
		m_ioThreadEvent.m_event,
		sshChannelEvent.m_event,
	};

	for (;;)
	{
		DWORD waitResult = ::WaitForMultipleObjects (countof (waitTable), waitTable, false, INFINITE);
		switch (waitResult)
		{
		case WAIT_FAILED:
			fireSshEvent (SshEventCode_ConnectError, err::Error (err::getLastSystemErrorCode ()));
			return false;

		case WAIT_OBJECT_0:
			fireSshEvent (SshEventCode_ConnectCancelled);
			return false;

		case WAIT_OBJECT_0 + 1:
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents (&networkEvents);
			if (!result)
			{
				fireSshEvent (SshEventCode_ConnectCancelled);
				return false;
			}

			if (networkEvents.lNetworkEvents & FD_CONNECT)
			{
				int error = networkEvents.iErrorCode [FD_CONNECT_BIT];
				if (error)
				{
					fireSshEvent (SshEventCode_ConnectError, err::Error (error));
					return false;
				}
				else
				{
					m_socket.setBlockingMode (true); // turn blocking mode back on
					fireSshEvent (SshEventCode_TcpConnectCompleted);
					return true;
				}
			}

			break;
		}
	}
}

#else

bool
SshChannel::tcpConnect ()
{
	int result;
	int selectFd = AXL_MAX (m_socket.m_socket, m_selfPipe.m_readFile) + 1;

	// connection loop

	for (;;)
	{
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET (m_selfPipe.m_readFile, &readSet);
		FD_SET (m_socket.m_socket, &writeSet);

		result = select (selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
		{
			fireSshEvent (SshEventCode_ConnectError, err::Error (errno));
			return false;
		}

		if (FD_ISSET (m_selfPipe.m_readFile, &readSet))
		{
			fireSshEvent (SshEventCode_ConnectCancelled);
			return false;
		}

		if (FD_ISSET (m_socket.m_socket, &writeSet))
		{
			int error = m_socket.getError ();
			if (error)
			{
				fireSshEvent (SshEventCode_ConnectError, err::Error (error));
				return false;
			}
			else
			{
				m_socket.setBlockingMode (true); // turn blocking mode back on
				fireSshEvent (SshEventCode_TcpConnectCompleted);
				return true;
			}
		}
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
