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
#include "jnc_ct_ImportType.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_DeclTypeCalc.h"

namespace jnc {
namespace ct {

//..............................................................................

void
ImportType::applyFixups() {
	ASSERT(m_actualType);

	size_t count = m_fixupArray.getCount();
	for (size_t i = 0; i < count; i++)
		*m_fixupArray[i] = m_actualType;
}

bool
ImportType::resolve() {
	ASSERT(!m_actualType);

	bool result;

	if (m_flags & ImportTypeFlag_InResolve) {
		err::setFormatStringError("can't resolve '%s' due to recursion", getTypeString().sz());
		result = false;
	} else {
		m_flags |= ImportTypeFlag_InResolve;
		result = resolveImports();
	}

	if (!result)
		m_resolveError = err::getLastError();

	return result;
}

//..............................................................................

sl::StringRef
ImportTypeName::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix:
		return TypeName::createTypeString("import ");

	default:
		return ImportType::createItemString(index);
	}
}

bool
ImportTypeName::resolveImports() {
	Namespace* nspace = m_anchor && m_anchor->m_namespace ?
		m_anchor->m_namespace :
		m_parentNamespace;

	m_actualType = lookupType(nspace);
	if (!m_actualType)
		return false;

	applyFixups();
	return true;
}

//..............................................................................

sl::StringRef
ImportPtrType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		sl::String string = m_baseType->getItemString(index);
		if (m_typeModifiers) {
			string += ' ';
			string += getTypeModifierString(m_typeModifiers);
		}

		string += '*';
		return string;
		}

	default:
		return ImportType::createItemString(index);
	}
}

bool
ImportPtrType::resolveImports() {
	bool result = m_baseType->ensureResolved();
	if (!result)
		return false;

	DeclTypeCalc typeCalc;
	m_actualType = typeCalc.calcPtrType(m_baseType->getActualType(), m_typeModifiers);
	if (!m_actualType)
		return false;

	applyFixups();
	return true;
}

//..............................................................................

sl::StringRef
ImportIntModType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		sl::String string = getTypeModifierString(m_typeModifiers);
		string += ' ';
		string += m_baseType->getItemString(index);
		return string;
		}

	default:
		return ImportType::createItemString(index);
	}
}

bool
ImportIntModType::resolveImports() {
	bool result = m_baseType->ensureResolved();
	if (!result)
		return false;

	DeclTypeCalc typeCalc;
	m_actualType = typeCalc.calcIntModType(m_baseType->getActualType(), m_typeModifiers);
	if (!m_actualType)
		return false;

	applyFixups();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
