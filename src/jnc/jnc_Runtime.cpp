#include "pch.h"
#include "jnc_Runtime.h"
#include "jnc_Module.h"
#include "jnc_StdLib.h"
#include "jnc_GcShadowStack.h"

namespace jnc {

//.............................................................................

Runtime::Runtime ()
{
	m_stackSizeLimit = RuntimeDef_StackSizeLimit;
	m_tlsSize = 0;
	m_state = State_Idle;
	m_noThreadEvent.signal ();
}

bool
Runtime::addModule (Module* module)
{
	ASSERT (m_state == State_Idle);

	m_gcHeap.registerStaticRootVariables (module->m_variableMgr.getStaticGcRootArray ());


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
	shutdown ();
	m_state = State_Running;
	m_noThreadEvent.signal ();
	return true;
}
	
void
Runtime::shutdown ()
{
	m_lock.lock ();
	if (m_state == State_Idle)
	{
		m_lock.unlock ();
		return;
	}

	ASSERT (m_state == State_Running); // otherwise, concurrent shutdowns
	m_state = State_ShuttingDown;
	m_lock.unlock ();

	for (;;)
	{
		ASSERT (!mt::getTlsSlotValue <Tls> ());
		m_noThreadEvent.wait (); // wait for other threads

		if (m_staticDestructorList.isEmpty ())
		{
			m_gcHeap.collect ();

			if (m_staticDestructorList.isEmpty ())
				break;
		}

		initializeThread ();

		m_lock.lock ();

		while (!m_staticDestructorList.isEmpty ())
		{
			StaticDestructor* destructor = m_staticDestructorList.removeTail ();
			m_lock.unlock ();

			if (destructor->m_iface)
				destructor->m_destruct (destructor->m_iface);
			else
				destructor->m_staticDestruct ();

			AXL_MEM_DELETE (destructor);

			m_lock.lock ();
		}

		m_lock.unlock ();

		uninitializeThread ();
	}

	m_state = State_Idle;
	
	ASSERT (m_tlsList.isEmpty ());
	ASSERT (m_staticDestructorList.isEmpty ());
	ASSERT (m_gcHeap.isEmpty ());
}

void
Runtime::initializeThread ()
{
	size_t size = sizeof (Tls) + m_tlsSize;

	Tls* tls = AXL_MEM_NEW_EXTRA (Tls, m_tlsSize);
	tls->m_prev = mt::setTlsSlotValue <Tls> (tls);
	tls->m_runtime = this;
	tls->m_stackEpoch = alloca (1);

	ASSERT (!tls->m_prev || tls->m_prev->m_runtime != this); // otherwise, double initialize

	m_lock.lock ();
	ASSERT (m_state == State_Running); // otherwise, creating threads during shutdown
	
	if (m_tlsList.isEmpty ())
		m_noThreadEvent.reset ();
	
	m_tlsList.insertTail (tls);	
	m_lock.unlock ();
	
	m_gcHeap.registerMutatorThread (&tls->m_gcThread);
}

void
Runtime::uninitializeThread ()
{
	Tls* tls = mt::getTlsSlotValue <Tls> ();
	ASSERT (tls && tls->m_runtime == this);

	m_gcHeap.unregisterMutatorThread (&tls->m_gcThread);

	m_lock.lock ();
	m_tlsList.remove (tls);
	
	if (m_tlsList.isEmpty ())
		m_noThreadEvent.signal ();
	
	m_lock.unlock ();
	
	mt::setTlsSlotValue <Tls> (tls->m_prev);
	AXL_MEM_DELETE (tls);
}

void
Runtime::checkStackOverflow ()
{
	Tls* tls = mt::getTlsSlotValue <Tls> ();
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

//.............................................................................

#if 0

Runtime::Runtime ()
{
	m_gcState = GcState_Idle;
	m_gcUnsafeThreadCount = 1;
	m_gcCurrentRootArrayIdx = 0;
	m_tlsSlot = -1;

	m_stackSizeLimit = StdRuntimeLimit_StackSize;
	m_gcPeriodSizeLimit = StdRuntimeLimit_GcPeriodSize;
}

bool
Runtime::create ()
{
	destroy ();

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
		Tls* tls = m_tlsList.removeHead ();
		AXL_MEM_FREE (tls);
	}

	size_t count = m_gcMemBlockArray.getCount ();
	for (intptr_t i = count - 1; i >= 0; i--)
	{
		Box* object = m_gcMemBlockArray [i];
		void* block = (object->m_flags & BoxFlag_DynamicArray) ?
			(void*) ((size_t*) object - 1) :
			object;

		AXL_MEM_FREE (block);
	}

	m_gcState = GcState_Idle;
	m_gcTotalAllocSize = 0;
	m_gcCurrentAllocSize = 0;
	m_gcCurrentPeriodSize = 0;
	m_gcUnsafeThreadCount = 1;
	m_gcCurrentRootArrayIdx = 0;

	m_moduleArray.clear ();
	m_gcObjectArray.clear ();
	m_gcMemBlockArray.clear ();
	m_gcPinTable.clear ();
	m_gcDestructGuardList.clear ();
	m_gcStaticRootArray.clear ();
	m_gcRootArray [0].clear ();
	m_gcRootArray [1].clear ();
	m_staticDestructList.clear ();
}

bool
Runtime::startup ()
{
	shutdown ();

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
	if (m_tlsSlot == -1)
		return;

	rtl::Array <GcRoot> saveStaticGcRootArray = m_gcStaticRootArray;
	m_gcStaticRootArray.clear ();

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

	Tls* tls = getTlsMgr ()->nullifyTls (this);
	if (tls)
	{
		m_tlsList.remove (tls);
		AXL_MEM_FREE (tls);
	}

	ASSERT (m_tlsList.isEmpty ());

	m_gcStaticRootArray = saveStaticGcRootArray; // recover
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

	runtimeError (err::createStringError (errorString));
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

	size_t size = sizeof (Box) + type->getSize () * elementCount;
	if (elementCount > 1)
		size += sizeof (size_t);

	waitGcIdleAndLock ();
	if (m_gcCurrentPeriodSize > m_gcPeriodSizeLimit)
	{
		runGc_l ();
		waitGcIdleAndLock ();
	}

	restoreGcLevel (prevGcLevel); // restore before unlocking

	void* block = AXL_MEM_ALLOC (size);
	if (!block)
	{
		m_lock.unlock ();
		return NULL;
	}

	memset (block, 0, size);

	Box* object;
	void* p;

	if (type->getTypeKind () == TypeKind_Class)
	{
		object = (Box*) block;
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
			object = (Box*) (size + 1);
			object->m_flags = BoxFlag_DynamicArray;
		}
		else
		{
			object = (Box*) block;
			object->m_flags = 0;
		}

		object->m_root = object;
		object->m_type = type;

		p = object + 1;
	}

