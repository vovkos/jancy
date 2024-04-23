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

namespace jnc {
namespace ct {

//..............................................................................

bool
Attribute::prepareValue() {
	ASSERT(!(m_flags & AttributeFlag_ValueReady));

	if (m_initializer.isEmpty()) {
		m_value.clear();
		m_variant = g_nullVariant;
		m_flags |= (AttributeFlag_ValueReady | AttributeFlag_VariantReady);
		return true;
	}

	bool result = m_module->m_operatorMgr.parseExpression(&m_initializer, &m_value);
	if (!result)
		return false;

	ValueKind valueKind = m_value.getValueKind();
	switch (valueKind) {
	case ValueKind_Null:
	case ValueKind_Const:
		break;

	case ValueKind_Variable:
		if (!(m_value.getVariable()->getFlags() & VariableFlag_Type)) {
			err::setFormatStringError(
				"non-type variable '%s' used as an attribute value",
				m_value.getVariable()->getQualifiedName().sz()
			);

			return false;
		}

		break;

	case ValueKind_Function:
		if (m_value.getFunction()->getStorageKind() != StorageKind_Static) {
			err::setFormatStringError(
				"non-static function '%s' used as an attribute value",
				m_value.getFunction()->getQualifiedName().sz()
			);

			return false;
		}

		result = m_value.getFunction()->getType()->getFunctionPtrType(FunctionPtrTypeKind_Thin)->ensureLayout();
		if (!result)
			return false;

		break;

	default:
		err::setFormatStringError("'%s' used as an attribute value", getValueKindString(m_value.getValueKind()));
		return false;
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
			variable->getType()->getTypeKind() != TypeKind_Class
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
AttributeBlock::prepareAttributeValues() {
	ASSERT(!(m_flags & AttributeBlockFlag_ValuesReady));

	Unit* prevUnit = m_module->m_unitMgr.setCurrentUnit(m_parentUnit);
	m_module->m_namespaceMgr.openNamespace(m_parentNamespace);

	bool finalResult = true;

	size_t count = m_attributeArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Attribute* attribute = m_attributeArray[i];
		bool result = attribute->ensureValueReady();
		if (!result)
			finalResult = false;
	}

	m_module->m_namespaceMgr.closeNamespace();
	m_module->m_unitMgr.setCurrentUnit(prevUnit);

	m_flags |= AttributeBlockFlag_ValuesReady;
	return finalResult;
}

void
AttributeBlock::setDynamicAttributeValue(
	const sl::StringRef& name,
	const Variant& value
) {
	ASSERT(m_flags & AttributeBlockFlag_Dynamic);

	sl::StringHashTableIterator<Attribute*> it = m_attributeMap.visit(name);
	if (it->m_value && ((it->m_value)->getFlags() & AttributeFlag_Dynamic)) {
		it->m_value->m_variant = value;
		return;
	}

	Attribute* attribute = new Attribute;
	attribute->m_flags |= AttributeFlag_Dynamic | AttributeFlag_ValueReady | AttributeFlag_VariantReady;
	attribute->m_module = m_module;
	attribute->m_parentUnit = m_parentUnit;
	attribute->m_parentNamespace = m_parentNamespace;
	attribute->m_name = name;
	attribute->m_variant = value;
	it->m_value = attribute;
	m_attributeArray.append(attribute);
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
