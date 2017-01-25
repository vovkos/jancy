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

namespace jnc {
namespace ct {

class ModuleItem;
class GlobalNamespace;
class ExtensionNamespace;
class NamedType;

//..............................................................................

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

	ModuleItem* findItem (const sl::StringRef& name);

	ModuleItem* findExtensionItem (
		NamedType* type,
		const sl::StringRef& name
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
