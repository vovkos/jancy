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

#pragma once

#include "jnc_io_SocketAddress.h"

namespace jnc {
namespace io {

JNC_DECLARE_TYPE (SocketEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (Socket)

//..............................................................................

enum SocketEventCode
{
	SocketEventCode_ConnectCompleted = 0,
	SocketEventCode_ConnectCancelled,
	SocketEventCode_ConnectError,
	SocketEventCode_Disconnected,
	SocketEventCode_IncomingData,
	SocketEventCode_IncomingConnection,
	SocketEventCode_TransmitBufferReady,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum SocketDisconnectEventFlag
{
	SocketDisconnectEventFlag_Reset = 0x01,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (SocketEventParams)

	SocketEventCode m_eventCode;
	uint_t m_syncId;
	uint_t m_flags;
	DataPtr m_errorPtr;
};

//..............................................................................

enum SocketCloseKind
{
	SocketCloseKind_Reset = 0,
	SocketCloseKind_Graceful,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum SocketOpenFlag
{
	SocketOpenFlag_Raw          = 0x01,
	SocketOpenFlag_Asynchronous = 0x02,
	SocketOpenFlag_ReuseAddress = 0x04,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
			containerof (this, Socket, m_ioThread)->ioThreadFunc ();
		}
	};

	enum IoFlag
	{
		IoFlag_Asynchronous          = 0x0001,
		IoFlag_Udp                   = 0x0002,
		IoFlag_Connected             = 0x0004,
		IoFlag_SocketEventDisabled   = 0x0008,
		IoFlag_Closing               = 0x0010,
		IoFlag_Connecting            = 0x0020,
		IoFlag_Listening             = 0x0040,
		IoFlag_WaitingTransmitBuffer = 0x0080,
#if (_JNC_OS_POSIX)
		IoFlag_IncomingData          = 0x0100,
		IoFlag_IncomingConnection    = 0x0200,
#endif
	};

	struct PendingEvent: sl::ListLink
	{
		SocketEventCode m_eventCode;
		uint_t m_flags;
		err::Error m_error;
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
	sl::StdList <PendingEvent> m_pendingEventList;

#if (_JNC_OS_WIN)
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
	JNC_CDECL
	isBroadcastEnabled ();

	bool
	JNC_CDECL
	setBroadcastEnabled (bool isEnabled);

	bool
	JNC_CDECL
	isNagleEnabled ();

	bool
	JNC_CDECL
	setNagleEnabled (bool isEnabled);

	bool
	JNC_CDECL
	isRawHdrIncluded ();

	bool
	JNC_CDECL
	setRawHdrIncluded (bool isIncluded);

	SocketCloseKind
	JNC_CDECL
	getCloseKind ();

	bool
	JNC_CDECL
	setCloseKind (SocketCloseKind closeKind);

	static
	SocketAddress
	JNC_CDECL
	getAddress (Socket* self);

	static
	SocketAddress
	JNC_CDECL
	getPeerAddress (Socket* self);

	bool
	JNC_CDECL
	open_0 (
		uint16_t family,
		int protocol,
		uint_t flags
		)
	{
		return openImpl (family, protocol, NULL, flags);
	}

	bool
	JNC_CDECL
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
	JNC_CDECL
	close ();

	bool
	JNC_CDECL
	connect (DataPtr addressPtr);

	bool
	JNC_CDECL
	listen (size_t backLog);

	Socket*
	JNC_CDECL
	accept (DataPtr addressPtr);

	size_t
	JNC_CDECL
	send (
		DataPtr ptr,
		size_t size
		);

	size_t
	JNC_CDECL
	recv (
		DataPtr ptr,
		size_t size
		);

	size_t
	JNC_CDECL
	sendTo (
		DataPtr ptr,
		size_t size,
		DataPtr addressPtr
		);

	size_t
	JNC_CDECL
	recvFrom (
		DataPtr ptr,
		size_t size,
		DataPtr addressPtr
		);

	bool
	JNC_CDECL
	isSocketEventEnabled ()
	{
		return !(m_ioFlags & IoFlag_SocketEventDisabled);
	}

	void
	JNC_CDECL
	setSocketEventEnabled (bool isEnabled);

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
		SocketEventCode eventCode,
		uint_t flags = 0,
		const err::ErrorHdr* error = NULL
		);

	void
	fireSocketEventImpl (
		SocketEventCode eventCode,
		uint_t flags,
		const err::ErrorHdr* error
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

//..............................................................................

} // namespace io
} // namespace jnc
