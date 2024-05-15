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
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_CallSite.h"

#if (__SANITIZE_ADDRESS__) // asan is using asan_stack_malloc so we can't assume the stack to grow up
#	define JNC_CHECK_STACK_PRECEDENCE(prev, next) true
#else
#	define JNC_CHECK_STACK_PRECEDENCE(prev, next) ((char*)(prev) > (char*)(next))
#endif

namespace jnc {
namespace rt {

//..............................................................................

Runtime::Runtime() {
	m_tlsSize = 0;
	m_state = State_Idle;
	m_module = NULL;
	m_userData = NULL;
	m_noThreadEvent.signal();
}

bool
Runtime::startup(ct::Module* module) {
	shutdown();

	m_tlsSize = module->m_variableMgr.getTlsStructType()->getSize();
	m_module = module;
	m_state = State_Running;
	m_noThreadEvent.signal();

	ct::Function* constructor = module->getConstructor();

	return
		m_gcHeap.startup(module) &&
		(!constructor || callVoidFunction(this, constructor));
}

void
Runtime::shutdown() {
	m_lock.lock();
	if (m_state == State_Idle) {
		m_lock.unlock();
		return;
	}

	ASSERT(m_state != State_ShuttingDown); // concurrent shutdowns?
	m_state = State_ShuttingDown;
	m_lock.unlock();

	m_gcHeap.beginShutdown();

	for (size_t i = 0; i < Shutdown_IterationCount; i++) {
		m_gcHeap.collect();

		bool result = m_noThreadEvent.wait(Shutdown_WaitThreadTimeout); // wait for other threads
		if (result)
			break;
	}

	m_gcHeap.finalizeShutdown();

	ASSERT(m_tlsList.isEmpty());
	m_state = State_Idle;
}

void
Runtime::abort() {
	m_lock.lock();

	if (m_state != State_Running) {
		m_lock.unlock();
		return;
	}

	m_lock.unlock();
	m_gcHeap.abort();
}

void
Runtime::initializeCallSite(CallSite* callSite) {
	memset(callSite, 0, sizeof(CallSite));
	callSite->m_prev = sys::getTlsPtrSlotValue<CallSite>();

	// initialize dynamic GC shadow stack frame

	ASSERT(sizeof(GcShadowStackFrameMapBuffer) >= sizeof(GcShadowStackFrameMap));
	new(&callSite->m_gcShadowStackDynamicFrameMap) GcShadowStackFrameMap;
	callSite->m_gcShadowStackDynamicFrameMap.m_mapKind = ct::GcShadowStackFrameMapKind_Dynamic;
	callSite->m_gcShadowStackDynamicFrame.m_map = (GcShadowStackFrameMap*)&callSite->m_gcShadowStackDynamicFrameMap;

	// try to find TLS of *this* runtime

	CallSite* prevCallSite = callSite->m_prev;
	while (prevCallSite) {
		if (prevCallSite->m_tls->m_runtime == this)
			break;

		prevCallSite = prevCallSite->m_prev;
	}

	if (prevCallSite) { // found!
		callSite->m_tls = prevCallSite->m_tls;
		TlsVariableTable* tlsVariableTable = (TlsVariableTable*)(callSite->m_tls + 1);
		ASSERT(JNC_CHECK_STACK_PRECEDENCE(tlsVariableTable->m_gcShadowStackTop, &callSite->m_gcShadowStackDynamicFrame));

		// save exception recovery snapshot

		callSite->m_nestLevel = prevCallSite->m_nestLevel + 1;
		callSite->m_noCollectRegionLevel = callSite->m_tls->m_gcMutatorThread.m_noCollectRegionLevel;
		callSite->m_waitRegionLevel = callSite->m_tls->m_gcMutatorThread.m_waitRegionLevel;
		callSite->m_gcShadowStackDynamicFrame.m_prev = tlsVariableTable->m_gcShadowStackTop;

		// don't nest dynamic frames (unnecessary) -- but prev pointer must be saved anyway
		// also, without nesting it's often OK to skip explicit GcHeap::addBoxToDynamicFrame

		if (!tlsVariableTable->m_gcShadowStackTop->m_map ||
			tlsVariableTable->m_gcShadowStackTop->m_map->getMapKind() != ct::GcShadowStackFrameMapKind_Dynamic)
			tlsVariableTable->m_gcShadowStackTop = &callSite->m_gcShadowStackDynamicFrame;
	} else { // not found, create a new one
		callSite->m_tls = new (mem::ExtraSize(m_tlsSize), mem::ZeroInit) Tls;
		callSite->m_tls->m_runtime = this;
		m_gcHeap.registerMutatorThread(&callSite->m_tls->m_gcMutatorThread); // register with GC heap first

		TlsVariableTable* tlsVariableTable = (TlsVariableTable*)(callSite->m_tls + 1);
		tlsVariableTable->m_gcShadowStackTop = &callSite->m_gcShadowStackDynamicFrame;

		m_lock.lock();
		if (m_tlsList.isEmpty())
			m_noThreadEvent.reset();

		m_tlsList.insertTail(callSite->m_tls);
		m_lock.unlock();
	}

	sys::setTlsPtrSlotValue<CallSite>(callSite);
}

void
Runtime::uninitializeCallSite(CallSite* callSite) {
	ASSERT(callSite == sys::getTlsPtrSlotValue<CallSite>());
	ASSERT(callSite->m_tls->m_runtime == this);
	ASSERT(callSite->m_waitRegionLevel == callSite->m_tls->m_gcMutatorThread.m_waitRegionLevel);

	ASSERT(
		callSite->m_noCollectRegionLevel == callSite->m_tls->m_gcMutatorThread.m_noCollectRegionLevel ||
		!callSite->m_result && callSite->m_noCollectRegionLevel < callSite->m_tls->m_gcMutatorThread.m_noCollectRegionLevel
	);

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*)(callSite->m_tls + 1);
	GcShadowStackFrame* prevGcShadowStackTop = callSite->m_gcShadowStackDynamicFrame.m_prev;

