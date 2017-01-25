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
