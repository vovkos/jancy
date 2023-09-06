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

#define _JNC_FUNCTIONTYPE_H

#include "jnc_Type.h"
#include "jnc_RuntimeStructs.h"

/**

\defgroup function-type Function Type
	\ingroup type-subsystem
	\import{jnc_FunctionType.h}

	\brief Function type defines a signature of a function -- what it the calling convention, which type is the return value of, how many and which type of arguments a function accepts and so on.

\addtogroup function-type
@{

\struct jnc_FunctionType
	\verbatim

	Opaque structure used as a handle to Jancy function type.

	Use functions from the `Function Type` group to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_FunctionTypeFlag {
	jnc_FunctionTypeFlag_VarArg         = 0x010000,
	jnc_FunctionTypeFlag_ErrorCode      = 0x020000,
	jnc_FunctionTypeFlag_ByValArgs      = 0x040000,
	jnc_FunctionTypeFlag_CoercedArgs    = 0x080000,
	jnc_FunctionTypeFlag_Unsafe         = 0x100000,
	jnc_FunctionTypeFlag_Async          = 0x200000,
	jnc_FunctionTypeFlag_AsyncErrorCode = 0x400000,
	jnc_FunctionTypeFlag_IntExtArgs     = 0x800000,
};

typedef enum jnc_FunctionTypeFlag jnc_FunctionTypeFlag;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getFunctionTypeFlagString(jnc_FunctionTypeFlag flag);

//..............................................................................

enum jnc_FunctionPtrTypeKind {
	jnc_FunctionPtrTypeKind_Normal = 0,
	jnc_FunctionPtrTypeKind_Weak,
	jnc_FunctionPtrTypeKind_Thin,
	jnc_FunctionPtrTypeKind__Count,
};

typedef enum jnc_FunctionPtrTypeKind jnc_FunctionPtrTypeKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getFunctionPtrTypeKindString(jnc_FunctionPtrTypeKind ptrTypeKind);

//..............................................................................

JNC_EXTERN_C
bool_t
jnc_FunctionArg_hasDefaultValue(jnc_FunctionArg* arg);

JNC_EXTERN_C
const char*
jnc_FunctionArg_getDefaultValueString_v(jnc_FunctionArg* arg);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_FunctionArg: jnc_ModuleItem {
	bool
	hasDefaultValue() {
		return jnc_FunctionArg_hasDefaultValue(this) != 0;
	}

	const char*
	getDefaultValueString() {
		return jnc_FunctionArg_getDefaultValueString_v(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
jnc_Type*
jnc_FunctionType_getReturnType(jnc_FunctionType* type);

JNC_EXTERN_C
size_t
jnc_FunctionType_getArgCount(jnc_FunctionType* type);

JNC_EXTERN_C
jnc_FunctionArg*
jnc_FunctionType_getArg(
	jnc_FunctionType* type,
	size_t index
);

JNC_EXTERN_C
jnc_FunctionPtrType*
jnc_FunctionType_getFunctionPtrType(
	jnc_FunctionType* type,
	jnc_TypeKind typeKind,
	jnc_FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
);

JNC_EXTERN_C
jnc_FunctionType*
jnc_FunctionType_getShortType(jnc_FunctionType* type);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_FunctionType: jnc_Type {
	jnc_Type*
	getReturnType() {
		return jnc_FunctionType_getReturnType(this);
	}

	size_t
	getArgCount() {
		return jnc_FunctionType_getArgCount(this);
	}

	jnc_FunctionArg*
	getArg(size_t index) {
		return jnc_FunctionType_getArg(this, index);
	}

	jnc_FunctionPtrType*
	getFunctionPtrType(
		jnc_TypeKind typeKind,
		jnc_FunctionPtrTypeKind ptrTypeKind = jnc_FunctionPtrTypeKind_Normal,
		uint_t flags = 0
	) {
		return jnc_FunctionType_getFunctionPtrType(this, typeKind, ptrTypeKind, flags);
	}

	jnc_FunctionPtrType*
	getFunctionPtrType(
		jnc_FunctionPtrTypeKind ptrTypeKind = jnc_FunctionPtrTypeKind_Normal,
		uint_t flags = 0
	) {
		return jnc_FunctionType_getFunctionPtrType(this, jnc_TypeKind_FunctionPtr, ptrTypeKind, flags);
	}

	jnc_FunctionType*
	getShortType() {
		return jnc_FunctionType_getShortType(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
jnc_FunctionPtrTypeKind
jnc_FunctionPtrType_getPtrTypeKind(jnc_FunctionPtrType* type);

JNC_EXTERN_C
jnc_FunctionType*
jnc_FunctionPtrType_getTargetType(jnc_FunctionPtrType* type);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_FunctionPtrType: jnc_Type {
	jnc_FunctionPtrTypeKind
	getPtrTypeKind() {
		return jnc_FunctionPtrType_getPtrTypeKind(this);
	}

	jnc_FunctionType*
	getTargetType() {
		return jnc_FunctionPtrType_getTargetType(this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
size_t
jnc_FunctionTypeOverload_getOverloadCount(jnc_FunctionTypeOverload* typeOverload);

JNC_EXTERN_C
jnc_FunctionType*
jnc_FunctionTypeOverload_getOverload(
	jnc_FunctionTypeOverload* typeOverload,
	size_t index
);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_FunctionTypeOverload {
	size_t
	getOverloadCount() {
		return jnc_FunctionTypeOverload_getOverloadCount(this);
	}

	jnc_FunctionType*
	getOverload(size_t index) {
		return jnc_FunctionTypeOverload_getOverload(this, index);
	}
};

#endif // _JNC_CORE

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_FunctionTypeFlag FunctionTypeFlag;

const FunctionTypeFlag
	FunctionTypeFlag_VarArg         = jnc_FunctionTypeFlag_VarArg,
	FunctionTypeFlag_ErrorCode      = jnc_FunctionTypeFlag_ErrorCode,
	FunctionTypeFlag_ByValArgs      = jnc_FunctionTypeFlag_ByValArgs,
	FunctionTypeFlag_CoercedArgs    = jnc_FunctionTypeFlag_CoercedArgs,
	FunctionTypeFlag_Unsafe         = jnc_FunctionTypeFlag_Unsafe,
	FunctionTypeFlag_Async          = jnc_FunctionTypeFlag_Async,
	FunctionTypeFlag_AsyncErrorCode = jnc_FunctionTypeFlag_AsyncErrorCode,
	FunctionTypeFlag_IntExtArgs     = jnc_FunctionTypeFlag_IntExtArgs;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getFunctionTypeFlagString(jnc_FunctionTypeFlag flag) {
	return jnc_getFunctionTypeFlagString(flag);
}

//..............................................................................

typedef jnc_FunctionPtrTypeKind FunctionPtrTypeKind;

const FunctionPtrTypeKind
	FunctionPtrTypeKind_Normal = jnc_FunctionPtrTypeKind_Normal,
	FunctionPtrTypeKind_Weak   = jnc_FunctionPtrTypeKind_Weak,
	FunctionPtrTypeKind_Thin   = jnc_FunctionPtrTypeKind_Thin,
	FunctionPtrTypeKind__Count = jnc_FunctionPtrTypeKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getFunctionPtrTypeKindString(jnc_FunctionPtrTypeKind ptrTypeKind) {
	return jnc_getFunctionPtrTypeKindString(ptrTypeKind);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
