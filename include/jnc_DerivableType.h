#pragma once

#define _JNC_DERIVABLETYPE_H

#include "jnc_Type.h"
#include "jnc_OpKind.h"

/// \addtogroup derivable-type
/// @{

//..............................................................................

JNC_INLINE
jnc_DerivableType*
jnc_BaseTypeSlot_getType (jnc_BaseTypeSlot* baseType)
{
	return (jnc_DerivableType*) jnc_ModuleItem_getType ((jnc_ModuleItem*) baseType);
}

JNC_EXTERN_C
size_t
jnc_BaseTypeSlot_getOffset (jnc_BaseTypeSlot* baseType);

JNC_EXTERN_C
size_t
jnc_BaseTypeSlot_getVTableIndex (jnc_BaseTypeSlot* baseType);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_BaseTypeSlot: jnc_ModuleItem
{
	jnc_DerivableType*
	getType ()
	{
		return jnc_BaseTypeSlot_getType (this);
	}

	size_t
	getOffset ()
	{
		return jnc_BaseTypeSlot_getOffset (this);
	}

	size_t
	getVTableIndex ()
	{
		return jnc_BaseTypeSlot_getVTableIndex (this);
	}
};

#endif // _JNC_CORE

//..............................................................................

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getStaticConstructor (jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getStaticDestructor (jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getPreConstructor (jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getConstructor (jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getDestructor (jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getUnaryOperator (
	jnc_DerivableType* type,
	jnc_UnOpKind opKind
	);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getBinaryOperator (
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
	);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCallOperator (jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCastOperator (
	jnc_DerivableType* type,
	size_t idx
	);

JNC_EXTERN_C
size_t
jnc_DerivableType_getBaseTypeCount (jnc_DerivableType* type);

JNC_EXTERN_C
jnc_BaseTypeSlot*
jnc_DerivableType_getBaseType (
	jnc_DerivableType* type,
	size_t index
	);

JNC_EXTERN_C
size_t
jnc_DerivableType_findBaseTypeOffset (
	jnc_DerivableType* type,
	jnc_Type* baseType
	);

JNC_EXTERN_C
size_t
jnc_DerivableType_getMemberFieldCount (jnc_DerivableType* type);

JNC_EXTERN_C
jnc_StructField*
jnc_DerivableType_getMemberField (
	jnc_DerivableType* type,
	size_t index
	);

JNC_EXTERN_C
size_t
jnc_DerivableType_getMemberMethodCount (jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getMemberMethod (
	jnc_DerivableType* type,
	size_t index
	);

JNC_EXTERN_C
size_t
jnc_DerivableType_getMemberPropertyCount (jnc_DerivableType* type);

JNC_EXTERN_C
jnc_Property*
jnc_DerivableType_getMemberProperty (
	jnc_DerivableType* type,
	size_t index
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_DerivableType: jnc_NamedType
{
	jnc_Function*
	getStaticConstructor ()
	{
		return jnc_DerivableType_getStaticConstructor (this);
	}

	jnc_Function*
	getStaticDestructor ()
	{
		return jnc_DerivableType_getStaticDestructor (this);
	}

	jnc_Function*
	getPreConstructor ()
	{
		return jnc_DerivableType_getPreConstructor (this);
	}

	jnc_Function*
	getConstructor ()
	{
		return jnc_DerivableType_getConstructor (this);
	}

	jnc_Function*
	getDestructor ()
	{
		return jnc_DerivableType_getDestructor (this);
	}

	jnc_Function*
	getUnaryOperator (jnc_UnOpKind opKind)
	{
		return jnc_DerivableType_getUnaryOperator (this, opKind);
	}

	jnc_Function*
	getBinaryOperator (jnc_BinOpKind opKind)
	{
		return jnc_DerivableType_getBinaryOperator (this, opKind);
	}

	jnc_Function*
	getCallOperator ()
	{
		return jnc_DerivableType_getCallOperator (this);
	}

	jnc_Function*
	getCastOperator (size_t idx)
	{
		return jnc_DerivableType_getCastOperator (this, idx);
	}

	size_t
	getBaseTypeCount ()
	{
		return jnc_DerivableType_getBaseTypeCount (this);
	}

	jnc_BaseTypeSlot*
	getBaseType (size_t index)
	{
		return jnc_DerivableType_getBaseType (this, index);
	}

	size_t
	findBaseTypeOffset (jnc_Type* baseType)
	{
		return jnc_DerivableType_findBaseTypeOffset (this, baseType);
	}

	size_t
	getMemberFieldCount ()
	{
		return jnc_DerivableType_getMemberFieldCount (this);
	}

	jnc_StructField*
	getMemberField (size_t index)
	{
		return jnc_DerivableType_getMemberField (this, index);
	}

	size_t
	getMemberMethodCount ()
	{
		return jnc_DerivableType_getMemberMethodCount (this);
	}

	jnc_Function*
	getMemberMethod (size_t index)
	{
		return jnc_DerivableType_getMemberMethod (this, index);
	}

	size_t
	getMemberPropertyCount ()
	{
		return jnc_DerivableType_getMemberPropertyCount (this);
	}

	jnc_Property*
	getMemberProperty (size_t index)
	{
		return jnc_DerivableType_getMemberProperty (this, index);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
