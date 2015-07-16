// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_RuntimeStructs.h"

namespace jnc {

class Runtime;
class Variable;
class ClassType;

//.............................................................................

enum GcDef
{
#ifdef _AXL_DEBUG
	GcDef_PeriodSizeLimit = 0, // run gc on every allocation
#elif (_AXL_CPU == AXL_CPU_X86)
	GcDef_PeriodSizeLimit = 1 * 1024 * 1024, // 1MB gc period
#else
	GcDef_PeriodSizeLimit = 2 * 1024 * 1024, // 2MB gc period
#endif
	GcDef_DataPtrValidatorPoolSize = 8,
};

//.............................................................................

class GcHeap
{
protected:
	enum State
	{
		State_Idle,
		State_StopTheWorld,
		State_Mark,
		State_Sweep,
		State_ResumeTheWorld,
	};

	struct Root
	{
		const void* m_p;
		Type* m_type;
	};

	struct DestructGuard: rtl::ListLink
	{
		rtl::Array <IfaceHdr*>* m_destructArray;
	};

protected:
	Runtime* m_runtime;

	mt::Lock m_lock;
	volatile State m_state;
	mt::NotificationEvent m_idleEvent;

	size_t m_currentAllocSize;
	size_t m_totalAllocSize;
	size_t m_peakAllocSize;
	size_t m_currentPeriodSize;

	rtl::AuxList <GcMutatorThread> m_mutatorThreadList;
	volatile size_t m_safeMutatorThreadCount;
	volatile size_t m_handshakeCount;

#if (_AXL_ENV == AXL_ENV_WIN)
	mt::Event m_handshakeEvent;
	mem::win::VirtualMemory m_guardPage;
	mt::NotificationEvent m_resumeEvent;
#elif (_AXL_ENV == AXL_ENV_POSIX)
	io::psx::Mapping m_guardPage;
	mt::psx::Sem m_handshakeSem; // only sems can be safely signalled from signal handlers
	sigset_t m_signalWaitMask;
#endif

	rtl::Array <Box*> m_allocBoxArray;
	rtl::Array <Box*> m_classBoxArray;
	rtl::AuxList <DestructGuard> m_destructGuardList;
	rtl::Array <Root> m_staticRootArray;
	rtl::Array <Root> m_markRootArray [2];
	size_t m_currentMarkRootArrayIdx;
	Type* m_dataPtrValidatorType;

public:
	size_t m_periodSizeLimit; // adjustable limit

public:
	GcHeap ();
	
	~GcHeap ()
	{
		ASSERT (isEmpty ()); // should be collected before runtime shutdown
	}

	// informational methods

	bool
	isEmpty ()
	{
		return m_currentAllocSize == 0;
	}

	size_t 
	getCurrentAllocSize ()
	{		
		return m_currentAllocSize;
	}

	size_t 
	getTotalAllocSize ()
	{		
		return m_totalAllocSize;
	}

	size_t 
	getPeakAllocSize ()
	{		
		return m_peakAllocSize;
	}

	size_t 
	getCurrentPeriodSize ()
	{		
		return m_currentPeriodSize;
	}

	// allocation methods

	Box* 
	tryAllocateClass (ClassType* type);

	Box* 
	allocateClass (ClassType* type);

	DataBox* 
	tryAllocateData (Type* type);

	DataBox* 
	allocateData (Type* type);

	DynamicArrayBox* 
	tryAllocateArray (
		Type* type,
		size_t count
		);

	DynamicArrayBox* 
	allocateArray (
		Type* type,
		size_t count
		);

	DynamicArrayBox* 
	tryAllocateBuffer (size_t size);

	DynamicArrayBox* 
	allocateBuffer (size_t size);

	DataPtrValidator*
	createDataPtrValidator (
		Box* box,
		void* rangeBegin,
		size_t rangeLength
		);

	// management methods

	void
	registerStaticRootVariables (		
		Variable* const* variableArray,
		size_t count
		);

	void
	registerStaticRootVariables (const rtl::Array <Variable*>& variableArray)
	{
		registerStaticRootVariables (variableArray, variableArray.getCount ());
	}

	void
	registerStaticRoot (
		void* p,
		Type* type
		);

	void
	registerMutatorThread (GcMutatorThread* thread);

	void
	unregisterMutatorThread (GcMutatorThread* thread);

	void
	enterSafeRegion ();

	void
	leaveSafeRegion ();	

	void
	safePoint ();

	void
	collect ()
	{
		waitIdleAndLock (true);
		collect_l ();
	}

	// marking

	static
	void
	weakMark (Box* box);

	void
	markData (
		Type* type,
		DataPtrValidator* validator
		);

	void
	markClass (Box* box);

	void
	weakMarkClosureClass (Box* box);

	void
	addRoot (
		const void* p,
		Type* type
		);

#if (_AXL_ENV == AXL_ENV_WIN)
	int 
	handleSehException (
		uint_t code, 
		EXCEPTION_POINTERS* exceptionPointers
		);
#endif

protected:
	void
	waitIdleAndLock (bool isSafeRegion);

	void
	incrementAllocSizeAndLock (size_t size);

	void
	collect_l ();

	void
	addClassBox_l (Box* box);

	void
	markClassFields (Box* box);

	void
	addShadowStackFrameRoots (GcShadowStackFrame* frame);

	void
	runMarkCycle ();

#if (_AXL_ENV == AXL_ENV_POSIX)
	void
	installSignalHandlers ();

	static
	void
	signalHandler_SIGSEGV (
		int signal,
		siginfo_t* signalInfo,
		void* context
		);

	static
	void
	signalHandler_SIGUSR1 (int signal)
	{
		// do nothing (we handshake manually). but we still need a handler
	}
#endif
};

//.............................................................................

} // namespace jnc
