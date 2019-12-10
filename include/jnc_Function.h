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

#define _JNC_FUNCTION_H

#include "jnc_FunctionType.h"
#include "jnc_ModuleItem.h"

/**

\defgroup function Function
	\ingroup module-subsystem
	\import{jnc_Function.h}

\addtogroup function
@{

\struct jnc_Function
	\verbatim

	Opaque structure used as a handle to Jancy function.

	Use functions from the `Function` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_FunctionKind
{
	jnc_FunctionKind_Undefined = 0,
	jnc_FunctionKind_Normal,
	jnc_FunctionKind_Getter,
	jnc_FunctionKind_Setter,
	jnc_FunctionKind_Binder,
	jnc_FunctionKind_StaticConstructor,
	jnc_FunctionKind_Constructor,
	jnc_FunctionKind_Destructor,
	jnc_FunctionKind_CallOperator,
	jnc_FunctionKind_CastOperator,
	jnc_FunctionKind_UnaryOperator,
	jnc_FunctionKind_BinaryOperator,
	jnc_FunctionKind_OperatorVararg,
	jnc_FunctionKind_OperatorCdeclVararg,
	jnc_FunctionKind_Internal,
	jnc_FunctionKind_Thunk,
	jnc_FunctionKind_SchedLauncher,
	jnc_FunctionKind_AsyncSchedLauncher,
	jnc_FunctionKind_AsyncSequencer,
	jnc_FunctionKind__Count
};

typedef enum jnc_FunctionKind jnc_FunctionKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getFunctionKindString(jnc_FunctionKind functionKind);

//..............................................................................

enum jnc_FunctionKindFlag
{
	jnc_FunctionKindFlag_NoStorage   = 0x01,
	jnc_FunctionKindFlag_NoOverloads = 0x02,
	jnc_FunctionKindFlag_NoArgs      = 0x04,
};

typedef enum jnc_FunctionKindFlag jnc_FunctionKindFlag;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
uint_t
jnc_getFunctionKindFlags(jnc_FunctionKind functionKind);

//..............................................................................

enum jnc_FunctionOverloadFlag
{
	jnc_FunctionOverloadFlag_HasMembers = 0x010000,
};

typedef enum jnc_FunctionOverloadFlag jnc_FunctionOverloadFlag;

//..............................................................................

JNC_EXTERN_C
jnc_FunctionKind
jnc_Function_getFunctionKind(jnc_Function* function);

JNC_INLINE
jnc_FunctionType*
jnc_Function_getType(jnc_Function* function)
{
	return (jnc_FunctionType*)jnc_ModuleItem_getType((jnc_ModuleItem*)function);
}

JNC_EXTERN_C
bool_t
jnc_Function_isMember(jnc_Function* function);

JNC_EXTERN_C
void*
jnc_Function_getMachineCode(jnc_Function* function);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Function: jnc_ModuleItem
{
	jnc_FunctionKind
	getFunctionKind()
	{
		return jnc_Function_getFunctionKind(this);
	}

	jnc_FunctionType*
	getType()
	{
		return jnc_Function_getType(this);
	}

	bool
	isMember()
	{
		return jnc_Function_isMember(this) != 0;
	}

	void*
	getMachineCode()
	{
		return jnc_Function_getMachineCode(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
jnc_FunctionKind
jnc_FunctionOverload_getFunctionKind(jnc_FunctionOverload* function);

JNC_EXTERN_C
size_t
jnc_FunctionOverload_getOverloadCount(jnc_FunctionOverload* function);

JNC_EXTERN_C
jnc_Function*
jnc_FunctionOverload_getOverload(
	jnc_FunctionOverload* function,
	size_t index
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_FunctionOverload: jnc_ModuleItem
{
	jnc_FunctionKind
	getFunctionKind()
	{
		return jnc_FunctionOverload_getFunctionKind(this);
	}

	size_t
	getOverloadCount()
	{
		return jnc_FunctionOverload_getOverloadCount(this);
	}

	jnc_Function*
	getOverload(size_t index)
	{
		return jnc_FunctionOverload_getOverload(this, index);
	}
};

#endif // _JNC_CORE

//..............................................................................

union jnc_OverloadableFunction
{
	jnc_ModuleItem* m_item;
	jnc_Function* m_function;
	jnc_FunctionOverload* m_functionOverload;
};

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_FunctionKind FunctionKind;

const FunctionKind
	FunctionKind_Undefined           = jnc_FunctionKind_Undefined,
	FunctionKind_Normal              = jnc_FunctionKind_Normal,
	FunctionKind_Getter              = jnc_FunctionKind_Getter,
	FunctionKind_Setter              = jnc_FunctionKind_Setter,
	FunctionKind_Binder              = jnc_FunctionKind_Binder,
	FunctionKind_StaticConstructor   = jnc_FunctionKind_StaticConstructor,
	FunctionKind_Constructor         = jnc_FunctionKind_Constructor,
	FunctionKind_Destructor          = jnc_FunctionKind_Destructor,
	FunctionKind_CallOperator        = jnc_FunctionKind_CallOperator,
	FunctionKind_CastOperator        = jnc_FunctionKind_CastOperator,
	FunctionKind_UnaryOperator       = jnc_FunctionKind_UnaryOperator,
	FunctionKind_BinaryOperator      = jnc_FunctionKind_BinaryOperator,
	FunctionKind_OperatorVararg      = jnc_FunctionKind_OperatorVararg,
	FunctionKind_OperatorCdeclVararg = jnc_FunctionKind_OperatorCdeclVararg,
	FunctionKind_Internal            = jnc_FunctionKind_Internal,
	FunctionKind_Thunk               = jnc_FunctionKind_Thunk,
	FunctionKind_SchedLauncher       = jnc_FunctionKind_SchedLauncher,
	FunctionKind_AsyncSchedLauncher  = jnc_FunctionKind_AsyncSchedLauncher,
	FunctionKind_AsyncSequencer      = jnc_FunctionKind_AsyncSequencer,
	FunctionKind__Count              = jnc_FunctionKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getFunctionKindString(FunctionKind functionKind)
{
	return jnc_getFunctionKindString(functionKind);
}

//..............................................................................

typedef jnc_FunctionKindFlag FunctionKindFlag;

const FunctionKindFlag
	FunctionKindFlag_NoStorage   = jnc_FunctionKindFlag_NoStorage,
	FunctionKindFlag_NoOverloads = jnc_FunctionKindFlag_NoOverloads,
	FunctionKindFlag_NoArgs      = jnc_FunctionKindFlag_NoArgs;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
uint_t
getFunctionKindFlags(FunctionKind functionKind)
{
	return jnc_getFunctionKindFlags(functionKind);
}

//..............................................................................

typedef jnc_FunctionOverloadFlag FunctionOverloadFlag;

const FunctionOverloadFlag
	FunctionOverloadFlag_HasMembers = jnc_FunctionOverloadFlag_HasMembers;

//..............................................................................

class OverloadableFunction
{
protected:
	union
	{
		ModuleItem* m_item;
		Function* m_function;
		FunctionOverload* m_functionOverload;
	};

public:
	OverloadableFunction()
	{
		m_item = NULL;
	}

	OverloadableFunction(Function* function)
	{
		m_function = function;
	}

	OverloadableFunction(FunctionOverload* functionOverload)
	{
		m_functionOverload = functionOverload;
	}

	OverloadableFunction(jnc_OverloadableFunction src)
	{
		m_item = src.m_item;
	}

	operator bool()
	{
		return m_item != NULL;
	}

	operator jnc_OverloadableFunction()
	{
		jnc_OverloadableFunction result = { m_item };
		return result;
	}

	ModuleItem*
	operator -> ()
	{
		JNC_ASSERT(m_item);
		return m_item;
	}

	Function*
	getFunction()
	{
		JNC_ASSERT(m_item && jnc_ModuleItem_getItemKind(m_item) == ModuleItemKind_Function);
		return m_function;
	}

	FunctionOverload*
	getFunctionOverload()
	{
		JNC_ASSERT(m_item && jnc_ModuleItem_getItemKind(m_item) == ModuleItemKind_FunctionOverload);
		return m_functionOverload;
	}

	bool
	ensureNoImports();
};

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
