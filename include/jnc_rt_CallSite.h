// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#ifdef _JNC_SHARED_EXTENSION_LIB
#	include "jnc_rt_RuntimeStructs.h"
#	include "jnc_ext_ExtensionLibHost.h"
#else
#	include "jnc_ct_Function.h"
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_MulticastClassType.h"
#endif

namespace jnc {
namespace rt {

//.............................................................................

#if (_AXL_ENV == AXL_ENV_WIN)
#	define JNC_BEGIN_GC_SITE() \
	__try {

#	ifdef _JNC_SHARED_EXTENSION_LIB
#		define JNC_END_GC_SITE() \
	} __except (jnc::ext::g_extensionLibHost->handleGcSehException (__jncRuntime, GetExceptionCode (), GetExceptionInformation ())) { }
#	else
#		define JNC_END_GC_SITE() \
	} __except (__jncRuntime->m_gcHeap.handleSehException (GetExceptionCode (), GetExceptionInformation ())) { }
#endif

#elif (_AXL_ENV == AXL_ENV_POSIX)
#	define JNC_BEGIN_GC_SITE()
#	define JNC_END_GC_SITE()
#endif

#ifdef _JNC_SHARED_EXTENSION_LIB

typedef ext::Runtime   RuntimeRef;
typedef ext::Module    Module;
typedef ext::Function  Function;
typedef ext::Type      Type;
typedef ext::ClassType ClassType;

inline
RuntimeRef* 
getCurrentThreadRuntimeRef ()
{
	return ext::g_extensionLibHost->getCurrentThreadRuntime ();	
}

inline
void
initializeThread (
	RuntimeRef* runtime,
	ExceptionRecoverySnapshot* ers
	)
{
	ext::g_extensionLibHost->initializeRuntimeThread (runtime, ers);
}

inline
void
uninitializeThread (
	RuntimeRef* runtime,
	ExceptionRecoverySnapshot* ers
	)
{
	ext::g_extensionLibHost->uninitializeRuntimeThread (runtime, ers);
}

inline
void
enterNoCollectRegion (RuntimeRef* runtime)
{
	ext::g_extensionLibHost->enterNoCollectRegion (runtime);
}

inline
void
leaveNoCollectRegion (
	RuntimeRef* runtime,
	bool canCollectNow
	)
{
	ext::g_extensionLibHost->leaveNoCollectRegion (runtime, canCollectNow);
}

inline
void
enterWaitRegion (RuntimeRef* runtime)
{
	ext::g_extensionLibHost->enterWaitRegion (runtime);
}

inline
void
leaveWaitRegion (RuntimeRef* runtime)
{
	ext::g_extensionLibHost->leaveWaitRegion (runtime);
}

inline
void*
getFunctionMachineCode (Function* function)
{
	ext::g_extensionLibHost->getFunctionMachineCode (function);
}

inline
void*
getMulticastCallMethodMachineCode (Multicast* multicast)
{
	return ext::g_extensionLibHost->getMulticastCallMethodMachineCode (multicast);
}

inline
IfaceHdr*
allocateClass (
	RuntimeRef* runtime,
	ClassType* type
	)
{
	return ext::g_extensionLibHost->allocateClass (runtime, type);
}

inline
DataPtr
allocateData (
	RuntimeRef* runtime,
	Type* type
	)
{
	return ext::g_extensionLibHost->allocateData (runtime, type);
}

inline
rt::DataPtrValidator*
createDataPtrValidator (
	RuntimeRef* runtime,	
	rt::Box* box,
	void* rangeBegin,
	size_t rangeLength
	)
{
	return ext::g_extensionLibHost->createDataPtrValidator (runtime, box, rangeBegin, rangeLength);
}

inline
void
primeClass (
	Box* box,
	Box* root,
	ClassType* type,
	void* vtable = NULL // if null then vtable of class type will be used
	)
{
	return ext::g_extensionLibHost->primeClass (box, root, type, vtable);
}

inline
size_t 
strLen (DataPtr ptr)
{
	return ext::g_extensionLibHost->strLen (ptr);
}

inline
DataPtr
strDup (
	const char* p,
	size_t length = -1
	)
{
	return ext::g_extensionLibHost->strDup (p, length);
}

inline
DataPtr
memDup (
	const void* p,
	size_t size
	)
{
	return ext::g_extensionLibHost->memDup (p, size);
}

#else

typedef Runtime       RuntimeRef;
typedef ct::Module    Module;
typedef ct::Function  Function;
typedef ct::Type      Type;
typedef ct::ClassType ClassType;

inline
RuntimeRef* 
getCurrentThreadRuntimeRef ()
{
	return getCurrentThreadRuntime ();
}

inline
void
initializeThread (
	RuntimeRef* runtime,
	ExceptionRecoverySnapshot* ers
	)
{
	runtime->initializeThread (ers);
}

inline
void
uninitializeThread (
	RuntimeRef* runtime,
	ExceptionRecoverySnapshot* ers
	)
{
	runtime->uninitializeThread (ers);
}

inline
void
enterNoCollectRegion (RuntimeRef* runtime)
{
	runtime->m_gcHeap.enterNoCollectRegion ();
}

inline
void
leaveNoCollectRegion (
	RuntimeRef* runtime,
	bool canCollectNow
	)
{
	runtime->m_gcHeap.leaveNoCollectRegion (canCollectNow);
}

inline
void
enterWaitRegion (RuntimeRef* runtime)
{
	runtime->m_gcHeap.enterWaitRegion ();
}

inline
void
leaveWaitRegion (RuntimeRef* runtime)
{
	runtime->m_gcHeap.leaveWaitRegion ();
}
inline
void*
getFunctionMachineCode (Function* function)
{
	return function->getMachineCode ();
}

inline
void*
getMulticastCallMethodMachineCode (Multicast* multicast)
{
	ct::MulticastClassType* type = (ct::MulticastClassType*) multicast->m_box->m_type;
	return type->getMethod (ct::MulticastMethodKind_Call)->getMachineCode ();
}

inline
IfaceHdr*
allocateClass (
	RuntimeRef* runtime,
	ClassType* type
	)
{
	return runtime->m_gcHeap.allocateClass (type);
}

inline
DataPtr
allocateData (
	RuntimeRef* runtime,
	Type* type
	)
{
	return runtime->m_gcHeap.allocateData (type);
}

inline
rt::DataPtrValidator*
createDataPtrValidator (
	RuntimeRef* runtime,	
	rt::Box* box,
	void* rangeBegin,
	size_t rangeLength
	)
{
	return runtime->m_gcHeap.createDataPtrValidator (box, rangeBegin, rangeLength);
}

void
primeClass (
	Box* box,
	Box* root,
	ClassType* type,
	void* vtable = NULL // if null then vtable of class type will be used
	);

size_t 
strLen (DataPtr ptr);

DataPtr
strDup (
	const char* p,
	size_t length = -1
	);

DataPtr
memDup (
	const void* p,
	size_t size
	);

#endif

inline
void
primeClass (
	Box* box,
	ClassType* type,
	void* vtable = NULL // if null then vtable of class type will be used
	)
{
	primeClass (box, box, type, vtable);
}

template <typename T>
void
primeClass (
	Module* module,
	ClassBox <T>* p,
	Box* root
	)
{
	primeClass (p, root, T::getType (module), T::getVTable ());
}

template <typename T>
void
primeClass (
	RuntimeRef* runtime,
	ClassBox <T>* p,
	Box* root
	)
{
	primeClass (p, root, T::getType (runtime), T::getVTable ());
}

template <typename T>
void
primeClass (
	Module* module,
	ClassBox <T>* p
	)
{
	primeClass (p, p, T::getType (module), T::getVTable ());
}

template <typename T>
void
primeClass (
	RuntimeRef* runtime,
	ClassBox <T>* p
	)
{
	primeClass (p, p, T::getType (runtime), T::getVTable ());
}

//.............................................................................

class ScopedNoCollectRegion
{
protected:
	RuntimeRef* m_runtime;
	bool m_canCollectOnLeave;

public:
	ScopedNoCollectRegion (
		RuntimeRef* runtime,
		bool canCollectOnLeave
		)
	{
		init (runtime, canCollectOnLeave);
	}

