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
#include "jnc_io_Socket.h"
#include "jnc_io_IoLib.h"
#include "jnc_Error.h"

#ifndef IPV6_HDRINCL
#	define IPV6_HDRINCL IP_HDRINCL
#endif

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE (
	SocketEventParams,
	"io.SocketEventParams",
	g_ioLibGuid,
	IoLibCacheSlot_SocketEventParams
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (SocketEventParams)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	Socket,
	"io.Socket",
	g_ioLibGuid,
	IoLibCacheSlot_Socket,
	Socket,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Socket)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <Socket>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <Socket>)
	JNC_MAP_CONST_PROPERTY ("m_address",      &Socket::getAddress)
	JNC_MAP_CONST_PROPERTY ("m_peerAddress",  &Socket::getPeerAddress)
	JNC_MAP_PROPERTY ("m_isBroadcastEnabled", &Socket::isBroadcastEnabled, &Socket::setBroadcastEnabled)
	JNC_MAP_PROPERTY ("m_isNagleEnabled",     &Socket::isNagleEnabled, &Socket::setNagleEnabled)
	JNC_MAP_PROPERTY ("m_isRawHdrIncluded",   &Socket::isRawHdrIncluded, &Socket::setRawHdrIncluded)
	JNC_MAP_PROPERTY ("m_closeKind",          &Socket::getCloseKind, &Socket::setCloseKind)
	JNC_MAP_FUNCTION ("open",     &Socket::open_0)
	JNC_MAP_OVERLOAD (&Socket::open_1)
	JNC_MAP_FUNCTION ("close",    &Socket::close)
	JNC_MAP_FUNCTION ("connect",  &Socket::connect)
	JNC_MAP_FUNCTION ("listen",   &Socket::listen)
	JNC_MAP_FUNCTION ("accept",   &Socket::accept)
	JNC_MAP_FUNCTION ("send",     &Socket::send)
	JNC_MAP_FUNCTION ("recv",     &Socket::recv)
	JNC_MAP_FUNCTION ("sendTo",   &Socket::sendTo)
	JNC_MAP_FUNCTION ("recvFrom", &Socket::recvFrom)
	JNC_MAP_FUNCTION ("firePendingEvents", &Socket::firePendingEvents)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

Socket::Socket ()
{
	m_runtime = getCurrentThreadRuntime ();
	m_ioFlags = 0;
	m_isOpen = false;
	m_syncId = 0;
	m_family = 0;
}

void
Socket::wakeIoThread ()
{
#if (_JNC_OS_WIN)
	m_ioThreadEvent.signal ();
#else
	m_selfPipe.write (" ", 1);
#endif
}

