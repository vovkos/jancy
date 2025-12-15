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

	switch (m_atomKind) {
	case QualifiedNameAtomKind_BaseType:
		return m_baseTypeIdx < countof(baseTypeStringTable) ?
			baseTypeStringTable[m_baseTypeIdx] :
			sl::formatString("basetype%d", m_baseTypeIdx);

	case QualifiedNameAtomKind_Name:
		return m_name;

	case QualifiedNameAtomKind_Template: {
		sl::String argString = Token::getText(m_templateTokenList);
		sl::String string;
		string.reserve(m_name.getLength() + argString.getLength() + 2);
		string.forceCopy(m_name);
		string += '<';
		string += argString;
		string += '>';
		return string;
		}

	default:
		ASSERT(false);
		return sl::StringRef();
	}
}

void
QualifiedNameAtom::copy(const QualifiedNameAtom& atom) {
	m_atomKind = atom.m_atomKind;

	switch (atom.m_atomKind) {
	case QualifiedNameAtomKind_BaseType:
		m_baseTypeIdx = atom.m_baseTypeIdx;
		break;

	case QualifiedNameAtomKind_Name:
		m_name = atom.m_name;
		break;

	case QualifiedNameAtomKind_Template: {
		m_name = atom.m_name;
		m_templateTokenList.clear();

		axl::mem::Pool<Token>* tokenPool = axl::mem::getCurrentThreadPool<Token>();
		sl::ConstIterator<Token> it = atom.m_templateTokenList.getHead();
		for (; it; it++)
			m_templateTokenList.insertTail(tokenPool->get(**it));

		break;
		}
	}
}

//..............................................................................

size_t
QualifiedName::appendFullName(sl::String* string) const {
	string->append(m_firstAtom.getString());
	sl::ConstBoxIterator<QualifiedNameAtom> it = m_atomList.getHead();
	for (; it; it++) {
		string->append('.');
		string->append(it->getString());
	}

	return string->getLength();
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
	m_firstAtom.copy(name.m_firstAtom);
	m_atomList.clear();

	sl::ConstBoxIterator<QualifiedNameAtom> it = name.m_atomList.getHead();
	for (; it; it++)
		m_atomList.insertTail()->copy(*it);
}

void
QualifiedName::copy(
	const QualifiedName& name,
	const QualifiedNamePos& pos0
) {
	QualifiedNamePos pos = pos0;
	m_firstAtom.copy(*pos);
	m_atomList.clear();

	for (pos.next(name); pos; pos.next(name))
		m_atomList.insertTail()->copy(*pos);
}

//..............................................................................

} // namespace ct
} // namespace jnc
