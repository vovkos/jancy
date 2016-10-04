// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_DoxyLexer.h"
#include "jnc_ct_DoxyBlock.h"
#include "jnc_ct_DoxyGroup.h"

namespace jnc {
namespace ct {

class Module;

//.............................................................................

class DoxyMgr
{
protected:
	struct Target: sl::ListLink
	{
		DoxyBlock* m_block;
		DoxyTokenKind m_tokenKind;
		sl::StringRef m_itemName;
	};

protected:
	Module* m_module;

	sl::StdList <DoxyBlock> m_blockList;
	sl::StdList <DoxyGroup> m_groupList;
	sl::StringHashTableMap <size_t> m_refIdMap;
	sl::StringHashTableMap <DoxyGroup*> m_groupMap;
	sl::StdList <Target> m_targetList;
	
public:
	DoxyMgr ();

	Module* 
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	sl::ConstList <DoxyBlock>
	getBlockList ()
	{
		return m_blockList;
	}

	sl::ConstList <DoxyGroup>
	getGroupList ()
	{
		return m_groupList;
	}

	DoxyGroup*
	getGroup (const sl::StringRef& name);

	DoxyBlock* 
	createBlock ();

	sl::String
	adjustRefId (const sl::StringRef& refId);

	void
	setBlockTarget (
		DoxyBlock* block,		
		DoxyTokenKind tokenKind,
		const sl::StringRef& itemName
		);

	bool
	resolveBlockTargets ();

	void
	deleteEmptyGroups ();

	bool
	generateGroupDocumentation (
		const sl::StringRef& outputDir,
		sl::String* indexXml
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
