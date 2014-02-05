#include "pch.h"
#include "jnc_QualifiedName.h"

namespace jnc {

//.............................................................................

void
CQualifiedName::AddName (const rtl::CString& Name)
{
	if (m_First.IsEmpty ())
		m_First = Name;
	else
		m_List.InsertTail (Name);
}

rtl::CString
CQualifiedName::GetFullName () const
{
	if (m_List.IsEmpty ())
		return m_First;

	rtl::CString Name = m_First;	
	rtl::CBoxIteratorT <rtl::CString> It = m_List.GetHead ();
	for (; It; It++)
	{
		Name.Append ('.');
		Name.Append (*It);
	}

	return Name;
}

void
CQualifiedName::Parse (const char* pName)
{
	Clear ();

	for (;;)
	{
		const char* pDot = strchr (pName, '.');
		if (!pDot)
		{
			AddName (pName);
			break;
		}

		AddName (rtl::CString (pName, pDot - pName));
		pName = pDot + 1;
	}
}

void
CQualifiedName::Copy (const CQualifiedName& Name)
{
	m_First = Name.m_First;
	m_List.Clear ();

	rtl::CBoxIteratorT <rtl::CString> It = Name.m_List.GetHead ();
	for (; It; It++)
		m_List.InsertTail (*It);
}

void
CQualifiedName::TakeOver (CQualifiedName* pName)
{
	m_First = pName->m_First;
	m_List.TakeOver (&pName->m_List);
	pName->Clear ();
}

//.............................................................................

} // namespace jnc {
