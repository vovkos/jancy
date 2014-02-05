// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {

//.............................................................................

class CQualifiedName
{
protected:
	rtl::CString m_First;
	rtl::CBoxListT <rtl::CString> m_List;

public:
	CQualifiedName ()
	{
	}

	CQualifiedName (const rtl::CString& Name)
	{
		m_First = Name;
	}	

	CQualifiedName (const char* pName)
	{
		m_First = pName;
	}	

	CQualifiedName (const CQualifiedName& Name)
	{
		Copy (Name);
	}	

	CQualifiedName&
	operator = (const CQualifiedName& Name)
	{
		Copy (Name);
		return *this;
	}	

	void
	Clear ()
	{
		m_First.Clear ();
		m_List.Clear ();
	}

	void
	Parse (const char* pName);

	void
	AddName (const rtl::CString& Name);

	bool
	IsEmpty () const
	{
		return m_First.IsEmpty ();
	}

	bool
	IsSimple () const
	{
		return m_List.IsEmpty ();
	}

	rtl::CString 
	GetFirstName () const
	{
		return m_First;
	}

	rtl::CConstBoxListT <rtl::CString> 
	GetNameList () const
	{
		return m_List;
	}

	rtl::CString
	GetShortName () const
	{
		return !m_List.IsEmpty () ? *m_List.GetTail () : m_First;
	}

	rtl::CString
	GetFullName () const;

	void
	Copy (const CQualifiedName& Name);

	void
	TakeOver (CQualifiedName* pName);
};

//.............................................................................

} // namespace jnc {
