// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Runtime.h"
#include "jnc_Function.h"

//.............................................................................

#if (_AXL_ENV == AXL_ENV_WIN)
#	define JNC_BEGIN_GC_SITE() \
	__try {

#define JNC_END_GC_SITE() \
	} __except (jnc_GcHeap_handleGcSehException ( \
		__jncGcHeap, \
		GetExceptionCode (), \
		GetExceptionInformation () \
		)) { }

#else
#	define JNC_BEGIN_GC_SITE()
#	define JNC_END_GC_SITE()
#endif

#define JNC_BEGIN_CALL_SITE(runtime) \
{ \
	jnc_Runtime* __jncRuntime = (runtime); \
	jnc_GcHeap* __jncGcHeap = jnc_Runtime_getGcHeap (runtime); \
	int __jncIsNoCollectRegion = 0; \
	int __jncCanCollectAtEnd = 0; \
	jnc_ExceptionRecoverySnapshot ___jncErs; \
	JNC_BEGIN_GC_SITE() \
	jnc_Runtime_initializeThread (__jncRuntime, &___jncErs); \
	AXL_SYS_BEGIN_SJLJ_TRY () \

#define JNC_BEGIN_CALL_SITE_NO_COLLECT(runtime, canCollectAtEnd) \
	JNC_BEGIN_CALL_SITE (runtime) \
	__jncIsNoCollectRegion = 1; \
	__jncCanCollectAtEnd = canCollectAtEnd; \
	jnc_GcHeap_enterNoCollectRegion (__jncGcHeap);

#define JNC_CALL_SITE_CATCH() \
	AXL_SYS_SJLJ_CATCH ()

#define JNC_CALL_SITE_FINALLY() \
	AXL_SYS_SJLJ_FINALLY ()

#define JNC_END_CALL_SITE_IMPL() \
	AXL_SYS_END_SJLJ_TRY_EX (&___jncErs.m_result) \
	if (__jncIsNoCollectRegion && ___jncErs.m_result) \
		jnc_GcHeap_leaveNoCollectRegion (__jncGcHeap, __jncCanCollectAtEnd); \
	jnc_Runtime_uninitializeThread (__jncRuntime, &___jncErs); \
	JNC_END_GC_SITE () \

#define JNC_END_CALL_SITE() \
	JNC_END_CALL_SITE_IMPL() \
}

#define JNC_END_CALL_SITE_EX(result) \
	JNC_END_CALL_SITE_IMPL() \
	*(result) = ___jncErs.m_result != 0; \
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

inline
void
primeClass (
	Box* box,
	ClassType* type,
	void* vtable = NULL // if null then vtable of class type will be used
	)
{
	jnc_primeClass (box, box, type, vtable);
}

template <typename T>
void
primeClass (
	Module* module,
	ClassBoxBase <T>* p,
	Box* root
	)
{
	primeClass (p, root, T::getType (module), T::getVTable ());
}

template <typename T>
void
primeClass (
	Runtime* runtime,
	ClassBoxBase <T>* p,
	Box* root
	)
{
	primeClass (p, root, T::getType (runtime), T::getVTable ());
}

template <typename T>
void
primeClass (
	Module* module,
	ClassBoxBase <T>* p
	)
{
	primeClass (p, p, T::getType (module), T::getVTable ());
}

template <typename T>
void
primeClass (
	Runtime* runtime,
	ClassBoxBase <T>* p
	)
{
	primeClass (p, p, T::getType (runtime), T::getVTable ());
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
		init (runtime->getGcHeap (), canCollectOnLeave);
	}

	ScopedNoCollectRegion (bool canCollectOnLeave)
	{
		GcHeap* gcHeap = getCurrentThreadGcHeap ();
		ASSERT (gcHeap);

		init (gcHeap, canCollectOnLeave);
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
		m_gcHeap->enterNoCollectRegion ();
	}
};

//.............................................................................

template <typename RetVal>
RetVal
callFunctionImpl_u (void* p)
{
	typedef
	RetVal
	TargetFunc ();

	return ((TargetFunc*) p) ();
}

template <
	typename RetVal,
	typename Arg
	>
