#include "pch.h"
#include "jnc_Runtime.h"
#include "jnc_Module.h"
#include "jnc_StdLib.h"
#include "jnc_GcShadowStack.h"

namespace jnc {

//.............................................................................

Runtime::Runtime ()
{
	m_gcState = GcState_Idle;
	m_gcHeapLimit = -1;
	m_totalGcAllocSize = 0;
	m_currentGcAllocSize = 0;
	m_periodGcAllocSize = 0;
	m_periodGcAllocLimit = 16 * 1024;
	m_gcUnsafeThreadCount = 1;
	m_currentGcRootArrayIdx = 0;

	m_stackLimit = -1;
	m_tlsSlot = -1;
	m_tlsSize = 0;
}

bool
Runtime::create (
	size_t heapLimit,
	size_t stackLimit
	)
{
	destroy ();

	m_gcHeapLimit = heapLimit;
	m_stackLimit = stackLimit;

	m_tlsSlot = getTlsMgr ()->createSlot ();
	return true;
}

void
Runtime::destroy ()
{
	if (m_tlsSlot != -1)
	{
		getTlsMgr ()->nullifyTls (this);
		getTlsMgr ()->destroySlot (m_tlsSlot);
	}

	m_tlsSlot = -1;
	m_tlsSize = 0;

	while (!m_tlsList.isEmpty ())
	{
		TlsHdr* tls = m_tlsList.removeHead ();
		AXL_MEM_FREE (tls);
	}

	m_staticGcRootArray.clear ();
	m_moduleArray.clear ();
}

bool
Runtime::addModule (Module* module)
{
	llvm::ExecutionEngine* llvmExecutionEngine = module->getLlvmExecutionEngine ();

	// static gc roots

	rtl::Array <Variable*> staticRootArray = module->m_variableMgr.getStaticGcRootArray ();
	size_t count = staticRootArray.getCount ();

	m_staticGcRootArray.setCount (count);
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = staticRootArray [i];
		void* p = llvmExecutionEngine->getPointerToGlobal ((llvm::GlobalVariable*) variable->getLlvmAllocValue ());
		ASSERT (p);

		m_staticGcRootArray [i].m_p = p;
		m_staticGcRootArray [i].m_type = variable->getType ();
	}

	// tls
	
	size_t tlsSize = module->m_variableMgr.getTlsStructType ()->getSize ();
	if (!m_tlsSize)
	{
		m_tlsSize = tlsSize >= 1024 ? tlsSize : 1024; // reserve at least 1K for TLS
	}
	else if (m_tlsSize < tlsSize)
	{
		err::setFormatStringError ("dynamic grow of TLS is not yet supported");
		return false;
	}

	m_moduleArray.append (module);
	return true;
}

bool
Runtime::startup ()
{
	// ensure correct state

	m_gcState = GcState_Idle;
	m_gcUnsafeThreadCount = 1;
	m_gcIdleEvent.signal ();

	getTlsMgr ()->nullifyTls (this);
	m_tlsList.clear ();
	return true;
}

void
Runtime::shutdown ()
{
	rtl::Array <GcRoot> saveStaticGcRootArray = m_staticGcRootArray;
	m_staticGcRootArray.clear ();

	runGc ();

	m_lock.lock ();
	
	while (!m_staticDestructList.isEmpty ())
	{
		StaticDestruct* destruct = m_staticDestructList.removeTail ();
		m_lock.unlock ();

		if (destruct->m_iface)
			destruct->m_dtor (destruct->m_iface);
		else
			destruct->m_staticDtor ();

		AXL_MEM_DELETE (destruct);

		m_lock.lock ();
	}

	m_lock.unlock ();

	runGc (); 

	TlsHdr* tls = getTlsMgr ()->nullifyTls (this);
	if (tls)
	{
		m_tlsList.remove (tls);
		AXL_MEM_FREE (tls);
	}

	ASSERT (m_tlsList.isEmpty ());

	m_staticGcRootArray = saveStaticGcRootArray; // recover
}

