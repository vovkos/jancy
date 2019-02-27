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
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

void
UsingSet::clear()
{
	m_globalNamespaceArray.clear();
	m_extensionNamespaceArray.clear();
	m_importNamespaceList.clear();
}

void
UsingSet::append(
	NamespaceMgr* importNamespaceMgr,
	const UsingSet* src
	)
{
	m_globalNamespaceArray.append(src->m_globalNamespaceArray);
	m_extensionNamespaceArray.append(src->m_extensionNamespaceArray);

	if (!src->m_importNamespaceList.isEmpty())
	{
		ASSERT(importNamespaceMgr);
		if (m_importNamespaceList.isEmpty())
			importNamespaceMgr->addImportUsingSet(this);

		sl::ConstIterator<ImportNamespace> it = src->m_importNamespaceList.getHead();
		for (; it; it++)
		{
			ImportNamespace* importNamespace = AXL_MEM_NEW(ImportNamespace);
			*importNamespace = **it;
			m_importNamespaceList.insertTail(importNamespace);
		}
	}
}

ModuleItem* UsingSet::findItem(const sl::StringRef& name)
{
	size_t count = m_globalNamespaceArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		ModuleItem* item = m_globalNamespaceArray[i]->findItem(name);
		if (item)
			return item;
	}

	return NULL;
}

ModuleItem* UsingSet::findExtensionItem(
	NamedType* type,
	const sl::StringRef& name
	)
{
	size_t count = m_extensionNamespaceArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		ExtensionNamespace* nspace = m_extensionNamespaceArray[i];
		if (nspace->getType()->cmp(type) == 0)
		{
			ModuleItem* item = nspace->findItem(name);
			if (item)
				return item;
		}
	}

	return NULL;
}

bool
UsingSet::addNamespace(
	NamespaceMgr* importNamespaceMgr,
	Namespace* anchorNamespace,
	NamespaceKind namespaceKind,
	const QualifiedName& name
	)
{
	ModuleItem* item = anchorNamespace->findItemTraverse(name);
	if (!item)
	{
		if (!importNamespaceMgr)
		{
			err::setFormatStringError("undeclared identifier '%s'", name.getFullName ().sz ());
			return false;
		}

		if (m_importNamespaceList.isEmpty())
			importNamespaceMgr->addImportUsingSet(this);

		ImportNamespace* importNamespace = AXL_MEM_NEW(ImportNamespace);
		importNamespace->m_anchorNamespace = anchorNamespace;
		importNamespace->m_namespaceKind = namespaceKind;
		importNamespace->m_name = name;
		m_importNamespaceList.insertTail(importNamespace);
		return true;
	}

	if (item->getItemKind() != ModuleItemKind_Namespace)
	{
		err::setFormatStringError("'%s' is not a namespace", name.getFullName ().sz ());
		return false;
	}

	GlobalNamespace* nspace = (GlobalNamespace*)item;
	if (nspace->getNamespaceKind() != namespaceKind)
	{
		err::setFormatStringError("'%s' is not %s", name.getFullName ().sz (), getNamespaceKindString (namespaceKind));
		return false;
	}

	switch(namespaceKind)
	{
	case NamespaceKind_Global:
		m_globalNamespaceArray.append(nspace);
		break;

	case NamespaceKind_Extension:
		m_extensionNamespaceArray.append((ExtensionNamespace*)nspace);
		break;

	default:
		err::setFormatStringError("invalid using: %s", getNamespaceKindString (namespaceKind));
		return false;
	}

	return true;
}

bool
UsingSet::resolveImportNamespaces()
{
	while (!m_importNamespaceList.isEmpty())
	{
		ImportNamespace* importNamespace = m_importNamespaceList.removeHead();
		addNamespace(
			NULL,
			importNamespace->m_anchorNamespace,
			importNamespace->m_namespaceKind,
			importNamespace->m_name
			);

		AXL_MEM_DELETE(importNamespace);
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
