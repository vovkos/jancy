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
SocketBase::getAddress() {
	axl::io::SockAddr sockAddr;
	m_socket.getAddress(&sockAddr);
	return SocketAddress::fromSockAddr(sockAddr);
}

SocketAddress
SocketBase::getPeerAddress() {
	axl::io::SockAddr sockAddr;
	m_socket.getPeerAddress(&sockAddr);
	return SocketAddress::fromSockAddr(sockAddr);
}

bool
SocketBase::setOptions(uint_t options) {
	bool result;

	if (!m_isOpen) {
		m_options = options;
		return true;
	}

	if ((options & SocketOption_TcpNagle) != (m_options & SocketOption_TcpNagle)) {
		int value = !(options & SocketOption_TcpNagle);
		result = m_socket.setOption(IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
		if (!result)
			return false;
	}

	if ((options & SocketOption_TcpReset) != (m_options & SocketOption_TcpReset)) {
		linger value;
		value.l_onoff = (options & SocketOption_TcpReset) != 0;
		value.l_linger = 0;

		result = m_socket.setOption(SOL_SOCKET, SO_LINGER, &value, sizeof(value));
		if (!result)
			return false;
	}

	if ((options & SocketOption_UdpBroadcast) != (m_options & SocketOption_UdpBroadcast)) {
		int value = (m_options & SocketOption_UdpBroadcast) != 0;

		result = m_socket.setOption(SOL_SOCKET, SO_BROADCAST, &value, sizeof(value));
		if (!result)
			return false;
	}

	m_lock.lock();

	if (m_ioThreadFlags & IoThreadFlag_Datagram)
		options |= AsyncIoDeviceOption_KeepReadWriteBlockSize;

	m_options = options;
	wakeIoThread();
	m_lock.unlock();

	return true;
}

bool
SocketBase::checkAccess(
	uint16_t family,
	int protocol
) {
	bool result;

	switch (family) {
	case AddressFamily_Ip4:
		result = requireSocketCapability(SocketCapability_Ip4);
		break;

	case AddressFamily_Ip6:
		result = requireSocketCapability(SocketCapability_Ip6);
		break;

	default:
		result = true;
	}

	if (!result)
		return false;

	switch (protocol) {
	case IPPROTO_ICMP:
		result = requireSocketCapability(SocketCapability_Icmp);
		break;

	case IPPROTO_TCP:
		result = requireSocketCapability(SocketCapability_Tcp);
		break;

	case IPPROTO_UDP:
		result = requireSocketCapability(SocketCapability_Udp);
		break;

	case IPPROTO_RAW:
		result = requireSocketCapability(SocketCapability_Raw);
		break;

	default:
		result = true;
	}

	return result;
}

bool
SocketBase::openSocket(
	uint16_t family_jnc,
	int protocol,
	const SocketAddress* address
) {
	bool result;

	int family_s = family_jnc == AddressFamily_Ip6 ? AF_INET6 : family_jnc;
	int socketKind =
		protocol == IPPROTO_TCP ? SOCK_STREAM :
		protocol == IPPROTO_RAW ? SOCK_RAW : SOCK_DGRAM;

#if (_AXL_OS_WIN)
	result = m_socket.m_socket.wsaOpen(family_s, socketKind, protocol, WSA_FLAG_OVERLAPPED);
#else
	result = m_socket.m_socket.open(family_s, socketKind, protocol);
#endif

	if (!result)
		return false;

	result = m_socket.setBlockingMode(false);
	if (!result)
		return false;

	switch (protocol) {
		int tcpNoDelayValue;
		int tcpKeepAlive;
		linger lingerValue;
		int udpBroadcastValue;
		int rawHdrInclValue;

	case IPPROTO_TCP:
		tcpNoDelayValue = !(m_options & SocketOption_TcpNagle);
		tcpKeepAlive = (m_options & SocketOption_TcpKeepAlive) != 0;
		lingerValue.l_onoff = (m_options & SocketOption_TcpReset) != 0;
		lingerValue.l_linger = 0;

		result =
			m_socket.setOption(IPPROTO_TCP, TCP_NODELAY, &tcpNoDelayValue, sizeof(tcpNoDelayValue)) &&
			m_socket.setOption(SOL_SOCKET, SO_KEEPALIVE, &tcpKeepAlive, sizeof(tcpKeepAlive)) &&
			m_socket.setOption(SOL_SOCKET, SO_LINGER, &lingerValue, sizeof(lingerValue));
		break;

	case IPPROTO_UDP:
		udpBroadcastValue = (m_options & SocketOption_UdpBroadcast) != 0;
		result = m_socket.setOption(SOL_SOCKET, SO_BROADCAST, &udpBroadcastValue, sizeof(udpBroadcastValue));
		break;

	case IPPROTO_RAW:
		rawHdrInclValue = (m_options & SocketOption_RawHdrIncl) != 0;
		result = family_s == AF_INET6 ?
			m_socket.setOption(IPPROTO_IPV6, IPV6_HDRINCL, &rawHdrInclValue, sizeof(rawHdrInclValue)) :
			m_socket.setOption(IPPROTO_IP, IP_HDRINCL, &rawHdrInclValue, sizeof(rawHdrInclValue));
		break;
	}

	if (!result)
		return false;

	if (address) {
		int reuseAddrValue = (m_options & SocketOption_ReuseAddr) != 0;

		result =
			m_socket.setOption(SOL_SOCKET, SO_REUSEADDR, &reuseAddrValue, sizeof(reuseAddrValue)) &&
			m_socket.bind(address->getSockAddr());

		if (!result)
			return false;
	}

	return true;
}

bool
SocketBase::open(
	uint16_t family_jnc,
	int protocol,
	const SocketAddress* address
) {
	close();

	bool result =
		checkAccess(family_jnc, protocol) &&
		openSocket(family_jnc, protocol, address);

	if (!result)
		return false;

	AsyncIoDevice::open();
	m_family = family_jnc;

	if (protocol != IPPROTO_TCP) {
		m_ioThreadFlags |= IoThreadFlag_Datagram;
		m_options |= AsyncIoDeviceOption_KeepReadWriteBlockSize;
	}

	return true;
}

void
SocketBase::close() {
	AsyncIoDevice::close();
	m_socket.close();

	sl::Iterator<IncomingConnection> it = m_pendingIncomingConnectionList.getHead();
	for (; it; it++)
		it->m_socket.close();

	m_incomingConnectionPool.put(&m_pendingIncomingConnectionList);
}

#if (_JNC_OS_WIN)

void
SocketBase::processFdClose(int error) {
	if (!error)
		setEvents(SocketEvent_TcpDisconnected);
	else if (error == WSAECONNRESET)
		setEvents(SocketEvent_TcpDisconnected | SocketEvent_TcpReset);
	else
		setIoErrorEvent(error);
}

void
SocketBase::processSendRecvError() {
	err::Error error = err::getLastError();
	ASSERT(error->m_guid == err::g_systemErrorGuid);

	if (error->m_code == WSAECONNRESET)
		setEvents(SocketEvent_TcpDisconnected | SocketEvent_TcpReset);
	else
		setIoErrorEvent(error);
}

bool
SocketBase::connectLoop(uint_t connectCompletedEvent) {
	sys::Event socketEvent;

	bool result = m_socket.m_socket.wsaEventSelect(socketEvent.m_event, FD_CONNECT);
	if (!result) {
		setIoErrorEvent();
		return false;
	}

	HANDLE waitTable[] = {
		m_ioThreadEvent.m_event,
		socketEvent.m_event,
	};

	for (;;) {
		DWORD waitResult = ::WaitForMultipleObjects(countof(waitTable), waitTable, false, INFINITE);
		switch (waitResult) {
		case WAIT_FAILED:
			setIoErrorEvent(err::getLastSystemErrorCode());
			return false;

		case WAIT_OBJECT_0:
			m_lock.lock();
			if (m_ioThreadFlags & IoThreadFlag_Closing) {
				m_lock.unlock();
				return false;
			}

			m_lock.unlock();
			break;

		case WAIT_OBJECT_0 + 1:
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents(&networkEvents);
			if (!result) {
				setIoErrorEvent();
				return false;
			}

			if (networkEvents.lNetworkEvents & FD_CONNECT) {
				int error = networkEvents.iErrorCode[FD_CONNECT_BIT];
				if (error) {
					setIoErrorEvent(error);
					return false;
				} else {
					setEvents(connectCompletedEvent);
					return true;
				}
			}

			break;
		}
	}
}

void
SocketBase::acceptLoop(uint_t incomingConnectionEvent) {
	sys::Event socketEvent;

	bool result = m_socket.m_socket.wsaEventSelect(socketEvent.m_event, FD_ACCEPT);
	if (!result)
		return;

	HANDLE waitTable[] = {
		m_ioThreadEvent.m_event,
		socketEvent.m_event,
	};

	for (;;) {
		DWORD waitResult = ::WaitForMultipleObjects(countof(waitTable), waitTable, false, INFINITE);
		switch (waitResult) {
		case WAIT_FAILED:
			setIoErrorEvent(err::getLastSystemErrorCode());
			return;

		case WAIT_OBJECT_0:
			m_lock.lock();
			if (m_ioThreadFlags & IoThreadFlag_Closing) {
				m_lock.unlock();
				return;
			}

			m_lock.unlock();
			break;

		case WAIT_OBJECT_0 + 1:
			WSANETWORKEVENTS networkEvents;
			result = m_socket.m_socket.wsaEnumEvents(&networkEvents);
			if (!result) {
				setIoErrorEvent();
				return;
			}

			if (networkEvents.lNetworkEvents & FD_ACCEPT) {
				int error = networkEvents.iErrorCode[FD_ACCEPT_BIT];
				if (error) {
					setIoErrorEvent(error);
					return;
				}

				axl::io::Socket socket;
				axl::io::SockAddr sockAddr;
				bool result = m_socket.accept(&socket, &sockAddr);
				if (!result)
					return;

				m_lock.lock();
				IncomingConnection* incomingConnection = m_incomingConnectionPool.get();
				incomingConnection->m_sockAddr = sockAddr;
				sl::takeOver(&incomingConnection->m_socket.m_socket, &socket.m_socket);
				m_pendingIncomingConnectionList.insertTail(incomingConnection);
				setEvents_l(incomingConnectionEvent);
			}

			break;
		}
	}
}

#elif (_JNC_OS_POSIX)

bool
SocketBase::connectLoop(uint_t connectCompletedEvent) {
	int result;
	int selectFd = AXL_MAX(m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	// connection loop

	for (;;) {
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET(m_ioThreadSelfPipe.m_readFile, &readSet);
		FD_SET(m_socket.m_socket, &writeSet);

		result = ::select(selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1) {
			setIoErrorEvent(err::Error(errno));
			return false;
		}

		if (FD_ISSET(m_ioThreadSelfPipe.m_readFile, &readSet)) {
			char buffer[256];
			m_ioThreadSelfPipe.read(buffer, sizeof(buffer));

			m_lock.lock();
			if (m_ioThreadFlags & IoThreadFlag_Closing) {
				m_lock.unlock();
				return false;
			}

			m_lock.unlock();
		}

		if (FD_ISSET(m_socket.m_socket, &writeSet)) {
			int error = m_socket.getError();
			if (error) {
				setIoErrorEvent(err::Errno(error));
				return false;
			} else {
				setEvents(connectCompletedEvent);
				return true;
			}
		}
	}
}

void
SocketBase::acceptLoop(uint_t incomingConnectionEvent) {
	int result;
	int selectFd = AXL_MAX(m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	bool canAcceptSocket = false;

	for (;;) {
		fd_set readSet = { 0 };

		FD_SET(m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canAcceptSocket)
			FD_SET(m_socket.m_socket, &readSet);

		result = ::select(selectFd, &readSet, NULL, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET(m_ioThreadSelfPipe.m_readFile, &readSet)) {
			char buffer[256];
			m_ioThreadSelfPipe.read(buffer, sizeof(buffer));
		}

		if (FD_ISSET(m_socket.m_socket, &readSet))
			canAcceptSocket = true;

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			break;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		while (canAcceptSocket) {
			axl::io::SockAddr sockAddr;
			socklen_t sockAddrSize = sizeof(sockAddr);
			int socket = ::accept(m_socket.m_socket, &sockAddr.m_addr, &sockAddrSize);
			if (socket == -1) {
				if (errno == EAGAIN) {
					canAcceptSocket = false;
				} else {
					setIoErrorEvent_l(err::Errno(errno));
					return;
				}
			} else {
				IncomingConnection* incomingConnection = m_incomingConnectionPool.get();
				incomingConnection->m_sockAddr = sockAddr;
				incomingConnection->m_socket.m_socket.attach(socket);
				m_pendingIncomingConnectionList.insertTail(incomingConnection);
			}
		}

		if (!m_pendingIncomingConnectionList.isEmpty())
			m_activeEvents |= incomingConnectionEvent;

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
