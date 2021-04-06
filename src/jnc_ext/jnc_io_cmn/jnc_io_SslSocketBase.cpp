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
#include "jnc_io_SslSocketBase.h"
#include "jnc_io_SslStateBase.h"

namespace jnc {
namespace io {

//..............................................................................

#if (_JNC_OS_WIN)

bool
SslSocketBase::sslHandshakeLoop(
	SslStateBase* sslState,
	bool isClient
	)
{
	if (isClient)
		sslState->m_ssl.setConnectState();
	else
		sslState->m_ssl.setAcceptState();

	sys::NotificationEvent socketEvent;
	HANDLE waitTable[] =
	{
		m_ioThreadEvent.m_event,
		socketEvent.m_event,
	};

	for (;;)
	{
		bool result = sslState->m_ssl.doHandshake();
		if (result)
			break;

		uint_t socketEventMask = FD_CLOSE;
		uint_t error = err::getLastError()->m_code;
		switch (error)
		{
		case SSL_ERROR_WANT_READ:
			socketEventMask |= FD_READ;
			break;

		case SSL_ERROR_WANT_WRITE:
			socketEventMask |= FD_WRITE;
			break;

		default:
			setIoErrorEvent();
			return false;
		}

		result = m_socket.m_socket.wsaEventSelect(socketEvent.m_event, socketEventMask);
		if (!result)
			return false;

		DWORD waitResult = ::WaitForMultipleObjects(countof(waitTable), waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent(err::getLastSystemErrorCode());
			return false;
		}

		WSANETWORKEVENTS networkEvents;
		result = m_socket.m_socket.wsaEnumEvents(&networkEvents);
		if (!result)
		{
			setIoErrorEvent();
			return false;
		}

		if ((networkEvents.lNetworkEvents & FD_CLOSE) && !(networkEvents.lNetworkEvents & FD_READ))
		{
			processFdClose(networkEvents.iErrorCode[FD_CLOSE_BIT]);
			return false;
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			return false;
		}

		m_lock.unlock();
	}

	m_lock.lock();
	m_activeEvents = SocketEvent_TcpConnected | SslSocketEvent_SslHandshakeCompleted;
	processWaitLists_l();
	return true;
}

#elif (_JNC_OS_POSIX)

bool
SslSocketBase::sslHandshakeLoop(
	SslStateBase* sslState,
	bool isClient
	)
{
	if (isClient)
		sslState->m_ssl.setConnectState();
	else
		sslState->m_ssl.setAcceptState();

	int selectFd = AXL_MAX(m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	for (;;)
	{
		int result = sslState->m_ssl.doHandshake();
		if (result)
			break;

		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET(m_ioThreadSelfPipe.m_readFile, &readSet);

		uint_t error = err::getLastError()->m_code;
		switch (error)
		{
		case SSL_ERROR_WANT_READ:
			FD_SET(m_socket.m_socket, &readSet);
			break;

		case SSL_ERROR_WANT_WRITE:
			FD_SET(m_socket.m_socket, &writeSet);
			break;

		default:
			setIoErrorEvent();
			return false;
		}

		result = ::select(selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET(m_ioThreadSelfPipe.m_readFile, &readSet))
		{
			char buffer[256];
			m_ioThreadSelfPipe.read(buffer, sizeof(buffer));
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			return false;
		}

		m_lock.unlock();
	}

	m_lock.lock();
	m_activeEvents = SocketEvent_TcpConnected | SslSocketEvent_SslHandshakeCompleted;
	processWaitLists_l();
	return true;
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
