#include "pch.h"
#include "jnc_io_SocketAddressResolver.h"

namespace jnc {
namespace io {

//.............................................................................

SocketAddressResolver::SocketAddressResolver ()
{
	m_runtime = rt::getCurrentThreadRuntime ();
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

	rt::enterWaitRegion (m_runtime);
	m_ioThread.waitAndClose ();
	rt::leaveWaitRegion (m_runtime);
}

void
SocketAddressResolver::wakeIoThread ()
{
#if (_AXL_ENV == AXL_ENV_WIN)
	m_ioThreadEvent.signal ();
#else
	m_selfPipe.write (" ", 1);
#endif
}

void
SocketAddressResolver::fireSocketAddressResolverEvent (
	SocketAddressResolverEventKind eventKind,
	uint_t syncId,
	const axl::io::SockAddr* addressTable,
	size_t addressCount,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	rt::DataPtr paramsPtr = rt::createData <SocketAddressResolverEventParams> (m_runtime);
	SocketAddressResolverEventParams* params = (SocketAddressResolverEventParams*) paramsPtr.m_p;
	params->m_eventKind = eventKind;
	params->m_syncId = syncId;
		
	if (addressCount)
		params->m_addressPtr = rt::memDup (addressTable, sizeof (axl::io::SockAddr) * addressCount);

	if (error)
		params->m_errorPtr = rt::memDup (error, error->m_size);

	rt::callMulticast (m_onSocketAddressResolverEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

bool
AXL_CDECL
SocketAddressResolver::resolve (
	rt::DataPtr namePtr,
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

		fireSocketAddressResolverEvent (SocketAddressResolverEventKind_ResolveCompleted, syncId, &sockAddr, 1);
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
				SocketAddressResolverEventKind_ResolveError,
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

#if (_AXL_ENV == AXL_ENV_POSIX)
		m_selfPipe.create ();
#endif
		m_ioThread.start ();
	}

	m_ioLock.unlock ();

	return true;
}

bool
AXL_CDECL
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

	fireSocketAddressResolverEvent (SocketAddressResolverEventKind_ResolveCancelled, syncId);

	return true;
}

void
AXL_CDECL
SocketAddressResolver::cancelAll ()
{
	m_ioLock.lock ();

	sl::StdList <Req> reqList;
	reqList.takeOver (&m_reqList);

	m_ioLock.unlock ();

	sl::Iterator <Req> it = m_reqList.getHead ();
	for (; it; it++)
		fireSocketAddressResolverEvent (SocketAddressResolverEventKind_ResolveCancelled, it->m_syncId);
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

#if (_AXL_ENV == AXL_ENV_WIN)
		m_ioThreadEvent.wait ();
#elif (_AXL_ENV == AXL_ENV_POSIX)
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
			SocketAddressResolverEventKind_ResolveError,
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
		SocketAddressResolverEventKind_ResolveCompleted, 
		req->m_syncId, 
		addrArray,
		count
		);
}

//.............................................................................

} // namespace io
} // namespace jnc
