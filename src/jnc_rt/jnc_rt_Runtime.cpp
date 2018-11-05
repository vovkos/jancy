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

namespace jnc {
namespace rt {

//..............................................................................

Runtime::Runtime ()
{
	m_stackSizeLimit = RuntimeDef_StackSizeLimit;
	m_tlsSize = 0;
	m_state = State_Idle;
	m_userData = NULL;
	m_noThreadEvent.signal ();
}

bool
Runtime::setStackSizeLimit (size_t sizeLimit)
{
	if (sizeLimit < RuntimeDef_MinStackSizeLimit)
	{
		err::setError (err::SystemErrorCode_InvalidParameter);
		return false;
	}

	m_stackSizeLimit = sizeLimit;
	return true;
}

bool
Runtime::startup (ct::Module* module)
{
	shutdown ();

	m_tlsSize = module->m_variableMgr.getTlsStructType ()->getSize ();
	m_module = module;
	m_state = State_Running;
	m_noThreadEvent.signal ();

	ct::Function* constructor = module->getConstructor ();
	ASSERT (constructor);

	return
		m_gcHeap.startup (module) &&
		callVoidFunction (this, constructor);
}

void
Runtime::shutdown ()
{
	ASSERT (!sys::getTlsPtrSlotValue <Tls> ());

	m_lock.lock ();
	if (m_state == State_Idle)
	{
		m_lock.unlock ();
		return;
	}

	ASSERT (m_state != State_ShuttingDown); // concurrent shutdowns?
	m_state = State_ShuttingDown;
	m_lock.unlock ();

	m_gcHeap.beginShutdown ();

	for (size_t i = 0; i < Shutdown_IterationCount; i++)
	{
		m_gcHeap.collect ();

		bool result = m_noThreadEvent.wait (Shutdown_WaitThreadTimeout); // wait for other threads
		if (result)
			break;
	}

	ASSERT (m_tlsList.isEmpty ());
	m_gcHeap.finalizeShutdown ();

	m_state = State_Idle;
}

void
Runtime::abort ()
{
	m_lock.lock ();
	if (m_state != State_Running)
	{
		m_lock.unlock ();
		return;
	}

	m_lock.unlock ();

	m_gcHeap.abort ();
}

void
Runtime::initializeCallSite (jnc_CallSite* callSite)
{
	// initialize dynamic GC shadow stack frame

	memset (callSite, 0, sizeof (jnc_CallSite));

	ASSERT (sizeof (GcShadowStackFrameMapBuffer) >= sizeof (GcShadowStackFrameMap));
	new (&callSite->m_gcShadowStackDynamicFrameMap) GcShadowStackFrameMap;
	callSite->m_gcShadowStackDynamicFrameMap.m_mapKind = ct::GcShadowStackFrameMapKind_Dynamic;
	callSite->m_gcShadowStackDynamicFrame.m_map = (GcShadowStackFrameMap*) &callSite->m_gcShadowStackDynamicFrameMap;

	Tls* prevTls = sys::getTlsPtrSlotValue <Tls> ();

	// try to find TLS of *this* runtime

	for (Tls* tls = prevTls; tls; tls = tls->m_prevTls)
		if (tls->m_runtime == this) // found
		{
			TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);
			ASSERT (tlsVariableTable->m_gcShadowStackTop > &callSite->m_gcShadowStackDynamicFrame);

			// save exception recovery snapshot

			callSite->m_initializeLevel = tls->m_initializeLevel;
			callSite->m_noCollectRegionLevel = tls->m_gcMutatorThread.m_noCollectRegionLevel;
			callSite->m_waitRegionLevel = tls->m_gcMutatorThread.m_waitRegionLevel;
			callSite->m_gcShadowStackDynamicFrame.m_prev = tlsVariableTable->m_gcShadowStackTop;

			// don't nest dynamic frames (unnecessary) -- but prev pointer must be saved anyway
			// also, without nesting it's often OK to skip explicit GcHeap::addBoxToDynamicFrame

			if (tlsVariableTable->m_gcShadowStackTop->m_map->getMapKind () != ct::GcShadowStackFrameMapKind_Dynamic)
				tlsVariableTable->m_gcShadowStackTop = &callSite->m_gcShadowStackDynamicFrame;

			tls->m_initializeLevel++;
			return;
		}

	// not found, create a new one

	Tls* tls = AXL_MEM_NEW_EXTRA (Tls, m_tlsSize);
	m_gcHeap.registerMutatorThread (&tls->m_gcMutatorThread); // register with GC heap first
	tls->m_prevTls = prevTls;
	tls->m_runtime = this;
	tls->m_initializeLevel = 1;
	tls->m_stackEpoch = callSite;

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);

	tlsVariableTable->m_gcShadowStackTop = &callSite->m_gcShadowStackDynamicFrame;

	sys::setTlsPtrSlotValue <Tls> (tls);

