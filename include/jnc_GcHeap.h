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

#ifdef _AXL_DEBUG
	GcDef_DataPtrValidatorPoolSize = 1, // don't use pool, allocate every time
#else
	GcDef_DataPtrValidatorPoolSize = 32,
#endif

	GcDef_ShutdownIterationLimit   = 3,
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

	struct StaticDestructor: rtl::ListLink
	{
		union
		{
			StaticDestructFunc* m_staticDestructFunc;
			DestructFunc* m_destructFunc;
		};

		IfaceHdr* m_iface;
	};

protected:
	Runtime* m_runtime;

	mt::Lock m_lock;
	volatile State m_state;
	bool m_isShuttingDown;
	mt::NotificationEvent m_idleEvent;

	size_t m_currentAllocSize;
	size_t m_totalAllocSize;
	size_t m_peakAllocSize;
	size_t m_currentPeriodSize;

	rtl::StdList <StaticDestructor> m_staticDestructorList;

	rtl::AuxList <GcMutatorThread> m_mutatorThreadList;
	volatile size_t m_waitingMutatorThreadCount;
	volatile size_t m_noCollectMutatorThreadCount;
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
	rtl::Array <Box*> m_destructibleClassBoxArray;
	rtl::Array <Box*> m_postponeFreeBoxArray;
	rtl::AuxList <DestructGuard> m_destructGuardList;
	rtl::Array <Root> m_staticRootArray;
	rtl::Array <Root> m_markRootArray [2];
	size_t m_currentMarkRootArrayIdx;

public:
	size_t m_periodSizeLimit; // adjustable limit

public:
	GcHeap ();
	
	~GcHeap ()
	{
		ASSERT (isEmpty ()); // should be collected during runtime shutdown
	}

	// informational methods

	bool
	isEmpty ()
	{
		return m_allocBoxArray.isEmpty ();
	}

	Runtime*
	getRuntime ()
	{		
		return m_runtime;
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

	void* 
	getSafePointTrigger ()
	{
		return m_guardPage;
	}

	// allocation methods

	IfaceHdr* 
	tryAllocateClass (ClassType* type);

	IfaceHdr* 
	allocateClass (ClassType* type);

	DataPtr
	tryAllocateData (Type* type);

	DataPtr
	allocateData (Type* type);

	DataPtr
	tryAllocateArray (
		Type* type,
		size_t count
		);

	DataPtr
	allocateArray (
		Type* type,
		size_t count
		);

	DataPtr
	tryAllocateBuffer (size_t size);

	DataPtr 
	allocateBuffer (size_t size);

	DataPtrValidator*
	createDataPtrValidator (
		Box* box,
		void* rangeBegin,
		size_t rangeLength
		);

	// management methods

	void
	beginShutdown ();

	void
	finalizeShutdown ();

	void
	registerMutatorThread (GcMutatorThread* thread);

	void
	unregisterMutatorThread (GcMutatorThread* thread);

	void
	addStaticRootVariables (		
		Variable* const* variableArray,
		size_t count
		);

	void
	addStaticRootVariables (const rtl::Array <Variable*>& variableArray)
	{
		addStaticRootVariables (variableArray, variableArray.getCount ());
	}

	void
	addStaticRoot (
		void* p,
		Type* type
		);

	void
	addStaticDestructor (StaticDestructFunc* destructFunc);

	void
	addStaticClassDestructor (
		DestructFunc* destructFunc,
		jnc::IfaceHdr* iface
		);

	void
	enterWaitRegion ();

	void
	leaveWaitRegion ();	

	void
	enterNoCollectRegion ();

	void
	leaveNoCollectRegion (bool canCollectNow);	

	void
	safePoint ();

	void
	collect ();

	// marking

	static
	void
	weakMark (Box* box);

	void
	markData (Box* box);

	void
	markClass (Box* box);

	void
	weakMarkClosureClass (Box* box);

	void
	addRoot (
		const void* p,
		Type* type
		);

	void
	addRootArray (
		const void* p,
		Type* type,
		size_t count
		);

#if (_AXL_ENV == AXL_ENV_WIN)
	int 
	handleSehException (
		uint_t code, 
		EXCEPTION_POINTERS* exceptionPointers
		);
#endif

protected:
	bool
	waitIdleAndLock (); // return true if this thread is registered mutator thread

	void
	incrementAllocSizeAndLock (size_t size);

	void
	incrementAllocSize_l (size_t size);

	void
	collect_l (bool isMutatorThread);

	void
	addClassBox_l (Box* box);

	void
	markClassFields (Box* box);

	void
	runMarkCycle ();

	void
	runDestructors (
		IfaceHdr* const* ifaceArray,
		size_t count
		);

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
