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

class ScopedNoCollectGarbageRegion
{
protected:
	GcHeap* m_gcHeap;
	bool m_canCollectOnLeave;

public:
	ScopedNoCollectGarbageRegion (
		GcHeap* gcHeap,
		bool canCollectOnLeave
		)
	{
		init (gcHeap, canCollectOnLeave);
	}

	ScopedNoCollectGarbageRegion (
		Runtime* runtime,
		bool canCollectOnLeave
		)
	{
		init (&runtime->m_gcHeap, canCollectOnLeave);
	}

	ScopedNoCollectGarbageRegion (bool canCollectOnLeave)
	{
		init (&getCurrentThreadRuntime ()->m_gcHeap, canCollectOnLeave);
	}

	~ScopedNoCollectGarbageRegion ()
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
#	define JNC_GC_BEGIN() \
	__try {

#	define JNC_GC_END() \
	} __except (__jncRuntime->m_gcHeap.handleSehException (GetExceptionCode (), GetExceptionInformation ())) { }

#elif (_AXL_ENV == AXL_ENV_POSIX)
#	define JNC_GC_BEGIN()
#	define JNC_END()
#endif

#define JNC_BEGIN(runtime) \
{ \
	jnc::Runtime* __jncRuntime = (runtime); \
	jnc::ExceptionRecoverySnapshot ___jncErs; \
	JNC_GC_BEGIN() \
	__jncRuntime->initializeThread (&___jncErs); \
	AXL_MT_BEGIN_LONG_JMP_TRY () \
	
#define JNC_CATCH() \
	AXL_MT_LONG_JMP_CATCH ()

#define JNC_END_IMPL() \
	AXL_MT_END_LONG_JMP_TRY_EX (&___jncErs.m_result) \
	__jncRuntime->uninitializeThread (&___jncErs); \
	JNC_GC_END () \

#define JNC_END() \
	JNC_END_IMPL() \
}

#define JNC_END_EX(result) \
	JNC_END_IMPL() \
	*(result) = ___jncErs.m_result; \
}

//.............................................................................

} // namespace jnc
