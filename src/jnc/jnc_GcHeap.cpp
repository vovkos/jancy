#include "pch.h"
#include "jnc_GcHeap.h"
#include "jnc_Module.h"
#include "jnc_Api.h"

namespace jnc {

//.............................................................................

GcHeap::GcHeap ()
{
	m_state = State_Idle;
	m_handshakeCount = 0;
	m_safeMutatorThreadCount = 0;
	m_totalAllocSize = 0;
	m_peakAllocSize = 0;
	m_currentAllocSize = 0;
	m_currentPeriodSize = 0;
	m_currentMarkRootArrayIdx = 0;
	m_dataPtrValidatorType = NULL;
	m_periodSizeLimit = GcDef_PeriodSizeLimit;

#if (_AXL_ENV == AXL_ENV_WIN)
	m_guardPage.alloc (4 * 1024); // typical page size (OS will not give us less than that anyway)
#elif (_AXL_ENV == AXL_ENV_POSIX)
	m_guardPage = m_guardPage.map (
		NULL,
		4 * 1024,
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS,
		-1,
		0
		);
#endif
}

// locking

void
GcHeap::waitIdleAndLock (bool isSafeRegion)
{
	if (!isSafeRegion)
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
		while (m_state != State_Idle)
		{
			if (m_state == State_StopTheWorld)
			{
				m_lock.unlock ();			
				safePoint ();
			}
			else
			{
				m_lock.unlock ();
				m_idleEvent.wait ();
			}

			m_lock.lock ();
		}
	}
}

void
GcHeap::incrementAllocSizeAndLock (size_t size)
{
	waitIdleAndLock (true);
	
	m_totalAllocSize += size;
	m_currentAllocSize += size;
	m_currentPeriodSize += size;

	if (m_currentAllocSize > m_peakAllocSize)
		m_peakAllocSize = m_currentAllocSize;

	if (m_currentPeriodSize > m_periodSizeLimit)
	{
		collect_l ();
		waitIdleAndLock (true);
	}
}

// allocation methods

Box* 
GcHeap::tryAllocateClass (ClassType* type)
{
	size_t size = type->getSize ();

	Value vtableValue = type->getVTablePtrValue ();
	void* vtable = vtableValue.getValueKind () == ValueKind_Variable ? 
		type->getModule ()->getLlvmExecutionEngine ()->getPointerToGlobal ((llvm::GlobalVariable*) vtableValue.getVariable ()->getLlvmValue ()) :
		NULL;

	Box* box = (Box*) AXL_MEM_ALLOC (size);
	if (!box)
		return NULL;

	prime (type, vtable, box, box);

	incrementAllocSizeAndLock (size);
	m_allocBoxArray.append (box);
	addClassBox_l (box);

	m_lock.unlock ();

	return box;
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
}

Box*
GcHeap::tryAllocateData (
	Type* type,
	size_t count
	)
{
	size_t dataSize = type->getSize ();

	Box* box;
	DataPtrValidator* validator;

	if (count > 1)
	{
		dataSize *= count;

		size_t allocSize = sizeof (DynamicArrayBox) + dataSize;
		DynamicArrayBox* dynamicArrayBox = (DynamicArrayBox*) AXL_MEM_ALLOC (allocSize);
		if (!dynamicArrayBox)
			return NULL;

		memset (dynamicArrayBox, 0, allocSize);
		dynamicArrayBox->m_type = type;
		dynamicArrayBox->m_flags = BoxFlag_DynamicArray | BoxFlag_StrongMark | BoxFlag_WeakMark;
		dynamicArrayBox->m_count = count;

		box = dynamicArrayBox;
		validator = (DataPtrValidator*) (dynamicArrayBox + 1);
	}
	else
	{
		size_t allocSize = sizeof (Box) + sizeof (DataPtrValidator) + dataSize;
		box = (Box*) AXL_MEM_ALLOC (allocSize);
		if (!box)
			return NULL;

		memset (box, 0, allocSize);
		box->m_type = type;
		box->m_flags = BoxFlag_StrongMark | BoxFlag_WeakMark;

		validator = (DataPtrValidator*) (box + 1);
	}

	char* p = (char*) (validator + 1);

	validator->m_validatorBox = box;
	validator->m_targetBox = box;
	validator->m_rangeBegin = p;
	validator->m_rangeEnd = p + dataSize;

	incrementAllocSizeAndLock (dataSize);
	m_allocBoxArray.append (box);
	m_lock.unlock ();
	return box;
}

