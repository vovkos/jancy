// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {
namespace ct {

class Module;
class ModuleItem;
class DoxyGroupLink;
class DoxyGroup;

//.............................................................................

enum DoxyBlockKind
{
	DoxyBlockKind_Normal,
	DoxyBlockKind_Group,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DoxyBlock: public sl::ListLink
{
	friend class DoxyMgr;
	friend class Parser;
	friend class ModuleItem;

protected:
	DoxyBlockKind m_blockKind;
	DoxyGroup* m_group;
	sl::String m_refId;
	sl::String m_title;
	sl::String m_briefDescription;
	sl::String m_detailedDescription;

public:
	DoxyBlock ()
	{
		m_blockKind = DoxyBlockKind_Normal;
		m_group = NULL;
	}

	DoxyBlockKind 
	getBlockKind ()
	{
		return m_blockKind;
	}

	DoxyGroup* 
	getGroup ()
	{
		return m_group;
	}

	sl::String 
	getRefId ()
	{
		return m_refId;
	}

	sl::String 
	getTitle ()
	{
		return m_title;
	}

	sl::String 
	getBriefDescription ()
	{
		return m_briefDescription;
	}

	sl::String 
	getDetailedDescription ()
	{		
		return m_detailedDescription;
	}

	bool
	isDescriptionEmpty ()
	{
		return m_briefDescription.isEmpty () && m_detailedDescription.isEmpty ();
	}

	sl::String
	createDoxyDescriptionString ();
};

//.............................................................................

class DoxyGroup: public DoxyBlock
{
	friend class Parser;
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

	sl::String 
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
		const char* outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);
};

//.............................................................................

class DoxyMgr
{
protected:
	struct Target: sl::ListLink
	{
		DoxyBlock* m_block;
		sl::StringRef m_targetName;
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
		const sl::StringRef& targetName
		);

	bool
	resolveBlockTargets ();

	void
	deleteEmptyGroups ();

	bool
	generateGroupDocumentation (
		const char* outputDir,
		sl::String* indexXml
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
