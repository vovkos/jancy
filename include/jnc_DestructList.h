// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

//.............................................................................

class DestructList
{
protected:
	struct Entry: rtl::ListLink 
	{
		Function* m_destructor;
		Value m_argValue; // could be null for static destructors
		Variable* m_flagVariable; // could be null for unconditional destructors
	};

protected:
	Module* m_module;
	rtl::StdList <Entry> m_list;

public:
	DestructList ();

	void
	clear ()
	{
		m_list.clear ();
	}

	bool
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void 
	addDestructor (
		Function* destructor,
		const Value& argValue
		)
	{
		addDestructor (destructor, argValue, NULL);
	}

	void 
	addDestructor (
		Function* destructor,
		const Value& argValue,
		Variable* flagVariable
		);

	void
	runDestructors ();
};

//.............................................................................

} // namespace jnc {
