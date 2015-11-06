#include "pch.h"
#include "jnc_io_Ssh.h"

namespace jnc {
namespace io {

//.............................................................................

SshChannel::SshChannel ()
{
	m_runtime = rt::getCurrentThreadRuntime ();
	m_connectParams = NULL;
	m_ioFlags = 0;
	m_isOpen = false;
	m_syncId = 0;
}

void
SshChannel::wakeIoThread ()
{
#if (_AXL_ENV == AXL_ENV_WIN)
	m_ioThreadEvent.signal ();
#else
	m_selfPipe.write (" ", 1);
#endif
}

void
SshChannel::sleepIoThread ()
{
#if (_AXL_ENV == AXL_ENV_WIN)
	m_ioThreadEvent.wait ();
#elif (_AXL_ENV == AXL_ENV_POSIX)
	char buffer [256];
	m_selfPipe.read (buffer, sizeof (buffer));
#endif
}

void
SshChannel::fireSshEvent (
	SshEventKind eventKind,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	rt::DataPtr paramsPtr = rt::createData <SshEventParams> (m_runtime);
	SshEventParams* params = (SshEventParams*) paramsPtr.m_p;
	params->m_eventKind = eventKind;
	params->m_syncId = m_syncId;

	if (error)
		params->m_errorPtr = rt::memDup (error, error->m_size);
	
	rt::callMulticast (m_onSshChannelEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

axl::io::SockAddr
AXL_CDECL
SshChannel::getAddress ()
{
	axl::io::SockAddr sockAddr;
	m_socket.getAddress (&sockAddr);
	return sockAddr;
}

axl::io::SockAddr
AXL_CDECL
SshChannel::getPeerAddress ()
{
	axl::io::SockAddr sockAddr;
	m_socket.getPeerAddress (&sockAddr);
	return sockAddr;
}

bool
AXL_CDECL
SshChannel::open (rt::DataPtr addressPtr)
{
	close ();

	bool result =
		m_socket.open (AF_INET, SOCK_STREAM, IPPROTO_TCP) &&
		!addressPtr.m_p || m_socket.bind ((sockaddr*) addressPtr.m_p);

	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	m_isOpen = true;
	m_readBuffer.setCount (4 * 1024); // 4K buffer for reading

#if (_AXL_ENV == AXL_ENV_WIN)
	m_ioThreadEvent.reset ();
#elif (_AXL_ENV == AXL_ENV_POSIX)
	m_selfPipe.create ();
#endif

	m_ioThread.start ();
	return true;
}

void
AXL_CDECL
SshChannel::close ()
{
	if (!m_socket.isOpen ())
		return;

	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	wakeIoThread ();
	m_ioLock.unlock ();

	rt::enterWaitRegion (m_runtime);
	m_ioThread.waitAndClose ();
	rt::leaveWaitRegion (m_runtime);


	m_ioFlags = 0;
	m_socket.close ();
	m_sshChannel.close ();
	m_sshSession.close ();

	if (m_connectParams)
	{
		AXL_MEM_DELETE (m_connectParams);
		m_connectParams = NULL;
	}

#if (_AXL_ENV == AXL_ENV_POSIX)
	m_selfPipe.close ();
#endif

	m_isOpen = false;
	m_syncId++;
}

bool
AXL_CDECL
SshChannel::connect (
	rt::DataPtr addressPtr,
	rt::DataPtr userNamePtr,
	rt::DataPtr passwordPtr,
	rt::DataPtr channelTypePtr,
	rt::DataPtr processTypePtr,
	rt::DataPtr ptyTypePtr,
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

		ext::setError (err::SystemErrorCode_InvalidDeviceState);
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
			ext::propagateLastError ();
			return false;
		}
	}

	result = m_socket.connect ((sockaddr*) addressPtr.m_p);
	if (!result)
		ext::propagateLastError ();

	return result;
}

bool
AXL_CDECL
SshChannel::authenticate (
	rt::DataPtr userNamePtr,
	rt::DataPtr passwordPtr
	)
{
	m_ioLock.lock ();
	if (!(m_ioFlags & IoFlag_AuthError))
	{
		m_ioLock.unlock ();

		ext::setError (err::SystemErrorCode_InvalidDeviceState);
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
AXL_CDECL
SshChannel::read (
	rt::DataPtr ptr,
	size_t size
	)
{
	if (!m_sshChannel)
	{
		ext::setError (err::SystemErrorCode_InvalidDeviceState);
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

		rt::enterWaitRegion (m_runtime);
		read.m_completionEvent.wait ();
		rt::leaveWaitRegion (m_runtime);

		if (read.m_result == -1)
			ext::setError (read.m_error);

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

		fireSshEvent (SshEventKind_IncomingData);
		return size;
	}
}

size_t
AXL_CDECL
SshChannel::write (
	rt::DataPtr ptr,
	size_t size
	)
{
	if (!m_sshChannel)
	{
		ext::setError (err::SystemErrorCode_InvalidDeviceState);
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
AXL_CDECL
SshChannel::resizePty (
	uint_t width,
	uint_t height,
	bool isSync
	)
{
	if (!m_sshChannel)
	{
		ext::setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}
	
	int result;

	if (!isSync)
	{
		result = libssh2_channel_request_pty_size (m_sshChannel, width, height);
		if (result && result != LIBSSH2_ERROR_EAGAIN)
		{
			ext::setError (getLastSshError ());
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
		fireSshEvent (SshEventKind_Disconnected);
	}
}

err::Error
SshChannel::getLastSshError ()
{
	char* errorString = NULL;
	int length = 0;

	libssh2_session_last_error (m_sshSession, &errorString, &length, false);

	err::Error error;
	error.createStringError (errorString, length);
	return error;
}

int
SshChannel::sshAsyncLoop (int result)
{
	if (result >= 0)
		return 0;

	if (result != LIBSSH2_ERROR_EAGAIN)
	{
		ext::setError (getLastSshError ());
		return result;	
	}

	if (m_ioFlags & IoFlag_Closing)
	{
		ext::setError (err::Error (err::SystemErrorCode_Cancelled));
		return LIBSSH2_ERROR_CHANNEL_CLOSED;
	}

	g::sleep (10);
	return LIBSSH2_ERROR_EAGAIN;
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
		fireSshEvent (SshEventKind_ConnectError, err::getLastError ());
		return false;
	}

	fireSshEvent (SshEventKind_SshHandshakeCompleted);

	// loop to give user a chance to re-authenticate

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
			fireSshEvent (SshEventKind_ConnectError, err::getLastError ());
			return false;
		}

		fireSshEvent (SshEventKind_SshAuthError, err::getLastError ());

		m_ioLock.lock ();
		m_ioFlags |= IoFlag_AuthError;
		m_ioLock.unlock ();

		sleepIoThread ();

		if (m_ioFlags & IoFlag_Closing)
			return false;
	}

	fireSshEvent (SshEventKind_SshAuthCompleted);

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
		fireSshEvent (SshEventKind_ConnectError, err::getLastError ());
		return false;
	}

	m_sshChannel.attach (channel);
	libssh2_channel_set_blocking (m_sshChannel, false);

	fireSshEvent (SshEventKind_SshChannelOpened);

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
		fireSshEvent (SshEventKind_ConnectError, err::getLastError ());
		return false;
	}

	fireSshEvent (SshEventKind_SshPtyRequested);

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
		fireSshEvent (SshEventKind_ConnectError, err::getLastError ());
		return false;
	}

