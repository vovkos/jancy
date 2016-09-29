#include "pch.h"
#include "jnc_io_SocketAddressResolver.h"
#include "jnc_io_IoLib.h"

namespace jnc {
namespace io {

//.............................................................................

JNC_DEFINE_TYPE (
	SocketAddressResolverEventParams,
	"io.SocketAddressResolverEventParams", 
	g_ioLibGuid, 
	IoLibCacheSlot_SocketAddressResolverEventParams
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (SocketAddressResolverEventParams)
JNC_END_TYPE_FUNCTION_MAP ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	SocketAddressResolver,
	"io.SocketAddressResolver", 
	g_ioLibGuid, 
	IoLibCacheSlot_SocketAddressResolver,
	SocketAddressResolver,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (SocketAddressResolver)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <SocketAddressResolver>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <SocketAddressResolver>)
	JNC_MAP_FUNCTION ("resolve",   &SocketAddressResolver::resolve)
	JNC_MAP_FUNCTION ("cancel",    &SocketAddressResolver::cancel)
	JNC_MAP_FUNCTION ("cancelAll", &SocketAddressResolver::cancelAll)
JNC_END_TYPE_FUNCTION_MAP ()

//.............................................................................

SocketAddressResolver::SocketAddressResolver ()
{
	m_runtime = getCurrentThreadRuntime ();
	m_ioFlags = 0;
	m_syncId = 0;
}

void
SocketAddressResolver::stopIoThread ()
{
	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	wakeIoThread ();
	m_ioLock.unlock ();

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	gcHeap->enterWaitRegion ();
	m_ioThread.waitAndClose ();
	gcHeap->leaveWaitRegion ();
}

void
SocketAddressResolver::wakeIoThread ()
{
#if (_JNC_OS_WIN)
	m_ioThreadEvent.signal ();
#else
	m_selfPipe.write (" ", 1);
#endif
}

void
SocketAddressResolver::fireSocketAddressResolverEvent (
	SocketAddressResolverEventCode eventCode,
	uint_t syncId,
	const axl::io::SockAddr* addressTable,
	size_t addressCount,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	DataPtr paramsPtr = createData <SocketAddressResolverEventParams> (m_runtime);
	SocketAddressResolverEventParams* params = (SocketAddressResolverEventParams*) paramsPtr.m_p;
	params->m_eventCode = eventCode;
	params->m_syncId = syncId;
		
	if (addressCount)
	{
		Type* addressType = SocketAddress::getType (m_runtime->getModule ());
		params->m_addressPtr = m_runtime->getGcHeap ()->allocateArray (addressType, addressCount);

		SocketAddress* dst = (SocketAddress*) params->m_addressPtr.m_p;
		const axl::io::SockAddr* src = addressTable;
		for (size_t i = 0; i < addressCount; i++, dst++, src++)
			dst->setSockAddr (*src);
	}

	if (error)
		params->m_errorPtr = memDup (error, error->m_size);

	callMulticast (m_onSocketAddressResolverEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

bool
JNC_CDECL
SocketAddressResolver::resolve (
	DataPtr namePtr,
	uint16_t addrFamily
	)
{
	const char* name = (const char*) namePtr.m_p;

	axl::io::SockAddr sockAddr;
	bool result = sockAddr.parse (name);
	if (result)
	{
		m_ioLock.lock ();
		uint_t syncId = m_syncId++;
		m_ioLock.unlock ();

		fireSocketAddressResolverEvent (SocketAddressResolverEventCode_ResolveCompleted, syncId, &sockAddr, 1);
		return true;
	}
	
	sl::String nameString;
	uint_t port;

	const char* p = strchr (name, ':');
	if (!p)
	{
		nameString = name;
		port = 0;
	}
	else
	{
		char* end;
		port = (ushort_t) strtol (p + 1, &end, 10);
		if (end == p)
		{
			m_ioLock.lock ();
			uint_t syncId = m_syncId++;
			m_ioLock.unlock ();

			fireSocketAddressResolverEvent (
				SocketAddressResolverEventCode_ResolveError,
				syncId,
				err::Error ("invalid port string")
				);
			return false;
		}

		nameString.copy (name, p - name);
	}

	Req* req = AXL_MEM_NEW (Req);
	req->m_name = nameString;
	req->m_port = port;
	req->m_addrFamily = addrFamily;
	m_ioLock.lock ();

	req->m_syncId = m_syncId++;
	m_reqList.insertTail (req);

	if (m_ioFlags & IoFlag_Running)
	{
		wakeIoThread ();
	}
	else
	{
		m_ioFlags |= IoFlag_Running;

#if (_JNC_OS_POSIX)
		m_selfPipe.create ();
#endif
		m_ioThread.start ();
	}

	m_ioLock.unlock ();

	return true;
}

bool
JNC_CDECL
SocketAddressResolver::cancel (uint_t syncId)
{
	m_ioLock.lock ();

	sl::Iterator <Req> it = m_reqList.getHead ();
	for (; it; it++)
	{
		if (it->m_syncId == syncId)
			break;
	}

	if (!it)
	{
		m_ioLock.unlock ();
		return false;
	}

	m_reqList.erase (it);
	m_ioLock.unlock ();

	fireSocketAddressResolverEvent (SocketAddressResolverEventCode_ResolveCancelled, syncId);

	return true;
}

void
JNC_CDECL
SocketAddressResolver::cancelAll ()
{
	m_ioLock.lock ();

	sl::StdList <Req> reqList;
	reqList.takeOver (&m_reqList);

	m_ioLock.unlock ();

	sl::Iterator <Req> it = m_reqList.getHead ();
	for (; it; it++)
		fireSocketAddressResolverEvent (SocketAddressResolverEventCode_ResolveCancelled, it->m_syncId);
}

void
SocketAddressResolver::ioThreadFunc ()
{
	for (;;)
	{
		m_ioLock.lock ();
		while (!m_reqList.isEmpty ())
		{
			if (m_ioFlags & IoFlag_Closing)
				break;

			Req* req = m_reqList.removeHead ();
			m_ioLock.unlock ();

			processReq (req);
			AXL_MEM_DELETE (req);

			m_ioLock.lock ();
		}

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioFlags &= ~IoFlag_Running;
			m_ioLock.unlock ();
			break;
		}

		m_ioLock.unlock ();

#if (_JNC_OS_WIN)
		m_ioThreadEvent.wait ();
#elif (_JNC_OS_POSIX)
		char buffer [256];
		m_selfPipe.read (buffer, sizeof (buffer));
#endif
	}

	cancelAll ();
}

void
SocketAddressResolver::processReq (Req* req)
{
	sl::Array <axl::io::SockAddr> addrArray;
	bool result = axl::io::resolveHostName (&addrArray, req->m_name, req->m_addrFamily);
	if (!result)
	{
		fireSocketAddressResolverEvent (
			SocketAddressResolverEventCode_ResolveError,
			req->m_syncId,
			err::getLastError ()
			);

		return;
	}

	uint_t port = sl::swapByteOrder16 (req->m_port);

	size_t count = addrArray.getCount ();
	for (size_t i = 0; i < count; i++)
		addrArray [i].m_addr_ip4.sin_port = port;

	fireSocketAddressResolverEvent (
		SocketAddressResolverEventCode_ResolveCompleted, 
		req->m_syncId, 
		addrArray,
		count
		);
}

//.............................................................................

} // namespace io
} // namespace jnc
