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

Socket::Socket ()
{
	m_readParallelism = Def_ReadParallelism;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options;

	m_readBuffer.setBufferSize (Def_ReadBufferSize);
	m_writeBuffer.setBufferSize (Def_WriteBufferSize);

#if (_AXL_OS_WIN)
	m_overlappedIo = NULL;
#endif
}

bool
Socket::openImpl (
	uint16_t family,
	int protocol,
	const SocketAddress* address
	)
{
	close ();

	bool result = SocketBase::open (family, protocol, address);
	if (!result)
		return false;

	if (m_ioThreadFlags & IoThreadFlag_Datagram)
		wakeIoThread ();

#if (_AXL_OS_WIN)
	ASSERT (!m_overlappedIo);
	m_overlappedIo = AXL_MEM_NEW (OverlappedIo);
#endif

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

	sl::Iterator <IncomingConnection> it = m_pendingIncomingConnectionList.getHead ();
	for (; it; it++)
		it->m_socket.close ();

	m_incomingConnectionPool.put (&m_pendingIncomingConnectionList);

#if (_AXL_OS_WIN)
	if (m_overlappedIo)
	{
		AXL_MEM_DELETE (m_overlappedIo);
		m_overlappedIo = NULL;
	}
#endif
}

bool
JNC_CDECL
Socket::connect (DataPtr addressPtr)
{
	SocketAddress* address = (SocketAddress*) addressPtr.m_p;
	bool result = m_socket.connect (address->getSockAddr ());
	if (!result)
		return false;

	m_lock.lock ();
	m_ioThreadFlags |= IoThreadFlag_Connecting;
	wakeIoThread ();
	m_lock.unlock ();
	return result;
}

