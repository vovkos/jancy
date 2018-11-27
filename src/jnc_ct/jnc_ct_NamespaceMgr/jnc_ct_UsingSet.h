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

#include "jnc_Namespace.h"
#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_QualifiedName.h"

namespace jnc {
namespace ct {

class NamespaceMgr;
class Namespace;
class GlobalNamespace;
class ExtensionNamespace;
class NamedType;

//..............................................................................

class UsingSet
{
protected:
	struct ImportNamespace: sl::ListLink
	{
		Namespace* m_anchorNamespace;
		NamespaceKind m_namespaceKind;
		QualifiedName m_name;
	};

protected:
	sl::Array <GlobalNamespace*> m_globalNamespaceArray;
	sl::Array <ExtensionNamespace*> m_extensionNamespaceArray;
	sl::List <ImportNamespace> m_importNamespaceList;

public:
	void
	clear ();

	void
	append (
		NamespaceMgr* importNamespaceMgr,
		const UsingSet* src
		);

	bool
	addNamespace (
		NamespaceMgr* importNamespaceMgr,
		Namespace* anchorNamespace,
		NamespaceKind namespaceKind,
		const QualifiedName& name
		);

	void
	addGlobalNamespace (GlobalNamespace* nspace)
	{
		m_globalNamespaceArray.append (nspace);
	}

	void
	addExtensionNamespace (ExtensionNamespace* nspace)
	{
		m_extensionNamespaceArray.append (nspace);
	}

	bool
	resolveImportNamespaces ();

	ModuleItem*
	findItem (const sl::StringRef& name);

	ModuleItem*
	findExtensionItem (
		NamedType* type,
		const sl::StringRef& name
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
