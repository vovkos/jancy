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
Attribute::parseInitializer()
{
	ASSERT(!m_initializer.isEmpty());

	bool result = m_module->m_operatorMgr.parseExpression(m_initializer, &m_value);
	if (!result)
		return false;

	ValueKind valueKind = m_value.getValueKind();
	switch (valueKind)
	{
	case ValueKind_Const:
		break;

	case ValueKind_Variable:
		if (!(m_value.getVariable()->getFlags() & VariableFlag_Type))
		{
			err::setFormatStringError(
				"non-type variable '%s' used as an attribute value",
				m_value.getVariable()->getQualifiedName().sz()
				);
		}

		break;

	case ValueKind_Function:
		if (m_value.getFunction()->getStorageKind() != StorageKind_Static)
		{
			err::setFormatStringError(
				"non-static function '%s' used in a const expression",
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

	return true;
}

//..............................................................................

Attribute*
AttributeBlock::createAttribute(
	const sl::StringRef& name,
	sl::BoxList<Token>* initializer
	)
{
	sl::StringHashTableIterator<Attribute*> it = m_attributeMap.visit(name);
	if (it->m_value)
	{
		err::setFormatStringError("redefinition of attribute '%s'", name.sz());
		return NULL;
	}

	Attribute* attribute = AXL_MEM_NEW(Attribute);
	attribute->m_module = m_module;
	attribute->m_name = name;

	if (initializer)
		sl::takeOver(&attribute->m_initializer, initializer);

	m_attributeList.insertTail(attribute);
	m_attributeArray.append(attribute);
	it->m_value = attribute;
	return attribute;
}

bool
AttributeBlock::prepareAttributeValues()
{
	ASSERT(!(m_flags & AttributeBlockFlag_ValuesReady));

	Unit* prevUnit = m_module->m_unitMgr.setCurrentUnit(m_parentUnit);
	m_module->m_namespaceMgr.openNamespace(m_parentNamespace);

	size_t count = m_attributeArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		Attribute* attribute = m_attributeArray[i];
		if (attribute->hasInitializer())
		{
			bool result = attribute->parseInitializer();
			if (!result)
				return false;
		}
	}

	m_module->m_namespaceMgr.closeNamespace();
	m_module->m_unitMgr.setCurrentUnit(prevUnit);

	m_flags |= AttributeBlockFlag_ValuesReady;
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
