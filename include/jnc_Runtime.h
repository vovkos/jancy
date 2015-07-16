// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_GcHeap.h"

namespace jnc {

class Module;

//.............................................................................

enum RuntimeDef
{
#if (_AXL_CPU == AXL_CPU_X86)
	RuntimeDef_StackSizeLimit = 2 * 1024 * 1024, // 2MB std stack 
#else
	RuntimeDef_StackSizeLimit = 4 * 1024 * 1024, // 4MB std stack limit
#endif
};

//.............................................................................

class Runtime
{
protected:
	enum State
	{
		State_Idle,
		State_Running,
		State_ShuttingDown,
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
	mt::Lock m_lock;
	rtl::Array <Module*> m_moduleArray;
	State m_state;
	mt::NotificationEvent m_noThreadEvent;
	size_t m_tlsSize;
	rtl::StdList <Tls> m_tlsList;
	rtl::StdList <StaticDestructor> m_staticDestructorList;

public:
	GcHeap m_gcHeap;
	size_t m_stackSizeLimit; // adjustable limits

public:
	Runtime ();

	~Runtime ()
	{
		shutdown ();
	}

	rtl::Array <Module*> 
	getModuleArray ()
	{
		return m_moduleArray;
	}

	Module* 
	getFirstModule ()
	{
		return !m_moduleArray.isEmpty () ? m_moduleArray [0] : NULL;
	}

	bool
	addModule (Module* module);

	void
	addStaticDestructor (StaticDestructFunc* destructFunc);

	void
	addStaticClassDestructor (
		DestructFunc* destructFunc,
		jnc::IfaceHdr* iface
		);

	bool 
	startup ();
	
	void
	shutdown ();

	void 
	initializeThread ();

	void 
	uninitializeThread ();

	void
	checkStackOverflow ();

	static
	void
	runtimeError (const err::Error& error)
	{
		err::setError (error);
		AXL_MT_LONG_JMP_THROW ();
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Runtime*
getCurrentThreadRuntime ()
{
	return mt::getTlsSlotValue <Runtime> ();
}

//.............................................................................

#define JNC_BEGIN(runtime) \
	jnc::Runtime* __jncRuntime = (runtime); \
	__jncRuntime->initializeThread (); \
	JNC_GC_BEGIN() \
	AXL_MT_BEGIN_LONG_JMP_TRY () \
	
#define JNC_CATCH() \
	AXL_MT_LONG_JMP_CATCH ()

#define JNC_END() \
	AXL_MT_END_LONG_JMP_TRY () \
	JNC_GC_END () \
	__jncRuntime->uninitializeThread ();

#if (_AXL_ENV == AXL_ENV_WIN)
#	define JNC_GC_BEGIN() \
	__try {

#	define JNC_GC_END() \
	} __except (__jncRuntime->m_gcHeap.handleSehException (GetExceptionCode (), GetExceptionInformation ())) { }

#elif (_AXL_ENV == AXL_ENV_POSIX)
#	define JNC_GC_BEGIN()
#	define JNC_END()
#endif

#if 0

class Module;

//.............................................................................


//.............................................................................

struct Tls: public rtl::ListLink
{
	Tls* m_prev;
	Runtime* m_runtime;
	GcMutatorThread m_gcThread;
	void* m_stackEpoch;

	// followed by user TLS variables
};

//.............................................................................

enum CreateObjectFlag
{
	CreateObjectFlag_Prime     = 0x01,
	CreateObjectFlag_Construct = 0x02,
	CreateObjectFlag_Pin       = 0x04,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

//.............................................................................

class Runtime
{
protected:
	enum GcState
	{
		GcState_Idle = 0,
		GcState_WaitSafePoint,
		GcState_Mark,
		GcState_Sweep,
	};

protected:
	rtl::Array <Module*> m_moduleArray;

	// gc-heap

	GcHeap m_gcHeap;

	volatile GcState m_gcState;

	size_t m_gcTotalAllocSize;
	size_t m_gcCurrentAllocSize;
	size_t m_gcCurrentPeriodSize;

	mt::NotificationEvent m_gcIdleEvent;
	mt::NotificationEvent m_gcSafePointEvent;
	volatile intptr_t m_gcUnsafeThreadCount;
	rtl::Array <Box*> m_gcObjectArray;
	rtl::Array <Box*> m_gcMemBlockArray;

	rtl::HashTable <IfaceHdr*, rtl::HashId <IfaceHdr*> > m_gcPinTable;
	rtl::AuxList <GcDestructGuard> m_gcDestructGuardList;
	rtl::Array <GcRoot> m_gcStaticRootArray;
	rtl::Array <GcRoot> m_gcRootArray [2];
	size_t m_gcCurrentRootArrayIdx;

	// tls

	size_t m_tlsSlot;

public:
	Runtime ();

	~Runtime ()
	{
		destroy ();
	}

	rtl::Array <Module*> 
	getModuleArray ()
	{
		return m_moduleArray;
	}

	Module* 
	getFirstModule ()
	{
		return !m_moduleArray.isEmpty () ? m_moduleArray [0] : NULL;
	}

	bool
	addModule (Module* module);

	bool
	create ();

	void
	destroy ();

	bool
	startup ();

	void
	shutdown ();
	
	void
	initializeThread ();

	void
	uninitializeThread ();

	static
	void
	runtimeError (
		int error,
		void* codeAddr,
		void* dataAddr
		);

	static
	void
	runtimeError (const err::Error& error)
	{
		err::setError (error);
		AXL_MT_LONG_JMP_THROW ();
	}

	// gc heap

	void
	runGc ();

	size_t
	gcMakeThreadSafe (); // return prev gc level

	void
	restoreGcLevel (size_t prevGcLevel);

	void
	gcEnter ();

	void
	gcLeave ();

	void
	gcSafePoint ();

	void*
	gcAllocate (
		Type* type,
		size_t elementCount = 1
		);

	void*
	gcTryAllocate (
		Type* type,
		size_t elementCount = 1
		);

	void*
	gcAllocate (size_t size);

	void*
	gcTryAllocate (size_t size);

	void
	addGcRoot (
		void* p,
		Type* type
		);

	// creating objects on gc heap

	IfaceHdr*
	createObject (
		ClassType* type,
		uint_t flags =
			CreateObjectFlag_Prime | 
			CreateObjectFlag_Construct | 
			CreateObjectFlag_Pin
		);

	void
	pinObject (IfaceHdr* object);

	void
	unpinObject (IfaceHdr* object);

	// tls

	size_t
	getTlsSlot ()
	{
		return m_tlsSlot;
	}

	Tls*
	getTls ();

	void
	destroyTls (Tls* tls);

protected:
	void
	waitGcIdleAndLock ();

	void
	gcAddObject (
		Box* object,
		ClassType* type
		);

	void
	runGc_l ();

	void
	gcMarkCycle ();

	void
	gcIncrementUnsafeThreadCount ();

	void
	gcDecrementUnsafeThreadCount ();

	void
	markGcLocalHeapRoot (
		void* p,
		Type* type
		);
};

//.............................................................................

#endif

} // namespace jnc
