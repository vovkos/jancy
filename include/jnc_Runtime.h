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
#if (_AXL_PTR_SIZE == 8)
	RuntimeDef_StackSizeLimit = 1 * 1024 * 1024, // 1MB std stack limit
#else
	RuntimeDef_StackSizeLimit = 512 * 1024, // 512KB std stack 
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

protected:
	mt::Lock m_lock;
	Module* m_module;
	State m_state;
	mt::NotificationEvent m_noThreadEvent;
	size_t m_tlsSize;
	rtl::StdList <Tls> m_tlsList;

public:
	GcHeap m_gcHeap;
	size_t m_stackSizeLimit; // adjustable limits

public:
	Runtime ();

	~Runtime ()
	{
		shutdown ();
	}

	Module* 
	getModule ()
	{
		return m_module;
	}

	bool 
	startup (Module* module);
	
	void
	shutdown ();

	void 
	initializeThread (ExceptionRecoverySnapshot* ers);

	void 
	uninitializeThread (ExceptionRecoverySnapshot* ers);

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

class ScopedNoCollectRegion
{
protected:
	GcHeap* m_gcHeap;
	bool m_canCollectOnLeave;

public:
	ScopedNoCollectRegion (
		GcHeap* gcHeap,
		bool canCollectOnLeave
		)
	{
		init (gcHeap, canCollectOnLeave);
	}

	ScopedNoCollectRegion (
		Runtime* runtime,
		bool canCollectOnLeave
		)
	{
		init (&runtime->m_gcHeap, canCollectOnLeave);
	}

	ScopedNoCollectRegion (bool canCollectOnLeave)
	{
		init (&getCurrentThreadRuntime ()->m_gcHeap, canCollectOnLeave);
	}

	~ScopedNoCollectRegion ()
	{
		m_gcHeap->leaveNoCollectRegion (m_canCollectOnLeave);
	}

protected:
	void
	init (
		GcHeap* gcHeap,
		bool canCollectOnLeave
		)
	{
		m_gcHeap = gcHeap;
		m_canCollectOnLeave = canCollectOnLeave;
		gcHeap->enterNoCollectRegion ();
	}
};

//.............................................................................

#if (_AXL_ENV == AXL_ENV_WIN)
#	define JNC_BEGIN_GC_SITE() \
	__try {

#	define JNC_END_GC_SITE() \
	} __except (__jncRuntime->m_gcHeap.handleSehException (GetExceptionCode (), GetExceptionInformation ())) { }

#elif (_AXL_ENV == AXL_ENV_POSIX)
#	define JNC_BEGIN_GC_SITE()
#	define JNC_END_GC_SITE()
#endif

#define JNC_BEGIN_CALL_SITE(runtime) \
{ \
	jnc::Runtime* __jncRuntime = (runtime); \
	bool __jncIsNoCollectRegion = false; \
	bool __jncCanCollectAtEnd = false; \
	jnc::ExceptionRecoverySnapshot ___jncErs; \
	JNC_BEGIN_GC_SITE() \
	__jncRuntime->initializeThread (&___jncErs); \
	AXL_MT_BEGIN_LONG_JMP_TRY () \

#define JNC_BEGIN_CALL_SITE_NO_COLLECT(runtime, canCollectAtEnd) \
	JNC_BEGIN_CALL_SITE (runtime) \
	__jncIsNoCollectRegion = true; \
	__jncCanCollectAtEnd = canCollectAtEnd; \
	__jncRuntime->m_gcHeap.enterNoCollectRegion ();

#define JNC_CALL_SITE_CATCH() \
	AXL_MT_LONG_JMP_CATCH ()

#define JNC_CALL_SITE_FINALLY() \
	AXL_MT_LONG_JMP_FINALLY ()

#define JNC_END_CALL_SITE_IMPL() \
	AXL_MT_END_LONG_JMP_TRY_EX (&___jncErs.m_result) \
	if (__jncIsNoCollectRegion) \
		__jncRuntime->m_gcHeap.leaveNoCollectRegion (__jncCanCollectAtEnd); \
	__jncRuntime->uninitializeThread (&___jncErs); \
	JNC_END_GC_SITE () \

#define JNC_END_CALL_SITE() \
	JNC_END_CALL_SITE_IMPL() \
}

#define JNC_END_CALL_SITE_EX(result) \
	JNC_END_CALL_SITE_IMPL() \
	*(result) = ___jncErs.m_result; \
}

//.............................................................................

} // namespace jnc