	m_lock.lock ();
	if (m_tlsList.isEmpty ())
		m_noThreadEvent.reset ();

	m_tlsList.insertTail (tls);
	m_lock.unlock ();
}

void
Runtime::uninitializeCallSite (jnc_CallSite* callSite)
{
	Tls* tls = sys::getTlsPtrSlotValue <Tls> ();
	ASSERT (tls && tls->m_runtime == this);

	ASSERT (
		callSite->m_initializeLevel == tls->m_initializeLevel - 1 &&
		callSite->m_waitRegionLevel == tls->m_gcMutatorThread.m_waitRegionLevel
		);

	ASSERT (
		callSite->m_noCollectRegionLevel == tls->m_gcMutatorThread.m_noCollectRegionLevel ||
		!callSite->m_result && callSite->m_noCollectRegionLevel < tls->m_gcMutatorThread.m_noCollectRegionLevel
		);

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);
	GcShadowStackFrame* prevGcShadowStackTop = callSite->m_gcShadowStackDynamicFrame.m_prev;

	ASSERT (
		tlsVariableTable->m_gcShadowStackTop == &callSite->m_gcShadowStackDynamicFrame ||
		tlsVariableTable->m_gcShadowStackTop == prevGcShadowStackTop &&
		prevGcShadowStackTop->m_map->getMapKind () == ct::GcShadowStackFrameMapKind_Dynamic ||
		!callSite->m_result &&
		tlsVariableTable->m_gcShadowStackTop < &callSite->m_gcShadowStackDynamicFrame
		);

	// restore exception recovery snapshot

	tls->m_initializeLevel = callSite->m_initializeLevel;
	tls->m_gcMutatorThread.m_noCollectRegionLevel = callSite->m_noCollectRegionLevel;
	tls->m_gcMutatorThread.m_waitRegionLevel = callSite->m_waitRegionLevel;
	tlsVariableTable->m_gcShadowStackTop = prevGcShadowStackTop;

	((GcShadowStackFrameMap*) &callSite->m_gcShadowStackDynamicFrameMap)->~GcShadowStackFrameMap ();

	if (tls->m_initializeLevel) // this thread was nested-initialized
	{
		m_gcHeap.safePoint ();
		return;
	}

	ASSERT (
		!callSite->m_initializeLevel &&
		!callSite->m_waitRegionLevel &&
		!callSite->m_noCollectRegionLevel &&
		!prevGcShadowStackTop
		);

	m_gcHeap.unregisterMutatorThread (&tls->m_gcMutatorThread);

	m_lock.lock ();
	m_tlsList.remove (tls);

	if (m_tlsList.isEmpty ())
		m_noThreadEvent.signal ();

	m_lock.unlock ();

	sys::setTlsPtrSlotValue <Tls> (tls->m_prevTls);

	AXL_MEM_DELETE (tls);
}

void
Runtime::checkStackOverflow ()
{
	Tls* tls = sys::getTlsPtrSlotValue <Tls> ();
	ASSERT (tls && tls->m_runtime == this);

	char* p = (char*) _alloca (1);
	ASSERT ((char*) tls->m_stackEpoch >= p);

	size_t stackSize = (char*) tls->m_stackEpoch - p;
	if (stackSize > m_stackSizeLimit)
	{
		err::setFormatStringError ("stack overflow (%dB)", stackSize);
		dynamicThrow ();
		ASSERT (false);
	}
}

SjljFrame*
Runtime::setSjljFrame (SjljFrame* frame)
{
	Tls* tls = rt::getCurrentThreadTls ();
	if (!tls)
	{
		TRACE ("-- WARNING: set external SJLJ frame: %p\n", frame);
		return sys::setTlsPtrSlotValue <SjljFrame> (frame);
	}

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);
	ASSERT (tls);

	SjljFrame* prevFrame = tlsVariableTable->m_sjljFrame;
	tlsVariableTable->m_sjljFrame = frame;
	return prevFrame;
}

void
Runtime::dynamicThrow ()
{
	Tls* tls = rt::getCurrentThreadTls ();
	ASSERT (tls);

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);
	if (tlsVariableTable->m_sjljFrame)
	{
#if (_JNC_OS_WIN && _JNC_CPU_AMD64)
		_JUMP_BUFFER* pBuffer = (_JUMP_BUFFER*) tlsVariableTable->m_sjljFrame->m_jmpBuf;
		pBuffer->Frame = 0; // prevent unwinding -- it doesn't work with the LLVM MCJIT-generated code
#endif

		longjmp (tlsVariableTable->m_sjljFrame->m_jmpBuf, -1);
	}
	else
	{
		SjljFrame* frame = sys::getTlsPtrSlotValue <SjljFrame> ();
		TRACE ("-- WARNING: jump to external SJLJ frame: %p\n", frame);

		ASSERT (frame);
		longjmp (frame->m_jmpBuf, -1);
	}

	ASSERT (false);
}

//..............................................................................

} // namespace rt
} // namespace jnc
