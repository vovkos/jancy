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
Attribute::calcLayout ()
{
	ASSERT (!m_initializer.isEmpty ());

	return m_module->m_operatorMgr.parseConstExpression (
		m_parentUnit,
		m_initializer,
		&m_value
		);
}

//..............................................................................

Attribute*
AttributeBlock::createAttribute (
	const sl::StringRef& name,
	sl::BoxList <Token>* initializer
	)
{
	sl::StringHashTableIterator <Attribute*> it = m_attributeMap.visit (name);
	if (it->m_value)
	{
		err::setFormatStringError ("redefinition of attribute '%s'", name.sz ());
		return NULL;
	}

	Attribute* attribute = AXL_MEM_NEW (Attribute);
	attribute->m_module = m_module;
	attribute->m_name = name;

	if (initializer)
	{
		attribute->m_initializer.takeOver (initializer);
		m_module->markForLayout (attribute);
	}

	m_attributeList.insertTail (attribute);
	m_attributeArray.append (attribute);
	it->m_value = attribute;
	return attribute;
}

//..............................................................................

} // namespace ct
} // namespace jnc
