#include "pch.h"
#include "jnc_GcHeap.h"
#include "jnc_Module.h"
#include "jnc_Api.h"
#include "jnc_CallFunction.h"

namespace jnc {

//.............................................................................

#if (_AXL_ENV == AXL_ENV_POSIX)
sigset_t GcHeap::m_signalWaitMask = { 0 };
#endif

GcHeap::GcHeap ()
{
	m_runtime = AXL_CONTAINING_RECORD (this, Runtime, m_gcHeap);
	m_state = State_Idle;
	m_flags = 0;
	m_handshakeCount = 0;
	m_waitingMutatorThreadCount = 0;
	m_noCollectMutatorThreadCount = 0;
	m_currentMarkRootArrayIdx = 0;
	m_allocSizeTrigger = GcDef_AllocSizeTrigger;
	m_periodSizeTrigger = GcDef_PeriodSizeTrigger;
	m_idleEvent.signal ();
	memset (&m_stats, 0, sizeof (m_stats));

#if (_AXL_ENV == AXL_ENV_WIN)
	m_guardPage.alloc (4 * 1024); // typical page size (OS will not give us less than that anyway)
#elif (_AXL_ENV == AXL_ENV_POSIX)
	m_guardPage.map (
		NULL,
		4 * 1024,
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS,
		-1,
		0
		);
#endif
}

void 
GcHeap::getStats (GcStats* stats)
{
	m_lock.lock (); // no need to wait for idle to just get stats
	*stats = m_stats;
	m_lock.unlock ();
}

void 
GcHeap::setSizeTriggers (
	size_t allocSizeTrigger,
	size_t periodSizeTrigger
	)
{
	bool isMutatorThread = waitIdleAndLock ();

	m_allocSizeTrigger = allocSizeTrigger;
	m_periodSizeTrigger = periodSizeTrigger;

	if (isCollectionTriggered_l ())
		collect_l (isMutatorThread);
	else
		m_lock.unlock ();
}

void
GcHeap::startup (Module* module)
{
	ASSERT (m_state == State_Idle);

	memset (&m_stats, 0, sizeof (m_stats));
	m_flags = 0;

	if (module->getCompileFlags () & ModuleCompileFlag_SimpleGcSafePoint)
	{
		m_flags |= GcHeapFlag_SimpleSafePoint;
	}
	else
	{
		Variable* safePointTriggerVariable = module->m_variableMgr.getStdVariable (StdVariable_GcSafePointTrigger);
		*(void**) safePointTriggerVariable->getStaticData () = m_guardPage;
	}

	addStaticRootVariables (module->m_variableMgr.getStaticGcRootArray ());

	Function* destructor = module->getDestructor ();
	if (destructor)
		addStaticDestructor ((StaticDestructFunc*) destructor->getMachineCode ());
}

// locking

bool
GcHeap::waitIdleAndLock ()
{
	GcMutatorThread* thread = getCurrentGcMutatorThread ();

	bool isMutatorThread = thread && !thread->m_waitRegionLevel;
	if (!isMutatorThread)
	{
		m_lock.lock ();
		while (m_state != State_Idle)
		{
			m_lock.unlock ();
			m_idleEvent.wait ();
			m_lock.lock ();
		}
	}
	else
	{
		m_lock.lock ();

		while (m_state == State_StopTheWorld)
		{
			m_lock.unlock ();			
			safePoint ();
			m_lock.lock ();
		}
		
		if (m_state != State_Idle) 
		{
			// some collection phase other than stop the world			
			// we can safely mark ourselves as waiting-mutator

			thread->m_waitRegionLevel = 1;
			m_waitingMutatorThreadCount++;

			do 
			{
				m_lock.unlock ();
				m_idleEvent.wait ();
				m_lock.lock ();
			} while (m_state != State_Idle);
				
			thread->m_waitRegionLevel = 0;
			m_waitingMutatorThreadCount--;
		}
	}

	return isMutatorThread;
}

bool
GcHeap::isCollectionTriggered_l ()
{
	return 
		m_noCollectMutatorThreadCount == 0 &&
		(m_stats.m_currentPeriodSize > m_periodSizeTrigger || m_stats.m_currentAllocSize > m_allocSizeTrigger);
}

void
GcHeap::incrementAllocSize_l (size_t size)
{
	m_stats.m_totalAllocSize += size;
	m_stats.m_currentAllocSize += size;
	m_stats.m_currentPeriodSize += size;

	if (m_stats.m_currentAllocSize > m_stats.m_peakAllocSize)
		m_stats.m_peakAllocSize = m_stats.m_currentAllocSize;
}

void
GcHeap::incrementAllocSizeAndLock (size_t size)
{
	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (isMutatorThread);  // allocations should only be done in registered mutator threads 
	                           // otherwise there is risk of loosing new object	

	incrementAllocSize_l (size);
	
	if (isCollectionTriggered_l ())
	{
		collect_l (isMutatorThread);
		waitIdleAndLock ();
	}
}

// allocation methods

IfaceHdr* 
GcHeap::tryAllocateClass (ClassType* type)
{
	size_t size = type->getSize ();
	Box* box = (Box*) AXL_MEM_ALLOCATE (size);
	if (!box)
	{
		err::setFormatStringError ("not enough memory for '%s'", type->getTypeString ().cc ());
		return NULL;
	}

	prime (box, type);

	incrementAllocSizeAndLock (size);
	m_allocBoxArray.append (box);
	addClassBox_l (box);
	m_lock.unlock ();

	return (IfaceHdr*) (box + 1);
}

IfaceHdr* 
GcHeap::allocateClass (ClassType* type)
{
	IfaceHdr* iface = tryAllocateClass (type);
	if (!iface)
	{
		Runtime::runtimeError (err::getLastError ());
		ASSERT (false);
	}

	return iface;
}

void
GcHeap::addClassBox_l (Box* box)
{
	ASSERT (box->m_type->getTypeKind () == TypeKind_Class);
	ClassType* classType = (ClassType*) box->m_type;

	char* p = (char*) (box + 1);

	rtl::Array <StructField*> classFieldArray = classType->getClassMemberFieldArray ();
	size_t count = classFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = classFieldArray [i];
		Box* childBox = (Box*) (p + field->getOffset ());

		ASSERT (
			field->getType ()->getTypeKind () == TypeKind_Class &&
			childBox->m_type == field->getType ());

		addClassBox_l (childBox);
	}

