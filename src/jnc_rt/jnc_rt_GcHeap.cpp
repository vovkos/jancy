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
#include "jnc_rt_GcHeap.h"
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_rtl_DynamicLayout.h"
#include "jnc_CallSite.h"

// #define _JNC_TRACE_GC_COLLECT
// #define _JNC_TRACE_GC_REGION

#ifdef _JNC_TRACE_GC_COLLECT
#	define JNC_TRACE_GC_COLLECT TRACE
#else
#	define JNC_TRACE_GC_COLLECT (void)
#endif

#ifdef _JNC_TRACE_GC_REGION
#	define JNC_TRACE_GC_REGION TRACE
#else
#	define JNC_TRACE_GC_REGION (void)
#endif

namespace jnc {
namespace rt {

//..............................................................................

GcHeap::GcHeap ()
{
	m_runtime = containerof (this, Runtime, m_gcHeap);
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

#if (_JNC_OS_WIN)
	m_guardPage.alloc (4 * 1024); // typical page size (OS will not give us less than that anyway)
#elif (_JNC_OS_POSIX)
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

GcMutatorThread*
GcHeap::getCurrentGcMutatorThread ()
{
	Tls* tls = getCurrentThreadTls ();
	return tls && tls->m_runtime == m_runtime ? &tls->m_gcMutatorThread : NULL;
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

bool
GcHeap::startup (ct::Module* module)
{
	ASSERT (m_state == State_Idle);

	memset (&m_stats, 0, sizeof (m_stats));
	m_flags = 0;

	if (module->getCompileFlags () & ModuleCompileFlag_SimpleGcSafePoint)
	{
		m_flags |= Flag_SimpleSafePoint;
	}
	else
	{
		ct::Variable* safePointTriggerVariable = module->m_variableMgr.getStdVariable (ct::StdVariable_GcSafePointTrigger);
		*(void**) safePointTriggerVariable->getStaticData () = m_guardPage;
	}

	addStaticRootVariables (module->m_variableMgr.getStaticGcRootArray ());

	ct::Function* destructor = module->getDestructor ();
	if (destructor)
		addStaticDestructor ((StaticDestructFunc*) destructor->getMachineCode ());

	return m_destructThread.start ();
}

#if (_AXL_OS_WIN)
static
void
JNC_STDCALL
abortApc (uintptr_t context)
{
}
#endif

void
GcHeap::abort ()
{
	bool isMutatorThread = waitIdleAndLock ();
	size_t handshakeCount = stopTheWorld_l (isMutatorThread);

	m_flags |= Flag_Abort;

	// try to interrupt all blocked threads (not guaranteed to succeed)

	MutatorThreadList::Iterator threadIt = m_mutatorThreadList.getHead ();
	for (; threadIt; threadIt++)
	{
		if (!threadIt->m_waitRegionLevel)
			continue;

#if (_AXL_OS_WIN)
		HANDLE h = ::OpenThread (THREAD_ALL_ACCESS, FALSE, (DWORD) threadIt->m_threadId);
		if (!h)
			continue;

		::QueueUserAPC (abortApc, h, 0);
		::CloseHandle (h);
#elif (_AXL_OS_POSIX)
		::pthread_kill ((pthread_t) threadIt->m_threadId, SIGUSR1);
#endif
	}

	resumeTheWorld (handshakeCount);

	// go to idle state

	m_lock.lock ();
	m_state = State_Idle;
	m_idleEvent.signal ();
	m_lock.unlock ();
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
	// allocations should only be done in registered mutator threads
	// otherwise there is risk of loosing new object

	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (isMutatorThread);

	incrementAllocSize_l (size);

	if (isCollectionTriggered_l ())
	{
		collect_l (isMutatorThread);
		waitIdleAndLock ();
	}
}

bool
GcHeap::addBoxIfDynamicFrame (Box* box)
{
	Tls* tls = rt::getCurrentThreadTls ();
	ASSERT (tls);

	TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (tls + 1);
	ASSERT (tlsVariableTable->m_gcShadowStackTop);

	GcShadowStackFrameMap* map = tlsVariableTable->m_gcShadowStackTop->m_map;
	if (!map || map->getMapKind () != ct::GcShadowStackFrameMapKind_Dynamic)
		return false;

	map->addBox (box);
	return false;
}

// allocation methods

IfaceHdr*
GcHeap::tryAllocateClass (ct::ClassType* type)
{
	size_t size = type->getSize ();
	Box* box = (Box*) AXL_MEM_ALLOCATE (size);
	if (!box)
	{
		err::setFormatStringError ("not enough memory for '%s'", type->getTypeString ().sz ());
		return NULL;
	}

	primeClass (box, type);
	addBoxIfDynamicFrame (box);

	incrementAllocSizeAndLock (size);
	m_allocBoxArray.append (box);
	addClassBox_l (box);
	m_lock.unlock ();

	return (IfaceHdr*) (box + 1);
}

IfaceHdr*
GcHeap::allocateClass (ct::ClassType* type)
{
	IfaceHdr* iface = tryAllocateClass (type);
	if (!iface)
	{
		Runtime::dynamicThrow ();
		ASSERT (false);
	}

	return iface;
}

void
GcHeap::addClassBox_l (Box* box)
{
	ASSERT (box->m_type->getTypeKind () == TypeKind_Class);
	ct::ClassType* classType = (ct::ClassType*) box->m_type;
	IfaceHdr* ifaceHdr = (IfaceHdr*) (box + 1);

	addBaseTypeClassFieldBoxes_l (classType, ifaceHdr);
	addClassFieldBoxes_l (classType, ifaceHdr);

	m_classBoxArray.append (box); // after all the fields

	if (classType->getDestructor ())
		m_destructibleClassBoxArray.append (box);
}

void
GcHeap::addBaseTypeClassFieldBoxes_l (
	ClassType* type,
	IfaceHdr* ifaceHdr
	)
{
	char* p = (char*) ifaceHdr;

	sl::Array <ct::BaseTypeSlot*> baseTypeArray = type->getBaseTypeArray ();
	size_t count = baseTypeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ct::BaseTypeSlot* slot = baseTypeArray [i];
		ct::Type* baseType = slot->getType ();
		if (baseType->getTypeKind () != TypeKind_Class)
			continue;

		ct::ClassType* baseClassType = (ct::ClassType*) baseType;
		IfaceHdr* baseIfaceHdr = (IfaceHdr*) (p + slot->getOffset ());
		addBaseTypeClassFieldBoxes_l (baseClassType, baseIfaceHdr);
		addClassFieldBoxes_l (baseClassType, baseIfaceHdr);
	}
}

void
GcHeap::addClassFieldBoxes_l (
	ClassType* type,
	IfaceHdr* ifaceHdr
	)
{
	char* p = (char*) ifaceHdr;

	sl::Array <ct::StructField*> classFieldArray = type->getClassMemberFieldArray ();
	size_t count = classFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ct::StructField* field = classFieldArray [i];
		Box* childBox = (Box*) (p + field->getOffset ());

		ASSERT (
			field->getType ()->getTypeKind () == TypeKind_Class &&
			childBox->m_type == field->getType ());

		addClassBox_l (childBox);
	}
}

DataPtr
GcHeap::tryAllocateData (ct::Type* type)
{
	size_t size = type->getSize ();

	DataBox* box = AXL_MEM_NEW_EXTRA (DataBox, size);
	if (!box)
	{
		err::setFormatStringError ("not enough memory for '%s'", type->getTypeString ().sz ());
		return g_nullPtr;
	}

	box->m_box.m_type = type;
	box->m_box.m_flags = BoxFlag_DataMark | BoxFlag_WeakMark;
	box->m_validator.m_validatorBox = (Box*) box;
	box->m_validator.m_targetBox = (Box*) box;
	box->m_validator.m_rangeBegin = box + 1;
	box->m_validator.m_rangeEnd = (char*) box->m_validator.m_rangeBegin + size;

	addBoxIfDynamicFrame (&box->m_box);

	incrementAllocSizeAndLock (size);
	m_allocBoxArray.append ((Box*) box);
	m_lock.unlock ();

	DataPtr ptr;
	ptr.m_p = box + 1;
	ptr.m_validator = &box->m_validator;
	return ptr;
}

DataPtr
GcHeap::allocateData (ct::Type* type)
{
	DataPtr ptr = tryAllocateData (type);
	if (!ptr.m_p)
	{
		Runtime::dynamicThrow ();
		ASSERT (false);
	}

	return ptr;
}

DataPtr
GcHeap::tryAllocateArray (
	ct::Type* type,
	size_t count
	)
{
	size_t size = type->getSize () * count;

	DynamicArrayBox* box = AXL_MEM_NEW_EXTRA (DynamicArrayBox, size);
	if (!box)
	{
		err::setFormatStringError ("not enough memory for '%s [%d]'", type->getTypeString ().sz (), count);
		return g_nullPtr;
	}

	box->m_box.m_type = type;
	box->m_box.m_flags = BoxFlag_DynamicArray | BoxFlag_DataMark | BoxFlag_WeakMark;
	box->m_count = count;
	box->m_validator.m_validatorBox = (Box*) box;
	box->m_validator.m_targetBox = (Box*) box;
	box->m_validator.m_rangeBegin = box + 1;
	box->m_validator.m_rangeEnd = (char*) box->m_validator.m_rangeBegin + size;

	addBoxIfDynamicFrame (&box->m_box);

	incrementAllocSizeAndLock (size);
	m_allocBoxArray.append ((Box*) box);
	m_lock.unlock ();

	DataPtr ptr;
	ptr.m_p = box + 1;
	ptr.m_validator = &box->m_validator;
	return ptr;
}

DataPtr
GcHeap::allocateArray (
	ct::Type* type,
	size_t count
	)
{
	DataPtr ptr = tryAllocateArray (type, count);
	if (!ptr.m_p)
	{
		Runtime::dynamicThrow ();
		ASSERT (false);
	}

	return ptr;
}

DataPtr
GcHeap::tryAllocateBuffer (size_t size)
{
	ct::Module* module = m_runtime->getModule ();
	ASSERT (module);

	ct::Type* type = module->m_typeMgr.getPrimitiveType (TypeKind_Char);
	return tryAllocateArray (type, size);
}

DataPtr
GcHeap::allocateBuffer (size_t size)
{
	ct::Module* module = m_runtime->getModule ();
	ASSERT (module);

	ct::Type* type = module->m_typeMgr.getPrimitiveType (TypeKind_Char);
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

		DynamicArrayBox* box = AXL_MEM_NEW_EXTRA (DynamicArrayBox, size);
		if (!box)
		{
			Runtime::dynamicThrow ();
			ASSERT (false);
		}

		box->m_box.m_type = (jnc::Type*) m_runtime->getModule ()->m_typeMgr.getStdType (StdType_DataPtrValidator);
		box->m_box.m_flags = BoxFlag_DynamicArray | BoxFlag_DataMark | BoxFlag_WeakMark;
		box->m_box.m_rootOffset = 0;
		box->m_count = GcDef_DataPtrValidatorPoolSize;

		incrementAllocSizeAndLock (size);
		m_allocBoxArray.append ((Box*) box);
		m_lock.unlock ();

		validator = &box->m_validator;
		validator->m_validatorBox = (Box*) box;

		if (GcDef_DataPtrValidatorPoolSize >= 2)
		{
			thread->m_dataPtrValidatorPoolBegin = validator + 1;
			thread->m_dataPtrValidatorPoolBegin->m_validatorBox = (Box*) box;
			thread->m_dataPtrValidatorPoolEnd = validator + GcDef_DataPtrValidatorPoolSize;
		}
	}

