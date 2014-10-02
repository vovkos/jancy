// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {

//.............................................................................

class QualifiedName
{
protected:
	rtl::String m_first;
	rtl::BoxList <rtl::String> m_list;

public:
	QualifiedName ()
	{
	}

	QualifiedName (const rtl::String& name)
	{
		m_first = name;
	}	

	QualifiedName (const char* name)
	{
		m_first = name;
	}	

	QualifiedName (const QualifiedName& name)
	{
		copy (name);
	}	

	QualifiedName&
	operator = (const QualifiedName& name)
	{
		copy (name);
		return *this;
	}	

	void
	clear ()
	{
		m_first.clear ();
		m_list.clear ();
	}

	void
	parse (const char* name);

	void
	addName (const rtl::String& name);

	bool
	isEmpty () const
	{
		return m_first.isEmpty ();
	}

	bool
	isSimple () const
	{
		return m_list.isEmpty ();
	}

	rtl::String 
	getFirstName () const
	{
		return m_first;
	}

	rtl::ConstBoxList <rtl::String> 
	getNameList () const
	{
		return m_list;
	}

	rtl::String
	getShortName () const
	{
		return !m_list.isEmpty () ? *m_list.getTail () : m_first;
	}

	rtl::String
	getFullName () const;

	void
	copy (const QualifiedName& name);

	void
	takeOver (QualifiedName* name);
};

//.............................................................................

} // namespace jnc {