void
Runtime::runtimeError (
	int error,
	void* codeAddr,
	void* dataAddr
	)
{
	const char* errorString;

	switch (error)
	{
	case RuntimeErrorKind_OutOfMemory:
		errorString = "OUT_OF_MEMORY";
		break;

	case RuntimeErrorKind_StackOverflow:
		errorString = "STACK_OVERFLOW";
		break;

	case RuntimeErrorKind_DataPtrOutOfRange:
		errorString = "DATA_PTR_OOR";
		break;

	case RuntimeErrorKind_ScopeMismatch:
		errorString = "SCOPE_MISMATCH";
		break;

	case RuntimeErrorKind_NullClassPtr:
		errorString = "NULL_CLASS_PTR";
		break;

	case RuntimeErrorKind_NullFunctionPtr:
		errorString = "NULL_FUNCTION_PTR";
		break;

	case RuntimeErrorKind_NullPropertyPtr:
		errorString = "NULL_PROPERTY_PTR";
		break;

	case RuntimeErrorKind_AbstractFunction:
		errorString = "ABSTRACT_FUNCTION";
		break;

	default:
		ASSERT (false);
		errorString = "<UNDEF>";
	}

	runtimeError (err::formatStringError (
		"RUNTIME ERROR: %s (code %x accessing data %x)",
		errorString,
		codeAddr,
		dataAddr
		));
}

void*
Runtime::gcAllocate (
	Type* type,
	size_t elementCount
	)
{
	void* p = gcTryAllocate (type, elementCount);
	if (!p)
	{
		runtimeError (RuntimeErrorKind_OutOfMemory, NULL, NULL);
		ASSERT (false);
	}

	return p;
}

void*
Runtime::gcTryAllocate (
	Type* type,
	size_t elementCount
	)
{
	size_t prevGcLevel = gcMakeThreadSafe ();
	ASSERT (prevGcLevel); // otherwise there is risk of losing return value

	size_t size = sizeof (ObjHdr) + type->getSize () * elementCount;
	if (elementCount > 1)
		size += sizeof (size_t);

	waitGcIdleAndLock ();
	if (m_periodGcAllocSize > m_periodGcAllocLimit)
	{
	//	RunGc_l ();
	//	WaitGcIdleAndLock ();
	}

	restoreGcLevel (prevGcLevel); // restore before unlocking

	void* block = AXL_MEM_ALLOC (size);
	if (!block)
	{
		m_lock.unlock ();
		return NULL;
	}

	memset (block, 0, size);

	ObjHdr* object;
	void* p;

	if (type->getTypeKind () == TypeKind_Class)
	{
		object = (ObjHdr*) block;
		gcAddObject (object, (ClassType*) type);

		// object will be primed by user code

		p = object;
	}
	else
	{
		if (elementCount > 1)
		{
			size_t* size = (size_t*) block;
			*size = elementCount;
			object = (ObjHdr*) (size + 1);
			object->m_flags = ObjHdrFlag_DynamicArray;
		}
		else
		{
			object = (ObjHdr*) block;
			object->m_flags = 0;
		}

		object->m_scopeLevel = 0;
		object->m_root = object;
		object->m_type = type;

		p = object + 1;
	}

	m_periodGcAllocSize += size;
	m_gcMemBlockArray.append (object);
	m_lock.unlock ();
	return p;
}

void*
Runtime::gcAllocate (size_t size)
{
	Type* type = getFirstModule ()->m_typeMgr.getPrimitiveType (TypeKind_Char);
	return gcAllocate (type, size);
}

void*
Runtime::gcTryAllocate (size_t size)
{
	Type* type = getFirstModule ()->m_typeMgr.getPrimitiveType (TypeKind_Char);
	return gcTryAllocate (type, size);
}

void
Runtime::gcAddObject (
	ObjHdr* object,
	ClassType* type
	)
{
	char* p = (char*) (object + 1);

	rtl::Array <StructField*> classFieldArray = type->getClassMemberFieldArray ();
	size_t count = classFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = classFieldArray [i];
		ASSERT (field->getType ()->getTypeKind () == TypeKind_Class);

		ObjHdr* childObject = (ObjHdr*) (p + field->getOffset ());
		gcAddObject (childObject, (ClassType*) field->getType ());
	}

	m_gcObjectArray.append (object);
}

void
Runtime::addGcRoot (
	void* p,
	Type* type
	)
{
	ASSERT (m_gcState == GcState_Mark);
	ASSERT (type->getFlags () & TypeFlag_GcRoot);

	GcRoot root = { p, type };
	m_gcRootArray [m_currentGcRootArrayIdx].append (root);
}

void
Runtime::markGcLocalHeapRoot (
	void* p,
	Type* type
	)
{
	if (type->getTypeKind () == TypeKind_Class)
		((ObjHdr*) p)->gcMarkObject (this);
	else
		((ObjHdr*) p - 1)->gcMarkData (this);

	if (type->getFlags () & TypeFlag_GcRoot)
		addGcRoot (p, type);
}