void
Socket::fireSocketEvent (
	SocketEventCode eventCode,
	uint_t flags,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	DataPtr paramsPtr = createData <SocketEventParams> (m_runtime);
	SocketEventParams* params = (SocketEventParams*) paramsPtr.m_p;
	params->m_eventCode = eventCode;
	params->m_syncId = m_syncId;
	params->m_flags = flags;

	if (error)
		params->m_errorPtr = memDup (error, error->m_size);

	callMulticast (m_onSocketEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

void
JNC_CDECL
Socket::firePendingEvents ()
{
}

SocketAddress
JNC_CDECL
Socket::getAddress (Socket* self)
{
	axl::io::SockAddr sockAddr;
	self->m_socket.getAddress (&sockAddr);
	return SocketAddress::fromSockAddr (sockAddr);
}

SocketAddress
JNC_CDECL
Socket::getPeerAddress (Socket* self)
{
	axl::io::SockAddr sockAddr;
	self->m_socket.getPeerAddress (&sockAddr);
	return SocketAddress::fromSockAddr (sockAddr);
}

bool
JNC_CDECL
Socket::isBroadcastEnabled ()
{
	int value = 0;
	bool result = m_socket.getOption (SOL_SOCKET, SO_BROADCAST, &value, sizeof (value));
	return result && value != 0;
}

bool
JNC_CDECL
Socket::setBroadcastEnabled (bool isEnabled)
{
	int value = isEnabled;
	bool result = m_socket.setOption (SOL_SOCKET, SO_BROADCAST, &value, sizeof (value));
	if (!result)
		propagateLastError ();

	return result;
}

bool
JNC_CDECL
Socket::isNagleEnabled ()
{
	int value = 0;
	bool result = m_socket.getOption (IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
	return result && value == 0;
}

bool
JNC_CDECL
Socket::setNagleEnabled (bool isEnabled)
{
	int value = !isEnabled;
	bool result = m_socket.setOption (IPPROTO_TCP, TCP_NODELAY, &value, sizeof (value));
	if (!result)
		propagateLastError ();

	return result;
}

bool
JNC_CDECL
Socket::isRawHdrIncluded ()
{
	int value = 0;

	bool result = m_family == AF_INET6 ?
		m_socket.getOption (IPPROTO_IPV6, IPV6_HDRINCL, &value, sizeof (value)) :
		m_socket.getOption (IPPROTO_IP, IP_HDRINCL, &value, sizeof (value));

	return result && value == 0;
}

bool
JNC_CDECL
Socket::setRawHdrIncluded (bool isIncluded)
{
	int value = isIncluded;

	bool result = m_family == AF_INET6 ?
		m_socket.setOption (IPPROTO_IPV6, IPV6_HDRINCL, &value, sizeof (value)) :
		m_socket.setOption (IPPROTO_IP, IP_HDRINCL, &value, sizeof (value));

	if (!result)
		propagateLastError ();

	return result;
}

SocketCloseKind
JNC_CDECL
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
JNC_CDECL
Socket::setCloseKind (SocketCloseKind closeKind)
{
	linger value;
	value.l_onoff = closeKind == SocketCloseKind_Reset;
	value.l_linger = 0;
	bool result = m_socket.setOption (SOL_SOCKET, SO_LINGER, &value, sizeof (value));
	if (!result)
		propagateLastError ();

	return result;
}

bool
Socket::openImpl (
	uint16_t family_jnc,
	int protocol,
	const SocketAddress* address,
	uint_t flags
	)
{
	close ();

	int family_s = family_jnc == AddressFamily_Ip6 ? AF_INET6 : family_jnc;
	int socketKind =
		(flags & SocketOpenFlag_Raw) ? SOCK_RAW :
		protocol == IPPROTO_TCP ? SOCK_STREAM : SOCK_DGRAM;

	bool result = m_socket.open (family_s, socketKind, protocol);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	if (flags & SocketOpenFlag_ReuseAddress)
	{
		int value = true;
		result = m_socket.setOption (SOL_SOCKET, SO_REUSEADDR, &value, sizeof (value));
		if (!result)
		{
			propagateLastError ();
			return false;
		}
	}

	if (address)
	{
		result = m_socket.bind (address->getSockAddr ());
		if (!result)
		{
			propagateLastError ();
			return false;
		}
	}

	m_isOpen = true;
	m_family = family_s;

	if (protocol == IPPROTO_UDP)
	{
		m_ioFlags |= IoFlag_Udp;
		setBroadcastEnabled (true);
	}

	if (flags & SocketOpenFlag_Asynchronous)
	{
		m_ioFlags |= IoFlag_Asynchronous;

		result = m_socket.setBlockingMode (false);
		if (!result)
		{
			propagateLastError ();
			return false;
		}

#if (_JNC_OS_WIN)
		m_ioThreadEvent.reset ();
#elif (_JNC_OS_POSIX)
		m_selfPipe.create ();
#endif

		if (protocol == IPPROTO_UDP)
			wakeIoThread (); // it's ok to wake before start

		m_ioThread.start ();
	}

	return true;
}

void
JNC_CDECL
Socket::close ()
{
	if (!m_socket.isOpen ())
		return;

	if (m_ioFlags & IoFlag_Asynchronous)
	{
		m_ioLock.lock ();
		m_ioFlags |= IoFlag_Closing;
		wakeIoThread ();
		m_ioLock.unlock ();

		GcHeap* gcHeap = m_runtime->getGcHeap ();
		gcHeap->enterWaitRegion ();
		m_ioThread.waitAndClose ();
		gcHeap->leaveWaitRegion ();

#if (_JNC_OS_POSIX)
		m_selfPipe.close ();
#endif
	}

	m_socket.close ();

	m_ioFlags = 0;
	m_isOpen = false;
	m_syncId++;
}

bool
JNC_CDECL
Socket::connect (DataPtr addressPtr)
{
	bool result;

	if (m_ioFlags & IoFlag_Asynchronous)
	{
		m_ioLock.lock ();
		if (m_ioFlags & ~IoFlag_Asynchronous)
		{
			m_ioLock.unlock ();
			setError (err::SystemErrorCode_InvalidDeviceState);
			return false;
		}

		m_ioFlags |= IoFlag_Connecting;
		wakeIoThread ();
		m_ioLock.unlock ();
	}

	SocketAddress* address = (SocketAddress*) addressPtr.m_p;
	result = m_socket.connect (address->getSockAddr ());
	if (!result)
		propagateLastError ();

	return result;
}

bool
JNC_CDECL
Socket::listen (size_t backLog)
{
	bool result;

	if (m_ioFlags & IoFlag_Asynchronous)
	{
		m_ioLock.lock ();
		if (m_ioFlags & ~IoFlag_Asynchronous)
		{
			m_ioLock.unlock ();
			setError (err::SystemErrorCode_InvalidDeviceState);
			return false;
		}

		m_ioFlags |= IoFlag_Listening;
		wakeIoThread ();
		m_ioLock.unlock ();
	}

	result = m_socket.listen (backLog);
	if (!result)
		propagateLastError ();

	return result;
}

Socket*
JNC_CDECL
Socket::accept (DataPtr addressPtr)
{
	Socket* connectionSocket = createClass <Socket> (m_runtime);

	axl::io::SockAddr sockAddr;
	bool result = m_socket.accept (&connectionSocket->m_socket, &sockAddr);

#if (_JNC_OS_POSIX)
	if (m_ioFlags & IoFlag_Asynchronous)
	{
		m_ioLock.lock ();
		m_ioFlags &= ~IoFlag_IncomingConnection;
		wakeIoThread ();
		m_ioLock.unlock ();
	}
#endif

	if (!result)
	{
		propagateLastError ();

		AXL_MEM_DELETE (connectionSocket);
		return NULL;
	}

	connectionSocket->setCloseKind (getCloseKind ());
	connectionSocket->setNagleEnabled (isNagleEnabled ());

	if (m_ioFlags & IoFlag_Asynchronous)
	{
		connectionSocket->m_ioFlags = IoFlag_Asynchronous | IoFlag_Connected;

#if (_JNC_OS_POSIX)
		connectionSocket->m_selfPipe.create ();
#endif
		connectionSocket->m_ioThread.start ();
		connectionSocket->wakeIoThread ();
	}

	if (addressPtr.m_p)
		((SocketAddress*) addressPtr.m_p)->setSockAddr (sockAddr);

	return connectionSocket;
}

size_t
Socket::postSend (
	size_t size,
	size_t result
	)
{
	if (!(m_ioFlags & IoFlag_Asynchronous))
	{
		if (result == -1)
			propagateLastError ();

		return result;
	}

	if (result == -1)
	{
		err::Error error = err::getLastError ();

#if (_JNC_OS_WIN)
		if (error->m_code != WSAEWOULDBLOCK)
#elif (_JNC_OS_POSIX)
		if (error->m_code != EWOULDBLOCK && error->m_code != EAGAIN)
#endif
		{
			setError (error);
			return -1;
		}

		result = 0;
	}

	if (result < size)
	{
		m_ioLock.lock ();
		if (!(m_ioFlags & IoFlag_WaitingTransmitBuffer))
		{
			m_ioFlags |= IoFlag_WaitingTransmitBuffer;
			wakeIoThread ();
		}

		m_ioLock.unlock ();
	}

	return result;

}

size_t
JNC_CDECL
Socket::send (
	DataPtr ptr,
	size_t size
	)
{
	size_t result = m_socket.send (ptr.m_p, size);
	return postSend (size, result);
}

size_t
JNC_CDECL
Socket::recv (
	DataPtr ptr,
	size_t size
	)
{
	size_t result = m_socket.recv (ptr.m_p, size);

#if (_JNC_OS_POSIX)
	if (m_ioFlags & IoFlag_Asynchronous)
	{
		m_ioLock.lock ();
		m_ioFlags &= ~IoFlag_IncomingData;
		wakeIoThread ();
		m_ioLock.unlock ();
	}
#endif

	if (result == -1)
		propagateLastError ();

	return result;
}

size_t
JNC_CDECL
Socket::sendTo (
	DataPtr ptr,
	size_t size,
	DataPtr addressPtr
	)
{
	axl::io::SockAddr sockAddr = ((const SocketAddress*) addressPtr.m_p)->getSockAddr ();
	size_t result = m_socket.sendTo (ptr.m_p, size, sockAddr);
	return postSend (size, result);
}

size_t
JNC_CDECL
Socket::recvFrom (
	DataPtr ptr,
	size_t size,
	DataPtr addressPtr
	)
{
	axl::io::SockAddr sockAddr;
	size_t result = m_socket.recvFrom (ptr.m_p, size, &sockAddr);

#if (_JNC_OS_POSIX)
	if (m_ioFlags & IoFlag_Asynchronous)
	{
		m_ioLock.lock ();
		m_ioFlags &= ~IoFlag_IncomingData;
		wakeIoThread ();
		m_ioLock.unlock ();
	}
#endif

	if (result == -1)
		propagateLastError ();

	if (addressPtr.m_p)
		((SocketAddress*) addressPtr.m_p)->setSockAddr (sockAddr);

	return result;
}

void
Socket::ioThreadFunc ()
{
	ASSERT (m_socket.isOpen ());

#if (_JNC_OS_WIN)
	m_ioThreadEvent.wait ();
#elif (_JNC_OS_POSIX)
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
		sendRecvLoop ();
	}
	else if (ioFlags & IoFlag_Connected)
	{
		bool result = sendRecvLoop ();
		uint_t flags = result ? 0 : SocketDisconnectEventFlag_Reset;
		fireSocketEvent (SocketEventCode_Disconnected, flags);
	}
	else if (ioFlags & IoFlag_Listening)
	{
		acceptLoop ();
	}
	else if (ioFlags & IoFlag_Connecting)
	{
		if (connectLoop ())
		{
			bool result = sendRecvLoop ();
			uint_t flags = result ? 0 : SocketDisconnectEventFlag_Reset;
			fireSocketEvent (SocketEventCode_Disconnected, flags);
		}
	}
}

#if (_JNC_OS_WIN)

bool
Socket::connectLoop ()
{
	sys::Event socketEvent;

	bool result = m_socket.m_socket.wsaEventSelect (socketEvent.m_event, FD_CONNECT);
	if (!result)
	{
		fireSocketEvent (SocketEventCode_ConnectError, 0, err::getLastError ());
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
			fireSocketEvent (SocketEventCode_ConnectError, 0, err::getLastError ());
			return false;

		case WAIT_OBJECT_0:
			fireSocketEvent (SocketEventCode_ConnectCancelled);
			return false;

		case WAIT_OBJECT_0 + 1:
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents (&networkEvents);
			if (!result)
			{
				fireSocketEvent (SocketEventCode_ConnectCancelled);
				return false;
			}

			if (networkEvents.lNetworkEvents & FD_CONNECT)
			{
				int error = networkEvents.iErrorCode [FD_CONNECT_BIT];
				if (error)
				{
					fireSocketEvent (SocketEventCode_ConnectError, 0, err::Error (error));
					return false;
				}
				else
				{
					fireSocketEvent (SocketEventCode_ConnectCompleted);
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
	sys::Event socketEvent;

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
					fireSocketEvent (SocketEventCode_IncomingConnection);
			}

			break;
		}
	}
}

bool
Socket::sendRecvLoop ()
{
	sys::Event socketEvent;

	HANDLE waitTable [] =
	{
		m_ioThreadEvent.m_event,
		socketEvent.m_event,
	};

	for (;;)
	{
		uint_t eventMask = FD_READ | FD_CLOSE;
		m_ioLock.lock ();
		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			return true;
		}

		if (m_ioFlags & IoFlag_WaitingTransmitBuffer)
			eventMask |= FD_WRITE;

		m_ioLock.unlock ();

		bool result = m_socket.m_socket.wsaEventSelect (socketEvent.m_event, eventMask);
		if (!result)
			return false;

		DWORD waitResult = ::WaitForMultipleObjects (countof (waitTable), waitTable, false, INFINITE);
		switch (waitResult)
		{
		case WAIT_FAILED:
			return false;

		case WAIT_OBJECT_0:
			break;

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
					fireSocketEvent (SocketEventCode_IncomingData);
			}

			if (networkEvents.lNetworkEvents & FD_WRITE)
			{
				int error = networkEvents.iErrorCode [FD_WRITE_BIT];
				if (error)
					return false;

				m_ioLock.lock ();
				ASSERT (m_ioFlags & IoFlag_WaitingTransmitBuffer);
				m_ioFlags &= ~IoFlag_WaitingTransmitBuffer;
				m_ioLock.unlock ();

				fireSocketEvent (SocketEventCode_TransmitBufferReady);
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
			fireSocketEvent (SocketEventCode_ConnectError, 0, err::Error (errno));
			return false;
		}

		if (FD_ISSET (m_selfPipe.m_readFile, &readSet))
		{
			fireSocketEvent (SocketEventCode_ConnectCancelled);
			return false;
		}

		if (FD_ISSET (m_socket.m_socket, &writeSet))
		{
			int error = m_socket.getError ();
			if (error)
			{
				fireSocketEvent (SocketEventCode_ConnectError, 0, err::Error (error));
				return false;
			}
			else
			{
				m_socket.setBlockingMode (true); // turn blocking mode back on
				fireSocketEvent (SocketEventCode_ConnectCompleted);
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

			fireSocketEvent (SocketEventCode_IncomingConnection);
		}
	}
}

bool
Socket::sendRecvLoop ()
{
	int result;
	int selectFd = AXL_MAX (m_socket.m_socket, m_selfPipe.m_readFile) + 1;

	// read/write loop

	for (;;)
	{
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };
		fd_set* writeSetPtr = NULL;

		FD_SET (m_selfPipe.m_readFile, &readSet);

		m_ioLock.lock ();

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			return true;
		}

		if (!(m_ioFlags & IoFlag_IncomingData)) // don't re-issue select if not handled yet
			FD_SET (m_socket.m_socket, &readSet);

		if (m_ioFlags & IoFlag_WaitingTransmitBuffer)
		{
			FD_SET (m_socket.m_socket, &writeSet);
			writeSetPtr = &writeSet;
		}

		m_ioLock.unlock ();

		result = select (selectFd, &readSet, writeSetPtr, NULL, NULL);
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

			fireSocketEvent (SocketEventCode_IncomingData);
		}

		if (FD_ISSET (m_socket.m_socket, &writeSet))
		{
			m_ioLock.lock ();
			ASSERT (m_ioFlags & IoFlag_WaitingTransmitBuffer);
			m_ioFlags &= ~IoFlag_WaitingTransmitBuffer;
			m_ioLock.unlock ();

			fireSocketEvent (SocketEventCode_IncomingData);
		}
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