	m_classBoxArray.append (box); // after all the children

	if (classType->getDestructor ())
		m_destructibleClassBoxArray.append (box);
}

DataPtr
GcHeap::tryAllocateData (Type* type)
{
	size_t size = type->getSize ();

	DataBox* box = AXL_MEM_NEW_EXTRA (DataBox, size);
	if (!box)
	{
		err::setFormatStringError ("not enough memory for '%s'", type->getTypeString ().cc ());
		return g_nullPtr;
	}

	box->m_type = type;
	box->m_flags = BoxFlag_DataMark | BoxFlag_WeakMark;
	box->m_validator.m_validatorBox = box;
	box->m_validator.m_targetBox = box;
	box->m_validator.m_rangeBegin = box + 1;
	box->m_validator.m_rangeLength = size;

	incrementAllocSizeAndLock (size);
	m_allocBoxArray.append (box);
	m_lock.unlock ();

	DataPtr ptr;
	ptr.m_p = box + 1;
	ptr.m_validator = &box->m_validator;
	return ptr;
}

DataPtr
GcHeap::allocateData (Type* type)
{
	DataPtr ptr = tryAllocateData (type);
	if (!ptr.m_p)
	{
		Runtime::runtimeError (err::getLastError ());
		ASSERT (false);
	}

	return ptr;
}

DataPtr
GcHeap::tryAllocateArray (
	Type* type,
	size_t count
	)
{
	size_t size = type->getSize () * count;

	DynamicArrayBox* box = AXL_MEM_NEW_EXTRA (DynamicArrayBox, size);
	if (!box)
	{
		err::setFormatStringError ("not enough memory for '%s [%d]'", type->getTypeString ().cc (), count);
		return g_nullPtr;
	}

	box->m_type = type;
	box->m_flags = BoxFlag_DynamicArray | BoxFlag_DataMark | BoxFlag_WeakMark;
	box->m_count = count;
	box->m_validator.m_validatorBox = box;
	box->m_validator.m_targetBox = box;
	box->m_validator.m_rangeBegin = box + 1;
	box->m_validator.m_rangeLength = size;

	incrementAllocSizeAndLock (size);
	m_allocBoxArray.append (box);
	m_lock.unlock ();

	DataPtr ptr;
	ptr.m_p = box + 1;
	ptr.m_validator = &box->m_validator;
	return ptr;
}

DataPtr
GcHeap::allocateArray (
	Type* type,
	size_t count
	)
{
	DataPtr ptr = tryAllocateArray (type, count);
	if (!ptr.m_p)
	{
		Runtime::runtimeError (err::getLastError ());
		ASSERT (false);
	}

	return ptr;
}

DataPtr
GcHeap::tryAllocateBuffer (size_t size)
{
	Module* module = m_runtime->getModule ();
	ASSERT (module);

	Type* type = module->m_typeMgr.getPrimitiveType (TypeKind_Char);
	return tryAllocateArray (type, size);
}

DataPtr
GcHeap::allocateBuffer (size_t size)
{
	Module* module = m_runtime->getModule ();
	ASSERT (module);

	Type* type = module->m_typeMgr.getPrimitiveType (TypeKind_Char);
	return allocateArray (type, size);
}