RetVal
callFunctionImpl_u (
	void* p,
	Arg arg
	)
{
	typedef
	RetVal
	TargetFunc (Arg);

	return ((TargetFunc*) p) (arg);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2
	>
RetVal
callFunctionImpl_u (
	void* p,
	Arg1 arg1,
	Arg2 arg2
	)
{
	typedef
	RetVal
	TargetFunc (
		Arg1,
		Arg2
		);

	return ((TargetFunc*) p)  (arg1, arg2);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
RetVal
callFunctionImpl_u (
	void* p,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	typedef
	RetVal
	TargetFunc (
		Arg1,
		Arg2,
		Arg3
		);

	return ((TargetFunc*) p) (arg1, arg2, arg3);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
RetVal
callFunctionImpl_u (
	void* p,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	typedef
	RetVal
	TargetFunc (
		Arg1,
		Arg2,
		Arg3,
		Arg4
		);

	return ((TargetFunc*) p) (arg1, arg2, arg3, arg4);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4,
	typename Arg5
	>
RetVal
callFunctionImpl_u (
	void* p,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4,
	Arg5 arg5
	)
{
	typedef
	RetVal
	TargetFunc (
		Arg1,
		Arg2,
		Arg3,
		Arg4,
		Arg5
		);

	return ((TargetFunc*) p) (arg1, arg2, arg3, arg4, arg5);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename RetVal>
bool
callFunctionImpl_s (
	Runtime* runtime,
	void* p,
	RetVal* retVal
	)
{
	bool result;

	JNC_BEGIN_CALL_SITE (runtime)
	*retVal = callFunctionImpl_u <RetVal> (p);
	JNC_END_CALL_SITE_EX (&result)

	return result;
}

template <
	typename RetVal,
	typename Arg
	>
bool
callFunctionImpl_s (
	Runtime* runtime,
	void* p,
	RetVal* retVal,
	Arg arg
	)
{
	bool result;

	JNC_BEGIN_CALL_SITE (runtime)
	*retVal = callFunctionImpl_u <RetVal> (p, arg);
	JNC_END_CALL_SITE_EX (&result)

	return result;
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2
	>
bool
callFunctionImpl_s (
	Runtime* runtime,
	void* p,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2
	)
{
	bool result;

	JNC_BEGIN_CALL_SITE (runtime)
	*retVal = callFunctionImpl_u <RetVal> (p, arg1, arg2);
	JNC_END_CALL_SITE_EX (&result)

	return result;
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callFunctionImpl_s (
	Runtime* runtime,
	void* p,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	bool result;

	JNC_BEGIN_CALL_SITE (runtime)
	*retVal = callFunctionImpl_u <RetVal> (p, arg1, arg2, arg3);
	JNC_END_CALL_SITE_EX (&result)

	return result;
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
bool
callFunctionImpl_s (
	Runtime* runtime,
	void* p,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	bool result;

	JNC_BEGIN_CALL_SITE (runtime)
	*retVal = callFunctionImpl_u <RetVal> (p, arg1, arg2, arg3, arg4);
	JNC_END_CALL_SITE_EX (&result)

	return result;
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4,
	typename Arg5
	>
bool
callFunctionImpl_s (
	Runtime* runtime,
	void* p,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4,
	Arg5 arg5
	)
{
	bool result;

	JNC_BEGIN_CALL_SITE (runtime)
	*retVal = callFunctionImpl_u <RetVal> (p, arg1, arg2, arg3, arg4, arg5);
	JNC_END_CALL_SITE_EX (&result)

	return result;
}

//.............................................................................

template <typename RetVal>
bool
callFunction (
	Runtime* runtime,
	Function* function,
	RetVal* retVal
	)
{
	void* p = function->getMachineCode ();
	return callFunctionImpl_s (runtime, p, retVal);
}

template <
	typename RetVal,
	typename Arg
	>
bool
callFunction (
	Runtime* runtime,
	Function* function,
	RetVal* retVal,
	Arg arg
	)
{
	void* p = function->getMachineCode ();
	return callFunctionImpl_s (runtime, p, retVal, arg);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2
	>
bool
callFunction (
	Runtime* runtime,
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2
	)
{
	void* p = function->getMachineCode ();
	return callFunctionImpl_s (runtime, p, retVal, arg1, arg2);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callFunction (
	Runtime* runtime,
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	void* p = function->getMachineCode ();
	return callFunctionImpl_s (runtime, p, retVal, arg1, arg2, arg3);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
bool
callFunction (
	Runtime* runtime,
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	void* p = function->getMachineCode ();
	return callFunctionImpl_s (runtime, p, retVal, arg1, arg2, arg3, arg4);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename RetVal>
RetVal
callFunction (Function* function)
{
	void* p = function->getMachineCode ();
	return callFunctionImpl_u <RetVal> (p);
}

template <
	typename RetVal,
	typename Arg
	>
RetVal
callFunction (
	Function* function,
	Arg arg
	)
{
	void* p = function->getMachineCode ();
	return callFunctionImpl_u <RetVal> (p, arg);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2
	>
RetVal
callFunction (
	Function* function,
	Arg1 arg1,
	Arg2 arg2
	)
{
	void* p = function->getMachineCode ();
	return callFunctionImpl_u <RetVal> (p, arg1, arg2);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
RetVal
callFunction (
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	void* p = function->getMachineCode ();
	return callFunctionImpl_u <RetVal> (p, arg1, arg2, arg3);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
RetVal
callFunction (
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	void* p = function->getMachineCode ();
	return callFunctionImpl_u <RetVal> (p, arg1, arg2, arg3, arg4);
}

//.............................................................................

inline
bool
callVoidFunction (
	Runtime* runtime,
	Function* function
	)
{
	int retVal;
	void* p = function->getMachineCode ();
	return callFunctionImpl_s (runtime, p, &retVal);
}

template <typename Arg>
bool
callVoidFunction (
	Runtime* runtime,
	Function* function,
	Arg arg
	)
{
	int retVal;
	void* p = function->getMachineCode ();
	return callFunctionImpl_s (runtime, p, &retVal, arg);
}

template <
	typename Arg1,
	typename Arg2
	>
bool
callVoidFunction (
	Runtime* runtime,
	Function* function,
	Arg1 arg1,
	Arg2 arg2
	)
{
	int retVal;
	void* p = function->getMachineCode ();
	return callFunctionImpl_s (runtime, p, &retVal, arg1, arg2);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callVoidFunction (
	Runtime* runtime,
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	int retVal;
	void* p = function->getMachineCode ();
	return callFunctionImpl_s (runtime, p, &retVal, arg1, arg2, arg3);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
bool
callVoidFunction (
	Runtime* runtime,
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	int retVal;
	void* p = function->getMachineCode ();
	return callFunctionImpl_s (runtime, p, &retVal, arg1, arg2, arg3, arg4);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
callVoidFunction (Function* function)
{
	void* p = function->getMachineCode ();
	callFunctionImpl_u <void> (p);
}

template <typename Arg>
void
callVoidFunction (
	Function* function,
	Arg arg
	)
{
	void* p = function->getMachineCode ();
	callFunctionImpl_u <void> (p, arg);
}

template <
	typename Arg1,
	typename Arg2
	>
void
callVoidFunction (
	Function* function,
	Arg1 arg1,
	Arg2 arg2
	)
{
	void* p = function->getMachineCode ();
	callFunctionImpl_u <void> (p, arg1, arg2);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
void
callVoidFunction (
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	void* p = function->getMachineCode ();
	callFunctionImpl_u <void> (p, arg1, arg2, arg3);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
void
callVoidFunction (
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	void* p = function->getMachineCode ();
	callFunctionImpl_u <void> (p, arg1, arg2, arg3, arg4);
}

//.............................................................................

template <typename RetVal>
bool
callFunctionPtr (
	Runtime* runtime,
	FunctionPtr ptr,
	RetVal* retVal
	)
{
	return callFunctionImpl_s (runtime, ptr.m_p, retVal, ptr.m_closure);
}

template <
	typename RetVal,
	typename Arg
	>
bool
callFunctionPtr (
	Runtime* runtime,
	FunctionPtr ptr,
	RetVal* retVal,
	Arg arg
	)
{
	return callFunctionImpl_s (runtime, ptr.m_p, retVal, ptr.m_closure, arg);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2
	>
bool
callFunctionPtr (
	Runtime* runtime,
	FunctionPtr ptr,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2
	)
{
	return callFunctionImpl_s (runtime, ptr.m_p, retVal, ptr.m_closure, arg1, arg2);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callFunctionPtr (
	Runtime* runtime,
	FunctionPtr ptr,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	return callFunctionImpl_s (runtime, ptr.m_p, retVal, ptr.m_closure, arg1, arg2, arg3);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
bool
callFunctionPtr (
	Runtime* runtime,
	FunctionPtr ptr,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	return callFunctionImpl_s (runtime, ptr.m_p, retVal, ptr.m_closure, arg1, arg2, arg3, arg4);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename RetVal>
RetVal
callFunctionPtr (FunctionPtr ptr)
{
	return callFunctionImpl_u <RetVal> (ptr.m_p, ptr.m_closure);
}

template <
	typename RetVal,
	typename Arg
	>
RetVal
callFunctionPtr (
	FunctionPtr ptr,
	RetVal* retVal,
	Arg arg
	)
{
	return callFunctionImpl_u <RetVal> (ptr.m_p, ptr.m_closure, arg);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2
	>
RetVal
callFunctionPtr (
	FunctionPtr ptr,
	Arg1 arg1,
	Arg2 arg2
	)
{
	return callFunctionImpl_u <RetVal> (ptr.m_p, ptr.m_closure, arg1, arg2);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
RetVal
callFunctionPtr (
	FunctionPtr ptr,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	return callFunctionImpl_u <RetVal> (ptr.m_p, ptr.m_closure, arg1, arg2, arg3);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
RetVal
callFunctionPtr (
	FunctionPtr ptr,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	return callFunctionImpl_u <RetVal> (ptr.m_p, ptr.m_closure, arg1, arg2, arg3, arg4);
}

//.............................................................................

inline
bool
callVoidFunctionPtr (
	Runtime* runtime,
	FunctionPtr ptr
	)
{
	int retVal;
	return callFunctionImpl_s (runtime, ptr.m_p, &retVal, ptr.m_closure);
}

template <typename Arg>
bool
callVoidFunctionPtr (
	Runtime* runtime,
	FunctionPtr ptr,
	Arg arg
	)
{
	int retVal;
	return callFunctionImpl_s (runtime, ptr.m_p, &retVal, ptr.m_closure, arg);
}

template <
	typename Arg1,
	typename Arg2
	>
bool
callVoidFunctionPtr (
	Runtime* runtime,
	FunctionPtr ptr,
	Arg1 arg1,
	Arg2 arg2
	)
{
	int retVal;
	return callFunctionImpl_s (runtime, ptr.m_p, &retVal, ptr.m_closure, arg1, arg2);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callVoidFunctionPtr (
	Runtime* runtime,
	FunctionPtr ptr,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	int retVal;
	return callFunctionImpl_s (runtime, ptr.m_p, &retVal, ptr.m_closure, arg1, arg2, arg3);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
bool
callVoidFunctionPtr (
	Runtime* runtime,
	FunctionPtr ptr,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	int retVal;
	return callFunctionImpl_s (runtime, ptr.m_p, &retVal, ptr.m_closure, arg1, arg2, arg3, arg4);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
callVoidFunctionPtr (FunctionPtr ptr)
{
	callFunctionImpl_u <void> (ptr.m_p, ptr.m_closure);
}

template <typename Arg>
void
callVoidFunctionPtr (
	FunctionPtr ptr,
	Arg arg
	)
{
	callFunctionImpl_u <void> (ptr.m_p, ptr.m_closure, arg);
}

template <
	typename Arg1,
	typename Arg2
	>
void
callVoidFunctionPtr (
	FunctionPtr ptr,
	Arg1 arg1,
	Arg2 arg2
	)
{
	callFunctionImpl_u <void> (ptr.m_p, ptr.m_closure, arg1, arg2);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
void
callVoidFunctionPtr (
	FunctionPtr ptr,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	callFunctionImpl_u <void> (ptr.m_p, ptr.m_closure, arg1, arg2, arg3);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
void
callVoidFunctionPtr (
	FunctionPtr ptr,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	callFunctionImpl_u <void> (ptr.m_p, ptr.m_closure, arg1, arg2, arg3, arg4);
}

//.............................................................................

inline
bool
callMulticast (
	Runtime* runtime,
	Multicast* multicast
	)
{
	void* p = getMulticastCallMethodMachineCode (multicast);
	FunctionPtr ptr = { p, multicast };
	return callVoidFunctionPtr (runtime, ptr);
}

template <typename Arg>
bool
callMulticast (
	Runtime* runtime,
	Multicast* multicast,
	Arg arg
	)
{
	void* p = getMulticastCallMethodMachineCode (multicast);
	FunctionPtr ptr = { p, multicast };
	return callVoidFunctionPtr (runtime, ptr, arg);
}

template <
	typename Arg1,
	typename Arg2
	>
bool
callMulticast (
	Runtime* runtime,
	Multicast* multicast,
	Arg1 arg1,
	Arg2 arg2
	)
{
	void* p = getMulticastCallMethodMachineCode (multicast);
	FunctionPtr ptr = { p, multicast };
	return callVoidFunctionPtr (runtime, ptr, arg1, arg2);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callMulticast (
	Runtime* runtime,
	Multicast* multicast,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	void* p = getMulticastCallMethodMachineCode (multicast);
	FunctionPtr ptr = { p, multicast };
	return callVoidFunctionPtr (runtime, ptr, arg1, arg2, arg3);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
bool
callMulticast (
	Runtime* runtime,
	Multicast* multicast,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	void* p = getMulticastCallMethodMachineCode (multicast);
	FunctionPtr ptr = { p, multicast };
	return callVoidFunctionPtr (runtime, ptr, arg1, arg2, arg3, arg4);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
callMulticast (Multicast* multicast)
{
	void* p = getMulticastCallMethodMachineCode (multicast);
	FunctionPtr ptr = { p, multicast };
	callVoidFunctionPtr (ptr);
}

template <typename Arg>
void
callMulticast (
	Multicast* multicast,
	Arg arg
	)
{
	void* p = getMulticastCallMethodMachineCode (multicast);
	FunctionPtr ptr = { p, multicast };
	callVoidFunctionPtr (ptr, arg);
}

template <
	typename Arg1,
	typename Arg2
	>
void
callMulticast (
	Multicast* multicast,
	Arg1 arg1,
	Arg2 arg2
	)
{
	void* p = getMulticastCallMethodMachineCode (multicast);
	FunctionPtr ptr = { p, multicast };
	callVoidFunctionPtr (ptr, arg1, arg2);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
void
callMulticast (
	Multicast* multicast,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	void* p = getMulticastCallMethodMachineCode (multicast);
	FunctionPtr ptr = { p, multicast };
	callVoidFunctionPtr (ptr, arg1, arg2, arg3);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
void
callMulticast (
	Multicast* multicast,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	void* p = getMulticastCallMethodMachineCode (multicast);
	FunctionPtr ptr = { p, multicast };
	callVoidFunctionPtr (ptr, arg1, arg2, arg3, arg4);
}

//.............................................................................

template <typename T>
T*
createClass (Runtime* runtime)
{
	ClassType* type = T::getType (runtime);
	T* p = (T*) allocateClass (runtime, type);
	sl::construct (p);
	
	return p;
}

template <
	typename T,
	typename Arg
	>
T*
createClass (
	Runtime* runtime,
	Arg arg
	)
{
	ClassType* type = T::getType (runtime);
	T* p = (T*) allocateClass (runtime, type);
	sl::construct (p, arg);

	return p;
}

template <
	typename T,
	typename Arg1,
	typename Arg2
	>
T*
createClass (
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2
	)
{
	ClassType* type = T::getType (runtime);
	T* p = (T*) allocateClass (runtime, type);
	sl::construct (p, arg1, arg2);

	return p;
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
T*
createClass (
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	ClassType* type = T::getType (runtime);
	T* p = (T*) allocateClass (runtime, type);
	sl::construct (p, arg1, arg2, arg3);

	return p;
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
T*
createClass (
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	ClassType* type = T::getType (runtime);
	T* p = (T*) allocateClass (runtime, type);
	sl::construct (p, arg1, arg2, arg3, arg4);

	return p;
}

//.............................................................................

template <typename T>
DataPtr
createData (Runtime* runtime)
{
	Type* type = T::getType (runtime);
	DataPtr ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p);

	return ptr;
}

template <
	typename T,
	typename Arg
	>
DataPtr
createData (
	Runtime* runtime,
	Arg arg
	)
{
	Type* type = T::getType (runtime);
	DataPtr ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p, arg);

	return ptr;
}

template <
	typename T,
	typename Arg1,
	typename Arg2
	>
DataPtr
createData (
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2
	)
{
	Type* type = T::getType (runtime);
	DataPtr ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p, arg1, arg2);

	return ptr;
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
DataPtr
createData (
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	Type* type = T::getType (runtime);
	DataPtr ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p, arg1, arg2, arg3);

	return ptr;
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3,
	typename Arg4
	>
DataPtr
createData (
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	Type* type = T::getType (runtime);
	DataPtr ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p, arg1, arg2, arg3, arg4);

	return ptr;
}

//.............................................................................

} // namespace jnc

#endif // __cplusplus
