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

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	Socket,
	"io.Socket",
	g_ioLibGuid,
	IoLibCacheSlot_Socket,
	Socket,
	&Socket::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Socket)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <Socket>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <Socket>)

	JNC_MAP_CONST_PROPERTY ("m_address",     &Socket::getAddress)
	JNC_MAP_CONST_PROPERTY ("m_peerAddress", &Socket::getPeerAddress)

	JNC_MAP_AUTOGET_PROPERTY ("m_readBlockSize",   &Socket::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_readBufferSize",  &Socket::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_writeBufferSize", &Socket::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY ("m_options",         &Socket::setOptions)

	JNC_MAP_FUNCTION ("open",          &Socket::open_0)
	JNC_MAP_OVERLOAD (&Socket::open_1)
	JNC_MAP_FUNCTION ("close",         &Socket::close)
	JNC_MAP_FUNCTION ("connect",       &Socket::connect)
	JNC_MAP_FUNCTION ("listen",        &Socket::listen)
	JNC_MAP_FUNCTION ("accept",        &Socket::accept)
	JNC_MAP_FUNCTION ("read",          &Socket::read)
	JNC_MAP_FUNCTION ("write",         &Socket::write)
	JNC_MAP_FUNCTION ("readDatagram",  &Socket::readDatagram)
	JNC_MAP_FUNCTION ("writeDatagram", &Socket::writeDatagram)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

Socket::Socket ()
{
	m_runtime = getCurrentThreadRuntime ();
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
Socket::openImpl (
	uint16_t family_jnc,
	int protocol,
	const SocketAddress* address
	)
{
	close ();

	int family_s = family_jnc == AddressFamily_Ip6 ? AF_INET6 : family_jnc;
	int socketKind =
		protocol == IPPROTO_TCP ? SOCK_STREAM :
		protocol == IPPROTO_RAW ? SOCK_RAW : SOCK_DGRAM;

	bool result =
		m_socket.m_socket.wsaOpen (family_s, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED) &&
		m_socket.setBlockingMode (false);

	if (!result)
	{
		propagateLastError ();
		return false;
	}

	switch (protocol)
	{
		int tcpNoDelayValue;
		linger lingerValue;
		int udpBroadcastValue;
		int rawHdrInclValue;

	case IPPROTO_TCP:
		tcpNoDelayValue = !(m_options & SocketOption_TcpNagle);
		lingerValue.l_onoff = (m_options & SocketOption_TcpReset) != 0;
		lingerValue.l_linger = 0;

		result =
			m_socket.setOption (IPPROTO_TCP, TCP_NODELAY, &tcpNoDelayValue, sizeof (tcpNoDelayValue)) &&
			m_socket.setOption (SOL_SOCKET, SO_LINGER, &lingerValue, sizeof (lingerValue));
		break;

	case IPPROTO_UDP:
		udpBroadcastValue = (m_options & SocketOption_UdpBroadcast) != 0;
		result = m_socket.setOption (SOL_SOCKET, SO_BROADCAST, &udpBroadcastValue, sizeof (udpBroadcastValue));
		break;

	case IPPROTO_RAW:
		rawHdrInclValue = (m_options & SocketOption_RawHdrIncl) != 0;
		result = family_s == AF_INET6 ?
			m_socket.setOption (IPPROTO_IPV6, IPV6_HDRINCL, &rawHdrInclValue, sizeof (rawHdrInclValue)) :
			m_socket.setOption (IPPROTO_IP, IP_HDRINCL, &rawHdrInclValue, sizeof (rawHdrInclValue));
		break;
	}

	if (!result)
	{
		propagateLastError ();
		return false;
	}

	if (address)
	{
		int reuseAddrValue = (m_options & SocketOption_ReuseAddr) != 0;

		result =
			m_socket.setOption (SOL_SOCKET, SO_REUSEADDR, &reuseAddrValue, sizeof (reuseAddrValue)) &&
			m_socket.bind (address->getSockAddr ());

		if (!result)
		{
			propagateLastError ();
			return false;
		}
	}

	AsyncIoDevice::open ();

	if (family_s == AF_INET6)
		m_ioThreadFlags |= IoThreadFlag_Ip6;

	if (protocol == IPPROTO_TCP)
		m_ioThreadFlags |= IoThreadFlag_Tcp;
	else
		m_options |= AsyncIoOption_KeepReadBlockSize | AsyncIoOption_KeepWriteBlockSize;

	m_ioThread.start ();
	return true;
}

void
JNC_CDECL
Socket::close ()
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

	m_socket.close ();

	AsyncIoDevice::close ();
}

bool
JNC_CDECL
Socket::connect (DataPtr addressPtr)
{
	SocketAddress* address = (SocketAddress*) addressPtr.m_p;
	bool result = m_socket.connect (address->getSockAddr ());
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	if (result)
	{
		m_lock.lock ();
		m_ioThreadFlags |= IoThreadFlag_Connecting;
		wakeIoThread ();
		m_lock.unlock ();
	}

	return result;
}

bool
JNC_CDECL
Socket::listen (size_t backLogLimit)
{
	bool result = m_socket.listen (backLogLimit);
	if (!result)
	{
		propagateLastError ();
		return false;
	}

	if (result)
	{
		m_lock.lock ();
		m_backLogLimit = backLogLimit;
		m_ioThreadFlags |= IoThreadFlag_Listening;
		wakeIoThread ();
		m_lock.unlock ();
	}

	return true;
}

Socket*
JNC_CDECL
Socket::accept (DataPtr addressPtr)
{
	SocketAddress* address = ((SocketAddress*) addressPtr.m_p);

	m_lock.lock ();
	if (m_pendingIncomingConnectionList.isEmpty ())
	{
		m_lock.unlock ();
		setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return NULL;
	}

	Accept* accept = m_pendingIncomingConnectionList.removeHead ();
	m_lock.unlock ();

	Socket* connectionSocket = createClass <Socket> (m_runtime);
	connectionSocket->m_socket.m_socket.takeOver (&socketEntry->m_value);
	connectionSocket->setOptions (m_options);
	connectionSocket->AsyncIoDevice::open ();
	connectionSocket->m_ioThreadFlags =
		(m_ioThreadFlags & IoThreadFlag_Ip6) |
		IoThreadFlag_Tcp |
		IoThreadFlag_Connected;

	connectionSocket->m_ioThread.start ();

	if (address)
		address->setSockAddr (sockAddr);

	return connectionSocket;
}

size_t
JNC_CDECL
Socket::readDatagram (
	DataPtr userBufferPtr,
	size_t userBufferSize,
	DataPtr addressPtr
	)
{
	if (!m_isOpen)
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	SocketAddress* address = ((SocketAddress*) addressPtr.m_p);

	m_lock.lock ();

	if (m_readMetaList.isEmpty ())
	{
		m_lock.unlock ();
		if (address)
			memset (address, 0, sizeof (SocketAddress));

		return 0;
	}

	// we need to remove the whole datagram, even if the user buffer is too small

	ReadWriteMeta* meta = *m_readMetaList.getHead ();
	size_t datagramSize = meta->m_blockSize;

	sl::Array <char> datagram;
	char* p;

	if (userBufferSize >= datagramSize)
	{
		p = (char*) userBufferPtr.m_p;
	}
	else
	{
		datagram.setCount (datagramSize);
		p = datagram;
	}

	size_t result = m_readBuffer.read (p, datagramSize);
	if (!m_readOverflowBuffer.isEmpty ())
	{
		p += result;
		datagramSize -= result;

		size_t overflowSize = m_readOverflowBuffer.getCount ();
		size_t extraSize = AXL_MIN (overflowSize, datagramSize);

		memcpy (p, m_readOverflowBuffer, extraSize);
		result += extraSize;

		size_t movedSize = m_readBuffer.write (m_readOverflowBuffer + extraSize, overflowSize - extraSize);
		m_readOverflowBuffer.remove (0, movedSize);
	}

	if (userBufferSize < datagramSize)
		memcpy (userBufferPtr.m_p, datagram, userBufferSize);

	if (address)
	{
		if (meta->m_paramSize >= sizeof (SocketAddress))
			(*(SocketAddress*) addressPtr.m_p) = *(SocketAddress*) (meta + 1);
		else
			memset (addressPtr.m_p, 0, sizeof (SocketAddress));
	}

	m_readMetaList.remove (meta);
	m_freeReadWriteMetaList.insertHead (meta);

	if (result)
	{
		if (m_readMetaList.isEmpty ())
			m_activeEvents &= ~AsyncIoEvent_IncomingData;

		wakeIoThread ();
	}

	ASSERT (isReadBufferValid ());
	m_lock.unlock ();

	return result;
}

size_t
JNC_CDECL
Socket::writeDatagram (
	DataPtr ptr,
	size_t size,
	DataPtr addressPtr
	)
{
	if (!m_isOpen || (m_ioThreadFlags & IoThreadFlag_Tcp))
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	SocketAddress* address = ((SocketAddress*) addressPtr.m_p);

	m_lock.lock ();
	size_t result = m_writeBuffer.write (ptr.m_p, size);

	if (result)
	{
		ReadWriteMeta* meta = createReadWriteMeta (sizeof (axl::io::SockAddr));
		meta->m_blockSize = result;

		if (address)
			*(axl::io::SockAddr*) (meta + 1) = address->getSockAddr ();

		m_writeMetaList.insertTail (meta);

		if (m_writeBuffer.isFull ())
			m_activeEvents &= ~AsyncIoEvent_TransmitBufferReady;

		wakeIoThread ();
	}

	ASSERT (isWriteBufferValid ());
	m_lock.unlock ();

	return result;
}

void
Socket::ioThreadFunc ()
{
	ASSERT (m_socket.isOpen ());

	m_lock.lock ();
	if (!(m_ioThreadFlags & IoThreadFlag_Tcp))
	{
		m_lock.unlock ();
		sendRecvLoop ();
	}
	else
	{
		sleepIoThread ();

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
		}
		else if (m_ioThreadFlags & IoThreadFlag_Connecting)
		{
			m_lock.unlock ();
			connectLoop () && sendRecvLoop ();
		}
		else if (m_ioThreadFlags & IoThreadFlag_Listening)
		{
			m_lock.unlock ();
			acceptLoop ();
		}
		else if (m_ioThreadFlags & IoThreadFlag_Connected)
		{
			m_lock.unlock ();
			sendRecvLoop ();
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
		setIoErrorEvent ();
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
				setIoErrorEvent (err::SystemErrorCode_Cancelled);
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
					setEvents (SocketEvent_Connected);
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
			setIoErrorEvent (err::getLastSystemErrorCode ());
			return;

		case WAIT_OBJECT_0:
			m_lock.lock ();
			if (m_ioThreadFlags & IoThreadFlag_Closing)
			{
				m_lock.unlock ();
				setIoErrorEvent (err::SystemErrorCode_Cancelled);
				return;
			}

			m_lock.unlock ();
			break;

		case WAIT_OBJECT_0 + 1:
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents (&networkEvents);
			if (!result)
			{
				setIoErrorEvent ();
				return;
			}

			if (networkEvents.lNetworkEvents & FD_ACCEPT)
			{
				int error = networkEvents.iErrorCode [FD_ACCEPT_BIT];
				if (error)
				{
					setIoErrorEvent (error);
					return;
				}

				axl::io::SockAddr sockAddr;
				bool result = m_socket.accept (&connectionSocket->m_socket, &sockAddr);
				if (!result)
				{
					propagateLastError ();
					return NULL;
				}

				setEvents (SocketEvent_IncomingConnection);
			}

			break;
		}
	}
}

