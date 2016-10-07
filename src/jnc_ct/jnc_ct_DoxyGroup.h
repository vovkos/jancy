// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_DoxyBlock.h"

namespace jnc {
namespace ct {

//.............................................................................

class DoxyGroup: public DoxyBlock
{
	friend class DoxyParser;
	friend class DoxyMgr;
	friend class DoxyGroupLink;

protected:
	sl::String m_name;
	sl::Array <ModuleItem*> m_itemArray;
	sl::BoxList <DoxyGroup*> m_groupList;
	sl::BoxIterator <DoxyGroup*> m_parentGroupListIt;

public:
	DoxyGroup ()
	{
		m_blockKind = DoxyBlockKind_Group;
	}

	bool
	isEmpty ()
	{
		return m_itemArray.isEmpty () && m_groupList.isEmpty ();
	}

	const sl::String&
	getName ()
	{
		return m_name;
	}

	sl::Array <ModuleItem*> 
	getItemArray ()
	{
		return m_itemArray;
	}

	sl::ConstBoxList <DoxyGroup*> 
	getGroupList ()
	{
		return m_groupList;
	}

	void
	addItem (ModuleItem* item)
	{
		m_itemArray.append (item);
	}

	sl::BoxIterator <DoxyGroup*>
	addGroup (DoxyGroup* group)
	{
		return m_groupList.insertTail (group);
	}

	bool
	generateDocumentation (
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
