#pragma once

#include "jnc_RuntimeStructs.h"
#include "jnc_OpKind.h"

typedef struct jnc_DerivableType jnc_DerivableType;
typedef struct jnc_ClassType jnc_ClassType;
typedef struct jnc_Function jnc_Function;
typedef struct jnc_Namespace jnc_Namespace;

//.............................................................................

jnc_Function*
jnc_DerivableType_getPreConstructor (jnc_DerivableType* type);

jnc_Function*
jnc_DerivableType_getConstructor (jnc_DerivableType* type);

jnc_Function*
jnc_DerivableType_getDestructor (jnc_DerivableType* type);

jnc_Function*
jnc_DerivableType_getUnaryOperator (
	jnc_DerivableType* type,
	jnc_UnOpKind opKind	
	);

jnc_Function*
jnc_DerivableType_getBinaryOperator (
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
	);

jnc_Function*
jnc_DerivableType_getCallOperator (jnc_DerivableType* type);

jnc_Function*
jnc_DerivableType_getCastOperator (
	jnc_DerivableType* type,
	size_t idx
	);

jnc_Namespace*
jnc_DerivableType_getNamespace (jnc_DerivableType* type);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
jnc_primeClass (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	void* vtable = NULL // if null then vtable of class type will be used
	);

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_DerivableType DerivableType;
typedef jnc_ClassType ClassType;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline 
void
primeClass (
	jnc_Box* box,
	jnc_Box* root,
	jnc_ClassType* type,
	void* vtable = NULL // if null then vtable of class type will be used
	)
{
	jnc_primeClass (box, root, type, vtable);
}

//.............................................................................

} // namespace jnc

#endif // __cplusplus
