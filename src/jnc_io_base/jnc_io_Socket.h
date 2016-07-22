#pragma once

#include "jnc_io_SocketAddress.h"

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (SocketEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (Socket)

//.............................................................................

enum SocketEventKind
{
	SocketEventKind_ConnectCompleted = 0,
	SocketEventKind_ConnectCancelled,
	SocketEventKind_ConnectError,
	SocketEventKind_Disconnected,
	SocketEventKind_IncomingData,
	SocketEventKind_IncomingConnection,
	SocketEventKind_TransmitBufferReady,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum SocketDisconnectEventFlag
{
	SocketDisconnectEventFlag_Reset = 0x01,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (SocketEventParams)

	SocketEventKind m_eventKind;
	uint_t m_syncId;
	uint_t m_flags;
	DataPtr m_errorPtr;
};

//.............................................................................

enum SocketCloseKind
{
	SocketCloseKind_Reset = 0,
	SocketCloseKind_Graceful,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum SocketOpenFlag
{
	SocketOpenFlag_Raw          = 0x01,
	SocketOpenFlag_Asynchronous = 0x02,
	SocketOpenFlag_ReuseAddress = 0x04,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Socket: public IfaceHdr
{
	friend class IoThread;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS (Socket)

protected:
	class IoThread: public sys::ThreadImpl <IoThread>
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
		IoFlag_Asynchronous          = 0x0001,
		IoFlag_Udp                   = 0x0002,
		IoFlag_Connected             = 0x0004,
		IoFlag_Closing               = 0x0010,
		IoFlag_Connecting            = 0x0020,
		IoFlag_Listening             = 0x0040,
		IoFlag_WaitingTransmitBuffer = 0x0080,

#if (_AXL_ENV == AXL_ENV_POSIX)
		IoFlag_IncomingData          = 0x0100,
		IoFlag_IncomingConnection    = 0x0200,
#endif
	};

protected:
	bool m_isOpen;
	uint_t m_syncId;

	ClassBox <Multicast> m_onSocketEvent;

protected:
	Runtime* m_runtime;

	axl::io::Socket m_socket;

	sys::Lock m_ioLock;
	volatile uint_t m_ioFlags;
	IoThread m_ioThread;

#if (_AXL_ENV == AXL_ENV_WIN)
	sys::Event m_ioThreadEvent;
#else
	axl::io::psx::Pipe m_selfPipe; // for self-pipe trick
#endif

	int m_family;

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

	bool
	AXL_CDECL 
	isRawHdrIncluded ();	

	bool
	AXL_CDECL 
	setRawHdrIncluded (bool isIncluded);	

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
		uint16_t family,
		int protocol,
		uint_t flags
		)
	{
		return openImpl (family, protocol, NULL, flags);
	}

	bool
	AXL_CDECL
	open_1 (
		int protocol,
		DataPtr addressPtr,
		uint_t flags
		)
	{
		const SocketAddress* address = (const SocketAddress*) addressPtr.m_p;
		return openImpl (address ? address->m_family : AddressFamily_Ip4, protocol, address, flags);
	}

	void
	AXL_CDECL
	close ();

	bool
	AXL_CDECL
	connect (DataPtr addressPtr);

	bool
	AXL_CDECL
	listen (size_t backLog);

	Socket*
	AXL_CDECL
	accept (DataPtr addressPtr);

	size_t
	AXL_CDECL
	send (
		DataPtr ptr,
		size_t size
		);

	size_t
	AXL_CDECL
	recv (
		DataPtr ptr,
		size_t size
		);

	size_t
	AXL_CDECL
	sendTo (
		DataPtr ptr,
		size_t size,
		DataPtr addressPtr
		);

	size_t
	AXL_CDECL
	recvFrom (
		DataPtr ptr,
		size_t size,
		DataPtr addressPtr
		);

	void
	AXL_CDECL
	firePendingEvents ();

protected:
	bool
	openImpl (
		uint16_t family,
		int protocol,
		const SocketAddress* address,
		uint_t flags
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
	sendRecvLoop ();

	size_t
	postSend (
		size_t size,
		size_t result
		);
};

//.............................................................................

} // namespace io
} // namespace jnc
