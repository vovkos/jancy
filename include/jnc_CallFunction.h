// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Function.h"
#include "jnc_Runtime.h"
#include "jnc_Api.h"
#include "jnc_MulticastClassType.h"

namespace jnc {

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
	return callFunctionImpl_s (runtime, function->getMachineCode (), retVal);
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
	return callFunctionImpl_s (runtime, function->getMachineCode (), retVal, arg);
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
	return callFunctionImpl_s (runtime, function->getMachineCode (), retVal, arg1, arg2);
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
	return callFunctionImpl_s (runtime, function->getMachineCode (), retVal, arg1, arg2, arg3);
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
	return callFunctionImpl_s (runtime, function->getMachineCode (), retVal, arg1, arg2, arg3, arg4);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename RetVal>
RetVal
callFunction (Function* function)
{
	return callFunctionImpl_u <RetVal> (runtime, function->getMachineCode ());
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
	return callFunctionImpl_u <RetVal> (runtime, function->getMachineCode (), sarg);
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
	return callFunctionImpl_u <RetVal> (runtime, function->getMachineCode (), arg1, arg2);
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
	return callFunctionImpl_u <RetVal> (runtime, function->getMachineCode (), arg1, arg2, arg3);
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
	return callFunctionImpl_u <RetVal> (function->getMachineCode (), arg1, arg2, arg3, arg4);
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
	return callFunctionImpl_s (runtime, function->getMachineCode (), &retVal);
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
	return callFunctionImpl_s (runtime, function->getMachineCode (), &retVal, arg);
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
	return callFunctionImpl_s (runtime, function->getMachineCode (), &retVal, arg1, arg2);
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
	return callFunctionImpl_s (runtime, function->getMachineCode (), &retVal, arg1, arg2, arg3);
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
	return callFunctionImpl_s (runtime, function->getMachineCode (), &retVal, arg1, arg2, arg3, arg4);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
callVoidFunction (Function* function)
{
	callFunctionImpl_u <void> (function->getMachineCode ());
}

template <typename Arg>
void
callVoidFunction (
	Function* function,
	Arg arg
	)
{
	callFunctionImpl_u <void> (function->getMachineCode (), arg);
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
	callFunctionImpl_u <void> (function->getMachineCode (), arg1, arg2);
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
	callFunctionImpl_u <void> (function->getMachineCode (), arg1, arg2, arg3);
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
	callFunctionImpl_u <void> (function->getMachineCode (), arg1, arg2, arg3, arg4);
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
	return callFunctionImpl_u <RetVal> (runtime, ptr.m_p, ptr.m_closure);
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
	return callFunctionImpl_u <RetVal> (runtime, ptr.m_p, ptr.m_closure, arg);
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
	return callFunctionImpl_u <RetVal> (runtime, ptr.m_p, ptr.m_closure, arg1, arg2);
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
	return callFunctionImpl_u <RetVal> (runtime, ptr.m_p, ptr.m_closure, arg1, arg2, arg3);
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
	return callFunctionImpl_u <RetVal> (runtime, ptr.m_p, ptr.m_closure, arg1, arg2, arg3, arg4);
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
	Function* method = ((MulticastClassType*) multicast->m_box->m_type)->getMethod (MulticastMethodKind_Call);
	return callVoidFunction (runtime, method, multicast);
}

template <typename Arg>
bool
callMulticast (
	Runtime* runtime,
	Multicast* multicast,
	Arg arg
	)
{
	Function* method = ((MulticastClassType*) multicast->m_box->m_type)->getMethod (MulticastMethodKind_Call);
	return callVoidFunction (runtime, method, multicast, arg);
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
	Function* method = ((MulticastClassType*) multicast->m_box->m_type)->getMethod (MulticastMethodKind_Call);
	return callVoidFunction (runtime, method, multicast, arg1, arg2);
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
	Function* method = ((MulticastClassType*) multicast->m_box->m_type)->getMethod (MulticastMethodKind_Call);
	return callVoidFunction (runtime, method, multicast, arg1, arg2, arg3);
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
	Function* method = ((MulticastClassType*) multicast->m_box->m_type)->getMethod (MulticastMethodKind_Call);
	return callVoidFunction (runtime, method, multicast, arg1, arg2, arg3, arg4);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
callMulticast (Multicast* multicast)
{
	Function* method = ((MulticastClassType*) multicast->m_box->m_type)->getMethod (MulticastMethodKind_Call);
	callVoidFunction (method, multicast);
}

template <typename Arg>
void
callMulticast (
	Multicast* multicast,
	Arg arg
	)
{
	Function* method = ((MulticastClassType*) multicast->m_box->m_type)->getMethod (MulticastMethodKind_Call);
	callVoidFunction (method, multicast, arg);
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
	Function* method = ((MulticastClassType*) multicast->m_box->m_type)->getMethod (MulticastMethodKind_Call);
	callVoidFunction (method, multicast, arg1, arg2);
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
	Function* method = ((MulticastClassType*) multicast->m_box->m_type)->getMethod (MulticastMethodKind_Call);
	callVoidFunction (method, multicast, arg1, arg2, arg3);
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
	Function* method = ((MulticastClassType*) multicast->m_box->m_type)->getMethod (MulticastMethodKind_Call);
	callVoidFunction (method, multicast, arg1, arg2, arg3, arg4);
}

//.............................................................................

template <typename T>
T*
createClass (Runtime* runtime)
{
	bool result;
	T* p;
	ClassType* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN_CALL_SITE (runtime)
	p = runtime->m_gcHeap.allocateClass (type);
	rtl::construct (p);
	JNC_END_CALL_SITE_EX (&result)

	return result ? p : NULL;
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
	bool result;
	T* p;
	ClassType* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN_CALL_SITE (runtime)
	p = (T*) runtime->m_gcHeap.allocateClass (type);
	rtl::construct (p, arg);
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
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2
	)
{
	bool result;
	T* p;
	ClassType* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN_CALL_SITE (runtime)
	p = (T*) runtime->m_gcHeap.allocateClass (type);
	rtl::construct (p, arg1, arg2);
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
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	bool result;
	T* p;
	ClassType* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN_CALL_SITE (runtime)
	p = (T*) runtime->m_gcHeap.allocateClass (type);
	rtl::construct (p, arg1, arg2, arg3);
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
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	bool result;
	T* p;
	ClassType* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN_CALL_SITE (runtime)
	p = (T*) runtime->m_gcHeap.allocateClass (type);
	rtl::construct (p, arg1, arg2, arg3, arg4);
	JNC_END_CALL_SITE_EX (&result)

	return result ? p : NULL;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
T*
createClass ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ClassType* type = T::getApiType (runtime->getModule ());
	T* p = runtime->m_gcHeap.allocateClass (type);
	rtl::construct (p);
	
	return p;
}

template <
	typename T,
	typename Arg
	>
T*
createClass (Arg arg)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ClassType* type = T::getApiType (runtime->getModule ());
	T* p = (T*) runtime->m_gcHeap.allocateClass (type);
	rtl::construct (p, arg);

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
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ClassType* type = T::getApiType (runtime->getModule ());
	T* p = (T*) runtime->m_gcHeap.allocateClass (type);
	rtl::construct (p, arg1, arg2);

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
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ClassType* type = T::getApiType (runtime->getModule ());
	T* p = (T*) runtime->m_gcHeap.allocateClass (type);
	rtl::construct (p, arg1, arg2, arg3);

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
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ClassType* type = T::getApiType (runtime->getModule ());
	T* p = (T*) runtime->m_gcHeap.allocateClass (type);
	rtl::construct (p, arg1, arg2, arg3, arg4);

	return p;
}

//.............................................................................

template <typename T>
DataPtr
createData (Runtime* runtime)
{
	bool result;
	DataPtr ptr;
	Type* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN_CALL_SITE (runtime)
	ptr = runtime->m_gcHeap.allocateData (type);
	rtl::construct ((T*) ptr.m_p);
	JNC_END_CALL_SITE_EX (&result)

	return result ? ptr : g_nullPtr;
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
	bool result;
	DataPtr ptr;
	Type* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN_CALL_SITE (runtime)
	ptr = runtime->m_gcHeap.allocateData (type);
	rtl::construct ((T*) ptr.m_p, arg);
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
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2
	)
{
	bool result;
	DataPtr ptr;
	Type* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN_CALL_SITE (runtime)
	ptr = runtime->m_gcHeap.allocateData (type);
	rtl::construct ((T*) ptr.m_p, arg1, arg2);
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
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3
	)
{
	bool result;
	DataPtr ptr;
	Type* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN_CALL_SITE (runtime)
	ptr = runtime->m_gcHeap.allocateData (type);
	rtl::construct ((T*) ptr.m_p, arg1, arg2, arg3);
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
	Runtime* runtime,
	Arg1 arg1,
	Arg2 arg2,
	Arg3 arg3,
	Arg4 arg4
	)
{
	bool result;
	DataPtr ptr;
	Type* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN_CALL_SITE (runtime)
	ptr = runtime->m_gcHeap.allocateData (type);
	rtl::construct ((T*) ptr.m_p, arg1, arg2, arg3, arg4);
	JNC_END_CALL_SITE_EX (&result)

	return result ? ptr : g_nullPtr;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
DataPtr
createData ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	Type* type = T::getApiType (runtime->getModule ());
	DataPtr ptr = runtime->m_gcHeap.allocateData (type);
	rtl::construct ((T*) ptr.m_p);

	return ptr;
}

template <
	typename T,
	typename Arg
	>
DataPtr
createData (Arg arg)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	Type* type = T::getApiType (runtime->getModule ());
	DataPtr ptr = runtime->m_gcHeap.allocateData (type);
	rtl::construct ((T*) ptr.m_p, arg);

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
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	Type* type = T::getApiType (runtime->getModule ());
	DataPtr ptr = runtime->m_gcHeap.allocateData (type);
	rtl::construct ((T*) ptr.m_p, arg1, arg2);

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
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	Type* type = T::getApiType (runtime->getModule ());
	DataPtr ptr = runtime->m_gcHeap.allocateData (type);
	rtl::construct ((T*) ptr.m_p, arg1, arg2, arg3);

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
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	Type* type = T::getApiType (runtime->getModule ());
	DataPtr ptr = runtime->m_gcHeap.allocateData (type);
	rtl::construct ((T*) ptr.m_p, arg1, arg2, arg3, arg4);

	return ptr;
}

//.............................................................................

} // namespace jnc {