DataPtrValidator*
GcHeap::createDataPtrValidator (
	Box* box,
	void* rangeBegin,
	size_t rangeLength
	)
{
	ASSERT (GcDef_DataPtrValidatorPoolSize >= 1);

	DataPtrValidator* validator;
	
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && !thread->m_waitRegionLevel);

	if (thread->m_dataPtrValidatorPoolBegin)
	{
		validator = thread->m_dataPtrValidatorPoolBegin;
		thread->m_dataPtrValidatorPoolBegin++;

		if (thread->m_dataPtrValidatorPoolBegin < thread->m_dataPtrValidatorPoolEnd)
		{
			thread->m_dataPtrValidatorPoolBegin->m_validatorBox = validator->m_validatorBox;
		}
		else
		{
			thread->m_dataPtrValidatorPoolBegin = NULL;
			thread->m_dataPtrValidatorPoolEnd = NULL;
		}
	}
	else
	{
		size_t size = sizeof (DataPtrValidator) * GcDef_DataPtrValidatorPoolSize;
		
		DynamicArrayBoxHdr* box = AXL_MEM_NEW_EXTRA (DynamicArrayBoxHdr, size);
		if (!box)
		{
			Runtime::runtimeError (err::getLastError ());
			ASSERT (false);
		}

		box->m_type = m_runtime->getModule ()->m_typeMgr.getStdType (StdType_DataPtrValidator);
		box->m_flags = BoxFlag_DynamicArray | BoxFlag_DataMark | BoxFlag_WeakMark;
		box->m_rootOffset = 0;
		box->m_count = GcDef_DataPtrValidatorPoolSize;

		incrementAllocSizeAndLock (size);
		m_allocBoxArray.append (box);
		m_lock.unlock ();

		validator = (DataPtrValidator*) (box + 1);
		validator->m_validatorBox = box;

		if (GcDef_DataPtrValidatorPoolSize >= 2)
		{
			thread->m_dataPtrValidatorPoolBegin = validator + 1;
			thread->m_dataPtrValidatorPoolBegin->m_validatorBox = box;
			thread->m_dataPtrValidatorPoolEnd = validator + GcDef_DataPtrValidatorPoolSize;
		}
	}

	validator->m_targetBox = box;
	validator->m_rangeBegin = rangeBegin;
	validator->m_rangeLength = rangeLength;
	return validator;
}

// management

void
GcHeap::beginShutdown ()
{
	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (!isMutatorThread);
	
	m_flags |= GcHeapFlag_ShuttingDown;  // this will prevent boxes from being actually freed

	// initial collect

	for (size_t i = 0; i < GcDef_ShutdownIterationLimit; i++)
	{
		m_staticRootArray.clear (); // drop roots before every collect
		collect_l (false);

		waitIdleAndLock ();

		if (m_allocBoxArray.isEmpty ())
			break;
	}

	m_lock.unlock ();
}

void
GcHeap::finalizeShutdown ()
{
	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (!isMutatorThread && (m_flags & GcHeapFlag_ShuttingDown));
	
	// static destructors

	while (!m_staticDestructorList.isEmpty ())
	{
		StaticDestructor* destructor = m_staticDestructorList.removeTail ();
		m_lock.unlock ();

		int retVal;

		bool result = destructor->m_iface ? 
			callFunctionImpl_s (m_runtime, (void*) destructor->m_destructFunc, &retVal, destructor->m_iface) :
			callFunctionImpl_s (m_runtime, (void*) destructor->m_staticDestructFunc, &retVal);

		AXL_MEM_DELETE (destructor);

		waitIdleAndLock ();
	}

	// final collect

	for (size_t i = 0; i < GcDef_ShutdownIterationLimit; i++)
	{
		m_staticRootArray.clear (); // drop roots before every collect
		collect_l (false);

		waitIdleAndLock ();

		if (m_allocBoxArray.isEmpty ())
			break;
	}

	// postponed free

	rtl::Array <Box*> postponeFreeBoxArray = m_postponeFreeBoxArray;
	m_postponeFreeBoxArray.clear ();
	m_flags &= ~GcHeapFlag_ShuttingDown;
	m_lock.unlock ();

	size_t count = postponeFreeBoxArray.getCount ();
	for (size_t i = 0; i < count; i++)
		AXL_MEM_FREE (postponeFreeBoxArray [i]);

	// everything should be empty now (if destructors don't play hardball)

	ASSERT (!m_noCollectMutatorThreadCount && !m_waitingMutatorThreadCount);
	ASSERT (m_allocBoxArray.isEmpty () && m_classBoxArray.isEmpty ());

	// force-clear anyway

	m_noCollectMutatorThreadCount = 0;
	m_waitingMutatorThreadCount = 0;

	m_allocBoxArray.clear ();
	m_classBoxArray.clear ();
	m_destructibleClassBoxArray.clear ();
	m_destructGuardList.clear ();
}

void
GcHeap::addStaticRootVariables (
	Variable* const* variableArray,
	size_t count
	)
{
	if (!count)
		return;

	char buffer [256];
	rtl::Array <Root> rootArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	rootArray.setCount (count);

	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = variableArray [i];
		
		rootArray [i].m_p = variable->getStaticData ();
		rootArray [i].m_type = variable->getType ();
	}

	waitIdleAndLock ();
	m_staticRootArray.append (rootArray);
	m_lock.unlock ();
}

void
GcHeap::addStaticRoot (
	void* p,
	Type* type
	)
{
	Root root = { p, type };

	waitIdleAndLock ();
	m_staticRootArray.append (root);
	m_lock.unlock ();
}

void
GcHeap::addStaticDestructor (StaticDestructFunc* func)
{
	StaticDestructor* destruct = AXL_MEM_NEW (StaticDestructor);
	destruct->m_staticDestructFunc = func;
	destruct->m_iface = NULL;

	waitIdleAndLock ();
	m_staticDestructorList.insertTail (destruct);
	m_lock.unlock ();
}