	m_gcCurrentPeriodSize += size;
	m_gcCurrentAllocSize += size;
	m_gcTotalAllocSize += size;
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

void*
gcAllocateOpaqueObject (
	ClassType* type,
	size_t size
	);

void*
gcTryAllocateOpaqueObject (
	ClassType* type,
	size_t size
	);


void
Runtime::gcAddObject (
	Box* object,
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

		Box* childObject = (Box*) (p + field->getOffset ());
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
	m_gcRootArray [m_gcCurrentRootArrayIdx].append (root);
}

void
Runtime::markGcLocalHeapRoot (
	void* p,
	Type* type
	)
{
	if (type->getTypeKind () == TypeKind_Class)
		((Box*) p)->gcMarkObject (this);
	else
		((Box*) p - 1)->gcMarkData (this);

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
	m_gcCurrentRootArrayIdx = 0;
	m_gcRootArray [0].clear ();

	// 2.0) unmark objects

	size_t count = m_gcObjectArray.getCount ();
	for (size_t i = 0; i < count; i++)
		m_gcObjectArray [i]->m_flags &= ~BoxFlag_GcMask;

	// 2.1) add static roots

	count = m_gcStaticRootArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ASSERT (m_gcStaticRootArray [i].m_type->getFlags () & TypeFlag_GcRoot);
		addGcRoot (
			m_gcStaticRootArray [i].m_p,
			m_gcStaticRootArray [i].m_type
			);
	}

