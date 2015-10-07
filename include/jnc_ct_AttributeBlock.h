// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

//.............................................................................

class Attribute: public UserModuleItem
{
	friend class AttributeBlock;

protected:
	Value* m_value;

public:
	Attribute ()
	{
		m_value = NULL;
	}

	Value* 
	getValue ()
	{
		return m_value;
	}
};

//.............................................................................

class AttributeBlock: public UserModuleItem
{
	friend class AttributeMgr;

protected:
	ModuleItem* m_parentItem;

	sl::StdList <Attribute> m_attributeList;
	sl::StringHashTableMap <Attribute*> m_attributeMap; 

public:
	AttributeBlock ()
	{
		m_parentItem = NULL;
	}

	ModuleItem*
	getParentItem ()
	{
		return m_parentItem;
	}

	sl::ConstList <Attribute>
	getAttributeList ()
	{
		return m_attributeList;
	}

	Attribute*
	findAttribute (const char* name)
	{
		sl::StringHashTableMapIterator <Attribute*> it = m_attributeMap.find (name); 
		return it ? it->m_value : NULL;
	}

	Attribute*
	createAttribute (
		const sl::String& name,
		Value* value = NULL
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
