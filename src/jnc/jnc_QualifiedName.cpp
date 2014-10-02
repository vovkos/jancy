#include "pch.h"
#include "jnc_QualifiedName.h"

namespace jnc {

//.............................................................................

void
QualifiedName::addName (const rtl::String& name)
{
	if (m_first.isEmpty ())
		m_first = name;
	else
		m_list.insertTail (name);
}

rtl::String
QualifiedName::getFullName () const
{
	if (m_list.isEmpty ())
		return m_first;

	rtl::String name = m_first;	
	rtl::BoxIterator <rtl::String> it = m_list.getHead ();
	for (; it; it++)
	{
		name.append ('.');
		name.append (*it);
	}

	return name;
}

void
QualifiedName::parse (const char* name)
{
	clear ();

	for (;;)
	{
		const char* dot = strchr (name, '.');
		if (!dot)
		{
			addName (name);
			break;
		}

		addName (rtl::String (name, dot - name));
		name = dot + 1;
	}
}

void
QualifiedName::copy (const QualifiedName& name)
{
	m_first = name.m_first;
	m_list.clear ();

	rtl::BoxIterator <rtl::String> it = name.m_list.getHead ();
	for (; it; it++)
		m_list.insertTail (*it);
}

void
QualifiedName::takeOver (QualifiedName* name)
{
	m_first = name->m_first;
	m_list.takeOver (&name->m_list);
	name->clear ();
}

//.............................................................................

} // namespace jnc {