	// 2.2) add stack roots

	rtl::Iterator <Tls> tlsIt = m_tlsList.getHead ();
	for (; tlsIt; tlsIt++)
	{
		Tls* tls = *tlsIt;

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
		jnc::Box* object = m_gcObjectArray [i];

		if (!(object->m_flags & (BoxFlag_GcMark | BoxFlag_GcWeakMark_c)))
		{
			m_gcObjectArray.remove (i);
			object->m_flags |= BoxFlag_Dead;

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

		count = destructGuard->m_destructArray->getCount ();
		for (size_t i = 0; i < count; i++)
		{
			addGcRoot (
				&(*destructGuard->m_destructArray) [i], 
				(*destructGuard->m_destructArray) [i]->m_box->m_classType->getClassPtrType ());
		}
	}

	gcMarkCycle ();

	// 3) sweep (free memory blocks)

	m_gcState = GcState_Sweep;

	size_t freeSize = 0;

	count = m_gcMemBlockArray.getCount ();
	for (intptr_t i = count - 1; i >= 0; i--)
	{
		Box* object = m_gcMemBlockArray [i];
		if (object->m_flags & BoxFlag_GcWeakMark)
			continue;

		m_gcMemBlockArray.remove (i);

		void* block;

		if (object->m_flags & BoxFlag_DynamicArray)
		{
			size_t* count = (size_t*) object - 1;
			block = count;
			freeSize += *count * object->m_type->getSize ();
		}
		else
		{
			block = object;
			freeSize += object->m_type->getSize ();
		}

		AXL_MEM_FREE (block);
	}

	// 4) gc run is done, resume all suspended threads

	mt::atomicInc (&m_gcUnsafeThreadCount);
	m_gcCurrentPeriodSize = 0;
	m_gcCurrentAllocSize -= freeSize;
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
			Function* destructor = iface->m_box->m_classType->getDestructor ();
			ASSERT (destructor);

			Class_DestructFunc* p = (Class_DestructFunc*) destructor->getMachineCode ();
			p (iface);
		}

		// now we can remove this thread' destruct guard

		m_lock.lock ();
		m_gcDestructGuardList.remove (&destructGuard);
		m_lock.unlock ();
	}
}

void
Runtime::gcMarkCycle ()
{
	// mark breadth first

	while (!m_gcRootArray [m_gcCurrentRootArrayIdx].isEmpty ())
	{
		size_t prevGcRootArrayIdx =  m_gcCurrentRootArrayIdx;
		m_gcCurrentRootArrayIdx = !m_gcCurrentRootArrayIdx;
		m_gcRootArray [m_gcCurrentRootArrayIdx].clear ();

		size_t count = m_gcRootArray [prevGcRootArrayIdx].getCount ();
		for (size_t i = 0; i < count; i++)
		{
			const GcRoot* root = &m_gcRootArray [prevGcRootArrayIdx] [i];
			root->m_type->markGcRoots (root->m_p, this);
		}
	}
}

void
Runtime::gcEnter ()
{
	Tls* tls = getTlsMgr ()->getTls (this);
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

	Tls* tls = getTlsMgr ()->getTls (this);
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

	Tls* tls = getTlsMgr ()->getTls (this);
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
	Tls* tls = getTlsMgr ()->getTls (this);
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

	Tls* tls = getTlsMgr ()->getTls (this);
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

	Box* object = (Box*) gcAllocate (type);
	if (!object)
		return NULL;

	ScopeThreadRuntime scopeRuntime (this);

	if (flags & CreateObjectFlag_Prime)
	{
		Class_Prime* prime = (Class_Prime*) primer->getMachineCode ();
		prime (object, object, 0);
	}

	IfaceHdr* iface = (IfaceHdr*) (object + 1);

	if ((flags & CreateObjectFlag_Construct) && type->getConstructor ())
	{
		Function* constructor = type->getDefaultConstructor ();
		if (!constructor)
			return NULL;

		Class_ConstructFunc* pfConstruct = (Class_ConstructFunc*) constructor->getMachineCode ();
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

#endif

//.............................................................................

} // namespace jnc
