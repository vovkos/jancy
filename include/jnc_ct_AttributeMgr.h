// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_AttributeBlock.h"

namespace jnc {
namespace ct {

class Module;

//.............................................................................

class AttributeMgr: public sl::ListLink
{
	friend class Module;

protected:
	Module* m_module;

	sl::StdList <AttributeBlock> m_attributeBlockList;

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
		const sl::String& name,
		Value* value = NULL
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
