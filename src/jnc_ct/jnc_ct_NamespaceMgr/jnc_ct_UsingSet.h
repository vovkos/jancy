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

#include "jnc_ct_NamespaceSet.h"

namespace jnc {
namespace ct {

class GlobalNamespace;
class ExtensionNamespace;

//..............................................................................

class UsingSet: public NamespaceSet {
protected:
	sl::Array<GlobalNamespace*> m_globalNamespaceArray;
	sl::Array<ExtensionNamespace*> m_extensionNamespaceArray;

public:
	void
	clear();

	void
	append(const UsingSet& src);

	void
	addGlobalNamespace(GlobalNamespace* nspace) {
		m_globalNamespaceArray.append(nspace);
	}

	void
	addExtensionNamespace(ExtensionNamespace* nspace) {
		m_extensionNamespaceArray.append(nspace);
	}

	FindModuleItemResult
	findItem(const sl::StringRef& name) const;

	FindModuleItemResult
	findExtensionItem(
		NamedType* type,
		const sl::StringRef& name
	) const;

protected:
	virtual
	bool
	addNamespaceImpl(
		ModuleItem* item,
		NamespaceKind namespaceKind
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
UsingSet::clear() {
	NamespaceSet::clear();
	m_globalNamespaceArray.clear();
	m_extensionNamespaceArray.clear();
}

inline
void
UsingSet::append(const UsingSet& src) {
	NamespaceSet::append(src);
	m_globalNamespaceArray.append(src.m_globalNamespaceArray);
	m_extensionNamespaceArray.append(src.m_extensionNamespaceArray);
}

//..............................................................................

class ModuleItemUsingSet {
	friend class Parser;

protected:
	UsingSet m_usingSet;

public:
	const UsingSet&
	getUsingSet() {
		return m_usingSet;
	}

	void
	addUsingSet(const UsingSet& usingSet) {
		m_usingSet.append(usingSet);
	}

	void
	addUsingSet(Namespace* anchorNamespace);
};

//..............................................................................

} // namespace ct
} // namespace jnc
