#pragma once

#define _JNC_DERIVABLETYPE_H

#include "jnc_Type.h"
#include "jnc_OpKind.h"

//.............................................................................

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
jnc_Namespace*
jnc_DerivableType_getNamespace (jnc_DerivableType* type);

JNC_EXTERN_C
size_t
jnc_DerivableType_findBaseTypeOffset (
	jnc_DerivableType* type,
	jnc_Type* baseType
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_DerivableType: jnc_Type
{
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

	jnc_Namespace*
	getNamespace ()
	{
		return jnc_DerivableType_getNamespace (this);
	}

	size_t
	findBaseTypeOffset (jnc_Type* baseType)
	{
		return jnc_DerivableType_findBaseTypeOffset (this, baseType);
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_StructType: jnc_DerivableType
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ClassType: jnc_DerivableType
{
};

#endif // _JNC_CORE

//.............................................................................
