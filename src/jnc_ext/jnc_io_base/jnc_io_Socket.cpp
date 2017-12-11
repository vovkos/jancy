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

bool
Socket::openImpl (
	uint16_t family,
	int protocol,
	const SocketAddress* address
	)
{
	bool result = SocketBase::open (family, protocol, address);
	if (!result)
		return false;

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

	SocketBase::close ();
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
	Socket* connectionSocket = createClass <Socket> (m_runtime);

	m_lock.lock ();
	if (m_pendingIncomingConnectionList.isEmpty ())
	{
		m_lock.unlock ();
		setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return NULL;
	}

	IncomingConnection* incomingConnection = m_pendingIncomingConnectionList.removeHead ();
	connectionSocket->m_socket.m_socket.takeOver (&incomingConnection->m_socket.m_socket);
	connectionSocket->setOptions (m_options);
	connectionSocket->AsyncIoDevice::open ();
	connectionSocket->m_ioThreadFlags =
		(m_ioThreadFlags & IoThreadFlag_Ip6) |
		IoThreadFlag_Tcp |
		IoThreadFlag_IncomingConnection;

	if (address)
		address->setSockAddr (incomingConnection->m_sockAddr);

	m_freeIncomingConnectionList.insertHead (incomingConnection);
	m_lock.unlock ();

	connectionSocket->m_ioThread.start ();
	return connectionSocket;
}

size_t
JNC_CDECL
Socket::readDatagram (
	DataPtr dataPtr,
	size_t dataSize,
	DataPtr addressPtr
	)
{
	if (!(m_ioThreadFlags & IoThreadFlag_Datagram))
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	size_t result;

	if (!addressPtr.m_p)
	{
		result = bufferedRead (dataPtr, dataSize);
	}
	else
	{
		char buffer [256];
		sl::Array <char> params (ref::BufKind_Stack, buffer, sizeof (buffer));
		result = bufferedRead (dataPtr, dataSize, &params);

		ASSERT (params.getCount () == sizeof (SocketAddress));
		memcpy (addressPtr.m_p, params, sizeof (SocketAddress));
	}

	return result;
}

size_t
JNC_CDECL
Socket::writeDatagram (
	DataPtr dataPtr,
	size_t dataSize,
	DataPtr addressPtr
	)
{
	if (!(m_ioThreadFlags & IoThreadFlag_Datagram))
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	return bufferedWrite (dataPtr, dataSize, addressPtr.m_p, sizeof (SocketAddress));
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
			bool result = tcpConnect (SocketEvent_Connected);
			if (result)
				sendRecvLoop ();
		}
		else if (m_ioThreadFlags & IoThreadFlag_Listening)
		{
			m_lock.unlock ();
			acceptLoop ();
		}
		else if (m_ioThreadFlags & IoThreadFlag_IncomingConnection)
		{
			m_lock.unlock ();
			sendRecvLoop ();
		}
	}
}

#if (_JNC_OS_WIN)

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
				setIoErrorEvent_l (err::SystemErrorCode_Cancelled);
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

void
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

#elif (_JNC_OS_POSIX)

