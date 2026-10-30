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

#define _JNC_CALLSITE_H

#include "jnc_Runtime.h"
#include "jnc_Function.h"
#include "jnc_ClassType.h"

/**

\defgroup call-site Call Site
	\ingroup runtime-subsystem

\addtogroup call-site
@{

*/

//..............................................................................

#if (_JNC_OS_POSIX)
#	define JNC_INIT_EXCEPTION_INFO() \
		__jncSjljFrame.m_signalInfo.m_signal = 0;
#	define JNC_SAVE_EXCEPTION_INFO() \
		if (__jncSjljBranch) \
			jnc_saveSignalInfo(&__jncSjljFrame);
#else // on windows, we save exception information right inside the handler
#	define JNC_INIT_EXCEPTION_INFO()
#	define JNC_SAVE_EXCEPTION_INFO()
#endif

#define JNC_BEGIN_CALL_SITE_IMPL(runtime) \
	jnc_Runtime* __jncRuntime = (runtime); \
	jnc_CallSite __jncCallSite; \
	jnc_SjljFrame __jncSjljFrame; \
	jnc_SjljFrame* __jncSjljPrevFrame; \
	int __jncSjljBranch; \
	int __jncIsSjljFrameRestored = 0; \
	JNC_ASSERT(runtime); \
	JNC_INIT_EXCEPTION_INFO(); \
	jnc_Runtime_initializeCallSite(__jncRuntime, &__jncCallSite); \
	__jncSjljPrevFrame = jnc_Runtime_setSjljFrame(__jncRuntime, &__jncSjljFrame); \
	__jncSjljBranch = jnc_setJmp(__jncSjljFrame.m_jmpBuf); \
	if (!__jncSjljBranch) {

#define JNC_RESTORE_SJLJ_FRAME() \
	if (!__jncIsSjljFrameRestored) { \
		jnc_SjljFrame* prev; \
		JNC_SAVE_EXCEPTION_INFO(); \
		prev = jnc_Runtime_setSjljFrame(__jncRuntime, __jncSjljPrevFrame); \
		JNC_ASSERT(prev == &__jncSjljFrame); \
		__jncIsSjljFrameRestored = 1; \
	}

#define JNC_CALL_SITE_CATCH() \
	} else { \
		JNC_ASSERT(!__jncIsSjljFrameRestored); \
		JNC_RESTORE_SJLJ_FRAME();

#define JNC_CALL_SITE_FINALLY() \
	} { \
		JNC_RESTORE_SJLJ_FRAME();

#define JNC_END_CALL_SITE_IMPL() \
	} \
	JNC_RESTORE_SJLJ_FRAME(); \
	__jncCallSite.m_result = __jncSjljBranch == 0; \
	jnc_Runtime_uninitializeCallSite(__jncRuntime, &__jncCallSite);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_CALL_SITE(runtime) \
	{ \
	JNC_BEGIN_CALL_SITE_IMPL(runtime)

#define JNC_END_CALL_SITE() \
	JNC_END_CALL_SITE_IMPL() \
	}

#define JNC_END_CALL_SITE_EX(result) \
	JNC_END_CALL_SITE_IMPL() \
	*(result) = __jncCallSite.m_result != 0; \
	}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

template <
	typename RetVal,
	typename... Args
>
RetVal
callFunctionImpl_u(
	void* p,
	Args... args
) {
	typedef
	RetVal
	TargetFunc(Args...);

	return ((TargetFunc*)p) (args...);
}

template <
	typename RetVal,
	typename... Args
>
bool
callFunctionImpl_s(
	Runtime* runtime,
	void* p,
	RetVal* retVal,
	Args... args
) {
	bool result;

	JNC_BEGIN_CALL_SITE(runtime)
		*retVal = callFunctionImpl_u<RetVal>(p, args...);
	JNC_END_CALL_SITE_EX(&result)

	return result;
}

// typed target -- arguments are checked against the target signature

template <
	typename RetVal,
	typename... TargetArgs,
	typename... Args
>
bool
callFunctionImpl_s(
	Runtime* runtime,
	RetVal (*p)(TargetArgs...),
	RetVal* retVal,
	Args... args
) {
	bool result;

	JNC_BEGIN_CALL_SITE(runtime)
		*retVal = p(args...);
	JNC_END_CALL_SITE_EX(&result)

	return result;
}

//..............................................................................

template <
	typename RetVal,
	typename... Args
>
bool
callFunction(
	Runtime* runtime,
	Function* function,
	RetVal* retVal,
	Args... args
) {
	void* p = function->getMachineCode();
	return callFunctionImpl_s(runtime, p, retVal, args...);
}

template <
	typename RetVal,
	typename... Args
>
RetVal
callFunction(
	Function* function,
	Args... args
) {
	void* p = function->getMachineCode();
	return callFunctionImpl_u<RetVal>(p, args...);
}

//..............................................................................

template <typename... Args>
bool
callVoidFunction(
	Runtime* runtime,
	Function* function,
	Args... args
) {
	int retVal;
	void* p = function->getMachineCode();
	return callFunctionImpl_s(runtime, p, &retVal, args...);
}

template <typename... Args>
void
callVoidFunction(
	Function* function,
	Args... args
) {
	void* p = function->getMachineCode();
	callFunctionImpl_u<void>(p, args...);
}

//..............................................................................

template <
	typename RetVal,
	typename... Args
>
bool
callFunctionPtr(
	Runtime* runtime,
	FunctionPtr ptr,
	RetVal* retVal,
	Args... args
) {
	return callFunctionImpl_s(runtime, ptr.m_p, retVal, ptr.m_closure, args...);
}

template <
	typename RetVal,
	typename... Args
>
RetVal
callFunctionPtr(
	FunctionPtr ptr,
	Args... args
) {
	return callFunctionImpl_u<RetVal>(ptr.m_p, ptr.m_closure, args...);
}

//..............................................................................

template <typename... Args>
bool
callVoidFunctionPtr(
	Runtime* runtime,
	FunctionPtr ptr,
	Args... args
) {
	int retVal;
	return callFunctionImpl_s(runtime, ptr.m_p, &retVal, ptr.m_closure, args...);
}

template <typename... Args>
void
callVoidFunctionPtr(
	FunctionPtr ptr,
	Args... args
) {
	callFunctionImpl_u<void>(ptr.m_p, ptr.m_closure, args...);
}

//..............................................................................

template <typename... Args>
bool
callMulticast(
	Runtime* runtime,
	Multicast* multicast,
	Args... args
) {
	void* p = getMulticastCallMethodMachineCode(multicast);
	FunctionPtr ptr = { p, &multicast->m_ifaceHdr };
	return callVoidFunctionPtr(runtime, ptr, args...);
}

template <typename... Args>
void
callMulticast(
	Multicast* multicast,
	Args... args
) {
	void* p = getMulticastCallMethodMachineCode(multicast);
	FunctionPtr ptr = { p, &multicast->m_ifaceHdr };
	callVoidFunctionPtr(ptr, args...);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
