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
#include "jnc_ct_TemplateType.h"

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

	case QualifiedNameAtomKind_TemplateDeclSuffix: {
		size_t count = m_templateDeclArgArray.getCount();
		ASSERT(count);
		sl::String string;
		string.reserve(m_name.getLength() + count * 3);
		string.forceCopy(m_name);
		string += '<';
		string += m_templateDeclArgArray[0]->getName();

		for (size_t i = 1; i < count; i++) {
			string += ", ";
			string += m_templateDeclArgArray[i]->getName();
		}

		string += '>';
		return string;
		}

	case QualifiedNameAtomKind_TemplateInstantiateOperator: {
		sl::String argString = Token::getText(m_templateInstTokenList);
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

	case QualifiedNameAtomKind_TemplateDeclSuffix:
		m_name = atom.m_name;
		m_templateDeclArgArray = atom.m_templateDeclArgArray;
		break;

	case QualifiedNameAtomKind_TemplateInstantiateOperator: {
		m_name = atom.m_name;
		m_templateInstTokenList.clear();

		axl::mem::Pool<Token>* tokenPool = axl::mem::getCurrentThreadPool<Token>();
		sl::ConstIterator<Token> it = atom.m_templateInstTokenList.getHead();
		for (; it; it++)
			m_templateInstTokenList.insertTail(tokenPool->get(**it));

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

//..............................................................................

} // namespace ct
} // namespace jnc
