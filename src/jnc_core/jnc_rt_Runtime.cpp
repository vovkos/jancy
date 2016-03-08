#include "pch.h"
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_CallSite.h"

namespace jnc {
namespace rt {

//.............................................................................

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

	m_gcHeap.startup (module);

	ct::Function* constructor = module->getConstructor ();
	ASSERT (constructor);
	
	return callVoidFunction (this, constructor);
}
	
void
Runtime::shutdown ()
{
	ASSERT (!sys::getTlsSlotValue <Tls> ());

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
		ers->m_initializeLevel == tls->m_initializeLevel &&
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
	Tls* prevTls = sys::getTlsSlotValue <Tls> ();
	if (prevTls && prevTls->m_runtime == this)
	{
		saveExceptionRecoverySnapshot (ers, prevTls);
		prevTls->m_initializeLevel++;
		return;
	}

	size_t size = sizeof (Tls) + m_tlsSize;
	
	Tls* tls = AXL_MEM_NEW_EXTRA (Tls, m_tlsSize);
	m_gcHeap.registerMutatorThread (&tls->m_gcMutatorThread); // register with GC heap first
	tls->m_prev = prevTls;
	tls->m_runtime = this;
	tls->m_initializeLevel = 1;
	tls->m_stackEpoch = ers;

	sys::setTlsSlotValue <Tls> (tls);
	sys::setTlsSlotValue <Runtime> (this);

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
	Tls* tls = sys::getTlsSlotValue <Tls> ();
	ASSERT (tls && tls->m_runtime == this);

	if (--tls->m_initializeLevel) // still 
	{
		restoreExceptionRecoverySnapshot (ers, tls);
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
	
	sys::setTlsSlotValue <Tls> (tls->m_prev);
	sys::setTlsSlotValue <Runtime> (tls->m_prev ? tls->m_prev->m_runtime : NULL);

	AXL_MEM_DELETE (tls);
}

void
Runtime::checkStackOverflow ()
{
	Tls* tls = sys::getTlsSlotValue <Tls> ();
	ASSERT (tls && tls->m_runtime == this);

	char* p = (char*) _alloca (1);
	ASSERT ((char*) tls->m_stackEpoch >= p);

	size_t stackSize = (char*) tls->m_stackEpoch - p;
	if (stackSize > m_stackSizeLimit)
	{
		err::Error error = err::formatStringError ("stack overflow (%dB)", stackSize);
		runtimeError (error);
		ASSERT (false);
	}
}

void
Runtime::dynamicThrow ()
{
	rt::Tls* tls = rt::getCurrentThreadTls ();
	ASSERT (tls);

	rt::TlsVariableTable* tlsVariableTable = (rt::TlsVariableTable*) (tls + 1);
	if (tlsVariableTable->m_sjljFrame)
		longjmp (tlsVariableTable->m_sjljFrame->m_jmpBuf, -1);
	else
		AXL_SYS_SJLJ_THROW ();

	ASSERT (false);
}

//.............................................................................

void
primeIface (
	Box* box,
	Box* root,
	IfaceHdr* iface,
	ct::ClassType* type,
	void* vtable
	)
{
	iface->m_vtable = vtable;
	iface->m_box = box;

	// primeClass all the base types

	sl::Array <ct::BaseTypeSlot*> baseTypePrimeArray = type->getBaseTypePrimeArray ();
	size_t count = baseTypePrimeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ct::BaseTypeSlot* slot = baseTypePrimeArray [i];
		ASSERT (slot->getType ()->getTypeKind () == ct::TypeKind_Class);
		
		primeIface (
			box,
			root,
			(IfaceHdr*) ((char*) iface + slot->getOffset ()),
			(ct::ClassType*) slot->getType (),
			(void**) vtable + slot->getVTableIndex ()
			);
	}

	// primeClass all the class fields

	sl::Array <ct::StructField*> fieldPrimeArray = type->getClassMemberFieldArray ();
	count = fieldPrimeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ct::StructField* field = fieldPrimeArray [i];
		ASSERT (field->getType ()->getTypeKind () == ct::TypeKind_Class);

		ct::ClassType* fieldType = (ct::ClassType*) field->getType ();
		Box* fieldBox = (Box*) ((char*) iface + field->getOffset ());

		primeClass (
			fieldBox,
			root,
			fieldType
			);
	}
}

void
primeClass (
	Box* box,
	Box* root,
	ct::ClassType* type,
	void* vtable
	)
{
	ASSERT (root <= box);

	if (!vtable)
		vtable = type->getVTableVariable ()->getStaticData ();

	memset (box, 0, type->getSize ());

	box->m_type = type;
	box->m_flags = BoxFlag_ClassMark | BoxFlag_DataMark | BoxFlag_WeakMark;
	box->m_rootOffset = (char*) box - (char*) root;

	primeIface (box, root, (IfaceHdr*) (box + 1), type, vtable);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

size_t
strLen (DataPtr ptr)
{
	if (!ptr.m_validator || ptr.m_p < ptr.m_validator->m_rangeBegin)
		return 0;

	char* p0 = (char*) ptr.m_p;
	char* end = (char*) ptr.m_validator->m_rangeBegin + ptr.m_validator->m_rangeLength;

	char* p = p0;
	while (p < end && *p)
		p++;

	return p - p0;
}

DataPtr
strDup (
	const char* p,
	size_t length
	)
{
	if (length == -1)
		length = p ? strlen (p) : 0;

	if (!length)
		return g_nullPtr;

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (length + 1);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (p)
		memcpy (resultPtr.m_p, p, length);

	return resultPtr;
}

DataPtr
memDup (
	const void* p,
	size_t size
	)
{
	if (!size)
		return g_nullPtr;

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (p)
		memcpy (resultPtr.m_p, p, size);

	return resultPtr;
}

//.............................................................................

} // namespace rt
} // namespace jnc