Box* 
GcHeap::tryAllocate (
	Type* type,
	size_t count
	)
{
	return type->getTypeKind () == TypeKind_Class ? 
		tryAllocateClass ((ClassType*) type) :
		tryAllocateData (type, count);
}

Box* 
GcHeap::allocate (
	Type* type,
	size_t count
	)
{
	Box* box = tryAllocate (type, count);
	if (!box)
	{
		err::Error error = err::formatStringError (
			count > 1 ? 
				"not enough memory for '%s [%d]'" : 
				"not enough memory for '%s'", 
			type->getTypeString ().cc (), 
			count
			);

		Runtime::runtimeError (error);
		ASSERT (false);
	}

	return box;
}

DataPtrValidator*
GcHeap::createDataPtrValidator (
	Box* box,
	void* rangeBegin,
	void* rangeEnd
	)
{
	DataPtrValidator* validator;
	
	GcMutatorThread* thread = &getCurrentThreadTls ()->m_gcThread;
	ASSERT (thread->m_enterSafeRegionCount == 0);

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
		size_t dataSize = sizeof (DataPtrValidator) * GcDef_DataPtrValidatorPoolSize;
		size_t allocSize = sizeof (DynamicArrayBox) + dataSize;
		DynamicArrayBox* box = (DynamicArrayBox*) AXL_MEM_ALLOC (allocSize);

		box->m_type = m_dataPtrValidatorType;
		box->m_flags = BoxFlag_DynamicArray | BoxFlag_StrongMark | BoxFlag_WeakMark;
		box->m_count = GcDef_DataPtrValidatorPoolSize;

		incrementAllocSizeAndLock (dataSize);
		m_allocBoxArray.append (box);
		m_lock.unlock ();

		validator = (DataPtrValidator*) (box + 1);
		validator->m_validatorBox = box;

		thread->m_dataPtrValidatorPoolBegin = validator + 1;
		thread->m_dataPtrValidatorPoolBegin->m_validatorBox = box;
		thread->m_dataPtrValidatorPoolEnd = validator + GcDef_DataPtrValidatorPoolSize;
	}

	validator->m_targetBox = box;
	validator->m_rangeBegin = rangeBegin;
	validator->m_rangeEnd = rangeEnd;
	return validator;
}

// management

void
GcHeap::registerStaticRootVariables (
	Variable* const* variableArray,
	size_t count
	)
{
	if (!count)
		return;

	llvm::ExecutionEngine* llvmExecutionEngine = variableArray [0]->getModule ()->getLlvmExecutionEngine ();

	char buffer [256];
	rtl::Array <Root> rootArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	rootArray.setCount (count);

	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = variableArray [i];
		llvm::GlobalVariable* llvmVariable = (llvm::GlobalVariable*) variable->getLlvmValue ();
		void* p = llvmExecutionEngine->getPointerToGlobal (llvmVariable);
		ASSERT (p);

		rootArray [i].m_p = p;
		rootArray [i].m_type = variable->getType ();
	}

	waitIdleAndLock (true);
	m_staticRootArray.append (rootArray);
	m_lock.unlock ();
}

void
GcHeap::registerStaticRoot (
	void* p,
	Type* type
	)
{
	Root root = { p, type };

	waitIdleAndLock (true);
	m_staticRootArray.append (root);
	m_lock.unlock ();
}

void
GcHeap::registerMutatorThread (GcMutatorThread* thread)
{
	thread->m_threadId = mt::getCurrentThreadId ();
	thread->m_enterSafeRegionCount = 0;
	thread->m_shadowStackTop = NULL;
	thread->m_dataPtrValidatorPoolBegin = NULL;
	thread->m_dataPtrValidatorPoolEnd = NULL;

	waitIdleAndLock (false); // during register we should treat this thread as unsafe
	m_mutatorThreadList.insertTail (thread);
	m_lock.unlock ();
}

