// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Function.h"
#include "jnc_Runtime.h"
#include "jnc_MulticastClassType.h"

namespace jnc {

//.............................................................................

template <typename RetVal>
bool
callFunctionImpl (
	Runtime* runtime,
	void* p,
	RetVal* retVal
	)
{
	typedef
	RetVal
	TargetFunc ();

	bool result;

	JNC_BEGIN (runtime)
	*retVal = ((TargetFunc*) p) ();
	JNC_END_EX (&result)

	return result;
}

template <
	typename RetVal,
	typename Arg
	>
bool
callFunctionImpl (
	Runtime* runtime,
	void* p,
	RetVal* retVal,
	Arg arg
	)
{
	typedef
	RetVal
	TargetFunc (Arg);

	bool result;

	JNC_BEGIN (runtime)
	*retVal = ((TargetFunc*) p) (arg);
	JNC_END_EX (&result)

	return result;
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2
	>
bool
callFunctionImpl (
	Runtime* runtime,
	void* p,
	RetVal* retVal,
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

	bool result;

	JNC_BEGIN (runtime)
	*retVal = ((TargetFunc*) p)  (arg1, arg2);
	JNC_END_EX (&result)

	return result;
}

template <
	typename RetVal,
	typename Arg1,
	typename Arg2,
	typename Arg3
	>
bool
callFunctionImpl (
	Runtime* runtime,
	void* p,
	RetVal* retVal,
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

	bool result;

	JNC_BEGIN (runtime)
	*retVal = ((TargetFunc*) p) (arg1, arg2, arg3);
	JNC_END_EX (&result)

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
callFunctionImpl (
	Runtime* runtime,
	void* p,
	RetVal* retVal,
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

	bool result;

	JNC_BEGIN (runtime)
	*retVal = ((TargetFunc*) p) (arg1, arg2, arg3, arg4);
	JNC_END_EX (&result)

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
callFunctionImpl (
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
	typedef
	RetVal
	TargetFunc (
		Arg1,
		Arg2,
		Arg3,
		Arg4,
		Arg5
		);

	bool result;

	JNC_BEGIN (runtime)
	*retVal = ((TargetFunc*) p) (arg1, arg2, arg3, arg4, arg5);
	JNC_END_EX (&result)

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
	return callFunctionImpl (runtime, function->getMachineCode (), retVal);
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
	return callFunctionImpl (runtime, function->getMachineCode (), retVal, arg);
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
	return callFunctionImpl (runtime, function->getMachineCode (), retVal, arg1, arg2);
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
	return callFunctionImpl (runtime, function->getMachineCode (), retVal, arg1, arg2, arg3);
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
	return callFunctionImpl (runtime, function->getMachineCode (), retVal, arg1, arg2, arg3, arg4);
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
	return callFunctionImpl (runtime, function->getMachineCode (), &retVal);
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
	return callFunctionImpl (runtime, function->getMachineCode (), &retVal, arg);
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
	return callFunctionImpl (runtime, function->getMachineCode (), &retVal, arg1, arg2);
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
	return callFunctionImpl (runtime, function->getMachineCode (), &retVal, arg1, arg2, arg3);
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
	return callFunctionImpl (runtime, function->getMachineCode (), &retVal, arg1, arg2, arg3, arg4);
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
	return callFunctionImpl (runtime, function->getMachineCode (), retVal, ptr.m_closure);
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
	return callFunctionImpl (runtime, function->getMachineCode (), retVal, ptr.m_closure, arg);
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
	return callFunctionImpl (runtime, ptr.m_p,  retVal, ptr.m_closure, arg1, arg2);
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
	return callFunctionImpl (runtime, ptr.m_p, retVal, ptr.m_closure, arg1, arg2, arg3);
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
	return callFunctionImpl (runtime, ptr.m_p, retVal, ptr.m_closure, arg1, arg2, arg3, arg4);
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
	return callFunctionImpl (runtime, ptr.m_p, &retVal, ptr.m_closure);
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
	return callFunctionImpl (runtime, ptr.m_p, &retVal, ptr.m_closure, arg);
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
	return callFunctionImpl (runtime, ptr.m_p, &retVal, ptr.m_closure, arg1, arg2);
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
	return callFunctionImpl (runtime, ptr.m_p, &retVal, ptr.m_closure, arg1, arg2, arg3);
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
	return callFunctionImpl (runtime, ptr.m_p, &retVal, ptr.m_closure, arg1, arg2, arg3, arg4);
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

//.............................................................................

template <typename T>
T*
createClass (Runtime* runtime)
{
	bool result;
	T* p;
	ClassType* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN (runtime)
	p = runtime->m_gcHeap.allocateClass (type);
	jnc::construct (p);
	JNC_END_EX (&result)

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

	JNC_BEGIN (runtime)
	p = (T*) runtime->m_gcHeap.allocateClass (type);
	jnc::construct (p, arg);
	JNC_END_EX (&result)

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

	JNC_BEGIN (runtime)
	p = (T*) runtime->m_gcHeap.allocateClass (type);
	jnc::construct (p, arg1, arg2);
	JNC_END_EX (&result)

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

	JNC_BEGIN (runtime)
	p = (T*) runtime->m_gcHeap.allocateClass (type);
	jnc::construct (p, arg1, arg2, arg3);
	JNC_END_EX (&result)

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

	JNC_BEGIN (runtime)
	p = (T*) runtime->m_gcHeap.allocateClass (type);
	jnc::construct (p, arg1, arg2, arg3, arg4);
	JNC_END_EX (&result)

	return result ? p : NULL;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
DataPtr
createData (Runtime* runtime)
{
	bool result;
	DataPtr ptr;
	Type* type = T::getApiType (runtime->getModule ());

	JNC_BEGIN (runtime)
	ptr = runtime->m_gcHeap.allocateData (type);
	jnc::construct ((T*) ptr.m_p);
	JNC_END_EX (&result)

	return result ? ptr : g_nullPtr;
}

//.............................................................................

} // namespace jnc {