	validator->m_targetBox = box;
	validator->m_rangeBegin = rangeBegin;
	validator->m_rangeEnd = (char*) rangeBegin + rangeLength;
	return validator;
}

// dynamic layout methods

IfaceHdr*
GcHeap::getDynamicLayout (Box* box)
{
	IfaceHdr* dynamicLayout;

	waitIdleAndLock ();
	sl::HashTableIterator <Box*, IfaceHdr*> it = m_dynamicLayoutMap.find (box);
	if (it)
	{
		dynamicLayout = it->m_value;
		m_lock.unlock ();
		return dynamicLayout;
	}

	m_lock.unlock ();

	// we need a call site so the newly allocated dynamic layout
	// does not get collected during waitIdleAndLock ()

	JNC_BEGIN_CALL_SITE (m_runtime)

	dynamicLayout = createClass <rtl::DynamicLayout> (m_runtime);

	waitIdleAndLock ();

	sl::HashTableIterator <Box*, IfaceHdr*> it = m_dynamicLayoutMap.visit (box);
	if (it->m_value)
		dynamicLayout = it->m_value;
	else
		it->m_value = dynamicLayout;

	m_lock.unlock ();

	JNC_END_CALL_SITE ()

	return dynamicLayout;
}

void
GcHeap::resetDynamicLayout (Box* box)
{
	waitIdleAndLock ();
	m_dynamicLayoutMap.eraseKey (box);
	m_lock.unlock ();
}

// management

void
GcHeap::beginShutdown ()
{
	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (!isMutatorThread);

	m_flags |= Flag_ShuttingDown;  // this will prevent boxes from being actually freed

	m_staticRootArray.clear (); // drop static roots
	m_lock.unlock ();
}

void
GcHeap::finalizeShutdown ()
{
	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (!isMutatorThread && (m_flags & Flag_ShuttingDown));

	// wait for destruct thread

	m_flags |= Flag_TerminateDestructThread;
	m_destructEvent.signal ();
	m_lock.unlock ();

	m_destructThread.waitAndClose ();

	// final collect

	waitIdleAndLock ();
	m_staticRootArray.clear (); // drop static roots one more time
	collect_l (false);

	waitIdleAndLock ();

	// postponed free

	sl::Array <Box*> postponeFreeBoxArray = m_postponeFreeBoxArray;
	m_postponeFreeBoxArray.clear ();
	m_flags &= ~Flag_ShuttingDown;
	m_lock.unlock ();

	size_t count = postponeFreeBoxArray.getCount ();
	for (size_t i = 0; i < count; i++)
		AXL_MEM_FREE (postponeFreeBoxArray [i]);

	// everything should be empty now (if destructors don't play hardball)

	ASSERT (
		!m_noCollectMutatorThreadCount &&
		!m_waitingMutatorThreadCount &&
		m_staticDestructorList.isEmpty () &&
		m_dynamicDestructArray.isEmpty () &&
		m_allocBoxArray.isEmpty () &&
		m_classBoxArray.isEmpty () &&
		m_dynamicLayoutMap.isEmpty ()
		);

	// force-clear anyway

	m_noCollectMutatorThreadCount = 0;
	m_waitingMutatorThreadCount = 0;

	m_staticDestructorList.clear ();
	m_dynamicDestructArray.clear ();
	m_allocBoxArray.clear ();
	m_classBoxArray.clear ();
	m_destructibleClassBoxArray.clear ();
	m_dynamicLayoutMap.clear ();
}

void
GcHeap::addStaticRootVariables (
	ct::Variable* const* variableArray,
	size_t count
	)
{
	if (!count)
		return;

	char buffer [256];
	sl::Array <Root> rootArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	rootArray.setCount (count);

	for (size_t i = 0; i < count; i++)
	{
		ct::Variable* variable = variableArray [i];

		rootArray [i].m_p = variable->getStaticData ();
		rootArray [i].m_type = variable->getType ();
	}

	waitIdleAndLock ();
	m_staticRootArray.append (rootArray);
	m_lock.unlock ();
}

void
GcHeap::addStaticRoot (
	const void* p,
	ct::Type* type
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
	IfaceHdr* iface
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

	thread->m_threadId = sys::getCurrentThreadId ();
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
	ASSERT (thread->m_threadId == sys::getCurrentThreadId ());

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
	ASSERT (thread);

	if (thread->m_waitRegionLevel) // already there
	{
		thread->m_waitRegionLevel++;
		return;
	}

	if (thread->m_noCollectRegionLevel)
		TRACE ("-- WARNING: GcHeap::enterWaitRegion () in no-collect-region (no collections until wait completes)\n");

	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (isMutatorThread);
	thread->m_waitRegionLevel = 1;
	m_waitingMutatorThreadCount++;
	ASSERT (m_waitingMutatorThreadCount <= m_mutatorThreadList.getCount ());

	JNC_TRACE_GC_REGION ("GcHeap::enterWaitRegion () (tid = %x)\n", (uint_t) sys::getCurrentThreadId ());

	m_lock.unlock ();
}

void
GcHeap::leaveWaitRegion ()
{
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && thread->m_waitRegionLevel);