void
GcHeap::unregisterMutatorThread (GcMutatorThread* thread)
{
	ASSERT (thread->m_threadId == mt::getCurrentThreadId ());
	ASSERT (thread->m_enterSafeRegionCount == 0);
	ASSERT (thread->m_shadowStackTop == NULL);

	waitIdleAndLock (true);
	m_mutatorThreadList.remove (thread);
	m_lock.unlock ();
}

void
GcHeap::enterSafeRegion ()
{
	Tls* tls = getCurrentThreadTls ();
	ASSERT (tls);
	
	tls->m_gcThread.m_enterSafeRegionCount++;
	if (tls->m_gcThread.m_enterSafeRegionCount)
		return;

	waitIdleAndLock (true);
	m_safeMutatorThreadCount++;
	ASSERT (m_safeMutatorThreadCount <= m_mutatorThreadList.getCount ());
	m_lock.unlock ();			
}

void
GcHeap::leaveSafeRegion ()
{
	Tls* tls = getCurrentThreadTls ();
	ASSERT (tls);

	tls->m_gcThread.m_enterSafeRegionCount--;
	if (tls->m_gcThread.m_enterSafeRegionCount)
		return;

	waitIdleAndLock (true);
	m_safeMutatorThreadCount--;
	m_lock.unlock ();
}

void
GcHeap::safePoint ()
{
#ifdef _AXL_DEBUG
	Tls* tls = getCurrentThreadTls ();
	ASSERT (tls && tls->m_gcThread.m_enterSafeRegionCount == 0); // otherwise we may finish handshake prematurely
#endif

	mt::atomicXchg ((volatile int32_t*) m_guardPage.p (), 0); // we need a fence, hence atomicXchg
}

void
GcHeap::weakMark (Box* box)
{
	if (box->m_flags & (BoxFlag_StrongMark | BoxFlag_WeakMarkClosure | BoxFlag_WeakMark))
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
GcHeap::markData (
	Type* type,
	DataPtrValidator* validator
	)
{
	Box* box = validator->m_targetBox;
	if (box->m_flags & BoxFlag_StrongMark)
		return;

	weakMark (box);

	if (!(type->getFlags () & TypeFlag_GcRoot))
		return;

	size_t size = type->getSize ();
	char* p = (char*) validator->m_rangeBegin;
	char* end = (char*) validator->m_rangeEnd;
	
	ASSERT ((end - p) % size == 0);

	for (; p < end; p += size)
		addRoot (p, type);
}

void
GcHeap::markClass (Box* box)
{
	if (box->m_flags & BoxFlag_StrongMark)
		return;

	weakMark (box);
	markClassFields (box);

	box->m_flags |= BoxFlag_StrongMark;

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

		if (fieldBox->m_flags & BoxFlag_StrongMark)
			continue;

		fieldBox->m_flags |= BoxFlag_StrongMark | BoxFlag_WeakMark;
		markClassFields (fieldBox);
	}
}

void
GcHeap::weakMarkClosureClass (Box* box)
{
	ASSERT (!box->m_rootOffset && box->m_type->getTypeKind () == TypeKind_Class);

	if (box->m_flags & (BoxFlag_StrongMark | BoxFlag_WeakMarkClosure))
		return;

	ASSERT (isClosureClassType (box->m_type));
	ClosureClassType* closureClassType = (ClosureClassType*) box->m_type;
	if (!closureClassType->getWeakMask ())
	{
		markClass (box);
		return;
	}

	weakMark (box);
	box->m_flags |= BoxFlag_WeakMarkClosure;

	char* p0 = (char*) (box + 1);

	rtl::Array <StructField*> gcRootMemberFieldArray = closureClassType->getGcRootMemberFieldArray ();
	size_t count = gcRootMemberFieldArray.getCount ();

	for (size_t i = 0; i < count; i++)
	{
		StructField* field = gcRootMemberFieldArray [i];
		Type* type = field->getType ();
		ASSERT (type->getFlags () & TypeFlag_GcRoot);		

		if (field->getFlags () & StructFieldFlag_WeakMasked)
			type = getWeakPtrType (type);

		addRoot (p0 + field->getOffset (), type);
	}
}

void
GcHeap::addRoot (
	void* p,
	Type* type
	)
{
	ASSERT (m_state == State_Mark && type->getFlags () & TypeFlag_GcRoot);

	Root root = { p, type };
	m_markRootArray [m_currentMarkRootArrayIdx].append (root);
}

