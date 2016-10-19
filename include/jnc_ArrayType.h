#pragma once

#define _JNC_ARRAYTYPE_H

#include "jnc_Type.h"

/// \addtogroup array-type
/// @{

//..............................................................................

enum jnc_ArrayTypeFlag
{
	jnc_ArrayTypeFlag_AutoSize = 0x010000,
};

typedef enum jnc_ArrayTypeFlag jnc_ArrayTypeFlag;

//..............................................................................

JNC_EXTERN_C
jnc_Type*
jnc_ArrayType_getElementType (jnc_ArrayType* type);

JNC_EXTERN_C
size_t
jnc_ArrayType_getElementCount (jnc_ArrayType* type);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_ArrayType: jnc_Type
{
	jnc_Type*
	getElementType ()
	{
		return jnc_ArrayType_getElementType (this);
	}

	size_t
	getElementCount ()
	{
		return jnc_ArrayType_getElementCount (this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_INLINE
int
jnc_isAutoSizeArrayType (jnc_Type* type)
{
	return
		jnc_Type_getTypeKind (type) == jnc_TypeKind_Array &&
		(jnc_ModuleItem_getFlags ((jnc_ModuleItem*) type) & jnc_ArrayTypeFlag_AutoSize) != 0;
}

JNC_INLINE
int
jnc_isCharArrayType (jnc_Type* type)
{
	return
		jnc_Type_getTypeKind (type) == jnc_TypeKind_Array &&
		jnc_Type_getTypeKind (jnc_ArrayType_getElementType ((jnc_ArrayType*) type)) == jnc_TypeKind_Char;
}

JNC_INLINE
int
jnc_isCharArrayRefType (jnc_Type* type)
{
	return
		jnc_Type_getTypeKind (type) == jnc_TypeKind_DataRef &&
		jnc_isCharArrayType (jnc_DataPtrType_getTargetType ((jnc_DataPtrType*) type));
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_ArrayTypeFlag ArrayTypeFlag;

const ArrayTypeFlag
	ArrayTypeFlag_AutoSize = jnc_ArrayTypeFlag_AutoSize;

//..............................................................................

JNC_INLINE
bool
isAutoSizeArrayType (Type* type)
{
	return jnc_isAutoSizeArrayType (type) != 0;
}

JNC_INLINE
bool
isCharArrayType (Type* type)
{
	return jnc_isCharArrayType (type) != 0;
}

JNC_INLINE
bool
isCharArrayRefType (Type* type)
{
	return jnc_isCharArrayRefType (type) != 0;
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