	if (thread->m_waitRegionLevel > 1) // still there
	{
		thread->m_waitRegionLevel--;
		return;
	}

	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (!isMutatorThread);
	thread->m_waitRegionLevel = 0;
	m_waitingMutatorThreadCount--;

	JNC_TRACE_GC_REGION ("GcHeap::leaveWaitRegion () (tid = %x)\n", (uint_t) sys::getCurrentThreadId ());

	bool isAbort = (m_flags & Flag_Abort) != 0;
	m_lock.unlock ();

	if (isAbort)
		abortThrow ();
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

	JNC_TRACE_GC_REGION ("GcHeap::enterNoCollectRegion () (tid = %x)\n", (uint_t) sys::getCurrentThreadId ());

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

	JNC_TRACE_GC_REGION ("GcHeap::leaveNoCollectRegion (%d) (tid = %x)\n", canCollectNow, (uint_t) sys::getCurrentThreadId ());

	if (canCollectNow && isCollectionTriggered_l ())
		collect_l (isMutatorThread);
	else
		m_lock.unlock ();
}

void
GcHeap::safePoint ()
{
#ifdef _JNC_DEBUG
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && thread->m_waitRegionLevel == 0); // otherwise we may finish handshake prematurely
#endif

	if (!(m_flags & Flag_SimpleSafePoint))
		sys::atomicXchg ((volatile int32_t*) m_guardPage.p (), 0); // we need a fence, hence atomicXchg
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
		addRootArray (arrayBox + 1, arrayBox->m_box.m_type, arrayBox->m_count);
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
	ct::ClassType* classType = (ct::ClassType*) box->m_type;
	sl::Array <ct::StructField*> classMemberFieldArray = classType->getClassMemberFieldArray ();
	size_t count = classMemberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		ct::StructField* field = classMemberFieldArray [i];
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
	ct::ClosureClassType* closureClassType = (ct::ClosureClassType*) box->m_type;
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

	ct::StructField* thisArgField = closureClassType->getFieldByIndex (thisArgFieldIdx);
	ASSERT (thisArgField && thisArgField->getType ()->getTypeKind () == TypeKind_ClassPtr);
	ct::ClassPtrType* weakPtrType = ((ct::ClassPtrType*) (thisArgField->getType ()))->getWeakPtrType ();
	addRoot (p0 + thisArgField->getOffset (), weakPtrType);

	sl::Array <ct::StructField*> gcRootFieldArray = closureClassType->getGcRootMemberFieldArray ();
	size_t count = gcRootFieldArray.getCount ();

	for (size_t i = 0; i < count; i++)
	{
		ct::StructField* field = gcRootFieldArray [i];
		if (field != thisArgField)
			addRoot (p0 + field->getOffset (), field->getType ());
	}
}

