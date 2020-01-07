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
#include "jnc_io_SslSocket.h"
#include "jnc_io_SslLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	SslSocket,
	"io.SslSocket",
	g_sslLibGuid,
	SslLibCacheSlot_SslSocket,
	SslSocket,
	&SslSocket::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(SslSocket)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<SslSocket>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<SslSocket>)

	JNC_MAP_CONST_PROPERTY("m_address",     &SslSocket::getAddress)
	JNC_MAP_CONST_PROPERTY("m_peerAddress", &SslSocket::getPeerAddress)

	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize",   &SslSocket::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize",  &SslSocket::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &SslSocket::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options",         &SslSocket::setOptions)

	JNC_MAP_FUNCTION("open",         &SslSocket::open_0)
	JNC_MAP_OVERLOAD(&SslSocket::open_1)
	JNC_MAP_FUNCTION("close",        &SslSocket::close)
	JNC_MAP_FUNCTION("connect",      &SslSocket::connect)
	JNC_MAP_FUNCTION("listen",       &SslSocket::listen)
	JNC_MAP_FUNCTION("accept",       &SslSocket::accept)
	JNC_MAP_FUNCTION("unsuspend",    &SslSocket::unsuspend)
	JNC_MAP_FUNCTION("read",         &SslSocket::read)
	JNC_MAP_FUNCTION("write",        &SslSocket::write)
	JNC_MAP_FUNCTION("wait",         &SslSocket::wait)
	JNC_MAP_FUNCTION("cancelWait",   &SslSocket::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &SslSocket::blockingWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

SslSocket::SslSocket()
{
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_writeBufferSize = Def_WriteBufferSize;
	m_options = Def_Options,

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
	m_writeBuffer.setBufferSize(Def_WriteBufferSize);
}

bool
JNC_CDECL
SslSocket::open_0(uint16_t family)
{
	close();

	bool result = SocketBase::open(family, IPPROTO_TCP, NULL);
	if (!result)
		return false;

	m_ioThread.start();
	return true;
}

bool
JNC_CDECL
SslSocket::open_1(DataPtr addressPtr)
{
	close();

	SocketAddress* address = (SocketAddress*)addressPtr.m_p;

	bool result = SocketBase::open(address ? address->m_family : AF_INET, IPPROTO_TCP, address);
	if (!result)
		return false;

	m_ioThread.start();
	return true;
}

void
JNC_CDECL
SslSocket::close()
{
	if (!m_socket.isOpen())
		return;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread();
	m_lock.unlock();

	GcHeap* gcHeap = m_runtime->getGcHeap();
	gcHeap->enterWaitRegion();
	m_ioThread.waitAndClose();
	gcHeap->leaveWaitRegion();

	m_localAddress.m_family = jnc::io::AddressFamily_Undefined;

	SocketBase::close();
}

bool
JNC_CDECL
SslSocket::connect(DataPtr addressPtr)
{
	SocketAddress* address = (SocketAddress*)addressPtr.m_p;
	bool result = m_socket.connect(address->getSockAddr());
	if (!result)
		return false;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Connecting;
	wakeIoThread();
	m_lock.unlock();
	return result;
}

bool
JNC_CDECL
SslSocket::listen(size_t backLogLimit)
{
	bool result = m_socket.listen(backLogLimit);
	if (!result)
		return false;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Listening;
	wakeIoThread();
	m_lock.unlock();
	return true;
}

SslSocket*
JNC_CDECL
SslSocket::accept(
	DataPtr addressPtr,
	bool isSuspended
	)
{
	err::setError(err::SystemErrorCode_NotImplemented);
	return NULL;
}

void
SslSocket::ioThreadFunc()
{
	ASSERT(m_socket.isOpen());

	sleepIoThread();

	m_lock.lock();
	if (m_ioThreadFlags & IoThreadFlag_Closing)
	{
		m_lock.unlock();
	}
	else if (m_ioThreadFlags & IoThreadFlag_Connecting)
	{
		m_lock.unlock();

		bool result =
			connectLoop(SslSocketEvent_TcpConnected) &&
			sslHandshakeLoop();

		if (result)
			sendRecvLoop();
	}
	else if (m_ioThreadFlags & IoThreadFlag_Listening)
	{
		m_lock.unlock();
		acceptLoop(SslSocketEvent_IncomingTcpConnection);
	}
	else if (m_ioThreadFlags & IoThreadFlag_IncomingConnection)
	{
		m_lock.unlock();
		sendRecvLoop();
	}
	else
	{
		m_lock.unlock();
		ASSERT(false); // shouldn't normally happen
	}
}

bool
SslSocket::sslHandshakeLoop()
{
	return true;
}

void
SslSocket::sendRecvLoop()
{
}

//..............................................................................

} // namespace io
} // namespace jnc
