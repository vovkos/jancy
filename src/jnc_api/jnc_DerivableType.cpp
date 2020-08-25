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
#include "jnc_DerivableType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_DerivableType.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_BaseTypeSlot_getOffset(jnc_BaseTypeSlot* baseType)
{
	return jnc_g_dynamicExtensionLibHost->m_baseTypeSlotFuncTable->m_getOffsetFunc(baseType);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_BaseTypeSlot_getVtableIndex(jnc_BaseTypeSlot* baseType)
{
	return jnc_g_dynamicExtensionLibHost->m_baseTypeSlotFuncTable->m_getVtableIndexFunc(baseType);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_Field_getPtrTypeFlags(jnc_Field* field)
{
	return jnc_g_dynamicExtensionLibHost->m_fieldFuncTable->m_getPtrTypeFlagsFunc(field);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Field_getOffset(jnc_Field* field)
{
	return jnc_g_dynamicExtensionLibHost->m_fieldFuncTable->m_getOffsetFunc(field);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_DerivableType_getStaticConstructor(jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getStaticConstructorFunc(type);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_OverloadableFunction
jnc_DerivableType_getConstructor(jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getConstructorFunc(type);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_DerivableType_getDestructor(jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getDestructorFunc(type);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_OverloadableFunction
jnc_DerivableType_getUnaryOperator(
	jnc_DerivableType* type,
	jnc_UnOpKind opKind
	)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getUnaryOperatorFunc(type, opKind);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_OverloadableFunction
jnc_DerivableType_getBinaryOperator(
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
	)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getBinaryOperatorFunc(type, opKind);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_OverloadableFunction
jnc_DerivableType_getCallOperator(jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getCallOperatorFunc(type);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DerivableType_getCastOperatorCount(jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getCastOperatorCountFunc(type);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_DerivableType_getCastOperator(
	jnc_DerivableType* type,
	size_t index
	)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getCastOperatorFunc(type, index);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_BaseTypeSlot_getOffset(jnc_BaseTypeSlot* baseType)
{
	return baseType->getOffset();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_BaseTypeSlot_getVtableIndex(jnc_BaseTypeSlot* baseType)
{
	return baseType->getVtableIndex();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
uint_t
jnc_Field_getPtrTypeFlags(jnc_Field* field)
{
	return field->getPtrTypeFlags();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_Field_getOffset(jnc_Field* field)
{
	return field->getOffset();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_DerivableType_getStaticConstructor(jnc_DerivableType* type)
{
	return type->getStaticConstructor();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_OverloadableFunction
jnc_DerivableType_getConstructor(jnc_DerivableType* type)
{
	return type->getConstructor();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_DerivableType_getDestructor(jnc_DerivableType* type)
{
	return type->getDestructor();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_OverloadableFunction
jnc_DerivableType_getUnaryOperator(
	jnc_DerivableType* type,
	jnc_UnOpKind opKind
	)
{
	return type->getUnaryOperator((jnc::UnOpKind)opKind);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_OverloadableFunction
jnc_DerivableType_getBinaryOperator(
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
	)
{
	return type->getBinaryOperator((jnc::BinOpKind)opKind);
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_OverloadableFunction
jnc_DerivableType_getCallOperator(jnc_DerivableType* type)
{
	return type->getCallOperator();
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DerivableType_getCastOperatorCount(jnc_DerivableType* type)
{
	return type->getCastOperatorArray().getCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_DerivableType_getCastOperator(
	jnc_DerivableType* type,
	size_t index
	)
{
	return type->getCastOperatorArray()[index];
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DerivableType_getBaseTypeCount(jnc_DerivableType* type)
{
	return type->getBaseTypeArray().getCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_BaseTypeSlot*
jnc_DerivableType_getBaseType(
	jnc_DerivableType* type,
	size_t index
	)
{
	return type->getBaseTypeArray() [index];
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DerivableType_findBaseTypeOffset(
	jnc_DerivableType* type,
	jnc_Type* baseType
	)
{
	return type->findBaseTypeOffset(baseType);
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DerivableType_getStaticVariableCount(jnc_DerivableType* type)
{
	return type->getStaticVariableArray().getCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Variable*
jnc_DerivableType_getStaticVariable(
	jnc_DerivableType* type,
	size_t index
	)
{
	return type->getStaticVariableArray()[index];
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DerivableType_getFieldCount(jnc_DerivableType* type)
{
	return type->getFieldArray().getCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Field*
jnc_DerivableType_getField(
	jnc_DerivableType* type,
	size_t index
	)
{
	return type->getFieldArray()[index];
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DerivableType_getMethodCount(jnc_DerivableType* type)
{
	return type->getMethodArray().getCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Function*
jnc_DerivableType_getMethod(
	jnc_DerivableType* type,
	size_t index
	)
{
	return type->getMethodArray()[index];
}

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_DerivableType_getPropertyCount(jnc_DerivableType* type)
{
	return type->getPropertyArray().getCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Property*
jnc_DerivableType_getProperty(
	jnc_DerivableType* type,
	size_t index
	)
{
	return type->getPropertyArray()[index];
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB
