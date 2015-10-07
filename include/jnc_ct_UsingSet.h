// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {
namespace ct {

class ModuleItem;
class GlobalNamespace;
class ExtensionNamespace;
class NamedType;

//.............................................................................

struct UsingSet
{
protected:
	sl::Array <GlobalNamespace*> m_globalNamespaceArray;
	sl::Array <ExtensionNamespace*> m_extensionNamespaceArray;

public:
	void clear ()
	{
		m_globalNamespaceArray.clear ();
		m_extensionNamespaceArray.clear ();
	}

	void append (const UsingSet* src)
	{
		m_globalNamespaceArray.append (src->m_globalNamespaceArray);
		m_extensionNamespaceArray.append (src->m_extensionNamespaceArray);
	}

	void addGlobalNamespace (GlobalNamespace* nspace)
	{
		m_globalNamespaceArray.append (nspace);
	}

	void addExtensionNamespace (ExtensionNamespace* nspace)
	{
		m_extensionNamespaceArray.append (nspace);
	}

	ModuleItem* findItem (const char* name);
	
	ModuleItem* findExtensionItem (
		NamedType* type,
		const char* name
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
