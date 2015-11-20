#include "pch.h"
#include "jnc_io_Socket.h"

namespace jnc {
namespace io {

//.............................................................................

Socket::Socket ()
{
	m_runtime = rt::getCurrentThreadRuntime ();
	m_ioFlags = 0;
	m_isOpen = false;
	m_syncId = 0;
}

void
Socket::wakeIoThread ()
{
#if (_AXL_ENV == AXL_ENV_WIN)
	m_ioThreadEvent.signal ();
#else
	m_selfPipe.write (" ", 1);
#endif
}

void
Socket::fireSocketEvent (
	SocketEventKind eventKind,
	uint_t flags,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	jnc::rt::DataPtr paramsPtr = jnc::rt::createData <SocketEventParams> (m_runtime);
	SocketEventParams* params = (SocketEventParams*) paramsPtr.m_p;
	params->m_eventKind = eventKind;
	params->m_syncId = m_syncId;
	params->m_flags = flags;

	if (error)
		params->m_errorPtr = jnc::rt::memDup (error, error->m_size);

	jnc::rt::callMulticast (m_onSocketEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

void
AXL_CDECL
Socket::firePendingEvents ()
{
}

SocketAddress
AXL_CDECL
Socket::getAddress (Socket* self)
{
	axl::io::SockAddr sockAddr;
	self->m_socket.getAddress (&sockAddr);
	return SocketAddress::fromSockAddr (sockAddr);
}

SocketAddress
AXL_CDECL
Socket::getPeerAddress (Socket* self)
{
	axl::io::SockAddr sockAddr;
	self->m_socket.getPeerAddress (&sockAddr);
	return SocketAddress::fromSockAddr (sockAddr);
}

bool
AXL_CDECL
Socket::isBroadcastEnabled ()
{
	int value = 0;
	bool result = m_socket.getOption (SOL_SOCKET, SO_BROADCAST, &value, sizeof (value));
	return result && value != 0;
}

bool
AXL_CDECL
Socket::setBroadcastEnabled (bool isEnabled)
{
	int value = isEnabled;
	bool result = m_socket.setOption (SOL_SOCKET, SO_BROADCAST, &value, sizeof (value));
	if (!result)
		ext::propagateLastError ();

	return result;
}

bool
AXL_CDECL
Socket::isNagleEnabled ()
{
	int value = 0;
	bool result = m_socket.getOption (IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
	return result && value == 0;
}

bool
AXL_CDECL
Socket::setNagleEnabled (bool isEnabled)
{
	int value = !isEnabled;
	bool result = m_socket.setOption (IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
	if (!result)
		ext::propagateLastError ();

	return result;
}

SocketCloseKind
AXL_CDECL
Socket::getCloseKind ()
{
	linger value = { 0 };
	bool result = m_socket.getOption (SOL_SOCKET, SO_LINGER, &value, sizeof (value));
	return result ? value.l_onoff ?
		SocketCloseKind_Reset :
		SocketCloseKind_Graceful :
		SocketCloseKind_Graceful;
}

bool
AXL_CDECL
Socket::setCloseKind (SocketCloseKind closeKind)
{
	linger value;
	value.l_onoff = closeKind == SocketCloseKind_Reset;
	value.l_linger = 0;
	bool result = m_socket.setOption (SOL_SOCKET, SO_LINGER, &value, sizeof (value));
	if (!result)
		ext::propagateLastError ();

	return result;
}

bool
Socket::open (
	int protocol,
	uint16_t family_jnc,
	const SocketAddress* address
	)
{
	close ();

	int family_s = family_jnc == AddressFamily_Ip6 ? AF_INET6 : family_jnc;
	int socketKind = protocol == IPPROTO_TCP ? SOCK_STREAM : SOCK_DGRAM;

	bool result = m_socket.open (family_s, socketKind, protocol);
	if (!result)
	{
		ext::propagateLastError ();
		return false;
	}

	if (address)
	{
		result = m_socket.bind (address->getSockAddr ());
		if (!result)
		{
			ext::propagateLastError ();
			return false;
		}
	}

	m_isOpen = true;

#if (_AXL_ENV == AXL_ENV_WIN)
	m_ioThreadEvent.reset ();
#elif (_AXL_ENV == AXL_ENV_POSIX)
	m_selfPipe.create ();
#endif

	if (protocol == IPPROTO_UDP)
	{
		m_ioFlags |= IoFlag_Udp;
		setBroadcastEnabled (true);
		wakeIoThread (); // it's ok to wake before start
	}

	m_ioThread.start ();
	return true;
}

void
AXL_CDECL
Socket::close ()
{
	if (!m_socket.isOpen ())
		return;

	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	wakeIoThread ();
	m_ioLock.unlock ();

	jnc::rt::enterWaitRegion (m_runtime);
	m_ioThread.waitAndClose ();
	jnc::rt::leaveWaitRegion (m_runtime);

	m_ioFlags = 0;
	m_socket.close ();

#if (_AXL_ENV == AXL_ENV_POSIX)
	m_selfPipe.close ();
#endif

	m_isOpen = false;
	m_syncId++;
}

bool
AXL_CDECL
Socket::connect (
	jnc::rt::DataPtr addressPtr,
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

	SocketAddress* address = (SocketAddress*) addressPtr.m_p;
	result = m_socket.connect (address->getSockAddr ());
	if (!result)
		ext::propagateLastError ();

	return result;
}

bool
AXL_CDECL
Socket::listen (size_t backLog)
{
	bool result;

	m_ioLock.lock ();
	if (m_ioFlags)
	{
		m_ioLock.unlock ();

		ext::setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	m_ioFlags |= IoFlag_Listening;
	wakeIoThread ();
	m_ioLock.unlock ();

	result = m_socket.listen (backLog);
	if (!result)
		ext::propagateLastError ();

	return result;
}

Socket*
AXL_CDECL
Socket::accept (jnc::rt::DataPtr addressPtr)
{
	Socket* connectionSocket = (Socket*) jnc::rt::allocateClass (m_runtime, (jnc::rt::ClassType*) m_box->m_type);
	sl::construct (connectionSocket);

	axl::io::SockAddr sockAddr;
	bool result = m_socket.accept (&connectionSocket->m_socket, &sockAddr);

#if (_AXL_ENV == AXL_ENV_POSIX)
	m_ioLock.lock ();
	m_ioFlags &= ~IoFlag_IncomingConnection;
	wakeIoThread ();
	m_ioLock.unlock ();
#endif

	if (!result)
	{
		ext::propagateLastError ();

		AXL_MEM_DELETE (connectionSocket);		
		return NULL;
	}

	connectionSocket->m_ioFlags = IoFlag_Connected;
	connectionSocket->setCloseKind (getCloseKind ());
	connectionSocket->setNagleEnabled (isNagleEnabled ());

#if (_AXL_ENV == AXL_ENV_POSIX)
	connectionSocket->m_selfPipe.create ();
#endif

	connectionSocket->m_ioThread.start ();
	connectionSocket->wakeIoThread ();

	if (addressPtr.m_p)
		((SocketAddress*) addressPtr.m_p)->setSockAddr (sockAddr);

	return connectionSocket;
}

size_t
AXL_CDECL
Socket::send (
	jnc::rt::DataPtr ptr,
	size_t size
	)
{
	size_t result = m_socket.send (ptr.m_p, size);

	if (result == -1)
		ext::propagateLastError ();

	return result;
}

size_t
AXL_CDECL
Socket::recv (
	jnc::rt::DataPtr ptr,
	size_t size
	)
{
	size_t result = m_socket.recv (ptr.m_p, size);

#if (_AXL_ENV == AXL_ENV_POSIX)
	m_ioLock.lock ();
	m_ioFlags &= ~IoFlag_IncomingData;
	wakeIoThread ();
	m_ioLock.unlock ();
#endif

	if (result == -1)
		ext::propagateLastError ();

	return result;
}

size_t
AXL_CDECL
Socket::sendTo (
	jnc::rt::DataPtr ptr,
	size_t size,
	jnc::rt::DataPtr addressPtr
	)
{
	axl::io::SockAddr sockAddr = ((const SocketAddress*) addressPtr.m_p)->getSockAddr ();
	
	size_t result = m_socket.sendTo (ptr.m_p, size, sockAddr);
	if (result == -1)
		ext::propagateLastError ();

	return result;
}

size_t
AXL_CDECL
Socket::recvFrom (
	jnc::rt::DataPtr ptr,
	size_t size,
	jnc::rt::DataPtr addressPtr
	)
{
	axl::io::SockAddr sockAddr;
	size_t result = m_socket.recvFrom (ptr.m_p, size, &sockAddr);

#if (_AXL_ENV == AXL_ENV_POSIX)
	m_ioLock.lock ();
	m_ioFlags &= ~IoFlag_IncomingData;
	wakeIoThread ();
	m_ioLock.unlock ();
#endif

	if (result == -1)
		ext::propagateLastError ();

	if (addressPtr.m_p)
		((SocketAddress*) addressPtr.m_p)->setSockAddr (sockAddr);

	return result;
}

void
Socket::ioThreadFunc ()
{
	ASSERT (m_socket.isOpen ());

#if (_AXL_ENV == AXL_ENV_WIN)
	m_ioThreadEvent.wait ();
#elif (_AXL_ENV == AXL_ENV_POSIX)
	char buffer [256];
	m_selfPipe.read (buffer, sizeof (buffer));
#endif

	uint_t ioFlags = m_ioFlags;
	if (ioFlags & IoFlag_Closing)
	{
		return;
	}
	else if (ioFlags & IoFlag_Udp)
	{
		recvLoop ();
	}
	else if (ioFlags & IoFlag_Connected)
	{
		bool result = recvLoop ();
		uint_t flags = result ? 0 : SocketDisconnectEventFlag_Reset;
		fireSocketEvent (SocketEventKind_Disconnected, flags);
	}
	else if (ioFlags & IoFlag_Listening)
	{
		acceptLoop ();
	}
	else if (ioFlags & IoFlag_Connecting)
	{
		if (connectLoop ())
		{
			bool result = recvLoop ();
			uint_t flags = result ? 0 : SocketDisconnectEventFlag_Reset;
			fireSocketEvent (SocketEventKind_Disconnected, flags);
		}
	}
}

#if (_AXL_ENV == AXL_ENV_WIN)

bool
Socket::connectLoop ()
{
	mt::Event socketEvent;

	bool result = m_socket.m_socket.wsaEventSelect (socketEvent.m_event, FD_CONNECT);
	if (!result)
	{
		fireSocketEvent (SocketEventKind_ConnectError, 0, err::getLastError ());
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
			fireSocketEvent (SocketEventKind_ConnectError, 0, err::getLastError ());
			return false;

		case WAIT_OBJECT_0:
			fireSocketEvent (SocketEventKind_ConnectCancelled);
			return false;

		case WAIT_OBJECT_0 + 1:
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents (&networkEvents);
			if (!result)
			{
				fireSocketEvent (SocketEventKind_ConnectCancelled);
				return false;
			}

			if (networkEvents.lNetworkEvents & FD_CONNECT)
			{
				int error = networkEvents.iErrorCode [FD_CONNECT_BIT];
				if (error)
				{
					fireSocketEvent (SocketEventKind_ConnectError, 0, err::Error (error));
					return false;
				}
				else
				{
					m_socket.setBlockingMode (true); // turn blocking mode back on
					fireSocketEvent (SocketEventKind_ConnectCompleted);
					return true;
				}
			}

			break;
		}
	}
}

void
Socket::acceptLoop ()
{
	mt::Event socketEvent;

	bool result = m_socket.m_socket.wsaEventSelect (socketEvent.m_event, FD_ACCEPT);
	if (!result)
		return;

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
		case WAIT_OBJECT_0:
			return;

		case WAIT_OBJECT_0 + 1:
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents (&networkEvents);
			if (!result)
				return;

			if (networkEvents.lNetworkEvents & FD_ACCEPT)
			{
				int error = networkEvents.iErrorCode [FD_ACCEPT_BIT];
				if (!error)
					fireSocketEvent (SocketEventKind_IncomingConnection);
			}

			break;
		}
	}
}

bool
Socket::recvLoop ()
{
	mt::Event socketEvent;

	bool result = m_socket.m_socket.wsaEventSelect (socketEvent.m_event, FD_READ | FD_CLOSE);
	if (!result)
		return false;

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
			return false;

		case WAIT_OBJECT_0:
			return true;

		case WAIT_OBJECT_0 + 1:
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents (&networkEvents);
			if (!result)
				return false;

			if (networkEvents.lNetworkEvents & FD_CLOSE)
			{
				int error = networkEvents.iErrorCode [FD_CLOSE_BIT];
				return error == 0;
			}

			if (networkEvents.lNetworkEvents & FD_READ)
			{
				int error = networkEvents.iErrorCode [FD_READ_BIT];
				if (error)
					return false;

				size_t incomingDataSize = m_socket.getIncomingDataSize ();
				if (incomingDataSize == -1)
					return false;

				if (incomingDataSize != 0)
					fireSocketEvent (SocketEventKind_IncomingData);
			}

			break;
		}
	}
}

#else

bool
Socket::connectLoop ()
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
			fireSocketEvent (SocketEventKind_ConnectError, 0, err::Error (errno));
			return false;
		}

