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

#pragma once

#include "jnc_GcHeap.h"

namespace jnc {
namespace rt {

//..............................................................................

enum GcHeapFlag
{
	GcHeapFlag_SimpleSafePoint         = 0x01,
	GcHeapFlag_ShuttingDown            = 0x02,
	GcHeapFlag_TerminateDestructThread = 0x04,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

	struct StaticDestructor: sl::ListLink
	{
		union
		{
			StaticDestructFunc* m_staticDestructFunc;
			DestructFunc* m_destructFunc;
		};

		IfaceHdr* m_iface;
	};

	class DestructThread: public axl::sys::ThreadImpl <DestructThread>
	{
	public:
		void
		threadFunc ()
		{
			containerof (this, GcHeap, m_destructThread)->destructThreadFunc ();
		}
	};

	typedef sl::AuxList <GcMutatorThread, GetGcMutatorThreadLink> MutatorThreadList;

protected:
	Runtime* m_runtime;

	sys::Lock m_lock;
	volatile State m_state;
	volatile uint_t m_flags;
	GcStats m_stats;
	sys::NotificationEvent m_idleEvent;
	sl::StdList <StaticDestructor> m_staticDestructorList;
	sl::Array <IfaceHdr*> m_dynamicDestructArray;

	DestructThread m_destructThread;

	MutatorThreadList m_mutatorThreadList;
	volatile size_t m_waitingMutatorThreadCount;
	volatile size_t m_noCollectMutatorThreadCount;
	volatile size_t m_handshakeCount;

	sys::Event m_destructEvent;
	sys::NotificationEvent m_noDestructorEvent;
	sys::Event m_handshakeEvent;
	sys::NotificationEvent m_resumeEvent;

#if (_JNC_OS_WIN)
	sys::win::VirtualMemory m_guardPage;
#elif (_JNC_OS_POSIX)
	io::psx::Mapping m_guardPage;
#	if (_JNC_OS_DARWIN)
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
	sl::Array <Root> m_staticRootArray;
	sl::Array <Root> m_markRootArray [2];
	size_t m_currentMarkRootArrayIdx;

	sl::HashTable <Box*, IfaceHdr*, sl::HashId <Box*> > m_dynamicLayoutMap;

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

	// dynamic layout methods

	IfaceHdr*
	getDynamicLayout (Box* box);

	void
	resetDynamicLayout (Box* box);

	// management methods

	void
	getStats (GcStats* stats);

	void
	getSizeTriggers (GcSizeTriggers* triggers)
	{
		triggers->m_allocSizeTrigger = m_allocSizeTrigger;
		triggers->m_periodSizeTrigger = m_periodSizeTrigger;
	}

	void
	setSizeTriggers (
		size_t allocSizeTrigger,
		size_t periodSizeTrigger
		);

	void
	setSizeTriggers (const GcSizeTriggers& triggers)
	{
		setSizeTriggers (triggers.m_allocSizeTrigger, triggers.m_periodSizeTrigger);
	}

	bool
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
		const void* p,
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

	void
	setFrameMap (
		GcShadowStackFrame* frame,
		GcShadowStackFrameMap* map,
		bool isOpen
		);

	// marking

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

#if (_JNC_OS_WIN)
	static
	int
	handleSehException (
		uint_t code,
		EXCEPTION_POINTERS* exceptionPointers
		);
#endif

	static
	bool
	addBoxIfDynamicFrame (Box* box);

protected:
	void
	destructThreadFunc ();

	GcMutatorThread*
	getCurrentGcMutatorThread ();

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
	runDestructCycle_l ();

	void
	parkAtSafePoint ();

#if (_JNC_OS_POSIX) // signal handlers
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

//..............................................................................

} // namespace rt
} // namespace jnc