	ScopedNoCollectRegion (bool canCollectOnLeave)
	{
		RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
		ASSERT (runtime);

		init (runtime, canCollectOnLeave);
	}

	~ScopedNoCollectRegion ()
	{
		leaveNoCollectRegion (m_runtime, m_canCollectOnLeave);
	}

protected:
	void
	init (
		RuntimeRef* runtime,
		bool canCollectOnLeave
		)
	{
		m_runtime = runtime;
		m_canCollectOnLeave = canCollectOnLeave;
		enterNoCollectRegion (runtime);
	}
};

//.............................................................................

#define JNC_BEGIN_CALL_SITE(runtime) \
{ \
	jnc::rt::RuntimeRef* __jncRuntime = (runtime); \
	bool __jncIsNoCollectRegion = false; \
	bool __jncCanCollectAtEnd = false; \
	jnc::rt::ExceptionRecoverySnapshot ___jncErs; \
	JNC_BEGIN_GC_SITE() \
	jnc::rt::initializeThread (__jncRuntime, &___jncErs); \
	AXL_MT_BEGIN_LONG_JMP_TRY () \

#define JNC_BEGIN_CALL_SITE_NO_COLLECT(runtime, canCollectAtEnd) \
	JNC_BEGIN_CALL_SITE (runtime) \
	__jncIsNoCollectRegion = true; \
	__jncCanCollectAtEnd = canCollectAtEnd; \
	jnc::rt::enterNoCollectRegion (__jncRuntime);

#define JNC_CALL_SITE_CATCH() \
	AXL_MT_LONG_JMP_CATCH ()

#define JNC_CALL_SITE_FINALLY() \
	AXL_MT_LONG_JMP_FINALLY ()

#define JNC_END_CALL_SITE_IMPL() \
	AXL_MT_END_LONG_JMP_TRY_EX (&___jncErs.m_result) \
	if (__jncIsNoCollectRegion && ___jncErs.m_result) \
		jnc::rt::leaveNoCollectRegion (__jncRuntime, __jncCanCollectAtEnd); \
	jnc::rt::uninitializeThread (__jncRuntime, &___jncErs); \
	JNC_END_GC_SITE () \

#define JNC_END_CALL_SITE() \
	JNC_END_CALL_SITE_IMPL() \
}

#define JNC_END_CALL_SITE_EX(result) \
	JNC_END_CALL_SITE_IMPL() \
	*(result) = ___jncErs.m_result; \
}

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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
	Function* function,
	RetVal* retVal
	)
{
	void* p = getFunctionMachineCode (function);
	return callFunctionImpl_s (runtime, p, retVal);
}

