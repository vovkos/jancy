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
#include "jnc_io_SocketOptions.h"
#include "jnc_io_AsyncIoDevice.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE (Socket)

//..............................................................................

enum SocketEvent
{
	SocketEvent_IncomingConnection = 0x10,
	SocketEvent_Connected          = 0x20,
	SocketEvent_Disconnected       = 0x40,
	SocketEvent_Reset              = 0x80,
};

//..............................................................................

struct SocketHdr: IfaceHdr
{
	uint_t m_readParallelism;
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_writeBufferSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Socket: 
	public SocketHdr,
	public AsyncIoDevice
{
	friend class IoThread;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS (Socket)

protected:
	enum Def
	{
		Def_ReadParallelism = 4,
		Def_ReadBlockSize   = 4 * 1024,
		Def_ReadBufferSize  = 16 * 1024,
		Def_WriteBufferSize = 16 * 1024,
		Def_Options         = 0,
	};

	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			containerof (this, Socket, m_ioThread)->ioThreadFunc ();
		}
	};

	enum IoThreadFlag
	{
		IoThreadFlag_Tcp                   = 0x0010,
		IoThreadFlag_Ip6                   = 0x0020,
		IoThreadFlag_Connecting            = 0x0100,
		IoThreadFlag_Listening             = 0x0200,
		IoThreadFlag_Connected             = 0x0400,
		IoThreadFlag_WaitingTransmitBuffer = 0x0800,
#if (_JNC_OS_POSIX)
		IoThreadFlag_IncomingData          = 0x1000,
		IoThreadFlag_IncomingConnection    = 0x2000,
#endif
	};

	struct IncomingConnection: sl::ListLink
	{
		axl::io::Socket m_socket;
		axl::io::SockAddr m_address;
	};

protected:
	axl::io::Socket m_socket;
	IoThread m_ioThread;

	sl::BoxList <IncomingConnection> m_pendingIncomingConnectionList;
	sl::BoxList <IncomingConnection> m_freeIncomingConnectionList;
	size_t m_backLogLimit;

public:
	Socket ();

	~Socket ()
	{
		close ();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots (jnc::GcHeap* gcHeap)
	{
		AsyncIoDevice::markOpaqueGcRoots (gcHeap);
	}

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
	setReadParallelism (uint_t count)
	{
		return setReadParallelismImpl (&m_readParallelism, count ? count : Def_ReadParallelism);
	}

	bool
	JNC_CDECL
	setReadBlockSize (size_t size)
	{
		return setReadBlockSizeImpl (&m_readBlockSize, size ? size : Def_ReadBlockSize);
	}

	bool
	JNC_CDECL
	setReadBufferSize (size_t size)
	{
		return setReadBufferSizeImpl (&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	bool
	JNC_CDECL
	setWriteBufferSize (size_t size)
	{
		return setWriteBufferSizeImpl (&m_writeBufferSize, size ? size : Def_WriteBufferSize);
	}

	bool
	JNC_CDECL
	setOptions (uint_t flags);

	bool
	JNC_CDECL
	open_0 (
		uint16_t family,
		int protocol
		)
	{
		return openImpl (family, protocol, NULL);
	}

	bool
	JNC_CDECL
	open_1 (
		int protocol,
		DataPtr addressPtr
		)
	{
		const SocketAddress* address = (const SocketAddress*) addressPtr.m_p;
		return openImpl (address ? address->m_family : AddressFamily_Ip4, protocol, address);
	}

	void
	JNC_CDECL
	close ();

	bool
	JNC_CDECL
	connect (DataPtr addressPtr);

	bool
	JNC_CDECL
	listen (size_t backLogLimit);

	Socket*
	JNC_CDECL
	accept (DataPtr addressPtr);

	size_t
	JNC_CDECL
	read (
		DataPtr ptr,
		size_t size
		)
	{
		return bufferedRead (ptr, size);
	}

	size_t
	JNC_CDECL
	write (
		DataPtr ptr,
		size_t size
		)
	{
		return bufferedWrite (ptr, size);
	}

	size_t
	JNC_CDECL
	readDatagram (
		DataPtr ptr,
		size_t size,
		DataPtr addressPtr
		);

	size_t
	JNC_CDECL
	writeDatagram (
		DataPtr ptr,
		size_t size,
		DataPtr addressPtr
		);

	handle_t 
	JNC_CDECL
	wait (
		uint_t eventMask,
		FunctionPtr handlerPtr
		)
	{
		return AsyncIoDevice::wait (eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait (handle_t handle)
	{
		return AsyncIoDevice::cancelWait (handle);
	}

	uint_t
	JNC_CDECL
	blockingWait (
		uint_t eventMask,
		uint_t timeout
		)
	{
		return AsyncIoDevice::blockingWait (eventMask, timeout);
	}

protected:
	bool
	openImpl (
		uint16_t family,
		int protocol,
		const SocketAddress* address
		);

	void
	ioThreadFunc ();

	bool
	connectLoop ();

	void
	acceptLoop ();

	bool
	sendRecvLoop ();
};

//..............................................................................

} // namespace io
} // namespace jnc
