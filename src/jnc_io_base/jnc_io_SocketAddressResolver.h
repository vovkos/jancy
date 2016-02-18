#pragma once

#include "jnc_io_SocketAddress.h"

namespace jnc {
namespace io {

//.............................................................................

enum SocketAddressResolverEventKind
{
	SocketAddressResolverEventKind_ResolveCompleted = 0,
	SocketAddressResolverEventKind_ResolveCancelled,
	SocketAddressResolverEventKind_ResolveError,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketAddressResolverEventParams
{
	JNC_BEGIN_TYPE_MAP ("io.SocketAddressResolverEventParams", g_ioLibCacheSlot, IoLibTypeCacheSlot_SocketAddressResolverEventParams)
	JNC_END_TYPE_MAP ()

	SocketAddressResolverEventKind m_eventKind;
	uint_t m_syncId;
	rt::DataPtr m_addressPtr;
	size_t m_addressCount;
	rt::DataPtr m_errorPtr;
};

//.............................................................................

class SocketAddressResolver: public jnc::rt::IfaceHdr
{
	friend class IoThread;

public:
	JNC_OPAQUE_CLASS_TYPE_INFO (SocketAddressResolver, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("io.SocketAddressResolver", g_ioLibCacheSlot, IoLibTypeCacheSlot_SocketAddressResolver)
		JNC_MAP_CONSTRUCTOR (&sl::construct <SocketAddressResolver>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <SocketAddressResolver>)
		JNC_MAP_FUNCTION ("resolve",   &SocketAddressResolver::resolve)
		JNC_MAP_FUNCTION ("cancel",    &SocketAddressResolver::cancel)
		JNC_MAP_FUNCTION ("cancelAll", &SocketAddressResolver::cancelAll)
	JNC_END_CLASS_TYPE_MAP ()

protected:
	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			AXL_CONTAINING_RECORD (this, SocketAddressResolver, m_ioThread)->ioThreadFunc ();
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

	rt::ClassBox <rt::Multicast> m_onSocketAddressResolverEvent;

protected:
	rt::Runtime* m_runtime;

	sys::Lock m_ioLock;
	uint_t m_ioFlags;
	IoThread m_ioThread;
	sl::StdList <Req> m_reqList;

#if (_AXL_ENV == AXL_ENV_WIN)
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
	AXL_CDECL
	resolve (
		rt::DataPtr namePtr,
		uint16_t addrFamily
		);

	bool
	AXL_CDECL
	cancel (uint_t syncId);

	void
	AXL_CDECL
	cancelAll ();

protected:
	void
	fireSocketAddressResolverEvent (
		SocketAddressResolverEventKind eventKind,
		uint_t syncId,
		const axl::io::SockAddr* addressTable = NULL,
		size_t addressCount = 0,
		const err::ErrorHdr* error = NULL
		);

	void
	fireSocketAddressResolverEvent (
		SocketAddressResolverEventKind eventKind,
		uint_t syncId,
		const err::ErrorHdr* error
		)
	{
		fireSocketAddressResolverEvent (eventKind, syncId, NULL, 0, error);
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

//.............................................................................

} // namespace io
} // namespace jnc
