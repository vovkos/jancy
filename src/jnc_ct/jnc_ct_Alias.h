// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ImportType.h"

namespace jnc {
namespace ct {

//..............................................................................

class Alias:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer
{
	friend class VariableMgr;

protected:
	Type* m_type;

public:
	Alias ()
	{
		m_itemKind = ModuleItemKind_Alias;
		m_type = NULL;
	}

	Type*
	getType ()
	{
		return m_type;
	}

	virtual
	bool
	generateDocumentation (
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
