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

	Unit* prevUnit = m_module->m_unitMgr.setCurrentUnit(m_parentUnit);
	bool result = m_module->m_operatorMgr.parseConstExpression(m_initializer, &m_value);
	if (!result)
		return false;

	if (m_value.getValueKind() == ValueKind_Function)
	{
		result = m_value.getFunction()->getType()->getFunctionPtrType(FunctionPtrTypeKind_Thin)->ensureLayout();
		if (!result)
			return false;
	}

	m_module->m_unitMgr.setCurrentUnit(prevUnit);
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

	m_flags |= AttributeBlockFlag_ValuesReady;
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
