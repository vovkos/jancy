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

	JNC_MAP_AUTOGET_PROPERTY ("m_readParallelism", &Socket::setReadParallelism)
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
	JNC_MAP_FUNCTION ("wait",          &Socket::wait)
	JNC_MAP_FUNCTION ("cancelWait",    &Socket::cancelWait)
	JNC_MAP_FUNCTION ("blockingWait",  &Socket::blockingWait)
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

	if (m_ioThreadFlags & IoThreadFlag_Datagram)
		wakeIoThread ();

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

		ASSERT (params.getCount () == sizeof (axl::io::SockAddr));
		((SocketAddress*) addressPtr.m_p)->setSockAddr (*(axl::io::SockAddr*) params.p ());
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

	axl::io::SockAddr sockAddr = ((SocketAddress*) addressPtr.m_p)->getSockAddr ();
	return bufferedWrite (dataPtr, dataSize, &sockAddr, sizeof (sockAddr));
}

void
Socket::ioThreadFunc ()
{
	ASSERT (m_socket.isOpen ());

	sleepIoThread ();

	m_lock.lock ();
	if (m_ioThreadFlags & IoThreadFlag_Closing)
	{
		m_lock.unlock ();
	}
	else if (m_ioThreadFlags & IoThreadFlag_Datagram)
	{
		m_lock.unlock ();
		sendRecvLoop (0, true);
	}
	else if (m_ioThreadFlags & IoThreadFlag_Connecting)
	{
		m_lock.unlock ();
		bool result = tcpConnect (SocketEvent_Connected);
		if (result)
			sendRecvLoop (SocketEvent_Connected, false);
	}
	else if (m_ioThreadFlags & IoThreadFlag_Listening)
	{
		m_lock.unlock ();
		acceptLoop ();
	}
	else if (m_ioThreadFlags & IoThreadFlag_IncomingConnection)
	{
		m_lock.unlock ();
		sendRecvLoop (SocketEvent_Connected, false);
	}
	else
	{
		m_lock.unlock ();
		ASSERT (false); // shouldn't normally happen
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

				axl::io::Socket socket;
				axl::io::SockAddr sockAddr;
				bool result = m_socket.accept (&socket, &sockAddr);
				if (!result)
				{
					propagateLastError ();
					return;
				}

				m_lock.lock ();
				IncomingConnection* incomingConnection = createIncomingConnection ();
				incomingConnection->m_sockAddr = sockAddr;
				incomingConnection->m_socket.m_socket.takeOver (&socket.m_socket);
				m_pendingIncomingConnectionList.insertTail (incomingConnection);

				setEvents_l (SocketEvent_IncomingConnection);
			}

			break;
		}
	}
}