void
GcHeap::addStaticClassDestructor (
	DestructFunc* func,
	jnc::IfaceHdr* iface
	)
{
	StaticDestructor* destruct = AXL_MEM_NEW (StaticDestructor);
	destruct->m_destructFunc = func;
	destruct->m_iface = iface;

	waitIdleAndLock ();
	m_staticDestructorList.insertTail (destruct);
	m_lock.unlock ();
}

void
GcHeap::registerMutatorThread (GcMutatorThread* thread)
{
	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (!isMutatorThread); // we are in the process of registering this thread

	thread->m_threadId = mt::getCurrentThreadId ();
	thread->m_isSafePoint = false;
	thread->m_waitRegionLevel = 0;
	thread->m_noCollectRegionLevel = 0;
	thread->m_dataPtrValidatorPoolBegin = NULL;
	thread->m_dataPtrValidatorPoolEnd = NULL;

	m_mutatorThreadList.insertTail (thread);
	m_lock.unlock ();
}

void
GcHeap::unregisterMutatorThread (GcMutatorThread* thread)
{
	ASSERT (thread->m_threadId == mt::getCurrentThreadId ());

	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (isMutatorThread);

	if (thread->m_waitRegionLevel) // might be non-zero on exception
		m_waitingMutatorThreadCount--;

	if (thread->m_noCollectRegionLevel) // might be non-zero on exception
		m_noCollectMutatorThreadCount--;

	m_mutatorThreadList.remove (thread);
	m_lock.unlock ();
}

void
GcHeap::enterWaitRegion ()
{
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && !thread->m_noCollectRegionLevel);

	if (thread->m_waitRegionLevel) // already there
	{
		thread->m_waitRegionLevel++;
		return;
	}

	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (isMutatorThread);
	thread->m_waitRegionLevel = 1;
	m_waitingMutatorThreadCount++;
	ASSERT (m_waitingMutatorThreadCount <= m_mutatorThreadList.getCount ());

	dbg::trace ("GcHeap::enterWaitRegion () (tid = %x)\n", (uint_t) mt::getCurrentThreadId ());

	m_lock.unlock ();			
}

void
GcHeap::leaveWaitRegion ()
{
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && !thread->m_noCollectRegionLevel && thread->m_waitRegionLevel);

	if (thread->m_waitRegionLevel > 1) // still there
	{
		thread->m_waitRegionLevel--;
		return;
	}

	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (!isMutatorThread);
	thread->m_waitRegionLevel = 0;
	m_waitingMutatorThreadCount--;

	dbg::trace ("GcHeap::leaveWaitRegion () (tid = %x)\n", (uint_t) mt::getCurrentThreadId ());

	m_lock.unlock ();
}

void
GcHeap::enterNoCollectRegion ()
{
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && !thread->m_waitRegionLevel);

	if (thread->m_noCollectRegionLevel) // already there
	{
		thread->m_noCollectRegionLevel++;
		return;
	}

	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (isMutatorThread);
	thread->m_noCollectRegionLevel = 1;
	m_noCollectMutatorThreadCount++;
	ASSERT (m_waitingMutatorThreadCount <= m_mutatorThreadList.getCount ());

	dbg::trace ("GcHeap::enterNoCollectRegion () (tid = %x)\n", (uint_t) mt::getCurrentThreadId ());

	m_lock.unlock ();			
}

void
GcHeap::leaveNoCollectRegion (bool canCollectNow)
{
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && !thread->m_waitRegionLevel && thread->m_noCollectRegionLevel);

	if (thread->m_noCollectRegionLevel > 1) // still there
	{
		thread->m_noCollectRegionLevel--;
		return;
	}

	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (isMutatorThread);
	thread->m_noCollectRegionLevel = 0;
	m_noCollectMutatorThreadCount--;

	dbg::trace ("GcHeap::leaveNoCollectRegion (%d) (tid = %x)\n", canCollectNow, (uint_t) mt::getCurrentThreadId ());

	if (canCollectNow && isCollectionTriggered_l ())
		collect_l (isMutatorThread);
	else
		m_lock.unlock ();
}

void
GcHeap::safePoint ()
{
#ifdef _AXL_DEBUG
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && thread->m_waitRegionLevel == 0); // otherwise we may finish handshake prematurely
#endif

	if (!(m_flags & GcHeapFlag_SimpleSafePoint))
		mt::atomicXchg ((volatile int32_t*) m_guardPage.p (), 0); // we need a fence, hence atomicXchg
	else if (m_state == State_StopTheWorld)
		parkAtSafePoint (); // parkAtSafePoint will force a fence with atomicDec
}

void
GcHeap::weakMark (Box* box)
{
	if (box->m_flags & BoxFlag_WeakMark)
		return;

	box->m_flags |= BoxFlag_WeakMark;

	if (box->m_rootOffset)
	{
		Box* root = (Box*) ((char*) box - box->m_rootOffset);
		if (!(root->m_flags & BoxFlag_WeakMark))
			root->m_flags |= BoxFlag_WeakMark;
	}
}