void
GcHeap::addShadowStackFrameRoots (GcShadowStackFrame* frame)
{
	size_t count = frame->m_map->m_count;
	void** rootArray = (void**) (frame + 1);
	Type** typeArray = (Type**) (frame->m_map + 1);

	for (size_t i = 0; i < count; i++)
	{
		void* p = rootArray [i];
		if (!p) // stack roots could be nullified
			continue;

		Type* type = typeArray [i];
		if (type->getTypeKind () != TypeKind_DataPtr ||
			((DataPtrType*) type)->getPtrTypeKind () != DataPtrTypeKind_Thin) 
		{
			addRoot (p, type);
		}
		else // local heap variable
		{
			ASSERT (type->getTypeKind () != TypeKind_Class);
			markData (type, (DataPtrValidator*) ((char*) p + type->getSize ()));
		}
	}
}

void
GcHeap::collect_l ()
{
	size_t count;

	// stop the world

	ASSERT (m_safeMutatorThreadCount <= m_mutatorThreadList.getCount ());
	size_t handshakeCount = m_mutatorThreadList.getCount () - m_safeMutatorThreadCount;

#if (_AXL_ENV == AXL_ENV_WIN)
	m_resumeEvent.reset ();
#endif

	mt::atomicXchg (&m_handshakeCount, handshakeCount);
	m_state = State_StopTheWorld;
	m_idleEvent.reset ();
	m_lock.unlock ();

#if (_AXL_ENV == AXL_ENV_WIN)
	m_guardPage.protect (PAGE_NOACCESS);
	m_handshakeEvent.wait ();
#elif (_AXL_ENV == AXL_ENV_POSIX)
	m_guardPage.protect (PROT_NONE);
	m_handshakeSem.wait ();
#endif

	// the world is stopped, mark (no lock is needed)

	m_state = State_Mark;
	m_currentMarkRootArrayIdx = 0;
	m_markRootArray [0].clear ();

	// unmark everything

	count = m_allocBoxArray.getCount ();
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

	// add stack roots

	rtl::Iterator <GcMutatorThread> threadIt = m_mutatorThreadList.getHead ();
	for (; threadIt; threadIt++)
	{
		GcShadowStackFrame* frame = threadIt->m_shadowStackTop;
		for (; frame; frame = frame->m_prev)
			addShadowStackFrameRoots (frame);
	}

	// run mark cycle

	runMarkCycle ();

	// remove unmarked class boxes and schedule destruction

	char buffer [256];
	rtl::Array <IfaceHdr*> destructArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	DestructGuard destructGuard;

	count = m_classBoxArray.getCount ();
	size_t dstIdx = 0;
	for (size_t i = 0; i < count; i++)
	{
		Box* box = m_classBoxArray [i];
		
		if (box->m_flags & (BoxFlag_StrongMark | BoxFlag_WeakMarkClosure))
		{
			m_classBoxArray [dstIdx] = box;
			dstIdx++;
		}
		else
		{
			ClassType* classType = (ClassType*) box->m_type;
			if (classType->getDestructor ())
				destructArray.append ((IfaceHdr*) (box + 1));
		}
	}

	// add destruct guard for this thread

	if (!destructArray.isEmpty ())
	{
		destructGuard.m_destructArray = &destructArray;
		m_destructGuardList.insertTail (&destructGuard);
	}

	// add destruct guard roots from all the threads

	rtl::Iterator <DestructGuard> destructGuardIt = m_destructGuardList.getHead ();
	for (; destructGuardIt; destructGuardIt++)
	{
		DestructGuard* destructGuard = *destructGuardIt;

		IfaceHdr** iface = *destructGuard->m_destructArray;
		count = destructGuard->m_destructArray->getCount ();
		for (size_t i = 0; i < count; i++)
		{
			ClassType* classType = (ClassType*) (*iface)->m_box->m_type;
			addRoot (iface, classType);
		}
	}

	// mark destruct guards

	runMarkCycle ();

	// sweep 

	m_state = State_Sweep;
	size_t freeSize = 0;

	count = m_allocBoxArray.getCount ();
	dstIdx = 0;
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
			AXL_MEM_FREE (box);
		}
	}

	// resume the world

