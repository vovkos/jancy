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

#include "jnc_ct_Namespace.h"

namespace jnc {
namespace ct {

//..............................................................................

class GlobalNamespace:
	public ModuleItem,
	public Namespace
{
	friend class NamespaceMgr;

public:
	GlobalNamespace()
	{
		m_itemKind = ModuleItemKind_Namespace;
		m_namespaceKind = NamespaceKind_Global;
	}

	virtual
	sl::String
	createDoxyRefId();

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);
};

//..............................................................................

class DynamicLibNamespace: public GlobalNamespace
{
	friend class NamespaceMgr;

protected:
	ClassType* m_dynamicLibType;

public:
	DynamicLibNamespace()
	{
		m_namespaceKind = NamespaceKind_DynamicLib;
		m_dynamicLibType = NULL;
	}

	ClassType* getLibraryType()
	{
		return m_dynamicLibType;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
