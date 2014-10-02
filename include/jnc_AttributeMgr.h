// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_AttributeBlock.h"

namespace jnc {

class Module;

//.............................................................................

class AttributeMgr: public rtl::ListLink
{
	friend class Module;

protected:
	Module* m_module;

	rtl::StdList <AttributeBlock> m_attributeBlockList;

public:
	AttributeMgr ();

	Module* 
	getModule ()
	{
		return m_module;
	}

	void
	clear ()
	{
		m_attributeBlockList.clear ();
	}

	AttributeBlock*
	createAttributeBlock ();

	Attribute*
	createAttribute (
		const rtl::String& name,
		Value* value = NULL
		);
};

//.............................................................................

} // namespace jnc {