bool
JNC_CDECL
Socket::listen (size_t backLogLimit)
{
	bool result = m_socket.listen (backLogLimit);
	if (!result)
		return false;

	m_lock.lock ();
	m_ioThreadFlags |= IoThreadFlag_Listening;
	wakeIoThread ();
	m_lock.unlock ();
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
	sl::takeOver (&connectionSocket->m_socket.m_socket, &incomingConnection->m_socket.m_socket);
	connectionSocket->setOptions (m_options);
	connectionSocket->AsyncIoDevice::open ();
	connectionSocket->m_ioThreadFlags =	IoThreadFlag_IncomingConnection;

#if (_AXL_OS_WIN)
	connectionSocket->m_overlappedIo = AXL_MEM_NEW (OverlappedIo);
#endif

	if (address)
		address->setSockAddr (incomingConnection->m_sockAddr);

	m_incomingConnectionPool.put (incomingConnection);

	if (m_pendingIncomingConnectionList.isEmpty ())
		m_activeEvents &= ~SocketEvent_IncomingConnection;

	m_lock.unlock ();

	bool result = connectionSocket->m_socket.setBlockingMode (false); // not guaranteed to be propagated across accept calls
	if (!result)
		return NULL;

	connectionSocket->wakeIoThread ();
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

	SocketAddress* address = (SocketAddress*) addressPtr.m_p;
	if (address->m_family != m_family)
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidParameter));
		return -1;
	}

	axl::io::SockAddr sockAddr = address->getSockAddr ();
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
				m_lock.unlock ();
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
					return;

				m_lock.lock ();
				IncomingConnection* incomingConnection = m_incomingConnectionPool.get ();
				incomingConnection->m_sockAddr = sockAddr;
				sl::takeOver (&incomingConnection->m_socket.m_socket, &socket.m_socket);
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
	ASSERT (m_socket.isOpen () && m_overlappedIo);

	bool result;

	HANDLE waitTable [3] =
	{
		m_ioThreadEvent.m_event,
		m_overlappedIo->m_sendOverlapped.m_completionEvent.m_event,
		NULL, // placeholder for recv completion event
	};

	size_t waitCount = 2; // always 2 or 3

	bool isSendingSocket = false;

	m_ioThreadEvent.signal (); // do initial update of active events

	for (;;)
	{
		dword_t waitResult = ::WaitForMultipleObjects (waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent (err::getLastSystemErrorCode ());
			break;
		}

		// do as much as we can without lock

		while (!m_overlappedIo->m_activeOverlappedRecvList.isEmpty ())
		{
			OverlappedRecv* recv = *m_overlappedIo->m_activeOverlappedRecvList.getHead ();
			result = recv->m_overlapped.m_completionEvent.wait (0);
			if (!result)
				break;

			dword_t actualSize;
			result = m_socket.m_socket.wsaGetOverlappedResult (&recv->m_overlapped, &actualSize);
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

			m_overlappedIo->m_activeOverlappedRecvList.remove (recv);

			if (actualSize == 0 && !isDatagram)
			{
				setEvents (SocketEvent_Disconnected);
				return;
			}

			// only the main read buffer must be lock-protected

			m_lock.lock ();

			if (isDatagram)
				addToReadBuffer (recv->m_buffer, actualSize, &recv->m_sockAddr, sizeof (recv->m_sockAddr));
			else
				addToReadBuffer (recv->m_buffer, actualSize);

			m_lock.unlock ();

			recv->m_overlapped.m_completionEvent.reset ();
			m_overlappedIo->m_overlappedRecvPool.put (recv);
		}

		if (isSendingSocket && m_overlappedIo->m_sendOverlapped.m_completionEvent.wait (0))
		{
			dword_t actualSize;
			result = m_socket.m_socket.wsaGetOverlappedResult (&m_overlappedIo->m_sendOverlapped, &actualSize);
			if (!result)
			{
				setIoErrorEvent ();
				break;
			}

			if (actualSize < m_overlappedIo->m_sendBlock.getCount () && !isDatagram) // shouldn't happen, actually (unless with a non-standard WSP)
				m_overlappedIo->m_sendBlock.remove (0, actualSize);
			else
				m_overlappedIo->m_sendBlock.clear ();

			m_overlappedIo->m_sendOverlapped.m_completionEvent.reset ();
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
			getNextWriteBlock (&m_overlappedIo->m_sendBlock, &m_overlappedIo->m_sendToParams) :
			getNextWriteBlock (&m_overlappedIo->m_sendBlock);

		updateReadWriteBufferEvents ();

		// take snapshots before releasing the lock

		bool isReadBufferFull = m_readBuffer.isFull ();
		size_t readParallelism = m_readParallelism;
		size_t readBlockSize = m_readBlockSize;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l ();
		else
			m_lock.unlock ();

		if (!isSendingSocket && !m_overlappedIo->m_sendBlock.isEmpty ())
		{
			result = isDatagram ?
				m_socket.m_socket.wsaSendTo (
					m_overlappedIo->m_sendBlock,
					m_overlappedIo->m_sendBlock.getCount (),
					NULL,
					0,
					&((axl::io::SockAddr*) m_overlappedIo->m_sendToParams.p ())->m_addr,
					&m_overlappedIo->m_sendOverlapped
					) :
				m_socket.m_socket.wsaSend (
					m_overlappedIo->m_sendBlock,
					m_overlappedIo->m_sendBlock.getCount (),
					NULL,
					0,
					&m_overlappedIo->m_sendOverlapped
					);

			if (!result)
			{
				setIoErrorEvent ();
				break;
			}

			isSendingSocket = true;
		}

		size_t activeReadCount = m_overlappedIo->m_activeOverlappedRecvList.getCount ();
		if (!isReadBufferFull && activeReadCount < readParallelism)
		{
			size_t newReadCount = readParallelism - activeReadCount;
			for (size_t i = 0; i < newReadCount; i++)
			{
				OverlappedRecv* recv = m_overlappedIo->m_overlappedRecvPool.get ();

				if (isDatagram)
				{
					recv->m_sockAddrSize = sizeof (recv->m_sockAddr);

					result =
						recv->m_buffer.setCount (readBlockSize) &&
						m_socket.m_socket.wsaRecvFrom (
							recv->m_buffer,
							readBlockSize,
							NULL,
							&recv->m_flags,
							&recv->m_sockAddr,
							&recv->m_sockAddrSize,
							&recv->m_overlapped
							);
				}
				else
				{
					result =
						recv->m_buffer.setCount (readBlockSize) &&
						m_socket.m_socket.wsaRecv (
							recv->m_buffer,
							readBlockSize,
							NULL,
							&recv->m_flags,
							&recv->m_overlapped
							);
				}

				if (!result)
				{
					setIoErrorEvent ();
					return;
				}

				m_overlappedIo->m_activeOverlappedRecvList.insertTail (recv);
			}
		}

		if (m_overlappedIo->m_activeOverlappedRecvList.isEmpty ())
		{
			waitCount = 2;
		}
		else
		{
			// wait-table may already hold correct value -- but there's no harm in writing it over

			OverlappedRecv* recv = *m_overlappedIo->m_activeOverlappedRecvList.getHead ();
			waitTable [2] = recv->m_overlapped.m_completionEvent.m_event;
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

		while (canAcceptSocket)
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
				IncomingConnection* incomingConnection = m_incomingConnectionPool.get ();
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

				if (actualSize == -1)
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
				if (actualSize == -1)
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
					setEvents_l (m_socket.getError () ?
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
					else
					{
						setIoErrorEvent_l (err::Errno (errno));
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
