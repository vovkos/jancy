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

class Unit;

//..............................................................................

enum QualifiedNameAtomKind {
	QualifiedNameAtomKind_Empty,
	QualifiedNameAtomKind_BaseType,
	QualifiedNameAtomKind_Name,
	QualifiedNameAtomKind_Template,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct QualifiedNameAtom {
	QualifiedNameAtomKind m_atomKind;
	size_t m_baseTypeIdx; // QualifiedNameAtomKind_BaseType
	sl::StringRef m_name; // QualifiedNameAtomKind_Name
	sl::List<Token> m_templateTokenList; // QualifiedNameAtomKind_Template

	QualifiedNameAtom() {
		m_atomKind = QualifiedNameAtomKind_Empty;
	}

	bool
	isEmpty() const {
		return !m_atomKind;
	}

	sl::StringRef
	getString() const;

	void
	clear() {
		m_atomKind = QualifiedNameAtomKind_Empty;
	}

	void
	copy(const QualifiedNameAtom& atom);

	void
	setName(const sl::StringRef& name) {
		m_atomKind = QualifiedNameAtomKind_Name;
		m_name = name;
	}

	void
	setBaseType(size_t baseTypeIdx) {
		m_atomKind = QualifiedNameAtomKind_BaseType;
		m_baseTypeIdx = baseTypeIdx;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class QualifiedName {
protected:
	Unit* m_unit;
	QualifiedNameAtom m_firstAtom;
	sl::BoxList<QualifiedNameAtom> m_atomList;

public:
	QualifiedName() {
		m_unit = NULL;
	}

	bool
	isEmpty() const {
		return m_firstAtom.isEmpty();
	}

	bool
	isQualified() const {
		return !m_atomList.isEmpty();
	}

	bool
	isSimple() const {
		return m_atomList.isEmpty() && m_firstAtom.m_atomKind != QualifiedNameAtomKind_Template;
	}

	Unit*
	getUnit() const {
		return m_unit;
	}

	const QualifiedNameAtom&
	getFirstAtom() const {
		return m_firstAtom;
	}

	QualifiedNameAtom*
	getLastAtom() {
		return !m_atomList.isEmpty() ? &*m_atomList.getTail() : &m_firstAtom;
	}

	const sl::BoxList<QualifiedNameAtom>&
	getAtomList() const {
		return m_atomList;
	}

	const sl::StringRef&
	getShortName() const;

	sl::StringRef
	getFullName() const;

	sl::List<Token>*
	getTemplateTokenList() {
		ASSERT(getLastAtom()->m_atomKind == QualifiedNameAtomKind_Name);
		return &getLastAtom()->m_templateTokenList;
	}

	void
	finalizeTemplateTokenList(Unit* unit);

	void
	clear() {
		m_firstAtom.clear();
		m_atomList.clear();
	}

	void
	copy(const QualifiedName& name);

	void
	parse(const sl::StringRef& name);

	void
	addName(const sl::StringRef& name) {
		addAtom()->setName(name);
	}

	void
	addBaseType(size_t baseTypeIdx) {
		addAtom()->setBaseType(baseTypeIdx);
	}

	void
	removeFirstAtom(QualifiedNameAtom* atom);

	void
	removeLastAtom();

protected:
	QualifiedNameAtom*
	addAtom() {
		return m_firstAtom.isEmpty() ? &m_firstAtom : &*m_atomList.insertTail();
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const sl::StringRef&
QualifiedName::getShortName() const {
	const QualifiedNameAtom& atom = !m_atomList.isEmpty() ? *m_atomList.getTail() : m_firstAtom;
	ASSERT(atom.m_atomKind == QualifiedNameAtomKind_Name);
	return atom.m_name;
}

inline
void
QualifiedName::finalizeTemplateTokenList(Unit* unit) {
	QualifiedNameAtom* atom = getLastAtom();

	ASSERT(
		atom->m_atomKind == QualifiedNameAtomKind_Name &&
		atom->m_templateTokenList.getHead()->m_tokenKind == '<' &&
		atom->m_templateTokenList.getTail()->m_tokenKind == '>' &&
		(!m_unit || m_unit == unit)
	);

	atom->m_atomKind = QualifiedNameAtomKind_Template;
	atom->m_templateTokenList.eraseHead();
	atom->m_templateTokenList.eraseTail();
	m_unit = unit;
}

inline
void
QualifiedName::removeLastAtom() {
	if (m_atomList.isEmpty())
		m_firstAtom.clear();
	else
		m_atomList.eraseTail();
}

//..............................................................................

} // namespace ct
} // namespace jnc