		if (FD_ISSET (m_selfPipe.m_readFile, &readSet))
		{
			fireSocketEvent (SocketEventKind_ConnectCancelled);
			return false;
		}

		if (FD_ISSET (m_socket.m_socket, &writeSet))
		{
			int error = m_socket.getError ();
			if (error)
			{
				fireSocketEvent (SocketEventKind_ConnectError, 0, err::Error (error));
				return false;
			}
			else
			{
				m_socket.setBlockingMode (true); // turn blocking mode back on
				fireSocketEvent (SocketEventKind_ConnectCompleted);
				return true;
			}
		}
	}
}

void
Socket::acceptLoop ()
{
	int result;
	int selectFd = AXL_MAX (m_socket.m_socket, m_selfPipe.m_readFile) + 1;

	// accept loop

	for (;;)
	{
		fd_set readSet = { 0 };

		FD_SET (m_selfPipe.m_readFile, &readSet);

		m_ioLock.lock ();

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			break;
		}

		if (!(m_ioFlags & IoFlag_IncomingConnection))  // don't re-issue select if not handled yet
			FD_SET (m_socket.m_socket, &readSet);

		m_ioLock.unlock ();

		result = select (selectFd, &readSet, NULL, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET (m_selfPipe.m_readFile, &readSet))
		{
			char buffer [256];
			m_selfPipe.read (buffer, sizeof (buffer));
		}

		if (FD_ISSET (m_socket.m_socket, &readSet))
		{
			m_ioLock.lock ();
			ASSERT (!(m_ioFlags & IoFlag_IncomingConnection));
			m_ioFlags |= IoFlag_IncomingConnection;
			m_ioLock.unlock ();

			fireSocketEvent (SocketEventKind_IncomingConnection);
		}
	}
}

