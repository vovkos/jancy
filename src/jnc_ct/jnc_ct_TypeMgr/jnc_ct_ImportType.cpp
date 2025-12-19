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
	const NamedImportAnchor* anchor
) {
	sl::StringRef parentSignature = parentNamespace->getDeclItem()->getLinkId();
	sl::String signature = "IN";
	signature += parentSignature;
	if (!parentSignature.isEmpty())
		signature += '.';

	name.appendFullName(&signature);
	signature += '$';

	if (anchor)
		signature.appendFormat("-%d", anchor->m_linkId);

	return signature;
}

sl::StringRef
NamedImportType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		sl::String string = "import ";

		Namespace* nspace = m_anchor && m_anchor->m_namespace ? m_anchor->m_namespace : m_parentNamespace;
		sl::StringRef parentName = nspace->getDeclItem()->getItemName();
		if (!parentName.isEmpty()) {
			string += parentName;
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
	Namespace* nspace;
	if (m_anchor && m_anchor->m_namespace)
		nspace = m_anchor->m_namespace;
	else if (m_parentNamespace->getNamespaceKind() != NamespaceKind_TemplateDeclaration)
		nspace = m_parentNamespace;
	else {
		nspace = m_module->m_namespaceMgr.getCurrentNamespace();
		ASSERT(nspace->getNamespaceKind() == NamespaceKind_TemplateInstantiation);
	}

	m_actualType = resolveImpl(ModuleItemContext(m_parentUnit, nspace), nspace);
	if (!m_actualType)
		return false;

	applyFixups();
	return true;
}

Type*
NamedImportType::resolveImpl(
	const ModuleItemContext& context,
	Namespace* nspace,
	bool isResolvingRecursion
) {
	FindModuleItemResult findResult = nspace->findItemTraverse(context, m_name);
	if (!findResult.m_result) {
		pushSrcPosError();
		return NULL;
	}

	if (!findResult.m_item) {
		err::setFormatStringError("unresolved import '%s'", getTypeString().sz());
		pushSrcPosError();
		return NULL;
	}

	ModuleItem* item = findResult.m_item;
	ModuleItemKind itemKind = item->getItemKind();
	Type* type;
	switch (itemKind) {
	case ModuleItemKind_Type:
		type = (Type*)item;
		break;

	case ModuleItemKind_Typedef:
		if (m_module->getCompileFlags() & ModuleCompileFlag_KeepTypedefShadow)
			return ((Typedef*)item)->getShadowType();

		if (((Typedef*)item)->isRecursive() &&
			!isResolvingRecursion &&
			(nspace = ((Typedef*)item)->getGrandParentNamespace())
		)
			return resolveImpl(context, nspace, true);

		type = ((Typedef*)item)->getType();
		break;

	case ModuleItemKind_Template:
		type = nspace->findTemplateInstanceType((Template*)item);
		if (type)
			break;

		// else fall through

	default:
		err::setFormatStringError("'%s' is not a type", getTypeString().sz());
		pushSrcPosError();
		return NULL;
	}

	if (!(type->getTypeKindFlags() & TypeKindFlag_Import))
		return type;

	ImportType* importType = (ImportType*)type;
	bool result = importType->ensureResolved();
	if (!result)
		return NULL;

	type = importType->getActualType();
	ASSERT(!(type->getTypeKindFlags() & TypeKindFlag_Import));
	return type;
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