template <
	typename RetVal,
	typename Arg
	>
bool
callFunction (
	RuntimeRef* runtime,
	Function* function,
	RetVal* retVal,
	Arg arg
	)
{
	void* p = getFunctionMachineCode (function);
	return callFunctionImpl_s (runtime, p, retVal, arg);
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2
	>
bool
callFunction (
	RuntimeRef* runtime,
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2
	)
{
	void* p = getFunctionMachineCode (function);
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
	RuntimeRef* runtime,
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	void* p = getFunctionMachineCode (function);
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
	RuntimeRef* runtime,
	Function* function,
	RetVal* retVal,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	void* p = getFunctionMachineCode (function);
	return callFunctionImpl_s (runtime, p, retVal, arg1, arg2, arg3, arg4);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename RetVal>
RetVal
callFunction (Function* function)
{
	void* p = getFunctionMachineCode (function);
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
	void* p = getFunctionMachineCode (function);
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
	void* p = getFunctionMachineCode (function);
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
	void* p = getFunctionMachineCode (function);
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
	void* p = getFunctionMachineCode (function);
	return callFunctionImpl_u <RetVal> (p, arg1, arg2, arg3, arg4);
}

//.............................................................................

inline
bool
callVoidFunction (
	RuntimeRef* runtime,
	Function* function
	)
{
	int retVal;
	void* p = getFunctionMachineCode (function);
	return callFunctionImpl_s (runtime, p, &retVal);
}

template <typename Arg>
bool
callVoidFunction (
	RuntimeRef* runtime,
	Function* function,
	Arg arg
	)
{
	int retVal;
	void* p = getFunctionMachineCode (function);
	return callFunctionImpl_s (runtime, p, &retVal, arg);
}

template <
	typename Arg1,
	typename Arg2
	>
bool
callVoidFunction (
	RuntimeRef* runtime,
	Function* function,
	Arg1 arg1,
	Arg2 arg2
	)
{
	int retVal;
	void* p = getFunctionMachineCode (function);
	return callFunctionImpl_s (runtime, p, &retVal, arg1, arg2);
}

template <
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callVoidFunction (
	RuntimeRef* runtime,
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	int retVal;
	void* p = getFunctionMachineCode (function);
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
	RuntimeRef* runtime,
	Function* function,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	int retVal;
	void* p = getFunctionMachineCode (function);
	return callFunctionImpl_s (runtime, p, &retVal, arg1, arg2, arg3, arg4);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
callVoidFunction (Function* function)
{
	void* p = getFunctionMachineCode (function);
	callFunctionImpl_u <void> (p);
}

template <typename Arg>
void
callVoidFunction (
	Function* function,
	Arg arg
	)
{
	void* p = getFunctionMachineCode (function);
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
	void* p = getFunctionMachineCode (function);
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
	void* p = getFunctionMachineCode (function);
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
	void* p = getFunctionMachineCode (function);
	callFunctionImpl_u <void> (p, arg1, arg2, arg3, arg4);
}

//.............................................................................

template <typename RetVal>
bool
callFunctionPtr (
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
	FunctionPtr ptr
	)
{
	int retVal;
	return callFunctionImpl_s (runtime, ptr.m_p, &retVal, ptr.m_closure);
}

template <typename Arg>
bool
callVoidFunctionPtr (
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
	RuntimeRef* runtime,
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
createClass (RuntimeRef* runtime)
{
	bool result;
	T* p;
	ClassType* type = T::getType (runtime);

	JNC_BEGIN_CALL_SITE (runtime)
	p = (T*) allocateClass (runtime, type);
	sl::construct (p);
	JNC_END_CALL_SITE_EX (&result)

	return result ? p : NULL;
}

template <
	typename T,
	typename Arg
	>
T*
createClass (
	RuntimeRef* runtime,
	Arg arg
	)
{
	bool result;
	T* p;
	ClassType* type = T::getType (runtime);

	JNC_BEGIN_CALL_SITE (runtime)
	p = (T*) allocateClass (runtime, type);
	sl::construct (p, arg);
	JNC_END_CALL_SITE_EX (&result)

	return result ? p : NULL;
}

template <
	typename T,
	typename Arg1,
	typename Arg2
	>
T*
createClass (
	RuntimeRef* runtime,
	Arg1 arg1,
	Arg2 arg2
	)
{
	bool result;
	T* p;
	ClassType* type = T::getType (runtime);

	JNC_BEGIN_CALL_SITE (runtime)
	p = (T*) allocateClass (runtime, type);
	sl::construct (p, arg1, arg2);
	JNC_END_CALL_SITE_EX (&result)

	return result ? p : NULL;
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
T*
createClass (
	RuntimeRef* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	bool result;
	T* p;
	ClassType* type = T::getType (runtime);

	JNC_BEGIN_CALL_SITE (runtime)
	p = (T*) allocateClass (runtime, type);
	sl::construct (p, arg1, arg2, arg3);
	JNC_END_CALL_SITE_EX (&result)

	return result ? p : NULL;
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
	RuntimeRef* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	bool result;
	T* p;
	ClassType* type = T::getType (runtime);

	JNC_BEGIN_CALL_SITE (runtime)
	p = (T*) allocateClass (runtime, type);
	sl::construct (p, arg1, arg2, arg3, arg4);
	JNC_END_CALL_SITE_EX (&result)

	return result ? p : NULL;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
T*
createClass ()
{
	RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
	ASSERT (runtime);

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
createClass (Arg arg)
{
	RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
	ASSERT (runtime);

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
	Arg1 arg1,
	Arg2 arg2
	)
{
	RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
	ASSERT (runtime);

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
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
	ASSERT (runtime);

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
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
	ASSERT (runtime);

	ClassType* type = T::getType (runtime);
	T* p = (T*) allocateClass (runtime, type);
	sl::construct (p, arg1, arg2, arg3, arg4);

	return p;
}

//.............................................................................

template <typename T>
DataPtr
createData (RuntimeRef* runtime)
{
	bool result;
	DataPtr ptr;
	Type* type = T::getType (runtime);

	JNC_BEGIN_CALL_SITE (runtime)
	ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p);
	JNC_END_CALL_SITE_EX (&result)

	return result ? ptr : g_nullPtr;
}

template <
	typename T,
	typename Arg
	>
DataPtr
createData (
	RuntimeRef* runtime,
	Arg arg
	)
{
	bool result;
	DataPtr ptr;
	Type* type = T::getType (runtime);

	JNC_BEGIN_CALL_SITE (runtime)
	ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p, arg);
	JNC_END_CALL_SITE_EX (&result)

	return result ? ptr : g_nullPtr;
}

template <
	typename T,
	typename Arg1,
	typename Arg2
	>
DataPtr
createData (
	RuntimeRef* runtime,
	Arg1 arg1,
	Arg2 arg2
	)
{
	bool result;
	DataPtr ptr;
	Type* type = T::getType (runtime);

	JNC_BEGIN_CALL_SITE (runtime)
	ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p, arg1, arg2);
	JNC_END_CALL_SITE_EX (&result)

	return result ? ptr : g_nullPtr;
}

template <
	typename T,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
DataPtr
createData (
	RuntimeRef* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	bool result;
	DataPtr ptr;
	Type* type = T::getType (runtime);

	JNC_BEGIN_CALL_SITE (runtime)
	ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p, arg1, arg2, arg3);
	JNC_END_CALL_SITE_EX (&result)

	return result ? ptr : g_nullPtr;
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
	RuntimeRef* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	bool result;
	DataPtr ptr;
	Type* type = T::getType (runtime);

	JNC_BEGIN_CALL_SITE (runtime)
	ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p, arg1, arg2, arg3, arg4);
	JNC_END_CALL_SITE_EX (&result)

	return result ? ptr : g_nullPtr;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
DataPtr
createData ()
{
	RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
	ASSERT (runtime);

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
createData (Arg arg)
{
	RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
	ASSERT (runtime);

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
	Arg1 arg1,
	Arg2 arg2
	)
{
	RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
	ASSERT (runtime);

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
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
	ASSERT (runtime);

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
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	RuntimeRef* runtime = getCurrentThreadRuntimeRef ();
	ASSERT (runtime);

	Type* type = T::getType (runtime);
	DataPtr ptr = allocateData (runtime, type);
	sl::construct ((T*) ptr.m_p, arg1, arg2, arg3, arg4);

	return ptr;
}

//.............................................................................

} // namespace rt
} // namespace jnc
