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

/**

\defgroup function Function
	\ingroup module-subsystem
	\import{jnc_Function.h}

\addtogroup function
@{

\struct jnc_Function
	\verbatim

	Opaque structure used as a handle to Jancy function.

	Use functions from the :ref:`Function <cid-function>` to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_FunctionKind
{
	jnc_FunctionKind_Undefined = 0,
	jnc_FunctionKind_Named,
	jnc_FunctionKind_Getter,
	jnc_FunctionKind_Setter,
	jnc_FunctionKind_Binder,
	jnc_FunctionKind_PreConstructor,
	jnc_FunctionKind_Constructor,
	jnc_FunctionKind_Destructor,
	jnc_FunctionKind_StaticConstructor,
	jnc_FunctionKind_StaticDestructor,
	jnc_FunctionKind_CallOperator,
	jnc_FunctionKind_CastOperator,
	jnc_FunctionKind_UnaryOperator,
	jnc_FunctionKind_BinaryOperator,
	jnc_FunctionKind_OperatorVararg,
	jnc_FunctionKind_OperatorCdeclVararg,
	jnc_FunctionKind_Internal,
	jnc_FunctionKind_Thunk,
	jnc_FunctionKind_Reaction,
	jnc_FunctionKind_ScheduleLauncher,
	jnc_FunctionKind__Count
};

typedef enum jnc_FunctionKind jnc_FunctionKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
jnc_getFunctionKindString (jnc_FunctionKind functionKind);

//..............................................................................

enum jnc_FunctionKindFlag
{
	jnc_FunctionKindFlag_NoStorage   = 0x01,
	jnc_FunctionKindFlag_NoOverloads = 0x02,
	jnc_FunctionKindFlag_NoArgs      = 0x04,
};

typedef enum jnc_FunctionKindFlag jnc_FunctionKindFlag;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

uint_t
jnc_getFunctionKindFlags (jnc_FunctionKind functionKind);

//..............................................................................

JNC_EXTERN_C
jnc_FunctionKind
jnc_Function_getFunctionKind (jnc_Function* function);

JNC_INLINE
jnc_FunctionType*
jnc_Function_getType (jnc_Function* function)
{
	return (jnc_FunctionType*) jnc_ModuleItem_getType ((jnc_ModuleItem*) function);
}

JNC_EXTERN_C
int
jnc_Function_isMember (jnc_Function* function);

JNC_EXTERN_C
int
jnc_Function_isOverloaded (jnc_Function* function);

JNC_EXTERN_C
size_t
jnc_Function_getOverloadCount (jnc_Function* function);

JNC_EXTERN_C
jnc_Function*
jnc_Function_getOverload (
	jnc_Function* function,
	size_t index
	);


JNC_EXTERN_C
void*
jnc_Function_getMachineCode (jnc_Function* function);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Function: jnc_ModuleItem
{
	jnc_FunctionKind
	getFunctionKind ()
	{
		return jnc_Function_getFunctionKind (this);
	}

	jnc_FunctionType*
	getType ()
	{
		return jnc_Function_getType (this);
	}

	bool
	isMember ()
	{
		return jnc_Function_isMember (this) != 0;
	}

	bool
	isOverloaded ()
	{
		return jnc_Function_isOverloaded (this) != 0;
	}

	size_t
	getOverloadCount ()
	{
		return jnc_Function_getOverloadCount (this);
	}

	jnc_Function*
	getOverload (size_t index)
	{
		return jnc_Function_getOverload (this, index);
	}

	void*
	getMachineCode ()
	{
		return jnc_Function_getMachineCode (this);
	}
};

#endif // _JNC_CORE

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_FunctionKind FunctionKind;

const FunctionKind
	FunctionKind_Undefined           = jnc_FunctionKind_Undefined,
	FunctionKind_Named               = jnc_FunctionKind_Named,
	FunctionKind_Getter              = jnc_FunctionKind_Getter,
	FunctionKind_Setter              = jnc_FunctionKind_Setter,
	FunctionKind_Binder              = jnc_FunctionKind_Binder,
	FunctionKind_PreConstructor      = jnc_FunctionKind_PreConstructor,
	FunctionKind_Constructor         = jnc_FunctionKind_Constructor,
	FunctionKind_Destructor          = jnc_FunctionKind_Destructor,
	FunctionKind_StaticConstructor   = jnc_FunctionKind_StaticConstructor,
	FunctionKind_StaticDestructor    = jnc_FunctionKind_StaticDestructor,
	FunctionKind_CallOperator        = jnc_FunctionKind_CallOperator,
	FunctionKind_CastOperator        = jnc_FunctionKind_CastOperator,
	FunctionKind_UnaryOperator       = jnc_FunctionKind_UnaryOperator,
	FunctionKind_BinaryOperator      = jnc_FunctionKind_BinaryOperator,
	FunctionKind_OperatorVararg      = jnc_FunctionKind_OperatorVararg,
	FunctionKind_OperatorCdeclVararg = jnc_FunctionKind_OperatorCdeclVararg,
	FunctionKind_Internal            = jnc_FunctionKind_Internal,
	FunctionKind_Thunk               = jnc_FunctionKind_Thunk,
	FunctionKind_Reaction            = jnc_FunctionKind_Reaction,
	FunctionKind_ScheduleLauncher    = jnc_FunctionKind_ScheduleLauncher,
	FunctionKind__Count              = jnc_FunctionKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getFunctionKindString (FunctionKind functionKind)
{
	return jnc_getFunctionKindString (functionKind);
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
getFunctionKindFlags (FunctionKind functionKind)
{
	return jnc_getFunctionKindFlags (functionKind);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