void
GcHeap::markData (Box* box)
{
	if (box->m_flags & BoxFlag_DataMark)
		return;

	weakMark (box);

	box->m_flags |= BoxFlag_DataMark;

	if (!(box->m_type->getFlags () & TypeFlag_GcRoot))
		return;

	ASSERT (!(box->m_flags & BoxFlag_StaticData));
	if (box->m_type->getTypeKind () == TypeKind_Class)
	{
		addRoot (box, box->m_type);
	}
	else if (!(box->m_flags & BoxFlag_DynamicArray))
	{
		addRoot ((DataBox*) box + 1, box->m_type);
	}
	else
	{
		DynamicArrayBox* arrayBox = (DynamicArrayBox*) box;		
		addRootArray (arrayBox + 1, arrayBox->m_type, arrayBox->m_count);
	}
}

void
GcHeap::markClass (Box* box)
{
	if (box->m_flags & BoxFlag_ClassMark)
		return;

	weakMark (box);
	markClassFields (box);

	box->m_flags |= BoxFlag_ClassMark | BoxFlag_DataMark;

	if (box->m_type->getFlags () & TypeFlag_GcRoot)
		addRoot (box, box->m_type);
}

void
GcHeap::markClassFields (Box* box)
{
	ASSERT (box->m_type->getTypeKind () == TypeKind_Class);
	
	char* p0 = (char*) (box + 1);
	ClassType* classType = (ClassType*) box->m_type;
	rtl::Array <StructField*> classMemberFieldArray = classType->getClassMemberFieldArray ();
	size_t count = classMemberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = classMemberFieldArray [i];
		Box* fieldBox = (Box*) (p0 + field->getOffset ());
		ASSERT (fieldBox->m_type == field->getType ());

		if (fieldBox->m_flags & BoxFlag_ClassMark)
			continue;

		fieldBox->m_flags |= BoxFlag_ClassMark | BoxFlag_DataMark | BoxFlag_WeakMark;
		markClassFields (fieldBox);
	}
}

void
GcHeap::weakMarkClosureClass (Box* box)
{
	ASSERT (!box->m_rootOffset && box->m_type->getTypeKind () == TypeKind_Class);

	if (box->m_flags & (BoxFlag_ClassMark | BoxFlag_ClosureWeakMark))
		return;

	ASSERT (isClosureClassType (box->m_type));
	ClosureClassType* closureClassType = (ClosureClassType*) box->m_type;
	size_t thisArgFieldIdx = closureClassType->getThisArgFieldIdx ();
	if (thisArgFieldIdx == -1)
	{
		markClass (box);
		return;
	}

	weakMark (box);
	box->m_flags |= BoxFlag_ClosureWeakMark;

	char* p0 = (char*) (box + 1);

	// add this arg as weak pointer

	StructField* thisArgField = closureClassType->getFieldByIndex (thisArgFieldIdx);
	ASSERT (thisArgField && thisArgField->getType ()->getTypeKind () == TypeKind_ClassPtr);
	ClassPtrType* weakPtrType = ((ClassPtrType*) (thisArgField->getType ()))->getWeakPtrType ();
	addRoot (p0 + thisArgField->getOffset (), weakPtrType);

	rtl::Array <StructField*> gcRootFieldArray = closureClassType->getGcRootMemberFieldArray ();
	size_t count = gcRootFieldArray.getCount ();	

	for (size_t i = 0; i < count; i++)
	{
		StructField* field = gcRootFieldArray [i];
		if (field != thisArgField)
			addRoot (p0 + field->getOffset (), field->getType ());
	}
}

void
GcHeap::addRoot (
	const void* p,
	Type* type
	)
{
	ASSERT (m_state == State_Mark && p);

	if (type->getFlags () & TypeFlag_GcRoot)
	{
		Root root = { p, type };
		m_markRootArray [m_currentMarkRootArrayIdx].append (root);
	}
	else // dynamic validator or heap variable
	{
		ASSERT (isDataPtrType (type, DataPtrTypeKind_Thin));
		Type* targetType = ((DataPtrType*) type)->getTargetType ();

		if (targetType->getStdType () == StdType_DataPtrValidator)
		{
			DataPtrValidator* validator = (DataPtrValidator*) p;
			ASSERT (validator->m_validatorBox->m_type == targetType);
			weakMark (validator->m_validatorBox);
		}
		else if (targetType->getTypeKind () == TypeKind_Class)
		{
			Box* box = ((Box*) p) - 1;
			ASSERT (box->m_type == targetType);
			markClass (box);
		}
		else
		{
			DataBox* box = ((DataBox*) p) - 1;
			ASSERT (box->m_type == targetType);
			markData (box);
		}
	}
}

void
GcHeap::addRootArray (
	const void* p0,
	Type* type,
	size_t count
	)
{
	ASSERT (type->getTypeKind () != TypeKind_Class && (type->getFlags () & TypeFlag_GcRoot));

	rtl::Array <Root>* markRootArray = &m_markRootArray [m_currentMarkRootArrayIdx];
	size_t baseCount = markRootArray->getCount ();
	markRootArray->setCount (baseCount + count);

	const char* p = (const char*) p0;
	for (size_t i = 0, j = baseCount; i < count; i++, j++)
	{
		(*markRootArray) [j].m_p = p;
		(*markRootArray) [j].m_type = type;
		p += type->getSize (); 
	}
}

