// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_rt_RuntimeStructs.h"

namespace jnc {
namespace ct {

class Module;
class Variable;
class ClassType;

} // namespace ct

namespace rt {

class Runtime;

//.............................................................................

enum GcDef
{
	GcDef_AllocSizeTrigger  = -1, // use period only
#ifdef _AXL_DEBUG
	GcDef_PeriodSizeTrigger = 0, // run gc on every allocation
#elif (_AXL_CPU == AXL_CPU_X86)
	GcDef_PeriodSizeTrigger = 1 * 1024 * 1024,  // 1MB gc period
#else
	GcDef_PeriodSizeTrigger = 2 * 1024 * 1024, // 2MB gc period
#endif

#ifdef _AXL_DEBUG
	GcDef_DataPtrValidatorPoolSize = 1, // don't use pool, allocate every time
#else
	GcDef_DataPtrValidatorPoolSize = 32,
#endif

	GcDef_ShutdownIterationLimit   = 3,
};

//.............................................................................

struct GcStats
{
	size_t m_currentAllocSize;
	size_t m_totalAllocSize;
	size_t m_peakAllocSize;
	size_t m_currentPeriodSize;
	size_t m_totalCollectCount;
	size_t m_lastCollectFreeSize;
	uint64_t m_lastCollectTime;
	uint64_t m_lastCollectTimeTaken;
	uint64_t m_totalCollectTimeTaken;
};

//.............................................................................

enum GcHeapFlag
{
	GcHeapFlag_SimpleSafePoint = 0x01,
	GcHeapFlag_ShuttingDown    = 0x02,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
		ct::Type* m_type;
	};

	struct DestructGuard: sl::ListLink
	{
		sl::Array <IfaceHdr*>* m_destructArray;
	};

	struct StaticDestructor: sl::ListLink
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

	sys::Lock m_lock;
	volatile State m_state;
	volatile uint_t m_flags;
	GcStats m_stats;
	sys::NotificationEvent m_idleEvent;
	sl::StdList <StaticDestructor> m_staticDestructorList;

	sl::AuxList <GcMutatorThread> m_mutatorThreadList;
	volatile size_t m_waitingMutatorThreadCount;
	volatile size_t m_noCollectMutatorThreadCount;
	volatile size_t m_handshakeCount;

	sys::Event m_handshakeEvent;
	sys::NotificationEvent m_resumeEvent;

#if (_AXL_ENV == AXL_ENV_WIN)
	sys::win::VirtualMemory m_guardPage;
#elif (_AXL_ENV == AXL_ENV_POSIX)
	io::psx::Mapping m_guardPage;
#	if (_AXL_POSIX == AXL_POSIX_DARWIN)
	sys::drw::Semaphore m_handshakeSem; // mach semaphores can be safely signalled from signal handlers
#	else
	sys::psx::Sem m_handshakeSem; // POSIX sems can be safely signalled from signal handlers
#	endif
	static sigset_t m_signalWaitMask;
#endif

	sl::Array <Box*> m_allocBoxArray;
	sl::Array <Box*> m_classBoxArray;
	sl::Array <Box*> m_destructibleClassBoxArray;
	sl::Array <Box*> m_postponeFreeBoxArray;
	sl::AuxList <DestructGuard> m_destructGuardList;
	sl::Array <Root> m_staticRootArray;
	sl::Array <Root> m_markRootArray [2];
	size_t m_currentMarkRootArrayIdx;

	// adjustable triggers

	size_t m_allocSizeTrigger; 
	size_t m_periodSizeTrigger;

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

	// allocation methods

	IfaceHdr* 
	tryAllocateClass (ct::ClassType* type);

	IfaceHdr* 
	allocateClass (ct::ClassType* type);

	DataPtr
	tryAllocateData (ct::Type* type);

	DataPtr
	allocateData (ct::Type* type);

	DataPtr
	tryAllocateArray (
		ct::Type* type,
		size_t count
		);

	DataPtr
	allocateArray (
		ct::Type* type,
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
	getStats (GcStats* stats);

	void 
	setSizeTriggers (
		size_t allocSizeTrigger,
		size_t periodSizeTrigger
		);

	void
	startup (ct::Module* module);

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
		ct::Variable* const* variableArray,
		size_t count
		);

	void
	addStaticRootVariables (const sl::Array <ct::Variable*>& variableArray)
	{
		addStaticRootVariables (variableArray, variableArray.getCount ());
	}

	void
	addStaticRoot (
		void* p,
		ct::Type* type
		);

	void
	addStaticDestructor (StaticDestructFunc* destructFunc);

	void
	addStaticClassDestructor (
		DestructFunc* destructFunc,
		IfaceHdr* iface
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
		ct::Type* type
		);

	void
	addRootArray (
		const void* p,
		ct::Type* type,
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
	isCollectionTriggered_l ();

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
	parkAtSafePoint ();

#if (_AXL_ENV == AXL_ENV_POSIX) // signal handlers
	static
	void
	installSignalHandlers (int);

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

} // namespace rt
} // namespace jnc
