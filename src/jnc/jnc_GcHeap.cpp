#include "pch.h"
#include "jnc_GcHeap.h"
#include "jnc_Module.h"
#include "jnc_Api.h"

namespace jnc {

//.............................................................................

GcHeap::GcHeap ()
{
	m_runtime = AXL_CONTAINING_RECORD (this, Runtime, m_gcHeap);
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

bool
GcHeap::waitIdleAndLock ()
{
	bool isMutatorThread = getCurrentGcMutatorThread () != NULL;
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

	return isMutatorThread;
}

void
GcHeap::incrementAllocSizeAndLock (size_t size)
{
	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (isMutatorThread); // allocations should only be done in registered mutator threads 
	                          // otherwise there is risk of loosing new object	
	m_totalAllocSize += size;
	m_currentAllocSize += size;
	m_currentPeriodSize += size;

	if (m_currentAllocSize > m_peakAllocSize)
		m_peakAllocSize = m_currentAllocSize;

	if (m_currentPeriodSize > m_periodSizeLimit)
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

	Box* box = (Box*) AXL_MEM_ALLOC (size);
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
}

DataPtr
GcHeap::tryAllocateData (Type* type)
{
	size_t dataSize = type->getSize ();

	size_t allocSize = sizeof (DataBox) + dataSize;
	DataBox* box = (DataBox*) AXL_MEM_ALLOC (allocSize);
	if (!box)
	{
		err::setFormatStringError ("not enough memory for '%s'", type->getTypeString ().cc ());
		return g_nullPtr;
	}

	memset (box, 0, allocSize);
	box->m_type = type;
	box->m_flags = BoxFlag_StrongMark | BoxFlag_WeakMark;
	box->m_validator.m_validatorBox = box;
	box->m_validator.m_targetBox = box;
	box->m_validator.m_rangeBegin = box + 1;
	box->m_validator.m_rangeLength = dataSize;

	incrementAllocSizeAndLock (dataSize);
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
	size_t dataSize = type->getSize () * count;
	size_t allocSize = sizeof (DynamicArrayBox) + dataSize;

	DynamicArrayBox* box = (DynamicArrayBox*) AXL_MEM_ALLOC (allocSize);
	if (!box)
	{
		err::setFormatStringError ("not enough memory for '%s [%d]'", type->getTypeString ().cc (), count);
		return g_nullPtr;
	}

	memset (box, 0, allocSize);
	box->m_type = type;
	box->m_flags = BoxFlag_DynamicArray | BoxFlag_StrongMark | BoxFlag_WeakMark;
	box->m_count = count;
	box->m_validator.m_validatorBox = box;
	box->m_validator.m_targetBox = box;
	box->m_validator.m_rangeBegin = box + 1;
	box->m_validator.m_rangeLength = dataSize;

	incrementAllocSizeAndLock (dataSize);
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
	DataPtrValidator* validator;
	
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && thread->m_enterSafeRegionCount == 0);

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
		size_t allocSize = sizeof (DynamicArrayBoxHdr) + dataSize;
		DynamicArrayBoxHdr* box = (DynamicArrayBoxHdr*) AXL_MEM_ALLOC (allocSize);

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
	validator->m_rangeLength = rangeLength;
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

	waitIdleAndLock ();
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

	waitIdleAndLock ();
	m_staticRootArray.append (root);
	m_lock.unlock ();
}

void
GcHeap::registerMutatorThread (GcMutatorThread* thread)
{
	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (!isMutatorThread); // we in the process of registering this thread

	thread->m_threadId = mt::getCurrentThreadId ();
	thread->m_enterSafeRegionCount = 0;
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

	if (thread->m_enterSafeRegionCount != 0) // might happen on exception
		m_safeMutatorThreadCount--;

	m_mutatorThreadList.remove (thread);
	m_lock.unlock ();
}

void
GcHeap::enterSafeRegion ()
{
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread);
	
	thread->m_enterSafeRegionCount++;
	if (thread->m_enterSafeRegionCount)
		return;

	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (isMutatorThread);
	m_safeMutatorThreadCount++;
	ASSERT (m_safeMutatorThreadCount <= m_mutatorThreadList.getCount ());
	m_lock.unlock ();			
}

