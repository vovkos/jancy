#pragma once

#include "jnc_io_IoLibGlobals.h"
#include "jnc_io_SocketAddress.h"

namespace jnc {
namespace io {

//.............................................................................

enum SocketEventKind
{
	SocketEventKind_ConnectCompleted = 0,
	SocketEventKind_ConnectCancelled,
	SocketEventKind_ConnectError,
	SocketEventKind_Disconnected,
	SocketEventKind_IncomingData,
	SocketEventKind_IncomingConnection,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum SocketDisconnectEventFlag
{
	SocketDisconnectEventFlag_Reset = 0x01,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketEventParams
{
	JNC_BEGIN_TYPE_MAP ("io.SocketEventParams", g_ioLibCacheSlot, IoLibTypeCacheSlot_SocketEventParams)
	JNC_END_TYPE_MAP ()

	SocketEventKind m_eventKind;
	uint_t m_syncId;
	uint_t m_flags;
	jnc::rt::DataPtr m_errorPtr;
};

//.............................................................................

enum SocketCloseKind
{
	SocketCloseKind_Reset = 0,
	SocketCloseKind_Graceful,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Socket: public rt::IfaceHdr
{
	friend class IoThread;

public:
	JNC_OPAQUE_CLASS_TYPE_INFO (Socket, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("io.Socket", g_ioLibCacheSlot, IoLibTypeCacheSlot_Socket)
		JNC_MAP_CONSTRUCTOR (&sl::construct <Socket>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <Socket>)
		JNC_MAP_CONST_PROPERTY ("m_address",      &Socket::getAddress)
		JNC_MAP_CONST_PROPERTY ("m_peerAddress",  &Socket::getPeerAddress)
		JNC_MAP_PROPERTY ("m_isBroadcastEnabled", &Socket::isBroadcastEnabled, &Socket::setBroadcastEnabled)
		JNC_MAP_PROPERTY ("m_isNagleEnabled",     &Socket::isNagleEnabled, &Socket::setNagleEnabled)
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
	JNC_END_CLASS_TYPE_MAP ()

protected:
	class IoThread: public mt::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			AXL_CONTAINING_RECORD (this, Socket, m_ioThread)->ioThreadFunc ();
		}
	};

	enum IoFlag
	{
		IoFlag_Udp        = 0x0001,
		IoFlag_Connected  = 0x0002,
		IoFlag_Closing    = 0x0010,
		IoFlag_Connecting = 0x0020,
		IoFlag_Listening  = 0x0040,

#if (_AXL_ENV == AXL_ENV_POSIX)
		IoFlag_IncomingData       = 0x0100,
		IoFlag_IncomingConnection = 0x0200,
#endif
	};

protected:
	bool m_isOpen;
	uint_t m_syncId;

	jnc::rt::ClassBox <jnc::rt::Multicast> m_onSocketEvent;

protected:
	jnc::rt::Runtime* m_runtime;

	axl::io::Socket m_socket;

	mt::Lock m_ioLock;
	volatile uint_t m_ioFlags;
	IoThread m_ioThread;

#if (_AXL_ENV == AXL_ENV_WIN)
	mt::Event m_ioThreadEvent;
#else
	axl::io::psx::Pipe m_selfPipe; // for self-pipe trick
#endif

public:
	Socket ();

	~Socket ()
	{
		close ();
	}

	bool
	AXL_CDECL 
	isBroadcastEnabled ();	

	bool
	AXL_CDECL 
	setBroadcastEnabled (bool isEnabled);	

	bool
	AXL_CDECL 
	isNagleEnabled ();	

	bool
	AXL_CDECL 
	setNagleEnabled (bool isEnabled);	

	SocketCloseKind
	AXL_CDECL 
	getCloseKind ();	

	bool
	AXL_CDECL 
	setCloseKind (SocketCloseKind closeKind);	

	static
	SocketAddress
	AXL_CDECL
	getAddress (Socket* self);

	static
	SocketAddress
	AXL_CDECL
	getPeerAddress (Socket* self);

	bool
	AXL_CDECL
	open_0 (
		int protocol,
		uint16_t family
		)
	{
		return open (protocol, family, NULL);
	}

	bool
	AXL_CDECL
	open_1 (
		int protocol,
		jnc::rt::DataPtr addressPtr
		)
	{
		const SocketAddress* address = (const SocketAddress*) addressPtr.m_p;
		return open (protocol, address ? address->m_family : AddressFamily_Ip4, address);
	}

	void
	AXL_CDECL
	close ();

	bool
	AXL_CDECL
	connect (
		jnc::rt::DataPtr addressPtr,
		bool isSync
		);

	bool
	AXL_CDECL
	listen (size_t backLog);

	Socket*
	AXL_CDECL
	accept (jnc::rt::DataPtr addressPtr);

	size_t
	AXL_CDECL
	send (
		jnc::rt::DataPtr ptr,
		size_t size
		);

	size_t
	AXL_CDECL
	recv (
		jnc::rt::DataPtr ptr,
		size_t size
		);

	size_t
	AXL_CDECL
	sendTo (
		jnc::rt::DataPtr ptr,
		size_t size,
		jnc::rt::DataPtr addressPtr
		);

	size_t
	AXL_CDECL
	recvFrom (
		jnc::rt::DataPtr ptr,
		size_t size,
		jnc::rt::DataPtr addressPtr
		);

	void
	AXL_CDECL
	firePendingEvents ();

protected:
	bool
	open (
		int protocol,
		uint16_t family,
		const SocketAddress* address
		);

	void
	fireSocketEvent (
		SocketEventKind eventKind,
		uint_t flags = 0,
		const err::ErrorHdr* error = NULL
		);

	void
	ioThreadFunc ();

	void
	wakeIoThread ();

	bool
	connectLoop ();

	void
	acceptLoop ();

	bool
	recvLoop ();
};

//.............................................................................

} // namespace io
} // namespace jnc
