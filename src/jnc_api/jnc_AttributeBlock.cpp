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
#include "jnc_AttributeBlock.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_ExtensionLib.h"
#elif defined(_JNC_CORE)
#	include "jnc_ct_AttributeBlock.h"
#	include "jnc_ct_Variable.h"
#	include "jnc_ct_Function.h"
#	include "jnc_Variant.h"
#endif

//..............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
size_t
jnc_AttributeBlock_getAttributeCount(jnc_AttributeBlock* block) {
	return jnc_g_dynamicExtensionLibHost->m_attributeBlockFuncTable->m_getAttributeCountFunc(block);
}

JNC_EXTERN_C
jnc_Attribute*
jnc_AttributeBlock_getAttribute(
	jnc_AttributeBlock* block,
	size_t index
) {
	return jnc_g_dynamicExtensionLibHost->m_attributeBlockFuncTable->m_getAttributeFunc(block, index);
}

JNC_EXTERN_C
jnc_Attribute*
jnc_AttributeBlock_findAttribute(
	jnc_AttributeBlock* block,
	const char* name
) {
	return jnc_g_dynamicExtensionLibHost->m_attributeBlockFuncTable->m_findAttributeFunc(block, name);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Variant
jnc_Attribute_getValueVariant(jnc_Attribute* attr) {
	jnc::Variant variant;

	switch (attr->getValue().getValueKind()) {
	case jnc::ct::ValueKind_Const: {
		variant.m_type = attr->getValue().getType();
		const void* p = attr->getValue().getConstData();
		size_t size = variant.m_type->getSize();
		if (size <= jnc::Variant::DataSize)
			memcpy(&variant.m_data, p, size);
		else {
			variant.m_type = (jnc::Type*)variant.m_type->getDataPtrType_c(jnc::TypeKind_DataRef);
			variant.m_p = (void*)p;
		}
		break;
		}

	case jnc::ct::ValueKind_Variable:
		variant.m_type = attr->getValue().getType();
		variant.m_p = attr->getValue().getVariable()->getStaticData();

		if (attr->getValue().getVariable()->getType()->getTypeKind() == jnc::TypeKind_Class)
			variant.m_p = (jnc::Box*)variant.m_p + 1; // make it IfaceHdr*
		break;

	case jnc::ct::ValueKind_Function:
		variant.m_type = attr->getValue().getType();
		variant.m_p = attr->getValue().getFunction()->getMachineCode();
		break;

	default:
		return jnc::g_nullVariant;
	}

	return variant;
}

JNC_EXTERN_C
JNC_EXPORT_O
const void*
jnc_Attribute_getValueConstData(jnc_Attribute* attr) {
	return attr->getValue().getConstData();
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_Attribute_getValueString_v(jnc_Attribute* attr) {
	sl::String* buffer = jnc::getTlsStringBuffer();
	jnc_Attribute_getValueVariant(attr).format(buffer, NULL);
	return buffer->sz();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
JNC_EXPORT_O
size_t
jnc_AttributeBlock_getAttributeCount(jnc_AttributeBlock* block) {
	return block->getAttributeArray().getCount();
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Attribute*
jnc_AttributeBlock_getAttribute(
	jnc_AttributeBlock* block,
	size_t index
) {
	return block->getAttributeArray() [index];
}

JNC_EXTERN_C
JNC_EXPORT_O
jnc_Attribute*
jnc_AttributeBlock_findAttribute(
	jnc_AttributeBlock* block,
	const char* name
) {
	return block->findAttribute(name);
}

JNC_EXTERN_C
bool_t
jnc_AttributeBlock_ensureAttributeValuesReady(jnc_AttributeBlock* block) {
	return block->ensureAttributeValuesReady();
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//..............................................................................