void
GcHeap::collect ()
{
	bool isMutatorThread = waitIdleAndLock ();

	if (!m_noCollectMutatorThreadCount)
		collect_l (isMutatorThread);
	else
		m_lock.unlock (); // not now
}

void
GcHeap::collect_l (bool isMutatorThread)
{
	ASSERT (!m_noCollectMutatorThreadCount && m_waitingMutatorThreadCount <= m_mutatorThreadList.getCount ());

	m_stats.m_totalCollectCount++;
	m_stats.m_lastCollectTime = g::getTimestamp ();

	bool isShuttingDown = (m_flags & GcHeapFlag_ShuttingDown) != 0;

	// stop the world
	
	size_t handshakeCount = m_mutatorThreadList.getCount () - m_waitingMutatorThreadCount;
	if (isMutatorThread)
	{
		ASSERT (handshakeCount);
		handshakeCount--; // minus this thread
	}

	dbg::trace (
		"+++ GcHeap::collect_l (tid = %x; isMutator = %d; mutatorCount = %d; waitingMutatorThreadCount = %d, handshakeCount = %d)\n",
		(uint_t) mt::getCurrentThreadId (),
		isMutatorThread,
		m_mutatorThreadList.getCount (),
		m_waitingMutatorThreadCount,
		handshakeCount
		);

	rtl::Iterator <GcMutatorThread> threadIt = m_mutatorThreadList.getHead ();
	for (; threadIt; threadIt++)
	{
		dbg::trace ("   *** mutator (tid = %x; wait = %d)\n",
			(uint_t) threadIt->m_threadId,
			threadIt->m_waitRegionLevel
			);
	}

	if (!handshakeCount)
	{
		m_state = State_Mark;
		m_idleEvent.reset ();
		m_lock.unlock ();
	}
	else if (m_flags & GcHeapFlag_SimpleSafePoint)
	{
		m_resumeEvent.reset ();
		mt::atomicXchg (&m_handshakeCount, handshakeCount);
		m_state = State_StopTheWorld;
		m_idleEvent.reset ();
		m_lock.unlock ();

		m_handshakeEvent.wait ();
	}
	else
	{
#if (_AXL_ENV == AXL_ENV_WIN)
		m_resumeEvent.reset ();
		mt::atomicXchg (&m_handshakeCount, handshakeCount);
		m_state = State_StopTheWorld;
		m_idleEvent.reset ();
		m_lock.unlock ();

		m_guardPage.protect (PAGE_NOACCESS);
		m_handshakeEvent.wait ();
#elif (_AXL_ENV == AXL_ENV_POSIX)
		mt::atomicXchg (&m_handshakeCount, handshakeCount);
		m_state = State_StopTheWorld;
		m_idleEvent.reset ();
		m_lock.unlock ();

		static volatile int32_t installSignalHandlersFlag = 0;
		mt::callOnce (installSignalHandlers, 0, &installSignalHandlersFlag);
		m_guardPage.protect (PROT_NONE);
		m_handshakeSem.wait ();
#endif

		m_state = State_Mark;
	}

	// the world is stopped, mark (no lock is needed)

	dbg::trace ("   ... GcHeap::collect_l () -- the world is stopped\n");

	m_currentMarkRootArrayIdx = 0;
	m_markRootArray [0].clear ();

	// unmark everything

	size_t count = m_allocBoxArray.getCount ();
	for (size_t i = 0; i < count; i++)
		m_allocBoxArray [i]->m_flags &= ~BoxFlag_MarkMask;

	count = m_classBoxArray.getCount ();
	for (size_t i = 0; i < count; i++)
		m_classBoxArray [i]->m_flags &= ~BoxFlag_MarkMask;

	// add static roots

	count = m_staticRootArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ASSERT (m_staticRootArray [i].m_type->getFlags () & TypeFlag_GcRoot);
		addRoot (
			m_staticRootArray [i].m_p,
			m_staticRootArray [i].m_type
			);
	}

	// add stack and tls roots and validator pools

	StructType* tlsType = m_runtime->getModule ()->m_variableMgr.getTlsStructType ();
	rtl::Array <StructField*> tlsRootFieldArray = tlsType->getGcRootMemberFieldArray ();
	size_t tlsRootFieldCount = tlsRootFieldArray.getCount ();

	threadIt = m_mutatorThreadList.getHead ();
	for (; threadIt; threadIt++)
	{
		GcMutatorThread* thread = *threadIt;

		// stack roots

		TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (thread + 1);
		GcShadowStackFrame* frame = tlsVariableTable->m_shadowStackTop;
		for (; frame; frame = frame->m_prev)
		{
			void** rootArray = (void**) (frame + 1);
			Type** typeArray = (Type**) (frame->m_map + 1);

			for (size_t i = 0; i < frame->m_map->m_count; i++)
			{
				void* p = rootArray [i];
				if (p) // stack roots could be nullified
					addRoot (p, typeArray [i]);
			}
		}

		// tls roots

		for (size_t i = 0; i < tlsRootFieldCount; i++)
		{
			StructField* field = tlsRootFieldArray [i];
			addRoot ((char*) tlsVariableTable + field->getOffset (), field->getType ());
		}

		// validator pool

		if (thread->m_dataPtrValidatorPoolBegin)
			weakMark (thread->m_dataPtrValidatorPoolBegin->m_validatorBox);
	}

	// run mark cycle

	runMarkCycle ();

	dbg::trace ("   ... GcHeap::collect_l () -- mark complete\n");

	// schedule destruction for unmarked class boxes

	char buffer [256];
	rtl::Array <IfaceHdr*> destructArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	DestructGuard destructGuard;

	size_t dstIdx = 0;
	count = m_destructibleClassBoxArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Box* box = m_destructibleClassBoxArray [i];
		ASSERT (!(box->m_flags & BoxFlag_Zombie) && ((ClassType*) box->m_type)->getDestructor ());
		
		if (box->m_flags & (BoxFlag_ClassMark | BoxFlag_ClosureWeakMark))
		{
			m_destructibleClassBoxArray [dstIdx++] = box;
		}
		else
		{
			IfaceHdr* iface = (IfaceHdr*) (box + 1);
			ASSERT (iface->m_box == box);

			box->m_flags |= BoxFlag_Zombie;
			destructArray.append (iface);
		}
	}

	m_destructibleClassBoxArray.setCount (dstIdx);

	// add destruct guard for this thread

	if (!destructArray.isEmpty ())
	{
		destructGuard.m_destructArray = &destructArray;
		m_destructGuardList.insertTail (&destructGuard);
	}
	
	if (!m_destructGuardList.isEmpty ())
	{
		rtl::Iterator <DestructGuard> destructGuardIt = m_destructGuardList.getHead ();
		for (; destructGuardIt; destructGuardIt++)
		{
			DestructGuard* destructGuard = *destructGuardIt;

			IfaceHdr** iface = *destructGuard->m_destructArray;
			count = destructGuard->m_destructArray->getCount ();
			for (size_t i = 0; i < count; i++, iface++)
			{
				ClassType* classType = (ClassType*) (*iface)->m_box->m_type;
				addRoot (iface, classType->getClassPtrType ());
			}
		}
				
		runMarkCycle ();
	}

	// remove unmarked class boxes

	dstIdx = 0;
	count = m_classBoxArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Box* box = m_classBoxArray [i];
		if (box->m_flags & (BoxFlag_ClassMark | BoxFlag_ClosureWeakMark))
			m_classBoxArray [dstIdx++] = box;
	}

	m_classBoxArray.setCount (dstIdx);

	// sweep 

	m_state = State_Sweep;
	size_t freeSize = 0;

	dstIdx = 0;
	count = m_allocBoxArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Box* box = m_allocBoxArray [i];
		if (box->m_flags & BoxFlag_WeakMark)
		{
			m_allocBoxArray [dstIdx] = box;
			dstIdx++;
		}
		else
		{
			size_t size = box->m_type->getSize ();
			if (box->m_flags & BoxFlag_DynamicArray)
				size *= ((DynamicArrayBoxHdr*) box)->m_count;

			freeSize += size;

			if (isShuttingDown)
				m_postponeFreeBoxArray.append (box);
			else
				AXL_MEM_FREE (box);
		}
	}

	m_allocBoxArray.setCount (dstIdx);

	dbg::trace ("   ... GcHeap::collect_l () -- sweep complete\n");

	// resume the world

	if (handshakeCount)
	{
		if (m_flags & GcHeapFlag_SimpleSafePoint)
		{
			mt::atomicXchg (&m_handshakeCount, handshakeCount);
			m_state = State_ResumeTheWorld;
			m_resumeEvent.signal ();
			m_handshakeEvent.wait ();
		}
		else
		{
#if (_AXL_ENV == AXL_ENV_WIN)
			m_guardPage.protect (PAGE_READWRITE);
			mt::atomicXchg (&m_handshakeCount, handshakeCount);
			m_state = State_ResumeTheWorld;
			m_resumeEvent.signal ();
			m_handshakeEvent.wait ();
#elif (_AXL_ENV == AXL_ENV_POSIX)
			m_guardPage.protect (PROT_READ | PROT_WRITE);
			mt::atomicXchg (&m_handshakeCount, handshakeCount);
			m_state = State_ResumeTheWorld;

			for (;;) // we need a loop -- sigsuspend can lose per-thread signals
			{
				bool result;

				threadIt = m_mutatorThreadList.getHead ();
				for (; threadIt; threadIt++)
				{
					if (threadIt->m_isSafePoint)
						pthread_kill (threadIt->m_threadId, SIGUSR1); // resume
				}

				result = m_handshakeSem.wait (200); // wait just a bit and retry sending signal
				if (result)
					break;
			}
#endif
		}
	}

	dbg::trace ("   ... GcHeap::collect_l () -- the world is resumed\n");

	// go to idle state

	m_lock.lock ();
	m_state = State_Idle;
	m_stats.m_currentAllocSize -= freeSize;
	m_stats.m_currentPeriodSize = 0;
	m_stats.m_lastCollectFreeSize = freeSize;
	m_stats.m_lastCollectTimeTaken = g::getTimestamp () - m_stats.m_lastCollectTime;
	m_stats.m_totalCollectTimeTaken += m_stats.m_lastCollectTimeTaken;
	m_idleEvent.signal ();
	m_lock.unlock ();

	dbg::trace ("--- GcHeap::collect_l ()\n");

	// 5) run destructors

	if (!destructArray.isEmpty ())
	{
		count = destructArray.getCount ();
		for (intptr_t i = count - 1; i >= 0; i--) // run in inversed order
		{
			IfaceHdr* iface = destructArray [i];
			ClassType* classType = (ClassType*) iface->m_box->m_type;
			Function* destructor = classType->getDestructor ();
			ASSERT (destructor);

			bool result = callVoidFunction (m_runtime, destructor, iface);
			if (!result)
			{
				dbg::trace (
					"runtime error in %s.destruct () : %s\n", 
					classType->m_tag.cc (),
					err::getLastErrorDescription ().cc ()
					);
			}
		}

		// now we can remove this thread' destruct guard

		bool isStillMutatorThread = waitIdleAndLock ();
		ASSERT (isStillMutatorThread == isMutatorThread);
		m_destructGuardList.remove (&destructGuard);
		m_lock.unlock ();
	}
}

