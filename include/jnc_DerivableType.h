#pragma once

#include "jnc_Def.h"
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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_DerivableType
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
};

#endif // _JNC_CORE

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_DerivableType DerivableType;
typedef jnc_ClassType ClassType;

//.............................................................................

} // namespace jnc

#endif // __cplusplus
