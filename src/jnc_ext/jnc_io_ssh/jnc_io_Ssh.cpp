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
#include "jnc_io_SocketOptions.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	SshChannel,
	"io.SshChannel",
	g_sshLibGuid,
	SshLibCacheSlot_SshChannel,
	SshChannel,
	&SshChannel::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (SshChannel)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <SshChannel>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <SshChannel>)

	JNC_MAP_CONST_PROPERTY ("m_address",     &SshChannel::getAddress)
	JNC_MAP_CONST_PROPERTY ("m_peerAddress", &SshChannel::getPeerAddress)

	JNC_MAP_AUTOGET_PROPERTY ("m_readBlockSize",   &SshChannel::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_readBufferSize",  &SshChannel::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_writeBufferSize", &SshChannel::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_options",         &SshChannel::setOptions)

	JNC_MAP_FUNCTION ("open",         &SshChannel::open)
	JNC_MAP_FUNCTION ("close",        &SshChannel::close)
	JNC_MAP_FUNCTION ("connect",      &SshChannel::connect)
	JNC_MAP_FUNCTION ("authenticate", &SshChannel::authenticate)
	JNC_MAP_FUNCTION ("resizePty",    &SshChannel::resizePty)
	JNC_MAP_FUNCTION ("read",         &SshChannel::read)
	JNC_MAP_FUNCTION ("write",        &SshChannel::write)
	JNC_MAP_FUNCTION ("wait",         &SshChannel::wait)
	JNC_MAP_FUNCTION ("cancelWait",   &SshChannel::cancelWait)
	JNC_MAP_FUNCTION ("blockingWait", &SshChannel::blockingWait)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

SshChannel::SshChannel ()
{
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options,

	m_readBuffer.setBufferSize (Def_ReadBufferSize);
	m_writeBuffer.setBufferSize (Def_WriteBufferSize);
}

bool
JNC_CDECL
SshChannel::setOptions (uint_t options)
{
	bool result;

	if (!m_isOpen)
	{
		m_options = options;
		return true;
	}

	if ((options & SocketOption_TcpNagle) ^ (m_options & SocketOption_TcpNagle))
	{
		int value = !(options & SocketOption_TcpNagle);
		result = m_socket.setOption (IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
		if (!result)
		{
			propagateLastError ();
			return false;
		}
	}

	if ((options & SocketOption_TcpReset) ^ (m_options & SocketOption_TcpReset))
	{
		linger value;
		value.l_onoff = (options & SocketOption_TcpReset) != 0;
		value.l_linger = 0;

		result = m_socket.setOption (SOL_SOCKET, SO_LINGER, &value, sizeof (value));
		if (!result)
		{
			propagateLastError ();
			return false;
		}
	}

	m_lock.lock ();
	m_options = options;
	wakeIoThread ();
	m_lock.unlock ();
	return true;
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

	int tcpNoDelayValue = !(m_options & SocketOption_TcpNagle);

	linger lingerValue;
	lingerValue.l_onoff = (m_options & SocketOption_TcpReset) != 0;
	lingerValue.l_linger = 0;

	int reuseAddrValue = (m_options & SocketOption_ReuseAddr) != 0;

	bool result = 
		m_socket.open (AF_INET, SOCK_STREAM, IPPROTO_TCP) &&
		m_socket.setBlockingMode (false) &&
		m_socket.setOption (IPPROTO_TCP, TCP_NODELAY, &tcpNoDelayValue, sizeof (tcpNoDelayValue)) &&
		m_socket.setOption (SOL_SOCKET, SO_LINGER, &lingerValue, sizeof (lingerValue));
		m_socket.setOption (SOL_SOCKET, SO_REUSEADDR, &reuseAddrValue, sizeof (reuseAddrValue));

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

	AsyncIoDevice::open ();
	m_ioThread.start ();
	return true;
}

void
JNC_CDECL
SshChannel::close ()
{
	if (!m_socket.isOpen ())
		return;

	m_lock.lock ();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread ();
	m_lock.unlock ();

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	gcHeap->enterWaitRegion ();
	m_ioThread.waitAndClose ();
	gcHeap->leaveWaitRegion ();

	m_localAddress.m_family = jnc::io::AddressFamily_Undefined;
	m_socket.close ();

	if (m_connectParams)
	{
		AXL_MEM_DELETE (m_connectParams);
		m_connectParams = NULL;
	}

	AsyncIoDevice::close ();
}

bool
JNC_CDECL
SshChannel::connect (
	DataPtr addressPtr,
	DataPtr userNamePtr,
	DataPtr privateKeyPtr,
	size_t privateKeySize,
	DataPtr passwordPtr,
	DataPtr channelTypePtr,
	DataPtr processTypePtr,
	DataPtr ptyTypePtr,
	uint_t ptyWidth,
	uint_t ptyHeight
	)
{
	bool result;

	m_lock.lock ();
	if (m_ioThreadFlags)
	{
		m_lock.unlock ();

		setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	ASSERT (!m_connectParams);
	m_connectParams = AXL_MEM_NEW (ConnectParams);
	m_connectParams->m_userName    = userNamePtr.m_p ? (const char*) userNamePtr.m_p : "anonymous";

	if (privateKeyPtr.m_p && privateKeySize)
		m_connectParams->m_privateKey.copy ((char*) privateKeyPtr.m_p, privateKeySize);

	m_connectParams->m_password    = (const char*) passwordPtr.m_p;
	m_connectParams->m_channelType = channelTypePtr.m_p ? (const char*) channelTypePtr.m_p : "session";
	m_connectParams->m_processType = processTypePtr.m_p ? (const char*) processTypePtr.m_p : "shell";
	m_connectParams->m_ptyType     = ptyTypePtr.m_p ? (const char*) ptyTypePtr.m_p : "ansi";
	m_ptyWidth = ptyWidth;
	m_ptyHeight = ptyHeight;
	m_lock.unlock ();

	m_remoteAddress = *(jnc::io::SocketAddress*) addressPtr.m_p;
	result = m_socket.connect (m_remoteAddress.getSockAddr ());
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	wakeIoThread ();
	return true;
}

bool
JNC_CDECL
SshChannel::authenticate (
	DataPtr userNamePtr,
	DataPtr privateKeyPtr,
	size_t privateKeySize,
	DataPtr passwordPtr
	)
{
	m_lock.lock ();
	if (!(m_activeEvents & SshEvent_SshAuthenticateError))
	{
		m_lock.unlock ();

		setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	m_activeEvents &= ~SshEvent_SshAuthenticateError;

	ASSERT (m_connectParams);
	m_connectParams->m_userName = userNamePtr.m_p ? (const char*) userNamePtr.m_p : "anonymous";
	m_connectParams->m_password = (const char*) passwordPtr.m_p;

	if (privateKeyPtr.m_p && privateKeySize)
		m_connectParams->m_privateKey.copy ((char*) privateKeyPtr.m_p, privateKeySize);

	wakeIoThread ();
	m_lock.unlock ();

	return true;
}

bool
JNC_CDECL
SshChannel::resizePty (
	uint_t width,
	uint_t height
	)
{
	m_lock.lock ();
	m_ptyWidth = width;
	m_ptyHeight = height;
	m_ioThreadFlags |= IoFlag_ResizePty;

	if (m_activeEvents & SshEvent_SshPtyRequestCompleted)
		wakeIoThread ();

	m_lock.unlock ();
	return true;
}

void
SshChannel::ioThreadFunc ()
{
	ASSERT (m_socket.isOpen ());

	sleepIoThread ();

	m_lock.lock ();
	if (m_ioThreadFlags & IoThreadFlag_Closing)
	{
		m_lock.unlock ();
		return;
	}

	m_lock.unlock ();

	bool result = tcpConnectLoop () && sshConnect ();
	if (result)
	{
		wakeIoThread ();
		sshReadWriteLoop ();
	}

	m_sshChannel.close ();
	m_sshSession.close ();
}

err::Error
SshChannel::getLastSshError ()
{
	char* errorString = NULL;
	int length = 0;
	libssh2_session_last_error (m_sshSession, &errorString, &length, false);
	return err::Error (sl::StringRef (errorString, length));
}

int
SshChannel::sshAsyncLoop (int result)
{
	if (result >= 0)
		return result;

	if (result != LIBSSH2_ERROR_EAGAIN)
	{
		setError (getLastSshError ());
		return result;
	}

	if (m_ioThreadFlags & IoThreadFlag_Closing)
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
		setIoErrorEvent (getSshLastError (m_sshSession));
		return false;
	}

	setEvents (SshEvent_SshHandshakeCompleted);

	// loop to give user a chance to re-authenticate

	sl::String prevUserName = m_connectParams->m_userName;

	for (;;)
	{
		do
		{
			result = m_connectParams->m_privateKey.isEmpty () ?
				libssh2_userauth_password (
					m_sshSession,
					m_connectParams->m_userName,
					m_connectParams->m_password
					) :
				libssh2_userauth_publickey_frommemory (
					m_sshSession,
					m_connectParams->m_userName.cp (),
					m_connectParams->m_userName.getLength (),
					NULL,
					0,
					m_connectParams->m_privateKey.cp (),
					m_connectParams->m_privateKey.getCount (),
					m_connectParams->m_password.sz ()
					);

			result = sshAsyncLoop (result);
		} while (result == LIBSSH2_ERROR_EAGAIN);

		if (!result)
			break;

		if (result != LIBSSH2_ERROR_AUTHENTICATION_FAILED &&
			result != LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED)
		{
			setIoErrorEvent (getSshLastError (m_sshSession));
			return false;
		}

		setErrorEvent (SshEvent_SshAuthenticateError, getSshLastError (m_sshSession));
		sleepIoThread ();

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			return false;
		}

		if (m_connectParams->m_userName == prevUserName)
		{
			m_lock.unlock ();
		}
		else // need to re-connect
		{
			prevUserName = m_connectParams->m_userName;
			m_lock.unlock ();

			m_socket.close ();

			result = m_socket.open (AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (!result)
			{
				setIoErrorEvent (err::getLastError ());
				return false;
			}

			if (m_localAddress.m_family)
			{
				result = m_socket.bind (m_localAddress.getSockAddr ());
				if (!result)
				{
					setIoErrorEvent (err::getLastError ());
					return false;
				}
			}

			result = m_socket.connect (m_remoteAddress.getSockAddr ());
			if (!result)
			{
				setIoErrorEvent (err::getLastError ());
				return false;
			}

			result = tcpConnectLoop ();
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
				setIoErrorEvent (getSshLastError (m_sshSession));
				return false;
			}

			setEvents (SshEvent_SshHandshakeCompleted);
		}
	}

	setEvents (SshEvent_SshAuthenticateCompleted);

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
		setIoErrorEvent (getSshLastError (m_sshSession));
		return false;
	}

	m_sshChannel.attach (channel);
	libssh2_channel_set_blocking (m_sshChannel, false);

	setEvents (SshEvent_SshChannelOpened);

	do
	{
		result = libssh2_channel_request_pty_ex (
			m_sshChannel,
			m_connectParams->m_ptyType,
			m_connectParams->m_ptyType.getLength (),
			NULL, 0,
			m_ptyWidth,
			m_ptyHeight,
			0, 0
			);

		result = sshAsyncLoop (result);
	} while (result == LIBSSH2_ERROR_EAGAIN);

	if (result)
	{
		setIoErrorEvent (getSshLastError (m_sshSession));
		return false;
	}

	setEvents (SshEvent_SshPtyRequestCompleted);

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
		setIoErrorEvent (getSshLastError (m_sshSession));
		return false;
	}

	setEvents (SshEvent_SshConnectCompleted);

	AXL_MEM_DELETE (m_connectParams);
	m_connectParams = NULL;
	return true;
}

