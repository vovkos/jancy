//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_ct_QualifiedName.h"

namespace jnc {
namespace ct {

//..............................................................................

sl::StringRef
QualifiedNameAtom::getString() const {
	static const sl::StringRef baseTypeStringTable[] = {
		"basetype",
		"basetype2",
		"basetype4",
		"basetype5",
		"basetype6",
		"basetype7",
		"basetype8",
		"basetype9",
	};

	return
		m_atomKind == QualifiedNameAtomKind_Name ? m_name :
		m_atomKind == QualifiedNameAtomKind_BaseTypeIdx ?
			m_baseTypeIdx < countof(baseTypeStringTable) ?
				baseTypeStringTable[m_baseTypeIdx] :
				sl::formatString("basetype%d", m_baseTypeIdx) :
		sl::StringRef();
}

//..............................................................................

void
QualifiedName::addName(const QualifiedNameAtom& name) {
	if (m_firstName.isEmpty())
		m_firstName = name;
	else
		m_nameList.insertTail(name);
}

QualifiedNameAtom
QualifiedName::removeFirstName() {
	QualifiedNameAtom name = m_firstName;

	if (m_nameList.isEmpty())
		m_firstName.clear();
	else
		m_firstName = m_nameList.removeHead();

	return name;
}

QualifiedNameAtom
QualifiedName::removeLastName() {
	QualifiedNameAtom name;

	if (m_nameList.isEmpty()) {
		name = m_firstName;
		m_firstName.clear();
	} else {
		name = m_nameList.removeTail();
	}

	return name;
}

sl::StringRef
QualifiedName::getFullName() const {
	if (m_nameList.isEmpty())
		return m_firstName.getString();

	sl::String name = m_firstName.getString();
	sl::ConstBoxIterator<QualifiedNameAtom> it = m_nameList.getHead();
	for (; it; it++) {
		name.append('.');
		name.append(it->getString());
	}

	return name;
}

void
QualifiedName::parse(const sl::StringRef& name) {
	clear();

	const char* p = name.cp();
	const char* end = name.getEnd();

	for (;;) {
		size_t length = end - p;
		const char* dot = (const char*)memchr(p, '.', length);
		if (!dot) {
			addName(sl::StringRef(p, length));
			break;
		}

		addName(sl::StringRef(p, dot - p));
		p = dot + 1;
	}
}

void
QualifiedName::copy(const QualifiedName& name) {
	m_firstName = name.m_firstName;
	m_nameList.clear();

	sl::ConstBoxIterator<QualifiedNameAtom> it = name.m_nameList.getHead();
	for (; it; it++)
		m_nameList.insertTail(*it);
}

//..............................................................................

} // namespace ct
} // namespace jnc
