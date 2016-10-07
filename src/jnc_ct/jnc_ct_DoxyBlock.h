// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {
namespace ct {

class DoxyGroup;
class ModuleItem;

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
	friend class DoxyParser;
	friend class ModuleItem;

protected:
	DoxyBlockKind m_blockKind;
	DoxyGroup* m_group;
	ModuleItem* m_item;

	sl::String m_refId;
	sl::String m_title;
	sl::String m_briefDescription;
	sl::String m_detailedDescription;

public:
	DoxyBlock ();

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

	ModuleItem* 
	getItem ()
	{
		return m_item;
	}

	const sl::String&
	getRefId ();

	const sl::String&
	getTitle ()
	{
		return m_title;
	}

	const sl::String&
	getBriefDescription ()
	{
		return m_briefDescription;
	}

	const sl::String&
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
	getDescriptionString ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