#if (_JNC_OS_WIN)

bool
SshChannel::tcpConnectLoop ()
{
	sys::Event socketEvent;

	bool result = m_socket.m_socket.wsaEventSelect (socketEvent.m_event, FD_CONNECT);
	if (!result)
	{
		setIoErrorEvent (err::getLastError ());
		return false;
	}

	HANDLE waitTable [] =
	{
		m_ioThreadEvent.m_event,
		socketEvent.m_event,
	};

	for (;;)
	{
		DWORD waitResult = ::WaitForMultipleObjects (countof (waitTable), waitTable, false, INFINITE);
		switch (waitResult)
		{
		case WAIT_FAILED:
			setIoErrorEvent (err::getLastSystemErrorCode ());
			return false;

		case WAIT_OBJECT_0:
			m_lock.lock ();
			if (m_ioThreadFlags & IoThreadFlag_Closing)
			{
				m_lock.unlock ();
				return false;
			}

			m_lock.unlock ();
			break;

		case WAIT_OBJECT_0 + 1:
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents (&networkEvents);
			if (!result)
			{
				setIoErrorEvent ();
				return false;
			}

			if (networkEvents.lNetworkEvents & FD_CONNECT)
			{
				int error = networkEvents.iErrorCode [FD_CONNECT_BIT];
				if (error)
				{
					setIoErrorEvent (error);
					return false;
				}
				else
				{
					setEvents (SshEvent_TcpConnectCompleted);
					return true;
				}
			}

			break;
		}
	}
}