void
GcHeap::addRoot (
	const void* p,
	ct::Type* type
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
		ct::Type* targetType = ((ct::DataPtrType*) type)->getTargetType ();

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
			ASSERT (box->m_box.m_type == targetType);
			markData ((Box*) box);
		}
	}
}

void
GcHeap::addRootArray (
	const void* p0,
	ct::Type* type,
	size_t count
	)
{
	ASSERT (type->getTypeKind () != TypeKind_Class && (type->getFlags () & TypeFlag_GcRoot));

	sl::Array <Root>* markRootArray = &m_markRootArray [m_currentMarkRootArrayIdx];
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
GcHeap::setFrameMap (
	GcShadowStackFrame* frame,
	GcShadowStackFrameMap* map,
	GcShadowStackFrameMapOp op
	)
{
	switch (op)
	{
		size_t count;
		const size_t* indexArray;

	case GcShadowStackFrameMapOp_Open:
		count = map->getGcRootCount ();
		ASSERT (map && count);

		indexArray = map->getGcRootIndexArray ();
		for (size_t i = 0; i < count; i++)
		{
			size_t j = indexArray [i];
			frame->m_gcRootArray [j] = NULL;
		}

		frame->m_map = map;
		break;

	case GcShadowStackFrameMapOp_Close:
		ASSERT (frame->m_map == map); // this assert helps catch wrongly sequenced closeScope & jumps
		frame->m_map = map->getPrev ();
		break;

	case GcShadowStackFrameMapOp_Restore:
		frame->m_map = map;
		break;

	default:
		ASSERT (false);
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

size_t
GcHeap::stopTheWorld_l (bool isMutatorThread)
{
	size_t handshakeCount = m_mutatorThreadList.getCount () - m_waitingMutatorThreadCount;
	if (isMutatorThread)
	{
		ASSERT (handshakeCount);
		handshakeCount--; // minus this thread
	}

	JNC_TRACE_GC_COLLECT (
		"+++ GcHeap::stopTheWorld_l (tid = %x; isMutator = %d; mutatorCount = %d; waitingMutatorThreadCount = %d, handshakeCount = %d)\n",
		(uint_t) sys::getCurrentThreadId (),
		isMutatorThread,
		m_mutatorThreadList.getCount (),
		m_waitingMutatorThreadCount,
		handshakeCount
		);

	MutatorThreadList::Iterator threadIt = m_mutatorThreadList.getHead ();
	for (; threadIt; threadIt++)
	{
		JNC_TRACE_GC_COLLECT ("   *** mutator (tid = %x; wait = %d)\n",
			(uint_t) threadIt->m_threadId,
			threadIt->m_waitRegionLevel
			);
	}

	if (!handshakeCount)
	{
		m_idleEvent.reset ();
		m_lock.unlock ();
	}
	else if (m_flags & Flag_SimpleSafePoint)
	{
		m_resumeEvent.reset ();
		sys::atomicXchg (&m_handshakeCount, handshakeCount);
		m_state = State_StopTheWorld;
		m_idleEvent.reset ();
		m_lock.unlock ();

		m_handshakeEvent.wait ();
	}
	else
	{
#if (_JNC_OS_WIN)
		m_resumeEvent.reset ();
		sys::atomicXchg (&m_handshakeCount, handshakeCount);
		m_state = State_StopTheWorld;
		m_idleEvent.reset ();
		m_lock.unlock ();

		m_guardPage.protect (PAGE_NOACCESS);
		m_handshakeEvent.wait ();
#elif (_JNC_OS_POSIX)
		sys::atomicXchg (&m_handshakeCount, handshakeCount);
		m_state = State_StopTheWorld;
		m_idleEvent.reset ();
		m_lock.unlock ();

		m_guardPage.protect (PROT_NONE);
		m_handshakeSem.wait ();
#endif
	}

	return handshakeCount;
}

void
GcHeap::resumeTheWorld (size_t handshakeCount)
{
	if (!handshakeCount)
		return;

	if (m_flags & Flag_SimpleSafePoint)
	{
		sys::atomicXchg (&m_handshakeCount, handshakeCount);
		m_state = State_ResumeTheWorld;
		m_resumeEvent.signal ();
		m_handshakeEvent.wait ();
	}
	else
	{
#if (_JNC_OS_WIN)
		m_guardPage.protect (PAGE_READWRITE);
		sys::atomicXchg (&m_handshakeCount, handshakeCount);
		m_state = State_ResumeTheWorld;
		m_resumeEvent.signal ();
		m_handshakeEvent.wait ();
#elif (_JNC_OS_POSIX)
		m_guardPage.protect (PROT_READ | PROT_WRITE);
		sys::atomicXchg (&m_handshakeCount, handshakeCount);
		m_state = State_ResumeTheWorld;

		for (;;) // we need a loop -- sigsuspend can lose per-thread signals
		{
			bool result;

			MutatorThreadList::Iterator threadIt = m_mutatorThreadList.getHead ();
			for (; threadIt; threadIt++)
			{
				if (threadIt->m_isSafePoint)
					::pthread_kill ((pthread_t) threadIt->m_threadId, SIGUSR1); // resume
			}

			result = m_handshakeSem.wait (200); // wait just a bit and retry sending signal
			if (result)
				break;
		}
#endif
	}
}

void
GcHeap::collect_l (bool isMutatorThread)
{
	ASSERT (!m_noCollectMutatorThreadCount && m_waitingMutatorThreadCount <= m_mutatorThreadList.getCount ());

	m_stats.m_totalCollectCount++;
	m_stats.m_lastCollectTime = sys::getTimestamp ();

	bool isShuttingDown = (m_flags & Flag_ShuttingDown) != 0;

	size_t handshakeCount = stopTheWorld_l (isMutatorThread);

	JNC_TRACE_GC_COLLECT ("   ... GcHeap::collect_l () -- the world is stopped\n");

	// the world is stopped, mark (no lock is needed)

	m_state = State_Mark;

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

	ct::StructType* tlsType = m_runtime->getModule ()->m_variableMgr.getTlsStructType ();
	sl::Array <ct::StructField*> tlsRootFieldArray = tlsType->getGcRootMemberFieldArray ();
	size_t tlsRootFieldCount = tlsRootFieldArray.getCount ();

	MutatorThreadList::Iterator threadIt = m_mutatorThreadList.getHead ();
	for (; threadIt; threadIt++)
	{
		GcMutatorThread* thread = *threadIt;

		// stack roots

		TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (thread + 1);
		GcShadowStackFrame* frame = tlsVariableTable->m_gcShadowStackTop;
		for (; frame; frame = frame->m_prev)
		{
			GcShadowStackFrameMap* frameMap = frame->m_map;
			for (; frameMap; frameMap = frameMap->getPrev ())
			{
				size_t gcRootCount = frameMap->getGcRootCount ();
				if (!gcRootCount)
					continue;

				ct::GcShadowStackFrameMapKind mapKind = frameMap->getMapKind ();
				if (mapKind == ct::GcShadowStackFrameMapKind_Dynamic)
				{
					Box* const* boxArray = frameMap->getBoxArray ();
					for (size_t i = 0; i < gcRootCount; i++)
					{
						Box* box = boxArray [i];
						if (box->m_type->getTypeKind () == TypeKind_Class)
							markClass (box);
						else
							markData (box);
					}
				}
				else
				{
					ASSERT (mapKind == ct::GcShadowStackFrameMapKind_Static);
					Type* const* typeArray = frameMap->getGcRootTypeArray ();

					const size_t* indexArray = frameMap->getGcRootIndexArray ();
					for (size_t i = 0; i < gcRootCount; i++)
					{
						size_t j = indexArray [i];
						void* p = frame->m_gcRootArray [j];
						if (p)
							addRoot (p, typeArray [i]);
					}
				}
			}
		}

		// tls roots

		for (size_t i = 0; i < tlsRootFieldCount; i++)
		{
			ct::StructField* field = tlsRootFieldArray [i];
			addRoot ((char*) tlsVariableTable + field->getOffset (), field->getType ());
		}

		// validator pool

		if (thread->m_dataPtrValidatorPoolBegin)
			weakMark (thread->m_dataPtrValidatorPoolBegin->m_validatorBox);
	}

	// run mark cycle

	runMarkCycle ();

	// mark used dynamic layouts and remove unused from the map

	sl::HashTableIterator <Box*, IfaceHdr*> it = m_dynamicLayoutMap.getHead ();
	sl::HashTableIterator <Box*, IfaceHdr*> nextIt;
	for (; it; it = nextIt)
	{
		nextIt = it.getNext ();

		if (it->m_key->m_flags & BoxFlag_WeakMark)
			markClass (it->m_value->m_box); // simple mark is enough -- DynamicLayout is a primitive opaque class
		else
			m_dynamicLayoutMap.erase (it);
	}

	JNC_TRACE_GC_COLLECT ("   ... GcHeap::collect_l () -- mark complete\n");

	// schedule destruction for unmarked class boxes

	sl::Array <IfaceHdr*> destructArray;

	size_t dstIdx = 0;
	count = m_destructibleClassBoxArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Box* box = m_destructibleClassBoxArray [i];
		ASSERT (!(box->m_flags & BoxFlag_Zombie) && ((ct::ClassType*) box->m_type)->getDestructor ());

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

	if (!destructArray.isEmpty ())
		m_dynamicDestructArray.append (destructArray);

	// mark all class boxes scheduled for destruction

	if (!m_dynamicDestructArray.isEmpty ())
	{
		size_t count = m_dynamicDestructArray.getCount ();
		IfaceHdr** iface = m_dynamicDestructArray;
		for (size_t i = 0; i < count; i++, iface++)
		{
			ct::ClassType* classType = (ct::ClassType*) (*iface)->m_box->m_type;
			addRoot (iface, classType->getClassPtrType ());
		}

		runMarkCycle ();
	}

	// sweep unmarked class boxes

	dstIdx = 0;
	count = m_classBoxArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Box* box = m_classBoxArray [i];
		if (box->m_flags & (BoxFlag_ClassMark | BoxFlag_ClosureWeakMark))
			m_classBoxArray [dstIdx++] = box;
	}

	m_classBoxArray.setCount (dstIdx);

	// sweep allocated boxes

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
				size *= ((DynamicArrayBox*) box)->m_count;

			freeSize += size;

			if (isShuttingDown)
				m_postponeFreeBoxArray.append (box);
			else
				AXL_MEM_FREE (box);
		}
	}

	m_allocBoxArray.setCount (dstIdx);

	JNC_TRACE_GC_COLLECT ("   ... GcHeap::collect_l () -- sweep complete\n");

	resumeTheWorld (handshakeCount);

	JNC_TRACE_GC_COLLECT ("   ... GcHeap::collect_l () -- the world is resumed\n");

	// go to idle state

	m_lock.lock ();
	m_state = State_Idle;
	m_stats.m_currentAllocSize -= freeSize;
	m_stats.m_currentPeriodSize = 0;
	m_stats.m_lastCollectFreeSize = freeSize;
	m_stats.m_lastCollectTimeTaken = sys::getTimestamp () - m_stats.m_lastCollectTime;
	m_stats.m_totalCollectTimeTaken += m_stats.m_lastCollectTimeTaken;
	m_idleEvent.signal ();
	m_lock.unlock ();

	JNC_TRACE_GC_COLLECT ("--- GcHeap::collect_l ()\n");
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
GcHeap::runDestructCycle_l ()
{
	while (!m_dynamicDestructArray.isEmpty ())
	{
		size_t i = m_dynamicDestructArray.getCount () - 1;
		IfaceHdr* iface = m_dynamicDestructArray [i];
		m_lock.unlock ();

		ct::ClassType* classType = (ct::ClassType*) iface->m_box->m_type;
		ct::Function* destructor = classType->getDestructor ();
		ASSERT (destructor);

		bool result = callVoidFunction (m_runtime, destructor, iface);
		if (!result)
		{
			TRACE (
				"-- WARNING: runtime error in %s.destruct (): %s\n",
				classType->m_tag.sz (),
				err::getLastErrorDescription ().sz ()
				);
		}

		waitIdleAndLock ();
		m_dynamicDestructArray.remove (i);
	}
}

void
GcHeap::destructThreadFunc ()
{
	for (;;)
	{
		m_destructEvent.wait ();

		waitIdleAndLock ();
		if (m_flags & Flag_TerminateDestructThread)
			break;

		runDestructCycle_l ();
		m_lock.unlock ();
	}

	for (size_t i = 0; i < GcDef_ShutdownIterationLimit; i++)
	{
		runDestructCycle_l ();

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

		m_staticRootArray.clear (); // drop roots before every collect
		collect_l (false);

		waitIdleAndLock ();

		if (m_allocBoxArray.isEmpty ())
			break;
	}

	m_lock.unlock ();
}

void
GcHeap::parkAtSafePoint ()
{
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread);

	parkAtSafePoint (thread);
}

