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
	Namespace* parentNamespace,
	const QualifiedName& name,
	const QualifiedName* baseName
) {
	sl::StringRef parentSignature = parentNamespace->getDeclItem()->getLinkId();
	sl::String signature = "IN";
	signature += parentSignature;
	if (!parentSignature.isEmpty())
		signature += '.';

	name.appendFullName(&signature);

	if (baseName) {
		signature += '-';
		baseName->appendFullName(&signature);
	}

	return signature;
}

sl::StringRef
NamedImportType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		sl::String string = "import ";
		sl::StringRef parentName = m_parentNamespace->getDeclItem()->getItemName();
		if (!parentName.isEmpty()) {
			string += parentName;
			string += '.';
		}

		if (!m_baseName.isEmpty()) {
			string += m_baseName.getFullName();
			string += '.';
		}

		string += m_name.getFullName();
		return string;
		}

	default:
		return ImportType::createItemString(index);
	}
}

bool
NamedImportType::resolveImports() {
	Namespace* anchorNamespace = m_parentNamespace;
	if (!m_baseName.isEmpty()) {
		FindModuleItemResult findResult = anchorNamespace->findItemTraverse(*this, m_baseName);
		if (!findResult.m_result) {
			pushSrcPosError();
			return false;
		}

		anchorNamespace = findResult.m_item ? findResult.m_item->getNamespace() : NULL;
	}

	FindModuleItemResult findResult = anchorNamespace ?
		anchorNamespace->findItemTraverse(*this, m_name) :
		g_nullFindModuleItemResult;

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
	Type* type;
	switch (itemKind) {
	case ModuleItemKind_Type:
		type = (Type*)item;
		break;

	case ModuleItemKind_Typedef:
		type = (m_module->getCompileFlags() & ModuleCompileFlag_KeepTypedefShadow) ?
			((Typedef*)item)->getShadowType() :
			((Typedef*)item)->getType();
		break;

	case ModuleItemKind_Template:
		type = anchorNamespace->findTemplateInstanceType((Template*)item);
		if (type)
			break;

		// else fall through

	default:
		err::setFormatStringError("'%s' is not a type", getTypeString().sz());
		pushSrcPosError();
		return false;
	}

	if (type->getTypeKindFlags() & TypeKindFlag_Import) {
		ImportType* importType = (ImportType*)type;
		bool result = importType->ensureResolved();
		if (!result)
			return false;

		type = importType->getActualType();
		ASSERT(!(type->getTypeKindFlags() & TypeKindFlag_Import));
	}

	m_actualType = type;
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
