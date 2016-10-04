#include "pch.h"
#include "jnc_ct_QualifiedName.h"

namespace jnc {
namespace ct {

//.............................................................................

void
QualifiedName::addName (const sl::StringRef& name)
{
	if (m_first.isEmpty ())
		m_first = name;
	else
		m_list.insertTail (name);
}

sl::String
QualifiedName::removeLastName ()
{
	sl::String name;

	if (m_list.isEmpty ())
	{
		name = m_first;
		m_first.clear ();
	}
	else
	{
		name = m_list.removeTail ();		
	}

	return name;
}

sl::String
QualifiedName::getFullName () const
{
	if (m_list.isEmpty ())
		return m_first;

	sl::String name = m_first;	
	sl::BoxIterator <sl::String> it = m_list.getHead ();
	for (; it; it++)
	{
		name.append ('.');
		name.append (*it);
	}

	return name;
}

void
QualifiedName::parse (const sl::StringRef& name0)
{
	clear ();

	const char* name = name0.sz ();
	for (;;)
	{
		const char* dot = strchr (name, '.');
		if (!dot)
		{
			addName (name);
			break;
		}

		addName (sl::StringRef (name, dot - name));
		name = dot + 1;
	}
}

void
QualifiedName::copy (const QualifiedName& name)
{
	m_first = name.m_first;
	m_list.clear ();

	sl::BoxIterator <sl::String> it = name.m_list.getHead ();
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

} // namespace ct
} // namespace jnc
