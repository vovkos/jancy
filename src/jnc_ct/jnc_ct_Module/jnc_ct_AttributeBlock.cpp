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
#include "jnc_ct_AttributeBlock.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ParseContext.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
Attribute::prepareValue(bool isDynamic) {
	ASSERT(
		!(m_flags & AttributeFlag_ValueReady) &&
		m_module->getCompileState() < ModuleCompileState_Compiled
	);

	if (m_initializer.isEmpty()) {
		m_value.clear();
		m_variant = g_nullVariant;
		m_flags |= (AttributeFlag_ValueReady | AttributeFlag_VariantReady);
		return true;
	}

	ParseContext parseContext(ParseContextKind_Expression, m_module, *this);
	bool result = m_module->m_operatorMgr.parseExpression(&m_initializer, &m_value);
	if (!result)
		return false;

	ValueKind valueKind = m_value.getValueKind();
	switch (valueKind) {
	case ValueKind_Null:
		break;

	case ValueKind_Const:
		if (isCharArrayType(m_value.getType())) {
			result = m_module->m_operatorMgr.castOperator(&m_value, TypeKind_String);
			if (!result)
				return false;
		}

		break;

	case ValueKind_Variable:
		if (!(m_value.getVariable()->getFlags() & VariableFlag_Type))
			if (isDynamic)
				m_flags |= AttributeFlag_DynamicValue | AttributeFlag_VariantReady; // this attribute will be shadowed by a dynamic one
			else {
				err::setFormatStringError(
					"non-type variable '%s' used as an attribute value",
					m_value.getVariable()->getItemName().sz()
				);

				return false;
			}

		break;

	case ValueKind_Function:
		if (m_value.getFunction()->getStorageKind() != StorageKind_Static) {
			err::setFormatStringError(
				"non-static function '%s' used as an attribute value",
				m_value.getFunction()->getItemName().sz()
			);

			return false;
		}

		result = m_value.getFunction()->getType()->getFunctionPtrType(FunctionPtrTypeKind_Thin)->ensureLayout();
		if (!result)
			return false;

		break;

	case ValueKind_Type:
		result = m_module->m_operatorMgr.typeofOperator(&m_value);
		if (!result)
			return false;

		break;

	default:
		if (isDynamic)
			m_flags |= AttributeFlag_DynamicValue | AttributeFlag_VariantReady; // this attribute will be shadowed by a dynamic one
		else {
			err::setFormatStringError("'%s' used as an attribute value", getValueKindString(m_value.getValueKind()));
			return false;
		}
	}

	m_flags |= AttributeFlag_ValueReady;
	return true;
}

void
Attribute::prepareVariant() {
	ASSERT(!(m_flags & AttributeFlag_VariantReady));

	m_variant = g_nullVariant;

	ct::ValueKind valueKind = m_value.getValueKind();
	switch (valueKind) {
	case ct::ValueKind_Const: {
		size_t size = m_value.getType()->getSize();
		if (size <= jnc::Variant::DataSize)
			memcpy(&m_variant.m_data, m_value.getConstData(), size);
		else {
			m_variant.m_type = m_value.getType()->getDataPtrType_c(jnc::TypeKind_DataRef);
			m_variant.m_p = m_value.getConstData();
		}

		m_variant.create(m_value.getConstData(), m_value.getType());
		break;
		}

	case ct::ValueKind_Variable: {
		Variable* variable = m_value.getVariable();
		ASSERT(
			variable->getStorageKind() == StorageKind_Static &&
			variable->getType()->getTypeKind() == TypeKind_Class
		);

		m_variant.m_type = (jnc::Type*)((ct::ClassType*)variable->getType())->getClassPtrType();
		m_variant.m_p = (Box*)variable->getStaticData() + 1; // we want IfaceHdr*
		break;
		}

	case ct::ValueKind_Function: {
		Function* function = m_value.getFunction();
		ASSERT(function->getStorageKind() == StorageKind_Static);

		m_variant.m_type = (jnc::Type*)function->getType()->getFunctionPtrType(FunctionPtrTypeKind_Thin);
		m_variant.m_p = function->getMachineCode();
		break;
		}
	}

	m_flags |= AttributeFlag_VariantReady;
}

//..............................................................................

bool
AttributeBlock::addAttribute(Attribute* attribute) {
	sl::StringHashTableIterator<Attribute*> it = m_attributeMap.visit(attribute->getName());

	// don't overwrite attributes declared in this very block
	if (it->m_value && !(it->m_value->getFlags() & AttributeFlag_Shared)) {
		if (attribute->getFlags() & AttributeFlag_Shared) // shared attribute; ignore
			return true;

		err::setFormatStringError("redefinition of attribute '%s'", attribute->getName().sz());
		return false;
	}

	m_attributeArray.append(attribute);
	it->m_value = attribute;
	return true;
}

void
AttributeBlock::addAttributeBlock(AttributeBlock* attributeBlock) {
	size_t count = attributeBlock->m_attributeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Attribute* attribute = attributeBlock->m_attributeArray[i];
		attribute->m_flags |= AttributeFlag_Shared;
		bool result = addAttribute(attribute);
		ASSERT(result);
	}
}

bool
AttributeBlock::prepareAttributeValues(bool isDynamic) {
	ASSERT(!(m_flags & AttributeBlockFlag_ValuesReady));

	bool finalResult = true;
	uint_t mask = 0;

	size_t count = m_attributeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Attribute* attribute = m_attributeArray[i];
		if (!(attribute->m_flags & AttributeFlag_ValueReady)) {
			bool result = attribute->prepareValue(isDynamic);
			if (!result)
				finalResult = false;

			mask |= attribute->m_flags;
		}
	}

	m_flags |= AttributeBlockFlag_ValuesReady | (mask & AttributeBlockFlag_DynamicValues);
	return finalResult;
}

void
AttributeBlock::setDynamicAttributeValue(
	size_t i,
	const Variant& value
) {
	ASSERT(m_flags & AttributeBlockFlag_Dynamic);

	Attribute* attribute = m_attributeArray[i];
	ASSERT(attribute->m_flags & AttributeFlag_DynamicValue);

	Attribute* dynamicAttribute = new Attribute;
	dynamicAttribute->m_flags |= AttributeFlag_Dynamic | AttributeFlag_ValueReady | AttributeFlag_VariantReady;
	dynamicAttribute->m_module = m_module;
	dynamicAttribute->m_parentUnit = m_parentUnit;
	dynamicAttribute->m_parentNamespace = m_parentNamespace;
	dynamicAttribute->m_name = attribute->m_name;
	dynamicAttribute->m_pos = attribute->m_pos;
	dynamicAttribute->m_variant = value;
	m_attributeArray.rwi()[i] = dynamicAttribute;
	m_attributeMap[attribute->m_name] = dynamicAttribute;
}

void
AttributeBlock::deleteDynamicAttributes() {
	size_t count = m_attributeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Attribute* attribute = m_attributeArray[i];
		if (attribute->m_flags & AttributeBlockFlag_Dynamic)
			delete attribute;
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