void
GcHeap::runMarkCycle ()
{
	// mark breadth first

	while (!m_markRootArray [m_currentMarkRootArrayIdx].isEmpty ())
	{
		size_t prevGcRootArrayIdx = m_currentMarkRootArrayIdx;
		m_currentMarkRootArrayIdx = !m_currentMarkRootArrayIdx;
		m_markRootArray [m_currentMarkRootArrayIdx].clear ();

		size_t count = m_markRootArray [prevGcRootArrayIdx].getCount ();
		for (size_t i = 0; i < count; i++)
		{
			const Root* root = &m_markRootArray [prevGcRootArrayIdx] [i];
			root->m_type->markGcRoots (root->m_p, this);
		}
	}
}

void
GcHeap::parkAtSafePoint ()
{
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && !thread->m_waitRegionLevel);

	thread->m_isSafePoint = true;

	int32_t count = mt::atomicDec (&m_handshakeCount);
	ASSERT (m_state == State_StopTheWorld && count >= 0);
	if (!count)
		m_handshakeEvent.signal ();

	m_resumeEvent.wait ();
	ASSERT (m_state == State_ResumeTheWorld);

	thread->m_isSafePoint = false;
	count = mt::atomicDec (&m_handshakeCount);
	ASSERT (count >= 0);
	if (!count)
		m_handshakeEvent.signal ();
}

