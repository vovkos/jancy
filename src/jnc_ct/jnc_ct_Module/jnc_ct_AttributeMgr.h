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

#pragma once

#include "jnc_ct_AttributeBlock.h"

namespace jnc {
namespace ct {

class Module;

//..............................................................................

class AttributeMgr
{
	friend class Module;

protected:
	Module* m_module;

	sl::List <AttributeBlock> m_attributeBlockList;

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
		const sl::StringRef& name,
		Value* value = NULL
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