void
GcHeap::leaveSafeRegion ()
{
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread);

	thread->m_enterSafeRegionCount--;
	if (thread->m_enterSafeRegionCount)
		return;

	bool isMutatorThread = waitIdleAndLock ();
	ASSERT (isMutatorThread);
	m_safeMutatorThreadCount--;
	m_lock.unlock ();
}

void
GcHeap::safePoint ()
{
#ifdef _AXL_DEBUG
	GcMutatorThread* thread = getCurrentGcMutatorThread ();
	ASSERT (thread && thread->m_enterSafeRegionCount == 0); // otherwise we may finish handshake prematurely
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
	if (validator->m_targetBox->m_flags & BoxFlag_StrongMark)
		return;

	weakMark (validator->m_targetBox);

	if (!(type->getFlags () & TypeFlag_GcRoot))
		return;

	size_t size = type->getSize ();
	char* p = (char*) validator->m_rangeBegin;
	char* end = p + validator->m_rangeLength;
	
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
	const void* p,
	Type* type
	)
{
	ASSERT (m_state == State_Mark);
	
	if (type->getFlags () & TypeFlag_GcRoot)
	{
		Root root = { p, type };
		m_markRootArray [m_currentMarkRootArrayIdx].append (root);
	}
	else // heap variable
	{
		ASSERT (isDataPtrType (type, DataPtrTypeKind_Thin));
		Type* targetType = ((DataPtrType*) type)->getTargetType ();
		if (targetType->getTypeKind () == TypeKind_Class)
		{
			Box* box = ((Box*) p) - 1;
			ASSERT (box->m_type == targetType);
			markClass (box);
		}
		else 
		{
			DataBox* box = ((DataBox*) p) - 1;
			ASSERT (box->m_type == targetType);
			markData (type, &box->m_validator);
		}
	}
}

void
GcHeap::collect_l (bool isMutatorThread)
{
	size_t count;

	// stop the world

	ASSERT (m_safeMutatorThreadCount <= m_mutatorThreadList.getCount ());
	
	size_t handshakeCount = m_mutatorThreadList.getCount () - m_safeMutatorThreadCount;
	if (isMutatorThread)
	{
		ASSERT (handshakeCount);
		handshakeCount--; // minus this thread
	}

	if (!handshakeCount)
	{
		m_state = State_Mark;
		m_idleEvent.reset ();
		m_lock.unlock ();
	}
	else
	{
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
		m_state = State_Mark;
	}

	// the world is stopped, mark (no lock is needed)

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

	// add stack & tls roots

	StructType* tlsType = m_runtime->getModule ()->m_variableMgr.getTlsStructType ();
	rtl::Array <StructField*> tlsRootFieldArray = tlsType->getGcRootMemberFieldArray ();
	size_t tlsRootFieldCount = tlsRootFieldArray.getCount ();

	rtl::Iterator <GcMutatorThread> threadIt = m_mutatorThreadList.getHead ();
	for (; threadIt; threadIt++)
	{
		TlsVariableTable* tlsVariableTable = (TlsVariableTable*) (*threadIt + 1);
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

		for (size_t i = 0; i < tlsRootFieldCount; i++)
		{
			StructField* field = tlsRootFieldArray [i];
			addRoot ((char*) tlsVariableTable + field->getOffset (), field->getType ());
		}
	}

	// run mark cycle

	runMarkCycle ();

	// remove unmarked class boxes, mark them as zombies and schedule destruction

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
			box->m_flags |= BoxFlag_Zombie;

			if (((ClassType*) box->m_type)->getDestructor ())
				destructArray.append ((IfaceHdr*) (box + 1));
		}
	}

	m_classBoxArray.setCount (dstIdx);

	// add destruct guard for this thread

	if (!destructArray.isEmpty ())
	{
		destructGuard.m_destructArray = &destructArray;
		m_destructGuardList.insertTail (&destructGuard);
	}

	// mark destruct guard roots from all the threads

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
				size *= ((DynamicArrayBoxHdr*) box)->m_count;

			freeSize += size;
			AXL_MEM_FREE (box);
		}
	}

	m_allocBoxArray.setCount (dstIdx);

	// resume the world

	if (handshakeCount)
	{
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
	}

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
