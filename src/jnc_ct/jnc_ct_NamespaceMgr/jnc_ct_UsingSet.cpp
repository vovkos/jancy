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

FindModuleItemResult
UsingSet::findItem(const sl::StringRef& name) const {
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
) const {
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
UsingSet::addNamespaceImpl(Namespace* nspace) {
	NamespaceKind namespaceKind = nspace->getNamespaceKind();

	switch (namespaceKind) {
	case NamespaceKind_Global:
		m_globalNamespaceArray.append((GlobalNamespace*)nspace);
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

//..............................................................................

} // namespace ct
} // namespace jnc
