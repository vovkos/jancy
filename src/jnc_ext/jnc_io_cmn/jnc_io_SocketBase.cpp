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
#include "jnc_io_SocketBase.h"
#include "jnc_Error.h"

#ifndef IPV6_HDRINCL
#	define IPV6_HDRINCL IP_HDRINCL
#endif

namespace jnc {
namespace io {

//..............................................................................

SocketAddress
SocketBase::getAddress ()
{
	axl::io::SockAddr sockAddr;
	m_socket.getAddress (&sockAddr);
	return SocketAddress::fromSockAddr (sockAddr);
}

SocketAddress
SocketBase::getPeerAddress ()
{
	axl::io::SockAddr sockAddr;
	m_socket.getPeerAddress (&sockAddr);
	return SocketAddress::fromSockAddr (sockAddr);
}

bool
SocketBase::setOptions (uint_t options)
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

bool
SocketBase::open (
	uint16_t family_jnc,
	int protocol,
	const SocketAddress* address
	)
{
	bool result;

	close ();

	int family_s = family_jnc == AddressFamily_Ip6 ? AF_INET6 : family_jnc;
	int socketKind =
		protocol == IPPROTO_TCP ? SOCK_STREAM :
		protocol == IPPROTO_RAW ? SOCK_RAW : SOCK_DGRAM;

#if (_AXL_OS_WIN)
	result = m_socket.m_socket.wsaOpen (family_s, SOCK_STREAM, IPPROTO_TCP, WSA_FLAG_OVERLAPPED);
#else
	result = m_socket.m_socket.open (family_s, SOCK_STREAM, IPPROTO_TCP);
#endif

	if (!result)
	{
		propagateLastError ();
		return false;
	}

	result = m_socket.setBlockingMode (false);
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

	if (protocol != IPPROTO_TCP)
	{
		m_ioThreadFlags |= IoThreadFlag_Datagram;
		m_options |= AsyncIoOption_KeepReadBlockSize | AsyncIoOption_KeepWriteBlockSize;
	}

	return true;
}

void
SocketBase::close ()
{
	AsyncIoDevice::close ();
	m_socket.close ();
}

#if (_JNC_OS_WIN)

bool
SocketBase::tcpConnect (uint_t connectCompletedEvent)
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
					setEvents (connectCompletedEvent);
					return true;
				}
			}

			break;
		}
	}
}

#elif (_JNC_OS_POSIX)

bool
SocketBase::tcpConnect (uint_t connectCompletedEvent)
{
	int result;
	int selectFd = AXL_MAX (m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	// connection loop

	for (;;)
	{
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET (m_ioThreadSelfPipe.m_readFile, &readSet);
		FD_SET (m_socket.m_socket, &writeSet);

		result = ::select (selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
		{
			setIoErrorEvent (err::Error (errno));
			return false;
		}

		if (FD_ISSET (m_ioThreadSelfPipe.m_readFile, &readSet))
		{
			char buffer [256];
			m_ioThreadSelfPipe.read (buffer, sizeof (buffer));
		}

		if (FD_ISSET (m_socket.m_socket, &writeSet))
		{
			int error = m_socket.getError ();
			if (error)
			{
				setIoErrorEvent (err::Errno (error));
				return false;
			}
			else
			{
				setEvents (connectCompletedEvent);
				return true;
			}
		}
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
