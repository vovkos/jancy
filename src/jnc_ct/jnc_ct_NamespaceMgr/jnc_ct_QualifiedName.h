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

#pragma once

#include "jnc_ct_Lexer.h"

namespace jnc {
namespace ct {

//..............................................................................

enum QualifiedNameAtomKind {
	QualifiedNameAtomKind_Empty,
	QualifiedNameAtomKind_Name,
	QualifiedNameAtomKind_BaseTypeIdx,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct QualifiedNameAtom {
	QualifiedNameAtomKind m_atomKind;
	size_t m_baseTypeIdx;
	sl::StringRef m_name;

	QualifiedNameAtom() {
		m_atomKind = QualifiedNameAtomKind_Empty;
		m_baseTypeIdx = -1;
	}

	QualifiedNameAtom(const sl::StringRef& name);

	QualifiedNameAtom(size_t baseTypeIdx) {
		m_atomKind = QualifiedNameAtomKind_BaseTypeIdx;
		m_baseTypeIdx = baseTypeIdx;
	}

	bool
	isEmpty() const {
		return !m_atomKind;
	}

	void
	clear() {
		m_atomKind = QualifiedNameAtomKind_Empty;
	}

	sl::StringRef
	getString() const;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
QualifiedNameAtom::QualifiedNameAtom(const sl::StringRef& name) {
	m_atomKind = QualifiedNameAtomKind_Name;
	m_baseTypeIdx = -1;
	m_name = name;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class QualifiedName {
protected:
	QualifiedNameAtom m_firstName;
	sl::BoxList<QualifiedNameAtom> m_nameList;
	sl::List<Token> m_templateTokenList;

public:
	QualifiedName() {}

	explicit
	QualifiedName(const sl::StringRef& name) {
		m_firstName.m_name = name;
	}

	explicit
	QualifiedName(size_t baseTypeIdx) {
		m_firstName.m_baseTypeIdx = baseTypeIdx;
	}

	QualifiedName(const QualifiedName& name) {
		copy(name);
	}

	QualifiedName&
	operator = (const QualifiedName& name) {
		copy(name);
		return *this;
	}

	bool
	isEmpty() const {
		return m_firstName.isEmpty();
	}

	bool
	isSimple() const {
		return m_nameList.isEmpty() && m_templateTokenList.isEmpty();
	}

	bool
	isTemplate() const {
		return !m_templateTokenList.isEmpty();
	}

	const QualifiedNameAtom&
	getFirstName() const {
		return m_firstName;
	}

	const sl::BoxList<QualifiedNameAtom>&
	getNameList() const {
		return m_nameList;
	}

	sl::List<Token>*
	getTemplateTokenList() {
		return &m_templateTokenList;
	}

	const QualifiedNameAtom&
	getShortName() const {
		return !m_nameList.isEmpty() ? *m_nameList.getTail() : m_firstName;
	}

	sl::StringRef
	getFullName() const;

	void
	clear();

	void
	copy(const QualifiedName& name);

	void
	parse(const sl::StringRef& name);

	void
	addName(const QualifiedNameAtom& name);

	void
	addTemplateToken(Token* token) {
		m_templateTokenList.insertTail(token);
	}

	QualifiedNameAtom
	removeFirstName();

	QualifiedNameAtom
	removeLastName();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
QualifiedName::clear() {
	m_firstName.clear();
	m_nameList.clear();
	m_templateTokenList.clear();
}

//..............................................................................

} // namespace ct
} // namespace jnc

