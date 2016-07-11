#include "pch.h"
#include "jnc_DerivableType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif (defined _JNC_CORE)
#	include "jnc_ct_DerivableType.h"
#	include "jnc_ct_ClassType.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getPreConstructor (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getPreConstructorFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getConstructor (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getConstructorFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getDestructor (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getDestructorFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getUnaryOperator (
	jnc_DerivableType* type,
	jnc_UnOpKind opKind	
	)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getUnaryOperatorFunc (type, opKind);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getBinaryOperator (
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
	)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getBinaryOperatorFunc (type, opKind);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCallOperator (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getCallOperatorFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCastOperator (
	jnc_DerivableType* type,
	size_t idx
	)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getCastOperatorFunc (type, idx);
}

JNC_EXTERN_C
jnc_Namespace*
jnc_DerivableType_getNamespace (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getNamespaceFunc (type);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getPreConstructor (jnc_DerivableType* type)
{
	jnc_Function* function = type->getPreConstructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no preconstructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getConstructor (jnc_DerivableType* type)
{
	jnc_Function* function = type->getConstructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no constructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getDestructor (jnc_DerivableType* type)
{
	jnc_Function* function = type->getDestructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no destructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getUnaryOperator (
	jnc_DerivableType* type,
	jnc_UnOpKind opKind	
	)
{
	jnc_Function* function = type->getUnaryOperator ((jnc::UnOpKind) opKind);
	if (!function)
	{
		err::setFormatStringError ("'%s' has no operator %s", type->getTypeString ().cc (), jnc_getUnOpKindString (opKind));
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getBinaryOperator (
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
	)
{
	jnc_Function* function = type->getBinaryOperator ((jnc::BinOpKind) opKind);
	if (!function)
	{
		err::setFormatStringError ("'%s' has no operator %s", type->getTypeString ().cc (), jnc_getBinOpKindString (opKind));
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCallOperator (jnc_DerivableType* type)
{
	jnc_Function* function = type->getCallOperator ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no operator ()", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCastOperator (
	jnc_DerivableType* type,
	size_t idx
	)
{
	jnc_Function* function = type->getCastOperator (idx);
	if (!function)
	{
		err::setFormatStringError ("'%s' has no cast operator #%d", type->getTypeString ().cc (), idx);
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Namespace*
jnc_DerivableType_getNamespace (jnc_DerivableType* type)
{
	return type;
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB