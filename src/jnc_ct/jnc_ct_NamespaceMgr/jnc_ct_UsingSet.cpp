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
#include "jnc_ct_UsingSet.h"
#include "jnc_ct_ExtensionNamespace.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

void
UsingSet::append(const UsingSet& src) {
	m_globalNamespaceArray.append(src.m_globalNamespaceArray);
	m_extensionNamespaceArray.append(src.m_extensionNamespaceArray);

	sl::ConstIterator<ImportNamespace> it = src.m_importNamespaceList.getHead();
	for (; it; it++) {
		ImportNamespace* importNamespace = new ImportNamespace;
		importNamespace->m_context = it->m_context;
		importNamespace->m_namespaceKind = it->m_namespaceKind;
		importNamespace->m_name.copy(it->m_name);
		m_importNamespaceList.insertTail(importNamespace);
	}
}

FindModuleItemResult
UsingSet::findItem(const sl::StringRef& name) {
	bool result = ensureResolved();
	if (!result)
		return g_errorFindModuleItemResult;

	size_t count = m_globalNamespaceArray.getCount();
	for (size_t i = 0; i < count; i++) {
		FindModuleItemResult findResult = m_globalNamespaceArray[i]->findDirectChildItem(name);
		if (!findResult.m_result || findResult.m_item)
			return findResult;
	}

	return g_nullFindModuleItemResult;
}

FindModuleItemResult
UsingSet::findExtensionItem(
	NamedType* type,
	const sl::StringRef& name
) {
	bool result = ensureResolved();
	if (!result)
		return g_errorFindModuleItemResult;

	size_t count = m_extensionNamespaceArray.getCount();
	for (size_t i = 0; i < count; i++) {
		ExtensionNamespace* nspace = m_extensionNamespaceArray[i];
		result = nspace->ensureNamespaceReady();
		if (!result)
			return g_errorFindModuleItemResult;

		if (nspace->getType()->isEqual(type)) {
			FindModuleItemResult findResult = nspace->findDirectChildItem(name);
			if (!findResult.m_result || findResult.m_item)
				return findResult;
		}
	}

	return g_nullFindModuleItemResult;
}

bool
UsingSet::addNamespace(
	const ModuleItemContext& context,
	NamespaceKind namespaceKind,
	QualifiedName* name
) {
	FindModuleItemResult findResult = context.getParentNamespace()->findItemTraverse(context, *name);
	if (!findResult.m_result)
		return false;

	if (!findResult.m_item) {
		Module* module = context.getParentUnit()->getModule();
		if (module->getCompileState() >= ModuleCompileState_Parsed) {
			err::setFormatStringError("namespace '%s' not found", name->getFullName().sz());
			return false;
		}

		ImportNamespace* importNamespace = new ImportNamespace;
		importNamespace->m_context = context;
		importNamespace->m_namespaceKind = namespaceKind;
		sl::takeOver(&importNamespace->m_name, name);
		m_importNamespaceList.insertTail(importNamespace);
		return true;
	}

	if (findResult.m_item->getItemKind() != ModuleItemKind_Namespace) {
		err::setFormatStringError("'%s' is a %s, not a namespace", name->getFullName ().sz(), getModuleItemKindString(findResult.m_item->getItemKind()));
		return false;
	}

	GlobalNamespace* nspace = (GlobalNamespace*)findResult.m_item;
	if (nspace->getNamespaceKind() != namespaceKind) {
		err::setFormatStringError("'%s' is not %s", name->getFullName ().sz(), getNamespaceKindString(namespaceKind));
		return false;
	}

	switch (namespaceKind) {
	case NamespaceKind_Global:
		m_globalNamespaceArray.append(nspace);
		break;

	case NamespaceKind_Extension:
		m_extensionNamespaceArray.append((ExtensionNamespace*)nspace);
		break;

	default:
		err::setFormatStringError("invalid using: %s", getNamespaceKindString(namespaceKind));
		return false;
	}

	return true;
}

bool
UsingSet::resolve() {
	bool result;

	while (!m_importNamespaceList.isEmpty()) {
		ImportNamespace* importNamespace = m_importNamespaceList.removeHead();
		result = addNamespace(
			importNamespace->m_context,
			importNamespace->m_namespaceKind,
			&importNamespace->m_name
		);

		if (!result)
			return false;

		delete importNamespace;
	}

	return true;
}

//..............................................................................

void
ModuleItemUsingSet::addUsingSet(Namespace* anchorNamespace) {
	for (Namespace* nspace = anchorNamespace; nspace; nspace = nspace->getParentNamespace())
		m_usingSet.append(*nspace->getUsingSet());
}

//..............................................................................

} // namespace ct
} // namespace jnc
