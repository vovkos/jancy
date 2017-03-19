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

#include "jnc_ct_DoxyLexer.h"
#include "jnc_ct_DoxyBlock.h"
#include "jnc_ct_DoxyGroup.h"

namespace jnc {
namespace ct {

class Module;

//..............................................................................

class DoxyMgr
{
protected:
	struct Target: sl::ListLink
	{
		DoxyBlock* m_block;
		DoxyTokenKind m_tokenKind;
		sl::StringRef m_itemName;
		size_t m_overloadIdx;
	};

protected:
	Module* m_module;

	sl::StdList <DoxyBlock> m_blockList;
	sl::StdList <DoxyGroup> m_groupList;
	sl::StdList <DoxyFootnote> m_footnoteList;
	sl::StringHashTable <size_t> m_refIdMap;
	sl::StringHashTable <DoxyGroup*> m_groupMap;
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

	DoxyFootnote*
	createFootnote ();

	sl::String
	adjustRefId (const sl::StringRef& refId);

	void
	setBlockTarget (
		DoxyBlock* block,
		DoxyTokenKind tokenKind,
		const sl::StringRef& itemName,
		size_t overloadIdx
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

//..............................................................................

} // namespace ct
} // namespace jnc