void
Runtime::runGc ()
{
	size_t prevGcLevel = gcMakeThreadSafe ();
	waitGcIdleAndLock ();
	runGc_l ();
	restoreGcLevel (prevGcLevel);
}

void
Runtime::runGc_l ()
{
	m_gcIdleEvent.reset ();

	// 1) suspend all mutator threads at safe points

	m_gcState = GcState_WaitSafePoint;

	ASSERT (m_gcUnsafeThreadCount);
	m_gcSafePointEvent.reset ();
	intptr_t unsafeCount = mt::atomicDec (&m_gcUnsafeThreadCount);
	if (unsafeCount)
	{
		m_lock.unlock ();
		m_gcSafePointEvent.wait ();
		m_lock.lock ();
	}

	// 2) mark

	m_gcState = GcState_Mark;

	m_gcRootArray [0].clear ();
	m_currentGcRootArrayIdx = 0;

	// 2.1) add static roots

	size_t count = m_staticGcRootArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ASSERT (m_staticGcRootArray [i].m_type->getFlags () & TypeFlag_GcRoot);
		addGcRoot (
			m_staticGcRootArray [i].m_p,
			m_staticGcRootArray [i].m_type
			);
	}

	// 2.2) add destructible roots

	// 2.2) add stack roots

	rtl::Iterator <TlsHdr> tlsIt = m_tlsList.getHead ();
	for (; tlsIt; tlsIt++)
	{
		Tls* tls = (Tls*) (*tlsIt + 1);

		GcShadowStackFrame* stackFrame = tls->m_gcShadowStackTop;
		for (; stackFrame; stackFrame = stackFrame->m_next)
		{
			size_t count = stackFrame->m_map->m_count;
			void** rootArray = (void**) (stackFrame + 1);
			Type** typeArray = (Type**) (stackFrame->m_map + 1);

			for (size_t i = 0; i < count; i++)
			{
				void* p = rootArray [i];
				Type* type = typeArray [i];

				if (p) // check needed, stack roots could be nullified
				{
					if (type->getTypeKind () == TypeKind_DataPtr &&
						((DataPtrType*) type)->getPtrTypeKind () == DataPtrTypeKind_Thin) // local heap variable
					{
						markGcLocalHeapRoot (p, ((DataPtrType*) type)->getTargetType ());
					}
					else
					{
						addGcRoot (p, type);
					}
				}
			}
		}
	}

	// 2.3) run mark cycle

	gcMarkCycle ();

	// 2.4) mark objects as dead and schedule destruction

	char buffer [256];
	rtl::Array <IfaceHdr*> destructArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	GcDestructGuard destructGuard;

	count = m_gcObjectArray.getCount ();
	for (intptr_t i = count - 1; i >= 0; i--)
	{
		jnc::ObjHdr* object = m_gcObjectArray [i];

		if (!(object->m_flags & (ObjHdrFlag_GcMark | ObjHdrFlag_GcWeakMark_c)))
		{
			m_gcObjectArray.remove (i);
			object->m_flags |= ObjHdrFlag_Dead;

			if (object->m_classType->getDestructor ())
				destructArray.append ((IfaceHdr*) (object + 1));
		}
	}

	if (!destructArray.isEmpty ())
	{
		destructGuard.m_destructArray = &destructArray;
		m_gcDestructGuardList.insertTail (&destructGuard);
	}

	// 2.5) mark all destruct guard gc roots

	rtl::Iterator <GcDestructGuard> it = m_gcDestructGuardList.getHead ();
	for (; it; it++)
	{
		GcDestructGuard* destructGuard = *it;

		count = destructArray.getCount ();
		for (size_t i = 0; i < count; i++)
		{
			addGcRoot (
				&destructArray [i],
				destructArray [i]->m_object->m_classType->getClassPtrType ()
				);
		}
	}

	gcMarkCycle ();

	// 3) sweep

	m_gcState = GcState_Sweep;

	// 3.4) free memory blocks

	count = m_gcMemBlockArray.getCount ();
	for (intptr_t i = count - 1; i >= 0; i--)
	{
		ObjHdr* object = m_gcMemBlockArray [i];
		if (!(object->m_flags & ObjHdrFlag_GcWeakMark))
		{
			m_gcMemBlockArray.remove (i);

			void* block = (object->m_flags & ObjHdrFlag_DynamicArray) ?
				(void*) ((size_t*) object - 1) :
				object;

			AXL_MEM_FREE (block);
		}
		else
		{
			object->m_flags &= ~ObjHdrFlag_GcMask; // unmark
		}
	}

	// 3.5) unmark the remaining objects

	count = m_gcObjectArray.getCount ();
	for (intptr_t i = count - 1; i >= 0; i--)
	{
		jnc::ObjHdr* object = m_gcObjectArray [i];
		object->m_flags &= ~ObjHdrFlag_GcMask;
	}

	// 4) gc run is done, resume all suspended threads

	mt::atomicInc (&m_gcUnsafeThreadCount);
	m_periodGcAllocSize = 0;
	m_gcState = GcState_Idle;
	m_gcIdleEvent.signal ();
	m_lock.unlock ();

	// 5) run destructors

	if (!destructArray.isEmpty ())
	{
		count = destructArray.getCount ();
		for (size_t i = 0; i < count; i++)
		{
			IfaceHdr* iface = destructArray [i];
			Function* destructor = iface->m_object->m_classType->getDestructor ();
			ASSERT (destructor);

			FObject_Destruct* pf = (FObject_Destruct*) destructor->getMachineCode ();
			pf (iface);
		}

		m_lock.lock ();
		m_gcDestructGuardList.remove (&destructGuard);
		m_lock.unlock ();
	}
}