void
SshChannel::sshReadWriteLoop ()
{
	bool result;

	sys::NotificationEvent socketEvent;
	HANDLE waitTable [] =
	{
		m_ioThreadEvent.m_event,
		socketEvent.m_event,
	};

	sl::Array <char> readBlock;
	sl::Array <char> writeBlock;

	bool canReadSocket = false;
	bool canWriteSocket = false;

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
			result = m_socket.m_socket.wsaEventSelect (socketEvent.m_event, socketEventMask);
			if (!result)
			{
				setIoErrorEvent (err::getLastError ());
				return;
			}

			prevSocketEventMask = socketEventMask;
		}

		DWORD waitResult = ::WaitForMultipleObjects (countof (waitTable), waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent (err::getLastSystemErrorCode ());
			return;
		}

		if (socketEvent.wait (0))
		{
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents (&networkEvents);
			if (!result)
			{
				setIoErrorEvent ();
				return;
			}

			if (networkEvents.lNetworkEvents & FD_READ)
			{
				int error = networkEvents.iErrorCode [FD_READ_BIT];
				if (error)
				{
					setIoErrorEvent (error);
					return;
				}

				canReadSocket = true;
			}

			if (networkEvents.lNetworkEvents & FD_WRITE)
			{
				int error = networkEvents.iErrorCode [FD_WRITE_BIT];
				if (error)
				{
					setIoErrorEvent (error);
					return;
				}

				canWriteSocket = true;
			}

			socketEvent.reset ();
		}

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = SshEvent_FullyConnected;

		readBlock.setCount (m_readBlockSize); // update read block size

		while (canReadSocket && !m_readBuffer.isFull ())
		{
			ssize_t actualSize = libssh2_channel_read (m_sshChannel, readBlock, readBlock.getCount ());
			if (actualSize == LIBSSH2_ERROR_EAGAIN)
			{
				actualSize = 0; // retry again after we receive FD_READ
				canReadSocket = false;
			}
			else if (actualSize < 0)
			{
				setIoErrorEvent (err::Errno ((int) actualSize));
				m_lock.unlock ();
				return;
			}
			else if (actualSize == 0) // disconnect by remote node
			{
				setEvents (SshEvent_TcpDisconnected);
				m_lock.unlock ();
				return;
			}
			else
			{
				addToReadBuffer (readBlock, actualSize);
			}
		}

		if (canWriteSocket)
		{
			getNextWriteBlock (&writeBlock);
			if (!writeBlock.isEmpty ())
			{
				size_t blockSize = writeBlock.getCount ();
				ssize_t actualSize = libssh2_channel_write (m_sshChannel, writeBlock, blockSize);
				if (actualSize == LIBSSH2_ERROR_EAGAIN)
				{
					canWriteSocket = false;
				}
				else if (actualSize < 0)
				{
					setIoErrorEvent (err::Errno ((int) actualSize));
					m_lock.unlock ();
					return;
				}
				else if ((size_t) actualSize < blockSize)
				{
					writeBlock.remove (0, actualSize);
				}
				else
				{
					writeBlock.clear ();
				}

				if (canWriteSocket)
					m_ioThreadEvent.signal (); // check again next cycle
			}
		}

		if (canWriteSocket && (m_ioThreadFlags & IoFlag_ResizePty))
		{
			ssize_t sshResult = libssh2_channel_request_pty_size (m_sshChannel, m_ptyWidth, m_ptyHeight);
			if (sshResult == LIBSSH2_ERROR_EAGAIN)
			{
				canWriteSocket = false;
			}
			else if (sshResult < 0)
			{
				setIoErrorEvent (err::Errno ((int) sshResult));
				m_lock.unlock ();
				return;
			}
			else
			{
				m_ioThreadFlags &= ~IoFlag_ResizePty;
			}
		}

		updateReadWriteBufferEvents ();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();
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
			setIoErrorEvent (errno);
			return false;
		}

		if (FD_ISSET (m_selfPipe.m_readFile, &readSet))
		{
			setEvents (SshEvent_ConnectCancelled);
			return false;
		}

		if (FD_ISSET (m_socket.m_socket, &writeSet))
		{
			int error = m_socket.getError ();
			if (error)
			{
				setIoErrorEvent (error);
				return false;
			}
			else
			{
				m_socket.setBlockingMode (true); // turn blocking mode back on
				setEvents (SshEvent_TcpConnectCompleted);
				return true;
			}
		}
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
