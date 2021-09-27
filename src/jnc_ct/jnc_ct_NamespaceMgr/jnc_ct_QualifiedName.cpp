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

void
QualifiedName::addName(const sl::StringRef& name) {
	if (m_first.isEmpty())
		m_first = name;
	else
		m_list.insertTail(name);
}

sl::StringRef
QualifiedName::removeFirstName() {
	sl::StringRef name = m_first;

	if (m_list.isEmpty())
		m_first.clear();
	else
		m_first = m_list.removeHead();

	return name;
}

sl::StringRef
QualifiedName::removeLastName() {
	sl::StringRef name;

	if (m_list.isEmpty()) {
		name = m_first;
		m_first.clear();
	} else {
		name = m_list.removeTail();
	}

	return name;
}

sl::String
QualifiedName::getFullName() const {
	if (m_list.isEmpty())
		return m_first;

	sl::String name = m_first;
	sl::ConstBoxIterator<sl::StringRef> it = m_list.getHead();
	for (; it; it++) {
		name.append('.');
		name.append(*it);
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
	m_first = name.m_first;
	m_list.clear();

	sl::ConstBoxIterator<sl::StringRef> it = name.m_list.getHead();
	for (; it; it++)
		m_list.insertTail(*it);
}

//..............................................................................

} // namespace ct
} // namespace jnc