void
Socket::acceptLoop ()
{
	int result;
	int selectFd = AXL_MAX (m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	bool canAcceptSocket = false;

	for (;;)
	{
		fd_set readSet = { 0 };

		FD_SET (m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canAcceptSocket)
			FD_SET (m_socket.m_socket, &readSet);

		result = ::select (selectFd, &readSet, NULL, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET (m_ioThreadSelfPipe.m_readFile, &readSet))
		{
			char buffer [256];
			m_ioThreadSelfPipe.read (buffer, sizeof (buffer));
		}

		if (FD_ISSET (m_socket.m_socket, &readSet))
			canAcceptSocket = true;

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			break;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		while (canAcceptSocket && m_freeIncomingConnectionList.getCount () < m_backLogLimit)
		{
			axl::io::SockAddr sockAddr;
			socklen_t sockAddrSize = sizeof (sockAddr);
			int socket = ::accept (m_socket.m_socket, &sockAddr.m_addr, &sockAddrSize);
			if (socket == -1)
			{
				if (errno == EAGAIN)
				{
					canAcceptSocket = false;
				}
				else
				{
					setIoErrorEvent_l (err::Errno (errno));
					return;
				}
			}
			else
			{
				IncomingConnection* incomingConnection = createIncomingConnection ();
				incomingConnection->m_sockAddr = sockAddr;
				incomingConnection->m_socket.m_socket.attach (socket);
				m_pendingIncomingConnectionList.insertTail (incomingConnection);
			}
		}

		if (!m_pendingIncomingConnectionList.isEmpty ())
			m_activeEvents |= SocketEvent_IncomingConnection;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();

	}
}

void
Socket::sendRecvLoop ()
{
	int result;
	int selectFd = AXL_MAX (m_socket.m_socket, m_ioThreadSelfPipe.m_readFile) + 1;

	sl::Array <char> readBlock;
	sl::Array <char> writeBlock;
	readBlock.setCount (Def_ReadBlockSize);

	char buffer [256];
	sl::Array <char> writeParams (ref::BufKind_Stack, buffer, sizeof (buffer));
	writeParams.setCount (sizeof (SocketAddress));

	bool canReadSocket = false;
	bool canWriteSocket = false;

	for (;;)
	{
		fd_set readSet = { 0 };
		fd_set writeSet = { 0 };

		FD_SET (m_ioThreadSelfPipe.m_readFile, &readSet);

		if (!canReadSocket)
			FD_SET (m_socket.m_socket, &readSet);

		if (!canWriteSocket)
			FD_SET (m_socket.m_socket, &writeSet);

		result = ::select (selectFd, &readSet, &writeSet, NULL, NULL);
		if (result == -1)
			break;

		if (FD_ISSET (m_ioThreadSelfPipe.m_readFile, &readSet))
		{
			char buffer [256];
			m_ioThreadSelfPipe.read (buffer, sizeof (buffer));
		}

		if (FD_ISSET (m_socket.m_socket, &readSet))
			canReadSocket = true;

		if (FD_ISSET (m_socket.m_socket, &writeSet))
			canWriteSocket = true;

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			return;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		readBlock.setCount (m_readBlockSize); // update read block size

		if (m_ioThreadFlags & IoThreadFlag_Tcp)
		{
			while (canReadSocket && !m_readBuffer.isFull ())
			{
				ssize_t actualSize = ::recv (m_socket.m_socket, readBlock, readBlock.getCount (), 0);
				if (actualSize != -1)
				{
					if (errno == EAGAIN)
					{
						canReadSocket = false;
					}
					else
					{
						setIoErrorEvent_l (err::Errno (errno));
						return;
					}
				}
				else if (actualSize == 0)
				{
					setEvents (m_socket.getError () ? SocketEvent_Reset : SocketEvent_Disconnected);
					return;
				}
				else
				{
					addToReadBuffer (readBlock, actualSize);
				}
			}

			while (canWriteSocket)
			{
				getNextWriteBlock (&writeBlock);
				if (writeBlock.isEmpty ())
					break;

				size_t blockSize = writeBlock.getCount ();
				ssize_t actualSize = ::send (m_socket.m_socket, writeBlock, blockSize, 0);
				if (actualSize == -1)
				{
					if (errno == EAGAIN)
					{
						canWriteSocket = false;
					}
					else if (actualSize < 0)
					{
						setIoErrorEvent_l (err::Errno ((int) actualSize));
						return;
					}
				}
				else if ((size_t) actualSize < blockSize)
				{
					writeBlock.remove (0, actualSize);
				}
				else
				{
					writeBlock.clear ();
				}
			}
		}
		else
		{
			while (canReadSocket && !m_readBuffer.isFull ())
			{
				axl::io::SockAddr sockAddr;
				socklen_t sockAddrSize = sizeof (sockAddr);
				ssize_t actualSize = ::recvfrom (m_socket.m_socket, readBlock, readBlock.getCount (), 0, &sockAddr.m_addr, &sockAddrSize);
				if (actualSize != -1)
				{
					if (errno == EAGAIN)
					{
						canReadSocket = false;
					}
					else
					{
						setIoErrorEvent_l (err::Errno (errno));
						return;
					}
				}
				else
				{
					SocketAddress socketAddress;
					socketAddress.setSockAddr (sockAddr);
					addToReadBuffer (readBlock, actualSize, &socketAddress, sizeof (socketAddress));
				}
			}

			while (canWriteSocket)
			{
				getNextWriteBlock (&writeBlock, &writeParams);
				if (writeBlock.isEmpty ())
					break;

				axl::io::SockAddr sockAddr = ((SocketAddress*) writeParams.p ())->getSockAddr ();
				socklen_t sockAddrSize = axl::io::getSockAddrSize (&sockAddr.m_addr);
				size_t blockSize = writeBlock.getCount ();
				ssize_t actualSize = ::sendto (m_socket.m_socket, writeBlock, blockSize, 0, &sockAddr.m_addr, sockAddrSize);
				if (actualSize == -1)
				{
					if (errno == EAGAIN)
					{
						canWriteSocket = false;
					}
					else if (actualSize < 0)
					{
						setIoErrorEvent_l (err::Errno ((int) actualSize));
						return;
					}
				}
				else
				{
					writeBlock.clear ();
				}
			}
		}

		updateReadWriteBufferEvents ();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();
	}
}

#endif

//..............................................................................

} // namespace io
} // namespace jnc
