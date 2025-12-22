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
class QualifiedName;
class TemplateArgType;

//..............................................................................

enum QualifiedNameAtomKind {
	QualifiedNameAtomKind_Empty = 0,
	QualifiedNameAtomKind_BaseType,
	QualifiedNameAtomKind_Name,
	QualifiedNameAtomKind_TemplateDeclSuffix,
	QualifiedNameAtomKind_TemplateInstantiateOperator,
	QualifiedNameAtomKind_FirstTemplate = QualifiedNameAtomKind_TemplateDeclSuffix,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct QualifiedNameAtom {
	QualifiedNameAtomKind m_atomKind;
	size_t m_baseTypeIdx; // QualifiedNameAtomKind_BaseType
	sl::StringRef m_name; // QualifiedNameAtomKind_Name
	sl::Array<TemplateArgType*> m_templateDeclArgArray; // QualifiedNameAtomKind_TemplateDeclSuffix
	sl::List<Token> m_templateInstTokenList; // QualifiedNameAtomKind_TemplateInstantiateOperator

	QualifiedNameAtom() {
		m_atomKind = QualifiedNameAtomKind_Empty;
	}

	bool
	isEmpty() const {
		return m_atomKind == QualifiedNameAtomKind_Empty;
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

class QualifiedNamePos {
protected:
	static const QualifiedNameAtom m_emptyAtom;
	const QualifiedNameAtom* m_atom;

public:
	QualifiedNamePos() {
		m_atom = &m_emptyAtom;
	}

	QualifiedNamePos(const QualifiedNameAtom* atom) {
		m_atom = atom;
	}

	operator bool () const {
		return !m_atom->isEmpty();
	}

	const QualifiedNameAtom& operator * () const {
		return *m_atom;
	}

	const QualifiedNameAtom* operator -> () const  {
		return m_atom;
	}

	const QualifiedNameAtom&
	next(const QualifiedName& name);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AXL_SELECT_ANY const QualifiedNameAtom QualifiedNamePos::m_emptyAtom;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class QualifiedName {
protected:
	QualifiedNameAtom m_firstAtom;
	sl::BoxList<QualifiedNameAtom> m_atomList;

public:
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
		return m_atomList.isEmpty() && m_firstAtom.m_atomKind < QualifiedNameAtomKind_FirstTemplate;
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

	sl::StringRef
	getFullName() const;

	size_t
	appendFullName(sl::String* string) const;

	sl::List<Token>*
	getTemplateInstTokenList() {
		ASSERT(getLastAtom()->m_atomKind == QualifiedNameAtomKind_Name);
		return &getLastAtom()->m_templateInstTokenList;
	}

	void
	finalizeTemplateInstTokenList();

	void
	clear() {
		m_firstAtom.clear();
		m_atomList.clear();
	}

	void
	copy(const QualifiedName& name);

	void
	copy(
		const QualifiedName& name,
		const QualifiedNamePos& pos
	);

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
	removeLastAtom();

protected:
	QualifiedNameAtom*
	addAtom() {
		return m_firstAtom.isEmpty() ? &m_firstAtom : &*m_atomList.insertTail();
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
sl::StringRef
QualifiedName::getFullName() const {
	if (m_atomList.isEmpty())
		return m_firstAtom.getString();

	sl::String string;
	appendFullName(&string);
	return string;
}

inline
void
QualifiedName::finalizeTemplateInstTokenList() {
	QualifiedNameAtom* atom = getLastAtom();

	ASSERT(
		atom->m_atomKind == QualifiedNameAtomKind_Name &&
		atom->m_templateInstTokenList.getHead()->m_tokenKind == '<' &&
		atom->m_templateInstTokenList.getTail()->m_tokenKind == '>'
	);

	atom->m_atomKind = QualifiedNameAtomKind_TemplateInstantiateOperator;
	atom->m_templateInstTokenList.eraseHead();
	atom->m_templateInstTokenList.eraseTail();
}

inline
void
QualifiedName::removeLastAtom() {
	if (m_atomList.isEmpty())
		m_firstAtom.clear();
	else
		m_atomList.eraseTail();
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const QualifiedNameAtom&
QualifiedNamePos::next(const QualifiedName & name) {
	if (m_atom->isEmpty())
		return m_emptyAtom;

	const QualifiedNameAtom& prevAtom = *m_atom;

	sl::ConstBoxIterator<QualifiedNameAtom> it = m_atom == &name.getFirstAtom() ?
		name.getAtomList().getHead() :
		sl::ConstBoxIterator<QualifiedNameAtom>(containerof(m_atom, sl::BoxListEntry<QualifiedNameAtom>, m_value)).getNext();

	m_atom = it ? &*it : &m_emptyAtom;
	return prevAtom;
}

//..............................................................................

} // namespace ct
} // namespace jnc

