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
#include "jnc_ct_Decl.h"
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

sl::String
NamedImportType::createSignature(
	const QualifiedName& name,
	Namespace* anchorNamespace,
	const QualifiedName* anchorName
) {
	sl::String signature = "IN" + anchorNamespace->createQualifiedName(name);
	if (anchorName) {
		signature += '-';
		signature += anchorName->getFullName();
	}

	return signature;
}

void
NamedImportType::prepareTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();

	tuple->m_typeStringPrefix = m_anchorName.isEmpty() ?
		sl::formatString(
			"import %s",
			m_anchorNamespace->createQualifiedName(m_name).sz()
		) :
		sl::formatString(
			"import %s.%s",
			m_anchorNamespace->createQualifiedName(m_anchorName).sz(),
			m_name.getFullName().sz()
		);
}

bool
NamedImportType::resolveImports() {
	Namespace* anchorNamespace = m_anchorNamespace;
	if (!m_anchorName.isEmpty()) {
		FindModuleItemResult findResult = anchorNamespace->findItemTraverse(m_anchorName);
		if (!findResult.m_result) {
			pushSrcPosError();
			return false;
		}

		anchorNamespace = findResult.m_item ? findResult.m_item->getNamespace() : NULL;
	}

	FindModuleItemResult findResult = anchorNamespace ? anchorNamespace->findItemTraverse(m_name) : g_nullFindModuleItemResult;
	if (!findResult.m_result) {
		pushSrcPosError();
		return false;
	}

	if (!findResult.m_item) {
		err::setFormatStringError("unresolved import '%s'", getTypeString().sz());
		pushSrcPosError();
		return false;
	}

	ModuleItem* item = findResult.m_item;
	ModuleItemKind itemKind = item->getItemKind();
	switch (itemKind) {
	case ModuleItemKind_Type:
		m_actualType = (Type*)item;
		break;

	case ModuleItemKind_Typedef:
		m_actualType = (m_module->getCompileFlags() & ModuleCompileFlag_KeepTypedefShadow) ?
			((Typedef*)item)->getShadowType() :
			((Typedef*)item)->getType();
		break;

	default:
		err::setFormatStringError("'%s' is not a type", getTypeString().sz());
		pushSrcPosError();
		return false;
	}

	if (m_actualType->getTypeKindFlags() & TypeKindFlag_Import) {
		ImportType* type = (ImportType*)m_actualType;
		bool result = ((ImportType*)type)->ensureResolved();
		if (!result)
			return false;

		m_actualType = type->getActualType();
		ASSERT(!(m_actualType->getTypeKindFlags() & TypeKindFlag_Import));
	}

	applyFixups();
	return true;
}

//..............................................................................

void
ImportPtrType::prepareTypeString() {
	ASSERT(m_baseType);
	TypeStringTuple* tuple = getTypeStringTuple();

	if (m_actualType) {
		tuple->m_typeStringPrefix = m_actualType->getTypeStringPrefix();
		tuple->m_typeStringSuffix = m_actualType->getTypeStringSuffix();
		return;
	}

	sl::String string = m_baseType->getTypeString();

	if (m_typeModifiers) {
		string += ' ';
		string += getTypeModifierString(m_typeModifiers);
	}

	string += '*';
	tuple->m_typeStringPrefix = string;
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

void
ImportIntModType::prepareTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();

	if (m_actualType) {
		tuple->m_typeStringPrefix = m_actualType->getTypeStringPrefix();
		return;
	}

	ASSERT(m_typeModifiers);

	sl::String string = getTypeModifierString(m_typeModifiers);
	string += ' ';
	string += m_baseType->getTypeString();
	tuple->m_typeStringPrefix = string;
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
