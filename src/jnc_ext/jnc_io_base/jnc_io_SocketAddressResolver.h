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

JNC_DECLARE_TYPE (SocketAddressResolverEventParams)
JNC_DECLARE_OPAQUE_CLASS_TYPE (SocketAddressResolver)

//..............................................................................

enum SocketAddressResolverEventCode
{
	SocketAddressResolverEventCode_ResolveCompleted = 0,
	SocketAddressResolverEventCode_ResolveCancelled,
	SocketAddressResolverEventCode_ResolveError,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketAddressResolverEventParams
{
	JNC_DECLARE_TYPE_STATIC_METHODS (SocketAddressResolverEventParams)

	SocketAddressResolverEventCode m_eventCode;
	uint_t m_syncId;
	DataPtr m_addressPtr;
	size_t m_addressCount;
	DataPtr m_errorPtr;
};

//..............................................................................

class SocketAddressResolver: public IfaceHdr
{
	friend class IoThread;

protected:
	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			containerof (this, SocketAddressResolver, m_ioThread)->ioThreadFunc ();
		}
	};

	enum IoFlag
	{
		IoFlag_Running = 0x0001,
		IoFlag_Closing = 0x0010,
	};

	struct Req: sl::ListLink
	{
		uint_t m_syncId;
		sl::String m_name;
		uint_t m_port;
		uint_t m_addrFamily;
	};

protected:
	uint_t m_syncId;

	ClassBox <Multicast> m_onSocketAddressResolverEvent;

protected:
	Runtime* m_runtime;

	sys::Lock m_ioLock;
	uint_t m_ioFlags;
	IoThread m_ioThread;
	sl::StdList <Req> m_reqList;

#if (_JNC_OS_WIN)
	sys::Event m_ioThreadEvent;
#else
	axl::io::psx::Pipe m_selfPipe; // for self-pipe trick
#endif

public:
	SocketAddressResolver ();

	~SocketAddressResolver ()
	{
		cancelAll ();
		stopIoThread ();
	}

	bool
	JNC_CDECL
	resolve (
		DataPtr namePtr,
		uint16_t addrFamily
		);

	bool
	JNC_CDECL
	cancel (uint_t syncId);

	void
	JNC_CDECL
	cancelAll ();

protected:
	void
	fireSocketAddressResolverEvent (
		SocketAddressResolverEventCode eventCode,
		uint_t syncId,
		const axl::io::SockAddr* addressTable = NULL,
		size_t addressCount = 0,
		const err::ErrorHdr* error = NULL
		);

	void
	fireSocketAddressResolverEvent (
		SocketAddressResolverEventCode eventCode,
		uint_t syncId,
		const err::ErrorHdr* error
		)
	{
		fireSocketAddressResolverEvent (eventCode, syncId, NULL, 0, error);
	}

	void
	processReq (Req* req);

	void
	stopIoThread ();

	void
	ioThreadFunc ();

	void
	wakeIoThread ();
};

//..............................................................................

} // namespace io
} // namespace jnc
