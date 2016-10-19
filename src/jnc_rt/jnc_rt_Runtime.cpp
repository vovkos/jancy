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

	ASSERT (m_state == State_Running); // otherwise, concurrent shutdowns
	m_state = State_ShuttingDown;
	m_lock.unlock ();

	m_gcHeap.beginShutdown ();

	bool result = m_noThreadEvent.wait (3000); // wait for other threads
	ASSERT (result && m_tlsList.isEmpty ());

	m_gcHeap.finalizeShutdown ();

	m_state = State_Idle;
}

void
saveExceptionRecoverySnapshot (
	ExceptionRecoverySnapshot* ers,
	Tls* tls
	)
{
	TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);

	ers->m_initializeLevel = tls->m_initializeLevel;
	ers->m_noCollectRegionLevel = tls->m_gcMutatorThread.m_noCollectRegionLevel;
	ers->m_waitRegionLevel = tls->m_gcMutatorThread.m_waitRegionLevel;
	ers->m_gcShadowStackTop = tlsVariableTable->m_gcShadowStackTop;
}

void
restoreExceptionRecoverySnapshot (
	ExceptionRecoverySnapshot* ers,
	Tls* tls
	)
{
	TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);

	ASSERT (
		ers->m_initializeLevel == tls->m_initializeLevel - 1 &&
		ers->m_waitRegionLevel == tls->m_gcMutatorThread.m_waitRegionLevel
		);

	ASSERT (
		ers->m_noCollectRegionLevel == tls->m_gcMutatorThread.m_noCollectRegionLevel ||
		!ers->m_result && ers->m_noCollectRegionLevel < tls->m_gcMutatorThread.m_noCollectRegionLevel
		);

	ASSERT (
		ers->m_gcShadowStackTop == tlsVariableTable->m_gcShadowStackTop ||
		!ers->m_result && (!ers->m_gcShadowStackTop || ers->m_gcShadowStackTop > tlsVariableTable->m_gcShadowStackTop)
		);

	tls->m_initializeLevel = ers->m_initializeLevel;
	tls->m_gcMutatorThread.m_noCollectRegionLevel = ers->m_noCollectRegionLevel;
	tls->m_gcMutatorThread.m_waitRegionLevel = ers->m_waitRegionLevel;
	tlsVariableTable->m_gcShadowStackTop = ers->m_gcShadowStackTop;
}

void
Runtime::initializeThread (ExceptionRecoverySnapshot* ers)
{
	Tls* prevTls = sys::getTlsPtrSlotValue <Tls> ();
	if (prevTls && prevTls->m_runtime == this)
	{
		saveExceptionRecoverySnapshot (ers, prevTls);
		prevTls->m_initializeLevel++;
		return;
	}

	size_t size = sizeof (Tls) + m_tlsSize;

	Tls* tls = AXL_MEM_NEW_EXTRA (Tls, m_tlsSize);
	m_gcHeap.registerMutatorThread (&tls->m_gcMutatorThread); // register with GC heap first
	tls->m_prevTls = prevTls;
	tls->m_runtime = this;
	tls->m_initializeLevel = 1;
	tls->m_stackEpoch = ers;

	sys::setTlsPtrSlotValue <Tls> (tls);

	m_lock.lock ();
	if (m_tlsList.isEmpty ())
		m_noThreadEvent.reset ();

	m_tlsList.insertTail (tls);
	m_lock.unlock ();

	memset (ers, 0, sizeof (ExceptionRecoverySnapshot));
}

void
Runtime::uninitializeThread (ExceptionRecoverySnapshot* ers)
{
	Tls* tls = sys::getTlsPtrSlotValue <Tls> ();
	ASSERT (tls && tls->m_runtime == this);

	restoreExceptionRecoverySnapshot (ers, tls);

	if (tls->m_initializeLevel) // this thread was nested-initialized
	{
		m_gcHeap.safePoint ();
		return;
	}

	ASSERT (
		!ers->m_initializeLevel &&
		!ers->m_waitRegionLevel &&
		!ers->m_noCollectRegionLevel &&
		!ers->m_gcShadowStackTop
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
	return sys::setTlsPtrSlotValue <SjljFrame> (frame);
}

void
Runtime::dynamicThrow ()
{
	Tls* tls = rt::getCurrentThreadTls ();
	ASSERT (tls);

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);
	if (tlsVariableTable->m_sjljFrame)
	{
		longjmp (tlsVariableTable->m_sjljFrame->m_jmpBuf, -1);
	}
	else
	{
		SjljFrame* sjljFrame = sys::getTlsPtrSlotValue <SjljFrame> ();
		ASSERT (sjljFrame);
		longjmp (sjljFrame->m_jmpBuf, -1);
	}

	ASSERT (false);
}

//..............................................................................

} // namespace rt
} // namespace jnc