bool
Socket::recvLoop ()
{
	int result;
	int selectFd = AXL_MAX (m_socket.m_socket, m_selfPipe.m_readFile) + 1;

	// read/write loop

	for (;;)
	{
		fd_set readSet = { 0 };

		FD_SET (m_selfPipe.m_readFile, &readSet);

		m_ioLock.lock ();

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			return true;
		}

		if (!(m_ioFlags & IoFlag_IncomingData)) // don't re-issue select if not handled yet
			FD_SET (m_socket.m_socket, &readSet);

		m_ioLock.unlock ();

		result = select (selectFd, &readSet, NULL, NULL, NULL);
		if (result == -1)
			return false;

		if (FD_ISSET (m_selfPipe.m_readFile, &readSet))
		{
			char buffer [256];
			m_selfPipe.read (buffer, sizeof (buffer));
		}

		if (FD_ISSET (m_socket.m_socket, &readSet))
		{
			size_t incomingDataSize = m_socket.getIncomingDataSize ();
			if (incomingDataSize == -1)
				return false;

			if (incomingDataSize == 0)
			{
				int error = m_socket.getError ();
				return error == 0; // probably, ECONNRESET
			}

			m_ioLock.lock ();
			ASSERT (!(m_ioFlags & IoFlag_IncomingData));
			m_ioFlags |= IoFlag_IncomingData;
			m_ioLock.unlock ();

			fireSocketEvent (SocketEventKind_IncomingData);
		}
	}
}

#endif

//.............................................................................

} // namespace io
} // namespace jnc