	ASSERT(
		tlsVariableTable->m_gcShadowStackTop == &callSite->m_gcShadowStackDynamicFrame ||
		tlsVariableTable->m_gcShadowStackTop == prevGcShadowStackTop &&
		prevGcShadowStackTop->m_map &&
		prevGcShadowStackTop->m_map->getMapKind() == ct::GcShadowStackFrameMapKind_Dynamic ||
		!callSite->m_result &&
		JNC_CHECK_STACK_PRECEDENCE(&callSite->m_gcShadowStackDynamicFrame, tlsVariableTable->m_gcShadowStackTop)
	);

	// restore exception recovery snapshot

	callSite->m_tls->m_gcMutatorThread.m_noCollectRegionLevel = callSite->m_noCollectRegionLevel;
	callSite->m_tls->m_gcMutatorThread.m_waitRegionLevel = callSite->m_waitRegionLevel;
	tlsVariableTable->m_gcShadowStackTop = prevGcShadowStackTop;

	((GcShadowStackFrameMap*)&callSite->m_gcShadowStackDynamicFrameMap)->~GcShadowStackFrameMap();

	if (callSite->m_nestLevel) { // this thread was nested-initialized
		sys::setTlsPtrSlotValue<CallSite>(callSite->m_prev);
		return;
	}

	ASSERT(
		!callSite->m_waitRegionLevel &&
		!callSite->m_noCollectRegionLevel &&
		!prevGcShadowStackTop
	);

	m_gcHeap.unregisterMutatorThread(&callSite->m_tls->m_gcMutatorThread);
	sys::setTlsPtrSlotValue<CallSite>(callSite->m_prev);

	m_lock.lock();
	m_tlsList.remove(callSite->m_tls);

	if (m_tlsList.isEmpty())
		m_noThreadEvent.signal();

	m_lock.unlock();
	delete callSite->m_tls;
}

SjljFrame*
Runtime::setSjljFrame(SjljFrame* frame) {
	Tls* tls = rt::getCurrentThreadTls();
	if (!tls) {
		TRACE("-- WARNING: set external SJLJ frame: %p\n", frame);
		return sys::setTlsPtrSlotValue<SjljFrame>(frame);
	}

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*)(tls + 1);
	ASSERT(tls);

	SjljFrame* prevFrame = tlsVariableTable->m_sjljFrame;
	tlsVariableTable->m_sjljFrame = frame;
	return prevFrame;
}

void
Runtime::dynamicThrow() {
	Tls* tls = rt::getCurrentThreadTls();
	ASSERT(tls);

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*)(tls + 1);
	if (tlsVariableTable->m_sjljFrame) {
		jnc_longJmp(tlsVariableTable->m_sjljFrame->m_jmpBuf, -1);
	} else {
		SjljFrame* frame = sys::getTlsPtrSlotValue<SjljFrame> ();
		TRACE("-- WARNING: jump to external SJLJ frame: %p\n", frame);

		ASSERT(frame);
		jnc_longJmp(frame->m_jmpBuf, -1);
	}

	ASSERT(false);
}

#if (_JNC_OS_POSIX)
void
Runtime::saveSignalInfo(SjljFrame* sjljFrame) {
	if (!sjljFrame->m_signalInfo.m_signal) // there was no signal or it's already saved
		return;

	sys::setPosixSignalError(
		sjljFrame->m_signalInfo.m_signal,
		sjljFrame->m_signalInfo.m_code,
		sjljFrame->m_signalInfo.m_codeAddress,
		sjljFrame->m_signalInfo.m_faultAddress
	);

	sjljFrame->m_signalInfo.m_signal = 0; // save only once
}
#endif

//..............................................................................

} // namespace rt
} // namespace jnc
