// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

class Module;

//.............................................................................

class ScopeLevelStack
{
	friend class NamespaceMgr;

	struct Entry: rtl::ListLink
	{
		Value m_scopeLevelValue;
	};

protected:
	Module* m_module;

	rtl::StdList <Entry> m_list;
	rtl::Array <Entry*> m_stack;

public:
	ScopeLevelStack ()
	{
		m_module = NULL;
	}

	void
	clear ()
	{
		m_list.clear ();
		m_stack.clear ();
	}

	void
	takeOver (ScopeLevelStack* srcStack);

	Value
	getScopeLevel (size_t level);

protected:
	Entry*
	getEntry (size_t level);
};

//.............................................................................

} // namespace jnc {