void
Socket::sendRecvLoop (
	uint_t baseEvents,
	bool isDatagram
	)
{
	ASSERT (m_socket.isOpen ());

	bool result;

	axl::io::win::StdOverlapped sendOverlapped;

	HANDLE waitTable [3] =
	{
		m_ioThreadEvent.m_event,
		sendOverlapped.m_completionEvent.m_event,
		NULL, // placeholder for recv completion event
	};

	size_t waitCount = 2; // always 2 or 3

	if (isDatagram)
		m_overlappedSendToParams.setCount (sizeof (axl::io::SockAddr));

	bool isSendingSocket = false;

	m_ioThreadEvent.signal (); // do initial update of active events

	for (;;)
	{
		DWORD waitResult = ::WaitForMultipleObjects (waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent (err::getLastSystemErrorCode ());
			return;
		}

		// do as much as we can without lock

		while (!m_activeOverlappedReadList.isEmpty ())
		{
			OverlappedRead* read = *m_activeOverlappedReadList.getHead ();
			result = read->m_overlapped.m_completionEvent.wait (0);
			if (!result)
				break;

			dword_t actualSize;
			result = m_socket.m_socket.wsaGetOverlappedResult (&read->m_overlapped, &actualSize);
			if (!result)
			{
				err::Error error = err::getLastError ();
				ASSERT (error->m_guid == err::g_systemErrorGuid);

				if (error->m_code == WSAECONNRESET)
					setEvents (SocketEvent_Disconnected | SocketEvent_Reset);
				else
					setIoErrorEvent (error);

				return;
			}

			read->m_overlapped.m_completionEvent.reset ();
			m_activeOverlappedReadList.remove (read);
			OverlappedRecvParams* params = (OverlappedRecvParams*) (read + 1);

			if (actualSize == 0 && !isDatagram)
			{
				setEvents (SocketEvent_Disconnected);
				return;
			}

			// only the main read buffer must be lock-protected

			m_lock.lock ();

			if (isDatagram)
				addToReadBuffer (read->m_buffer, actualSize, &params->m_sockAddr, sizeof (params->m_sockAddr));
			else
				addToReadBuffer (read->m_buffer, actualSize);

			m_lock.unlock ();

			m_freeOverlappedReadList.insertHead (read);
		}

		if (sendOverlapped.m_completionEvent.wait (0))
		{
			ASSERT (isSendingSocket);

			dword_t actualSize;
			result = m_socket.m_socket.wsaGetOverlappedResult (&sendOverlapped, &actualSize);
			if (!result)
			{
				setIoErrorEvent ();
				return;
			}

			if (actualSize < m_overlappedWriteBlock.getCount () && !isDatagram) // shouldn't happen, actually (unless with a non-standard WSP)
				m_overlappedWriteBlock.remove (0, actualSize);
			else
				m_overlappedWriteBlock.clear ();

			sendOverlapped.m_completionEvent.reset ();
			isSendingSocket = false;
		}

		m_lock.lock ();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock ();
			break;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = baseEvents;

		isDatagram ?
			getNextWriteBlock (&m_overlappedWriteBlock) :
			getNextWriteBlock (&m_overlappedWriteBlock, &m_overlappedSendToParams);

		updateReadWriteBufferEvents ();

		// take snapshots before releasing the lock

		bool isReadBufferFull = m_readBuffer.isFull ();
		size_t readParallelism = m_readParallelism;
		size_t readBlockSize = m_readBlockSize;
		uint_t compatibilityFlags = m_options;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();

		if (!isSendingSocket && !m_overlappedWriteBlock.isEmpty ())
		{
			result = isDatagram ?
				m_socket.m_socket.wsaSendTo (
					m_overlappedWriteBlock,
					m_overlappedWriteBlock.getCount (),
					NULL,
					0,
					&((axl::io::SockAddr*) m_overlappedSendToParams.p ())->m_addr,
					&sendOverlapped
					) :
				m_socket.m_socket.wsaSend (
					m_overlappedWriteBlock,
					m_overlappedWriteBlock.getCount (),
					NULL,
					0,
					&sendOverlapped
					);

			if (!result)
			{
				setIoErrorEvent ();
				break;
			}

			isSendingSocket = true;
		}

		size_t activeReadCount = m_activeOverlappedReadList.getCount ();
		if (!isReadBufferFull && activeReadCount < readParallelism)
		{
			size_t newReadCount = readParallelism - activeReadCount;

			for (size_t i = 0; i < newReadCount; i++)
			{
				OverlappedRead* read = createOverlappedRead (sizeof (OverlappedRecvParams));
				OverlappedRecvParams* params = (OverlappedRecvParams*) (read + 1);

				result =
					read->m_buffer.setCount (readBlockSize) &&
					(isDatagram ?
						m_socket.m_socket.wsaRecvFrom (
							read->m_buffer,
							readBlockSize,
							NULL,
							&params->m_flags,
							&params->m_sockAddr,
							&params->m_sockAddrSize,
							&read->m_overlapped
							) :
						m_socket.m_socket.wsaRecv (
							read->m_buffer,
							readBlockSize,
							NULL,
							&params->m_flags,
							&read->m_overlapped
							));

				if (!result)
				{
					setIoErrorEvent ();
					return;
				}

				m_activeOverlappedReadList.insertTail (read);
			}
		}

		if (m_activeOverlappedReadList.isEmpty ())
		{
			waitCount = 2;
		}
		else
		{
			// wait-table may already hold correct value -- but there's no harm in writing it over

			axl::io::win::StdOverlapped* overlapped = &m_activeOverlappedReadList.getHead ()->m_overlapped;
			waitTable [2] = overlapped->m_completionEvent.m_event;
			waitCount = 3;
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
Socket::sendRecvLoop (
	uint_t baseEvents,
	bool isDatagram
	)
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
		m_activeEvents = baseEvents;

		readBlock.setCount (m_readBlockSize); // update read block size

		if (isDatagram)
		{
			while (canReadSocket && !m_readBuffer.isFull ())
			{
				axl::io::SockAddr sockAddr;
				socklen_t sockAddrSize = sizeof (sockAddr);

				ssize_t actualSize = ::recvfrom (
					m_socket.m_socket,
					readBlock,
					readBlock.getCount (),
					0,
					&sockAddr.m_addr,
					&sockAddrSize
					);

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
					addToReadBuffer (readBlock, actualSize, &sockAddr, sizeof (sockAddr));
				}
			}

			while (canWriteSocket)
			{
				getNextWriteBlock (&writeBlock, &writeParams);
				if (writeBlock.isEmpty ())
					break;

				axl::io::SockAddr* sockAddr = ((axl::io::SockAddr*) writeParams.p ());
				socklen_t sockAddrSize = axl::io::getSockAddrSize (&sockAddr->m_addr);
				size_t blockSize = writeBlock.getCount ();

				ssize_t actualSize = ::sendto (
					m_socket.m_socket,
					writeBlock,
					blockSize,
					0,
					&sockAddr->m_addr,
					sockAddrSize
					);

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
		else
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
					setEvents (m_socket.getError () ?
						SocketEvent_Disconnected | SocketEvent_Reset :
						SocketEvent_Disconnected
						);
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