void
Runtime::gcMarkCycle ()
{
	// mark breadth first

	while (!m_gcRootArray [m_currentGcRootArrayIdx].isEmpty ())
	{
		size_t prevGcRootArrayIdx =  m_currentGcRootArrayIdx;
		m_currentGcRootArrayIdx = !m_currentGcRootArrayIdx;
		m_gcRootArray [m_currentGcRootArrayIdx].clear ();

		size_t count = m_gcRootArray [prevGcRootArrayIdx].getCount ();
		for (size_t i = 0; i < count; i++)
		{
			const GcRoot* root = &m_gcRootArray [prevGcRootArrayIdx] [i];
			root->m_type->gcMark (this, root->m_p);
		}
	}
}

void
Runtime::gcEnter ()
{
	TlsHdr* tls = getTlsMgr ()->getTls (this);
	ASSERT (tls);

	tls->m_gcLevel++;
	if (tls->m_gcLevel > 1) // was already unsafe
		gcPulse (); // pulse on enter only, no pulse on leave: might lose retval gcroot
	else
		gcIncrementUnsafeThreadCount ();
}

void
Runtime::gcLeave ()
{
	ASSERT (m_gcState == GcState_Idle || m_gcState == GcState_WaitSafePoint);

	TlsHdr* tls = getTlsMgr ()->getTls (this);
	ASSERT (tls && tls->m_gcLevel);

	tls->m_gcLevel--;
	if (!tls->m_gcLevel) // not unsafe anymore
		gcDecrementUnsafeThreadCount ();
}

void
Runtime::gcPulse ()
{
	ASSERT (m_gcState == GcState_Idle || m_gcState == GcState_WaitSafePoint);

	if (m_gcState != GcState_WaitSafePoint)
		return;

	TlsHdr* tls = getTlsMgr ()->getTls (this);
	ASSERT (tls);

	if (tls->m_gcLevel)
	{
		gcDecrementUnsafeThreadCount ();
		gcIncrementUnsafeThreadCount ();
	}
}

void
Runtime::gcIncrementUnsafeThreadCount ()
{
	// what we try to prevent here is entering an unsafe region when collector thread
	// thinks all the mutators are parked at safe regions and therefore moves on to mark/sweep

	for (;;)
	{
		if (m_gcState != GcState_Idle)
			m_gcIdleEvent.wait ();

		intptr_t oldCount = m_gcUnsafeThreadCount;
		if (oldCount == 0) // oh-oh -- we started gc run in between these two 'if's
			continue;

		intptr_t newCount = oldCount + 1;
		intptr_t prevCount = mt::atomicCmpXchg (&m_gcUnsafeThreadCount, oldCount, newCount);
		if (prevCount == oldCount)
			break;
	}
}

void
Runtime::gcDecrementUnsafeThreadCount ()
{
	intptr_t count = mt::atomicDec (&m_gcUnsafeThreadCount);
	if (m_gcState == GcState_WaitSafePoint)
	{
		if (!count)
			m_gcSafePointEvent.signal ();

		m_gcIdleEvent.wait ();
	}
}

size_t
Runtime::gcMakeThreadSafe ()
{
	TlsHdr* tls = getTlsMgr ()->getTls (this);
	ASSERT (tls);

	if (!tls->m_gcLevel)
		return 0;

	size_t prevGcLevel = tls->m_gcLevel;
	tls->m_gcLevel = 0;
	gcDecrementUnsafeThreadCount ();
	return prevGcLevel;
}