#if 0

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
		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			return true;
		}

		if (m_ioThreadFlags & IoThreadFlag_WaitingTransmitBuffer)
			eventMask |= FD_WRITE;

		m_lock.unlock ();

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

				m_lock.lock ();
				ASSERT (m_ioThreadFlags & IoThreadFlag_WaitingTransmitBuffer);
				m_ioThreadFlags &= ~IoThreadFlag_WaitingTransmitBuffer;
				m_lock.unlock ();

				fireSocketEvent (SocketEventCode_TransmitBufferReady);
			}

			break;
		}
	}
}

#endif

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

		m_lock.lock ();

		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			break;
		}

		if (!(m_ioThreadFlags & IoThreadFlag_IncomingConnection))  // don't re-issue select if not handled yet
			FD_SET (m_socket.m_socket, &readSet);

		m_lock.unlock ();

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
			m_lock.lock ();
			ASSERT (!(m_ioThreadFlags & IoThreadFlag_IncomingConnection));
			m_ioThreadFlags |= IoThreadFlag_IncomingConnection;
			m_lock.unlock ();

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

		m_lock.lock ();

		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			return true;
		}

		if (!(m_ioThreadFlags & IoThreadFlag_IncomingData)) // don't re-issue select if not handled yet
			FD_SET (m_socket.m_socket, &readSet);

		if (m_ioThreadFlags & IoThreadFlag_WaitingTransmitBuffer)
		{
			FD_SET (m_socket.m_socket, &writeSet);
			writeSetPtr = &writeSet;
		}

		m_lock.unlock ();

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

			m_lock.lock ();
			ASSERT (!(m_ioThreadFlags & IoThreadFlag_IncomingData));
			m_ioThreadFlags |= IoThreadFlag_IncomingData;
			m_lock.unlock ();

			fireSocketEvent (SocketEventCode_IncomingData);
		}

		if (FD_ISSET (m_socket.m_socket, &writeSet))
		{
			m_lock.lock ();
			ASSERT (m_ioThreadFlags & IoThreadFlag_WaitingTransmitBuffer);
			m_ioThreadFlags &= ~IoThreadFlag_WaitingTransmitBuffer;
			m_lock.unlock ();

			fireSocketEvent (SocketEventCode_TransmitBufferReady);
		}
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
