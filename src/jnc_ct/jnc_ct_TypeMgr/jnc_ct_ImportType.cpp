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
ImportType::ensureResolved() {
	if (m_actualType)
		return true;

	if (m_flags & ImportTypeFlag_InResolve) {
		err::setFormatStringError("can't resolve '%s' due to recursion", getTypeString().sz());
		return false;
	}

	m_flags |= ImportTypeFlag_InResolve;
	return resolveImports();
}

//..............................................................................

NamedImportType::NamedImportType() {
	m_typeKind = TypeKind_NamedImport;
	m_anchorNamespace = NULL;
}

ImportPtrType*
NamedImportType::getImportPtrType(uint_t typeModifiers) {
	return m_module->m_typeMgr.getImportPtrType(this, typeModifiers);
}

sl::String
NamedImportType::createSignature(
	const QualifiedName& name,
	Namespace* anchorNamespace,
	const QualifiedName& orphanName
) {
	sl::String signature = sl::formatString("ZN%s", anchorNamespace->createQualifiedName (name).sz());

	if (!orphanName.isEmpty()) {
		signature += '-';
		signature += orphanName.getFullName();
	}

	return signature;
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

ImportPtrType::ImportPtrType() {
	m_typeKind = TypeKind_ImportPtr;
	m_targetType = NULL;
	m_typeModifiers = 0;
}

void
ImportPtrType::prepareTypeString() {
	ASSERT(m_targetType);
	TypeStringTuple* tuple = getTypeStringTuple();

	if (m_actualType) {
		tuple->m_typeStringPrefix = m_actualType->getTypeStringPrefix();
		tuple->m_typeStringSuffix = m_actualType->getTypeStringSuffix();
		return;
	}

	tuple->m_typeStringPrefix = "import ";

	if (m_typeModifiers) {
		tuple->m_typeStringPrefix += getTypeModifierString(m_typeModifiers);
		tuple->m_typeStringPrefix += ' ';
	}

	tuple->m_typeStringPrefix += m_targetType->getQualifiedName();
	tuple->m_typeStringPrefix += '*';
}

bool
ImportPtrType::resolveImports() {
	bool result = m_targetType->ensureResolved();
	if (!result)
		return false;

	DeclTypeCalc typeCalc;
	m_actualType = typeCalc.calcPtrType(m_targetType->getActualType(), m_typeModifiers);
	if (!m_actualType)
		return false;

	applyFixups();
	return true;
}

//..............................................................................

ImportIntModType::ImportIntModType() {
	m_typeKind = TypeKind_ImportPtr;
	m_importType = NULL;
	m_typeModifiers = 0;
}

void
ImportIntModType::prepareTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();

	if (m_actualType) {
		tuple->m_typeStringPrefix = m_actualType->getTypeStringPrefix();
		return;
	}

	tuple->m_typeStringPrefix = "import ";

	if (m_typeModifiers) {
		tuple->m_typeStringPrefix += getTypeModifierString(m_typeModifiers);
		tuple->m_typeStringPrefix += ' ';
	}

	tuple->m_typeStringPrefix += m_importType->getQualifiedName();
}

bool
ImportIntModType::resolveImports() {
	bool result = m_importType->ensureResolved();
	if (!result)
		return false;

	DeclTypeCalc typeCalc;
	m_actualType = typeCalc.calcIntModType(m_importType->getActualType(), m_typeModifiers);
	if (!m_actualType)
		return false;

	applyFixups();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