void
Runtime::restoreGcLevel (size_t prevGcLevel)
{
	if (!prevGcLevel)
		return;

	TlsHdr* tls = getTlsMgr ()->getTls (this);
	ASSERT (tls);

	tls->m_gcLevel = prevGcLevel;
	gcIncrementUnsafeThreadCount ();
}

void
Runtime::waitGcIdleAndLock ()
{
	ASSERT (!getTls ()->m_gcLevel); // otherwise we have a potential deadlock

	for (;;)
	{
		m_lock.lock ();

		if (m_gcState == GcState_Idle)
			break;

		m_lock.unlock ();
		m_gcIdleEvent.wait ();
	}
}

TlsHdr*
Runtime::getTls ()
{
	TlsHdr* tls = getTlsMgr ()->getTls (this);
	ASSERT (tls);

	// check for stack overflow

	char* p = (char*) _alloca (1);

	if (!tls->m_stackEpoch) // first time call
	{
		tls->m_stackEpoch = p;
	}
	else
	{
		char* p0 = (char*) tls->m_stackEpoch;
		if (p0 >= p) // the opposite could happen, but it's stack-overflow-safe
		{
			size_t stackSize = p0 - p;
			if (stackSize > m_stackLimit)
			{
				StdLib::runtimeError (RuntimeErrorKind_StackOverflow, NULL);
				ASSERT (false);
			}
		}
	}

	return tls;
}

TlsHdr*
Runtime::createTls ()
{
	size_t size = sizeof (TlsHdr) + m_tlsSize;

	TlsHdr* tls = (TlsHdr*) AXL_MEM_ALLOC (size);
	memset (tls, 0, size);
	tls->m_runtime = this;
	tls->m_stackEpoch = NULL;

	m_lock.lock ();
	m_tlsList.insertTail (tls);
	m_lock.unlock ();

	return tls;
}

void
Runtime::destroyTls (TlsHdr* tls)
{
	m_lock.lock ();
	m_tlsList.remove (tls);
	m_lock.unlock ();

	AXL_MEM_FREE (tls);
}

IfaceHdr*
Runtime::createObject (
	ClassType* type,
	uint_t flags
	)
{
	Function* primer = type->getPrimer ();
	if (!primer) // abstract
	{
		err::setFormatStringError ("cannot create abstract '%s'", type->m_tag.cc ());
		return NULL;
	}

	gcEnter ();

	ObjHdr* object = (ObjHdr*) gcAllocate (type);
	if (!object)
		return NULL;

	ScopeThreadRuntime scopeRuntime (this);

	if (flags & CreateObjectFlag_Prime)
	{
		FObject_Prime* pfPrime = (FObject_Prime*) primer->getMachineCode ();
		pfPrime (object, 0, object, 0);
	}

	IfaceHdr* iface = (IfaceHdr*) (object + 1);

	if ((flags & CreateObjectFlag_Construct) && type->getConstructor ())
	{
		Function* constructor = type->getDefaultConstructor ();
		if (!constructor)
			return NULL;

		FObject_Construct* pfConstruct = (FObject_Construct*) constructor->getMachineCode ();
		pfConstruct (iface);
	}

	if (flags & CreateObjectFlag_Pin)
		pinObject (iface);

	gcLeave ();

	return iface;
}

void
Runtime::pinObject (IfaceHdr* object)
{
	m_lock.lock ();
	m_gcPinTable.visit (object);
	m_lock.unlock ();
}

void
Runtime::unpinObject (IfaceHdr* object)
{
	m_lock.lock ();
	m_gcPinTable.eraseByKey (object);
	m_lock.unlock ();
}

void
Runtime::addStaticDestructor (StaticDestructor *dtor)
{
	StaticDestruct* destruct = AXL_MEM_NEW (StaticDestruct);
	destruct->m_staticDtor = dtor;
	destruct->m_iface = NULL;

	m_lock.lock ();
	m_staticDestructList.insertTail (destruct);
	m_lock.unlock ();
}

void
Runtime::addDestructor (
	Destructor *dtor,
	jnc::IfaceHdr* iface
	)
{
	StaticDestruct* destruct = AXL_MEM_NEW (StaticDestruct);
	destruct->m_dtor = dtor;
	destruct->m_iface = iface;

	m_lock.lock ();
	m_staticDestructList.insertTail (destruct);
	m_lock.unlock ();
}

//.............................................................................

} // namespace jnc