#if (_AXL_ENV == AXL_ENV_WIN)	
	m_guardPage.protect (PAGE_READWRITE);
#elif (_AXL_ENV == AXL_ENV_POSIX)
	m_guardPage.protect (PROT_READ | PROT_WRITE);
#endif

	mt::atomicXchg (&m_handshakeCount, handshakeCount);
	m_state = State_ResumeTheWorld;

#if (_AXL_ENV == AXL_ENV_WIN)	
	m_resumeEvent.signal ();
	m_handshakeEvent.wait ();
#elif (_AXL_ENV == AXL_ENV_POSIX)
	threadIt = m_mutatorThreadList.getHead ();
	for (; threadIt; threadIt++)
		pthread_kill (threadIt->m_threadId, SIGUSR1); // resume

	m_handshakeSem.wait ();
#endif

	// go to idle state

	m_lock.lock ();
	m_state = State_Idle;
	m_currentAllocSize -= freeSize;
	m_currentPeriodSize = 0;
	m_idleEvent.signal ();
	m_lock.unlock ();

	// 5) run destructors

	if (!destructArray.isEmpty ())
	{
		count = destructArray.getCount ();
		for (size_t i = 0; i < count; i++)
		{
			IfaceHdr* iface = destructArray [i];
			ClassType* classType = (ClassType*) iface->m_box->m_type;
			Function* destructor = classType->getDestructor ();
			ASSERT (destructor);

			Class_DestructFunc* p = (Class_DestructFunc*) destructor->getMachineCode ();
			p (iface);
		}

		// now we can remove this thread' destruct guard

		waitIdleAndLock (true);
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

#if (_AXL_ENV == AXL_ENV_WIN)

int 
GcHeap::handleSehException (
	uint_t code, 
	EXCEPTION_POINTERS* exceptionPointers
	) 
{
	if (code != EXCEPTION_ACCESS_VIOLATION || 
		exceptionPointers->ExceptionRecord->ExceptionInformation [1] != (uintptr_t) m_guardPage.p () ||
		m_state != State_StopTheWorld) 
		return EXCEPTION_CONTINUE_SEARCH;

	int32_t count = mt::atomicDec (&m_handshakeCount);
	if (!count)
		m_handshakeEvent.signal ();

	do
	{
		m_resumeEvent.wait ();
	} while (m_state != State_ResumeTheWorld);

	count = mt::atomicDec (&m_handshakeCount);
	if (!count)
		m_handshakeEvent.signal ();

	return EXCEPTION_CONTINUE_EXECUTION;
}

#elif (_AXL_ENV == AXL_ENV_POSIX)

void
GcHeap::installSignalHandlers ()
{
	sigemptyset (&m_signalWaitMask); // don't block any signals when servicing SIGSEGV

	struct sigaction sigAction = { 0 };
	sigAction.sa_flags = SA_SIGINFO;
	sigAction.sa_sigaction = signalHandler_SIGSEGV;
	sigAction.sa_mask = m_signalWaitMask;

	struct sigaction prevSigAction;
	int result = sigaction (SIGSEGV, &sigAction, &prevSigAction);
	ASSERT (result == 0);

	sigAction.sa_flags = 0;
	sigAction.sa_handler = signalHandler_SIGUSR1;
	result = sigaction (SIGUSR1, &sigAction, &prevSigAction);
	ASSERT (result == 0);
}

static
void
GcHeap::signalHandler_SIGSEGV (
	int signal,
	siginfo_t* signalInfo,
	void* context
	)
{
	if (signal != SIGSEGV || 
		signalInfo->si_addr != g_gc->m_guardPage ||
		g_gc->m_handshakeKind != HandshakeKind_StopTheWorld)
		return; // ignore

	int32_t count = mt::atomicDec (&g_gc->m_handshakeCounter);
	if (!count)
		g_gc->m_handshakeSem.post ();

	do
	{
		sigsuspend (&g_gc->m_signalWaitMask);
	} while (g_gc->m_handshakeKind != HandshakeKind_ResumeTheWorld);

	count = mt::atomicDec (&g_gc->m_handshakeCounter);
	if (!count)
		g_gc->m_handshakeSem.post ();
}

#endif

//.............................................................................

} // namespace jnc