#if (_AXL_ENV == AXL_ENV_WIN)

int 
GcHeap::handleSehException (
	uint_t code, 
	EXCEPTION_POINTERS* exceptionPointers
	) 
{
	if (code != EXCEPTION_ACCESS_VIOLATION || 
		exceptionPointers->ExceptionRecord->ExceptionInformation [1] != (uintptr_t) m_guardPage.p ())
		return EXCEPTION_CONTINUE_SEARCH;

	parkAtSafePoint ();

	return EXCEPTION_CONTINUE_EXECUTION;
}

#elif (_AXL_ENV == AXL_ENV_POSIX)

void
GcHeap::installSignalHandlers (int)
{
	sigemptyset (&m_signalWaitMask); // don't block any signals when servicing SIGSEGV

	struct sigaction prevSigAction;
	struct sigaction sigAction = { 0 };
	sigAction.sa_flags = SA_SIGINFO;
	sigAction.sa_sigaction = signalHandler_SIGSEGV;
	sigAction.sa_mask = m_signalWaitMask;

	int result = sigaction (SIGSEGV, &sigAction, &prevSigAction);
	ASSERT (result == 0);

	sigAction.sa_flags = 0;
	sigAction.sa_handler = signalHandler_SIGUSR1;
	result = sigaction (SIGUSR1, &sigAction, &prevSigAction);
	ASSERT (result == 0);
}

void
GcHeap::signalHandler_SIGSEGV (
	int signal,
	siginfo_t* signalInfo,
	void* context
	)
{
	// while POSIX does not require that pthread_getspecific be async-signal-safe, in practice it is

	Tls* tls = getCurrentThreadTls ();
	if (!tls)
		return;

	GcHeap* self = &tls->m_runtime->m_gcHeap;

	if (signal != SIGSEGV || 
		signalInfo->si_addr != self->m_guardPage)
		return; // ignore

	GcMutatorThread* thread = &tls->m_gcMutatorThread;
	thread->m_isSafePoint = true;

	size_t count = mt::atomicDec (&self->m_handshakeCount);
	ASSERT (self->m_state == State_StopTheWorld && count >= 0);
	if (!count)
		self->m_handshakeSem.post ();

	do
	{
		sigsuspend (&self->m_signalWaitMask);
	} while (self->m_state != State_ResumeTheWorld);
	
	thread->m_isSafePoint = false;
	count = mt::atomicDec (&self->m_handshakeCount);
	ASSERT (count >= 0);
	if (!count)
		self->m_handshakeSem.post ();
}

#endif // _AXL_ENV == AXL_ENV_POSIX

//.............................................................................

} // namespace jnc
