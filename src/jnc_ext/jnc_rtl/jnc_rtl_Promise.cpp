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
#include "jnc_rtl_Promise.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_GcHeap.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

#if 0

//..............................................................................

JNC_DEFINE_CLASS_TYPE (
	Promise,
	"jnc.Promise",
	sl::g_nullGuid,
	-1
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Promise)
	JNC_MAP_FUNCTION ("asyncWait",    &Promise::asyncWait)
	JNC_MAP_FUNCTION ("blockingWait", &Promise::blockingWait)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_CLASS_TYPE (
	Promisifier,
	"jnc.Promisifier",
	sl::g_nullGuid,
	-1
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Promisifier)
	JNC_MAP_FUNCTION ("complete", &Promisifier::complete_0)
	JNC_MAP_OVERLOAD (&Promisifier::complete_1)
	JNC_MAP_OVERLOAD (&Promisifier::complete_2)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

void
JNC_CDECL
Promise::markOpaqueGcRoots (GcHeap* gcHeap)
{
	m_lock.lock ();

	if (m_result.m_type && (m_result.m_type->getFlags () & TypeFlag_GcRoot))
		m_result.m_type->markGcRoots (&m_result, gcHeap);

	sl::Iterator <AsyncWait> it = m_asyncWaitList.getHead ();
	for (; it; it++)
		if (it->m_handlerPtr.m_closure)
			gcHeap->markClass (it->m_handlerPtr.m_closure->m_box);

	it = m_pendingAsyncWaitList.getHead ();
	for (; it; it++)
		if (it->m_handlerPtr.m_closure)
			gcHeap->markClass (it->m_handlerPtr.m_closure->m_box);

	m_lock.unlock ();
}

intptr_t
JNC_CDECL
Promise::wait (FunctionPtr handlerPtr)
{
	if (!m_isOpen)
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return (handle_t) (intptr_t) -1;
	}

	m_lock.lock ();

	uint_t triggeredEvents = eventMask & m_activeEvents;
	if (triggeredEvents)
	{
		m_lock.unlock ();
		callVoidFunctionPtr (handlerPtr, triggeredEvents);
		return 0; // not added
	}

	AsyncWait* wait = AXL_MEM_NEW (AsyncWait);
	wait->m_mask = eventMask;
	wait->m_handlerPtr = handlerPtr;
	m_asyncWaitList.insertTail (wait);
	handle_t handle = (handle_t) m_asyncWaitMap.add (wait);
	wait->m_handle = handle;
	m_lock.unlock ();

	return handle;
}

bool
JNC_CDECL
Promise::cancelWait (intptr_t handle)
{
	m_lock.lock ();

	sl::HandleTableIterator <AsyncWait*> it = m_asyncWaitMap.find ((uintptr_t) handle);
	if (!it)
	{
		m_lock.unlock ();
		jnc::setError (err::Error (err::SystemErrorCode_InvalidParameter));
		return false; // not found
	}

	m_asyncWaitList.erase (it->m_value);
	m_asyncWaitMap.erase (it);
	m_lock.unlock ();

	return true;
}

Variant
Promise::blockingWait (Promise* self)
{
	self->m_lock.lock ();
	if (self->m_isCompleted)
	{
		self->m_lock.unlock ();
		return self->m_result;
	}

	sys::Event event;

	SyncWait wait;
	wait.m_event = &event;
	self->m_syncWaitList.insertTail (&wait);
	self->m_lock.unlock ();

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	gcHeap->enterWaitRegion ();
	event.wait ();
	gcHeap->leaveWaitRegion ();

	self->m_lock.lock ();
	self->m_syncWaitList.remove (&wait);
	self->m_lock.unlock ();

	return self->m_result;
}

//..............................................................................

void
JNC_CDECL
Promisifier::complete_2 (
	Variant result,
	DataPtr errorPtr
	)
{
	m_lock.lock ();
	if (m_isCompleted)
	{
		m_lock.unlock ();
		TRACE ("-- WARNING: ignoring repetitve completion for jnc.Promise: %p\n", this);
		return;
	}

	m_result = result;
	m_errorPtr = errorPtr;
	m_isCompleted = true;

	m_lock.unlock ();
}

//..............................................................................

Promise*
createMultiPromise (
	DataPtr promiseArrayPtr,
	size_t count,
	bool waitAll
	)
{
	return NULL;
}

//..............................................................................

#endif

} // namespace rtl
} // namespace jnc
