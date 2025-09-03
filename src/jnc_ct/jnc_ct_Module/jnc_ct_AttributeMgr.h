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

class AttributeMgr {
	friend class Module;

protected:
	Module* m_module;

	sl::List<Attribute> m_attributeList;
	sl::List<AttributeBlock> m_attributeBlockList;

public:
	AttributeMgr();

	Module*
	getModule() {
		return m_module;
	}

	void
	clear() {
		m_attributeList.clear();
		m_attributeBlockList.clear();
	}

	Attribute*
	createAttribute(
		const sl::StringRef& name,
		sl::List<Token>* initializer
	);

	AttributeBlock*
	createAttributeBlock();

	AttributeBlock*
	createDynamicAttributeBlock(ModuleItemDecl* decl);
};

//..............................................................................

} // namespace ct
} // namespace jnc