void
GcHeap::parkAtSafePoint (GcMutatorThread* thread)
{
	ASSERT (m_state == State_StopTheWorld); // shouldn't be here otherwise
	ASSERT (!thread->m_waitRegionLevel && !thread->m_isSafePoint);

	thread->m_isSafePoint = true;

	intptr_t count = sys::atomicDec (&m_handshakeCount);
	ASSERT (count >= 0);
	if (!count)
		m_handshakeEvent.signal ();

	m_resumeEvent.wait ();
	ASSERT (m_state == State_ResumeTheWorld);

	bool isAbort = (m_flags & Flag_Abort) != 0;

	thread->m_isSafePoint = false;
	count = sys::atomicDec (&m_handshakeCount);
	ASSERT (count >= 0);
	if (!count)
		m_handshakeEvent.signal ();

	if (isAbort)
		abortThrow ();
}

void
GcHeap::abortThrow ()
{
	err::setError ("Jancy script execution forcibly interrupted");
	Runtime::dynamicThrow ();
	ASSERT (false);
}

#if (_JNC_OS_WIN)

void
GcHeap::handleGuardPageHit (GcMutatorThread* thread)
{
	parkAtSafePoint (thread);
}

#elif (_JNC_OS_POSIX)

void
GcHeap::handleGuardPageHit (GcMutatorThread* thread)
{
	ASSERT (m_state == State_StopTheWorld); // shouldn't be here otherwise
	ASSERT (!thread->m_waitRegionLevel && !thread->m_isSafePoint);

	thread->m_isSafePoint = true;

	intptr_t count = sys::atomicDec (&m_handshakeCount);
	ASSERT (count >= 0);
	if (!count)
		m_handshakeSem.signal ();

	do
	{
		static sigset_t signalWaitMask = { 0 }; // the triggering signal is already excluded
		sigsuspend (&signalWaitMask);
	} while (m_state != State_ResumeTheWorld);

	bool isAbort = (m_flags & Flag_Abort) != 0;

	thread->m_isSafePoint = false;
	count = sys::atomicDec (&m_handshakeCount);
	ASSERT (count >= 0);
	if (!count)
		m_handshakeSem.signal ();

	if (isAbort)
		abortThrow ();
}

#endif // _JNC_OS_POSIX

//..............................................................................

} // namespace rt
} // namespace jnc
