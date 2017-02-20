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
	friend class NamespaceMgr;

protected:
	ModuleItem* m_targetItem;
	Type* m_type;
	uint_t m_ptrTypeFlags;

public:
	Alias ();

	ModuleItem*
	getTargetItem ()
	{
		ASSERT (m_targetItem);
		return m_targetItem;
	}

	virtual
	bool
	generateDocumentation (
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);

protected:
	virtual
	bool
	calcLayout ();
};

//..............................................................................

} // namespace ct
} // namespace jnc
