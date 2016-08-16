// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {
namespace ct {

class Module;

//.............................................................................

class DoxyBlock: public sl::ListLink
{
	friend class Parser;
	friend class ModuleItem;

protected:
	sl::String m_refId;
	sl::String m_briefDescription;
	sl::String m_detailedDescription;

public:
	sl::String 
	getRefId ()
	{
		return m_refId;
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
};

//.............................................................................

class DoxyMgr
{
protected:
	Module* m_module;

	sl::StdList <DoxyBlock> m_doxyBlockList;
	sl::StringHashTableMap <size_t> m_doxyRefIdMap;
	
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
	getDoxyBlockList ()
	{
		return m_doxyBlockList;
	}

	DoxyBlock* 
	createDoxyBlock ();

	sl::String
	adjustDoxyRefId (const sl::StringRef& refId);

protected:
};

//.............................................................................

} // namespace ct
} // namespace jnc
