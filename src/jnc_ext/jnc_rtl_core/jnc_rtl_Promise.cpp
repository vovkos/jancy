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
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Promise,
	"jnc.Promise",
	sl::g_nullGuid,
	-1,
	PromiseImpl,
	&PromiseImpl::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Promise)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<PromiseImpl>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<PromiseImpl>)

	JNC_MAP_FUNCTION("wait",         &PromiseImpl::wait_0)
	JNC_MAP_OVERLOAD(&PromiseImpl::wait_1)
	JNC_MAP_OVERLOAD(&PromiseImpl::wait_2)
	JNC_MAP_FUNCTION("blockingWait", &PromiseImpl::blockingWait)
	JNC_MAP_FUNCTION("asyncWait",    &PromiseImpl::asyncWait)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_CLASS_TYPE(
	Promisifier,
	"jnc.Promisifier",
	sl::g_nullGuid,
	-1
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Promisifier)
	JNC_MAP_FUNCTION("complete", &PromiseImpl::complete_0)
	JNC_MAP_OVERLOAD(&PromiseImpl::complete_1)
	JNC_MAP_OVERLOAD(&PromiseImpl::complete_2)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

PromiseImpl::PromiseImpl() {
	// claim scheduler (if any)

	Tls* tls = getCurrentThreadTls();
	TlsVariableTable* tlsVariableTable = (TlsVariableTable*)(tls + 1);
	m_scheduler = tlsVariableTable->m_asyncScheduler;
	tlsVariableTable->m_asyncScheduler = NULL;
}

void
JNC_CDECL
PromiseImpl::markOpaqueGcRoots(GcHeap* gcHeap) {
	m_lock.lock();

	if (m_result.m_type && (m_result.m_type->getFlags() & TypeFlag_GcRoot))
		m_result.m_type->markGcRoots(&m_result, gcHeap);

	sl::Iterator<AsyncWait> it = m_asyncWaitList.getHead();
	for (; it; it++)
		if (it->m_handlerPtr.m_closure)
			gcHeap->markClass(it->m_handlerPtr.m_closure->m_box);

	if (m_gcShadowStackFrame)
		gcHeap->addShadowStackFrame(m_gcShadowStackFrame);

	m_lock.unlock();
}

void
PromiseImpl::reset() {
	ASSERT(m_state == State_Completed && m_asyncWaitList.isEmpty() && m_syncWaitList.isEmpty());

	m_lock.lock();
	m_state = State_Initial;
	m_result = g_nullVariant;
	m_errorPtr = g_nullDataPtr;
	m_lock.unlock();
}

uintptr_t
JNC_CDECL
PromiseImpl::wait_0(FunctionPtr handlerPtr) {
	m_lock.lock();

	if (m_state != State_Completed)
		return addAsyncWait_l(AsyncWaitKind_NoArgs, handlerPtr);

	m_lock.unlock();
	callVoidFunctionPtr(handlerPtr);
	return 0; // not added
}

uintptr_t
JNC_CDECL
PromiseImpl::wait_1(FunctionPtr handlerPtr) {
	m_lock.lock();

	if (m_state != State_Completed)
		return addAsyncWait_l(AsyncWaitKind_ErrorArg, handlerPtr);

	m_lock.unlock();
	callVoidFunctionPtr(handlerPtr, m_errorPtr);
	return 0; // not added
}

uintptr_t
JNC_CDECL
PromiseImpl::wait_2(FunctionPtr handlerPtr) {
	m_lock.lock();

	if (m_state != State_Completed)
		return addAsyncWait_l(AsyncWaitKind_ResultErrorArgs, handlerPtr);

	m_lock.unlock();
	callVoidFunctionPtr(handlerPtr, m_result, m_errorPtr);
	return 0; // not added
}

bool
JNC_CDECL
PromiseImpl::cancelWait(uintptr_t handle) {
	m_lock.lock();

	sl::HandleTableIterator<AsyncWait*> it = m_asyncWaitMap.find((uintptr_t)handle);
	if (!it) {
		m_lock.unlock();
		err::setError(err::Error(err::SystemErrorCode_InvalidParameter));
		return false; // not found
	}

	m_asyncWaitList.erase(it->m_value);
	m_asyncWaitMap.erase(it);
	m_lock.unlock();

	return true;
}

void
JNC_CDECL
PromiseImpl::complete_2(
	Variant result,
	DataPtr errorPtr
) {
	m_lock.lock();
	if (m_state == State_Completed) {
		m_lock.unlock();
		TRACE("-- WARNING: multiple completions for jnc.Promise %p\n", this);
		return;
	}

	m_result = result;
	m_errorPtr = errorPtr;
	m_state = State_Completed;

	if (errorPtr.m_p && m_syncWaitList.isEmpty() && m_asyncWaitList.isEmpty()) {
		m_lock.unlock();

		const err::ErrorHdr* error = (err::ErrorHdr*)errorPtr.m_p;
		TRACE("-- WARNING: uncaught async exception for jnc.Promise %p: %s\n", this, error->getDescription().sz());
		err::setError(error);
		dynamicThrow(); // rethrow
	}

	sl::Iterator<SyncWait> syncIt = m_syncWaitList.getHead();
	for (; syncIt; syncIt++)
		syncIt->m_event->signal();

	while (!m_asyncWaitList.isEmpty()) {
		AsyncWait* wait = *m_asyncWaitList.getHead();
		m_asyncWaitMap.eraseKey(wait->m_handle);
		m_lock.unlock();

		callVoidFunctionPtr(wait->m_handlerPtr, m_result, m_errorPtr);

		m_lock.lock();
		m_asyncWaitList.erase(wait);
	}

	m_lock.unlock();
}

uintptr_t
PromiseImpl::addAsyncWait_l(
	AsyncWaitKind waitKind,
	FunctionPtr handlerPtr
) {
	AsyncWait* wait = new AsyncWait;
	wait->m_waitKind = waitKind;
	wait->m_handlerPtr = handlerPtr;
	m_asyncWaitList.insertTail(wait);
	uintptr_t handle = m_asyncWaitMap.add(wait);
	wait->m_handle = handle;
	m_lock.unlock();

	return handle;
}

Variant
PromiseImpl::blockingWaitImpl() {
	m_lock.lock();

	if (m_state != State_Completed) {
		sys::Event event;

		SyncWait wait;
		wait.m_event = &event;
		m_syncWaitList.insertTail(&wait);
		m_lock.unlock();

		GcHeap* gcHeap = getCurrentThreadGcHeap();
		ASSERT(gcHeap);

		gcHeap->enterWaitRegion();
		event.wait();
		gcHeap->leaveWaitRegion();

		m_lock.lock();
		m_syncWaitList.remove(&wait);
	}

	ASSERT(m_state == State_Completed);
	m_lock.unlock();

	if (m_errorPtr.m_p) {
		err::setError((const err::ErrorHdr*)m_errorPtr.m_p);

		GcHeap* gcHeap = getCurrentThreadGcHeap();
		ASSERT(gcHeap);

		gcHeap->getRuntime()->dynamicThrow();
		ASSERT(false);
	}

	return m_result;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