	fireSshEvent (SshEventKind_SshProcessStarted);
	fireSshEvent (SshEventKind_ConnectCompleted);

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

			g::sleep (10);
		}

		if (result < 0)
		{
			if (read)
			{
				read->m_result = -1;
				read->m_error = err::getLastError ();
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

			fireSshEvent (SshEventKind_IncomingData);

			if (result == 0) // EOF
				return;
		}
	}
}

#if (_AXL_ENV == AXL_ENV_WIN)

bool
SshChannel::tcpConnect ()
{
	mt::Event sshChannelEvent;

	bool result = m_socket.m_socket.wsaEventSelect (sshChannelEvent.m_event, FD_CONNECT);
	if (!result)
	{
		fireSshEvent (SshEventKind_ConnectError, err::getLastError ());
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
			fireSshEvent (SshEventKind_ConnectError, err::getLastError ());
			return false;

		case WAIT_OBJECT_0:
			fireSshEvent (SshEventKind_ConnectCancelled);
			return false;

		case WAIT_OBJECT_0 + 1:
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents (&networkEvents);
			if (!result)
			{
				fireSshEvent (SshEventKind_ConnectCancelled);
				return false;
			}

			if (networkEvents.lNetworkEvents & FD_CONNECT)
			{
				int error = networkEvents.iErrorCode [FD_CONNECT_BIT];
				if (error)
				{
					fireSshEvent (SshEventKind_ConnectError, err::Error (error));
					return false;
				}
				else
				{
					m_socket.setBlockingMode (true); // turn blocking mode back on
					fireSshEvent (SshEventKind_TcpConnectCompleted);
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
			fireSshEvent (SshEventKind_ConnectError, err::Error (errno));
			return false;
		}

		if (FD_ISSET (m_selfPipe.m_readFile, &readSet))
		{
			fireSshEvent (SshEventKind_ConnectCancelled);
			return false;
		}

		if (FD_ISSET (m_socket.m_socket, &writeSet))
		{
			int error = m_socket.getError ();
			if (error)
			{
				fireSshEvent (SshEventKind_ConnectError, err::Error (error));
				return false;
			}
			else
			{
				m_socket.setBlockingMode (true); // turn blocking mode back on
				fireSshEvent (SshEventKind_TcpConnectCompleted);
				return true;
			}
		}
	}
}

#endif

//.............................................................................

} // namespace io
} // namespace jnc
