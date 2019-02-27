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

#include "pch.h"
#include "jnc_Function.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getFunctionKindString(jnc_FunctionKind functionKind)
{
	static const char* stringTable[jnc_FunctionKind__Count] =
	{
		"undefined-function-kind",  // jnc_FunctionKind_Undefined,
		"normal-function",          // jnc_FunctionKind_Normal,
		"get",                      // jnc_FunctionKind_Getter,
		"set",                      // jnc_FunctionKind_Setter,
		"bindingof",                // jnc_FunctionKind_Binder,
		"preconstruct",             // jnc_FunctionKind_PreConstructor,
		"construct",                // jnc_FunctionKind_Constructor,
		"destruct",                 // jnc_FunctionKind_Destructor,
		"static construct",         // jnc_FunctionKind_StaticConstructor,
		"static destruct",          // jnc_FunctionKind_StaticDestructor,
		"call-operator",            // jnc_FunctionKind_CallOperator,
		"cast-operator",            // jnc_FunctionKind_CastOperator,
		"unary-operator",           // jnc_FunctionKind_UnaryOperator,
		"binary-operator",          // jnc_FunctionKind_BinaryOperator,
		"operator_vararg",          // jnc_FunctionKind_OperatorVararg,
		"operator_cdecl_vararg",    // jnc_FunctionKind_OperatorCdeclVararg,
		"internal",                 // jnc_FunctionKind_Internal,
		"thunk",                    // jnc_FunctionKind_Thunk,
		"schedule-launcher",        // jnc_FunctionKind_ScheduleLauncher,
		"async",                    // jnc_FunctionKind_Async,
	};

	return (size_t)functionKind < jnc_FunctionKind__Count ?
		stringTable[functionKind] :
		stringTable[jnc_FunctionKind_Undefined];
}

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_getFunctionKindFlags(jnc_FunctionKind functionKind)
{
	static int flagTable[jnc_FunctionKind__Count] =
	{
		0,                                  // jnc_FunctionKind_Undefined,
		0,                                  // jnc_FunctionKind_Normal,
		jnc_FunctionKindFlag_NoOverloads,   // jnc_FunctionKind_Getter,
		0,                                  // jnc_FunctionKind_Setter,
		jnc_FunctionKindFlag_NoOverloads |  // jnc_FunctionKind_Binder,
		jnc_FunctionKindFlag_NoArgs,
		jnc_FunctionKindFlag_NoStorage   |  // jnc_FunctionKind_PreConstructor,
		jnc_FunctionKindFlag_NoOverloads |
		jnc_FunctionKindFlag_NoArgs,
		jnc_FunctionKindFlag_NoStorage,     // jnc_FunctionKind_Constructor,
		jnc_FunctionKindFlag_NoStorage   |  // jnc_FunctionKind_Destructor,
		jnc_FunctionKindFlag_NoOverloads |
		jnc_FunctionKindFlag_NoArgs,
		jnc_FunctionKindFlag_NoStorage   |  // jnc_FunctionKind_StaticConstructor,
		jnc_FunctionKindFlag_NoOverloads |
		jnc_FunctionKindFlag_NoArgs,
		jnc_FunctionKindFlag_NoStorage   |  // jnc_FunctionKind_StaticDestructor,
		jnc_FunctionKindFlag_NoOverloads |
		jnc_FunctionKindFlag_NoArgs,
		0,                                  // jnc_FunctionKind_CallOperator,
		jnc_FunctionKindFlag_NoOverloads |  // jnc_FunctionKind_CastOperator,
		jnc_FunctionKindFlag_NoArgs,
		jnc_FunctionKindFlag_NoOverloads |  // jnc_FunctionKind_UnaryOperator,
		jnc_FunctionKindFlag_NoArgs,
		0,                                  // jnc_FunctionKind_BinaryOperator,
		jnc_FunctionKindFlag_NoOverloads |  // jnc_FunctionKind_OperatorVararg,
		jnc_FunctionKindFlag_NoArgs,
		jnc_FunctionKindFlag_NoOverloads |  // jnc_FunctionKind_OperatorCdeclVararg,
		jnc_FunctionKindFlag_NoArgs,
		0,                                  // jnc_FunctionKind_Internal,
		0,                                  // jnc_FunctionKind_Thunk,
		0,                                  // jnc_FunctionKind_ScheduleLauncher,
		0,                                  // jnc_FunctionKind_Async,
	};

	return (size_t)functionKind < jnc_FunctionKind__Count ? flagTable[functionKind] : 0;
}

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionKind
jnc_Function_getFunctionKind(jnc_Function* function)
{
	return jnc_g_dynamicExtensionLibHost->m_functionFuncTable->m_getFunctionKindFunc(function);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Function_isMember(jnc_Function* function)
{
	return jnc_g_dynamicExtensionLibHost->m_functionFuncTable->m_isMemberFunc(function);
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Function_isOverloaded(jnc_Function* function)
{
	return jnc_g_dynamicExtensionLibHost->m_functionFuncTable->m_isOverloadedFunc(function);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Function_getOverloadCount(jnc_Function* function)
{
	return jnc_g_dynamicExtensionLibHost->m_functionFuncTable->m_getOverloadCountFunc(function);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_Function_getOverload(
	jnc_Function* function,
	size_t overloadIdx
	)
{
	return jnc_g_dynamicExtensionLibHost->m_functionFuncTable->m_getOverloadFunc(function, overloadIdx);
}

JNC_EXTERN_C
JNC_EXPORT_O
void*
jnc_Function_getMachineCode(jnc_Function* function)
{
	return jnc_g_dynamicExtensionLibHost->m_functionFuncTable->m_getMachineCodeFunc(function);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_FunctionKind
jnc_Function_getFunctionKind(jnc_Function* function)
{
	return function->getFunctionKind();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Function_isMember(jnc_Function* function)
{
	return function->isMember();
}

JNC_EXTERN_C
JNC_EXPORT_O
bool_t
jnc_Function_isOverloaded(jnc_Function* function)
{
	return function->isOverloaded();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Function_getOverloadCount(jnc_Function* function)
{
	return function->getOverloadCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_Function_getOverload(
	jnc_Function* function,
	size_t index
	)
{
	jnc_Function* overload = function->getOverload(index);
	if (!overload)
	{
		err::setFormatStringError("'%s' has no overload #%d", function->getQualifiedName ().sz (), index);
		return NULL;
	}

	return overload;
}

JNC_EXTERN_C
JNC_EXPORT_O
void*
jnc_Function_getMachineCode(jnc_Function* function)
{
	return function->getMachineCode();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
