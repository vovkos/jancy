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
#include "jnc_io_SocketAddressResolver.h"
#include "jnc_io_IoLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	SocketAddressResolver,
	"io.SocketAddressResolver",
	g_ioLibGuid,
	IoLibCacheSlot_SocketAddressResolver,
	SocketAddressResolver,
	&SocketAddressResolver::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (SocketAddressResolver)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <SocketAddressResolver>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <SocketAddressResolver>)
	JNC_MAP_FUNCTION ("resolve",   &SocketAddressResolver::resolve)
	JNC_MAP_FUNCTION ("cancel",    &SocketAddressResolver::cancel)
	JNC_MAP_FUNCTION ("cancelAll", &SocketAddressResolver::cancelAll)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

SocketAddressResolver::SocketAddressResolver ()
{
	m_runtime = getCurrentThreadRuntime ();
	ASSERT (m_runtime);
}

SocketAddressResolver::~SocketAddressResolver ()
{
	cancelAll ();
	stopIoThread ();
}

void
JNC_CDECL
SocketAddressResolver::markOpaqueGcRoots (jnc::GcHeap* gcHeap)
{
	sl::Iterator <Req> it = m_pendingReqList.getHead ();
	for (; it; it++)
		if (it->m_completionFuncPtr.m_closure)
			gcHeap->markClass (it->m_completionFuncPtr.m_closure->m_box);

	it = m_activeReqList.getHead ();
	for (; it; it++)
		if (it->m_completionFuncPtr.m_closure)
			gcHeap->markClass (it->m_completionFuncPtr.m_closure->m_box);
}

void
SocketAddressResolver::stopIoThread ()
{
	m_lock.lock ();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread ();
	m_lock.unlock ();

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

uintptr_t
JNC_CDECL
SocketAddressResolver::resolve (
	DataPtr namePtr,
	uint16_t addrFamily,
	FunctionPtr completionFuncPtr
	)
{
	const char* name = (const char*) namePtr.m_p;

	axl::io::SockAddr sockAddr;
	bool result = sockAddr.parse (name);
	if (result)
	{
		callCompletionFunc (completionFuncPtr, &sockAddr, 1);
		return 0;
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
			callCompletionFunc (completionFuncPtr, "invalid port string");
			return -1;
		}

		nameString.copy (name, p - name);
	}

	Req* req = AXL_MEM_NEW (Req);
	req->m_name = nameString;
	req->m_port = port;
	req->m_addrFamily = addrFamily;
	req->m_completionFuncPtr = completionFuncPtr;

	m_lock.lock ();

	uintptr_t id = m_reqMap.add (req);
	m_pendingReqList.insertTail (req);
	req->m_id = id;

	if (m_ioThreadFlags & IoThreadFlag_Running)
	{
		wakeIoThread ();
	}
	else
	{
		m_ioThreadFlags |= IoThreadFlag_Running;

#if (_JNC_OS_POSIX)
		m_selfPipe.create ();
#endif
		m_ioThread.start ();
	}

	m_lock.unlock ();

	return id;
}

bool
JNC_CDECL
SocketAddressResolver::cancel (uintptr_t id)
{
	m_lock.lock ();

	Req* req = m_reqMap.findValue (id, NULL);
	if (!req)
	{
		m_lock.unlock ();
		return false;
	}

	ASSERT (req->m_id == id);
	m_reqMap.eraseKey (req->m_id);
	m_activeReqList.insertTail (req);
	m_lock.unlock ();

	callCompletionFunc (req->m_completionFuncPtr, err::SystemErrorCode_Cancelled); 
		
	m_lock.lock ();
	m_activeReqList.erase (req);
	m_lock.unlock ();

	return true;
}

void
JNC_CDECL
SocketAddressResolver::cancelAll ()
{
	err::Error error (err::SystemErrorCode_Cancelled);

	m_lock.lock ();

	while (!m_activeReqList.isEmpty ())
	{
		Req* req = m_pendingReqList.removeTail ();
		m_reqMap.eraseKey (req->m_id);
		m_activeReqList.insertTail (req);
		m_lock.unlock ();

		callCompletionFunc (req->m_completionFuncPtr, error); 
		
		m_lock.lock ();
		m_activeReqList.erase (req);
	}

	m_lock.unlock ();
}

void
SocketAddressResolver::ioThreadFunc ()
{
	for (;;)
	{
		m_lock.lock ();
		while (!m_pendingReqList.isEmpty ())
		{
			if (m_ioThreadFlags & IoThreadFlag_Closing)
				break;

			Req* req = m_pendingReqList.removeHead ();
			m_reqMap.eraseKey (req->m_id);
			m_activeReqList.insertTail (req);
			m_lock.unlock ();

			processReq (req);

			m_lock.lock ();
			m_activeReqList.erase (req);
		}

		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_ioThreadFlags &= ~IoThreadFlag_Running;
			m_lock.unlock ();
			break;
		}

		m_lock.unlock ();

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
SocketAddressResolver::callCompletionFunc (
	FunctionPtr completionFuncPtr,
	const axl::io::SockAddr* addressTable,
	size_t addressCount,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE (m_runtime);

	DataPtr addressTablePtr = g_nullPtr;

	if (addressCount)
	{
		Type* addressType = SocketAddress::getType (m_runtime->getModule ());
		addressTablePtr = m_runtime->getGcHeap ()->allocateArray (addressType, addressCount);

		SocketAddress* dst = (SocketAddress*) addressTablePtr.m_p;
		const axl::io::SockAddr* src = addressTable;
		for (size_t i = 0; i < addressCount; i++, dst++, src++)
			dst->setSockAddr (*src);
	}

	DataPtr errorPtr = error ? memDup (error, error->m_size) : g_nullPtr;

	callVoidFunctionPtr (completionFuncPtr, addressTablePtr, addressCount, errorPtr);

	JNC_END_CALL_SITE ();
}

void
SocketAddressResolver::processReq (Req* req)
{
	sl::Array <axl::io::SockAddr> addrArray;
	bool result = axl::io::resolveHostName (&addrArray, req->m_name, req->m_addrFamily);
	if (!result)
	{
		callCompletionFunc (req->m_completionFuncPtr, err::getLastError ());
		return;
	}

	uint_t port = sl::swapByteOrder16 (req->m_port);

	size_t count = addrArray.getCount ();
	for (size_t i = 0; i < count; i++)
		addrArray [i].m_addr_ip4.sin_port = port;

	callCompletionFunc (req->m_completionFuncPtr, addrArray, count);
}

//..............................................................................

} // namespace io
} // namespace jnc
