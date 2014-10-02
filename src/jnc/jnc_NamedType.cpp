#include "pch.h"
#include "jnc_NamedType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

NamedType::NamedType ()
{
	m_namespaceKind = NamespaceKind_Type;
	m_itemDecl = this;
}

FunctionType*
NamedType::getMemberMethodType (
	FunctionType* shortType,
	uint_t thisArgTypeFlags
	)
{
	return m_module->m_typeMgr.getMemberMethodType (this, shortType, thisArgTypeFlags);
}

PropertyType*
NamedType::getMemberPropertyType (PropertyType* shortType)
{
	return m_module->m_typeMgr.getMemberPropertyType (this, shortType);
}

ModuleItem*
NamedType::findItemTraverseImpl (
	const char* name,
	MemberCoord* coord,
	uint_t flags
	)
{
	ModuleItem* item;

	if (!(flags & TraverseKind_NoThis))
	{
		item = findItem (name);
		if (item)
			return item;
	}

	if (!(flags & TraverseKind_NoExtensionNamespace) && m_extensionNamespace)
	{
		item = m_extensionNamespace->findItem (name);
		if (item)
			return item;
	}

	if (!(flags & TraverseKind_NoParentNamespace) && m_parentNamespace)
	{
		item = m_parentNamespace->findItemTraverse (name, coord, flags & ~TraverseKind_NoThis);
		if (item)
			return item;
	}

	return NULL;
}

bool
NamedType::calcLayout ()
{
	if (m_extensionNamespace)
		applyExtensionNamespace ();

	return true;
}

void
NamedType::applyExtensionNamespace ()
{
	ASSERT (m_extensionNamespace);

	size_t count = m_extensionNamespace->getItemCount ();
	for (size_t i = 0; i < count; i++)
	{
		ModuleItem* item = m_extensionNamespace->getItem (i);

		ModuleItemKind itemKind = item->getItemKind ();
		switch (itemKind)
		{
		case ModuleItemKind_Function:
			if (((Function*) item)->getStorageKind () != StorageKind_Static)
				((Function*) item)->convertToMemberMethod (this);
			break;

		case ModuleItemKind_Property:
			if (((Property*) item)->getStorageKind () != StorageKind_Static)
				((Property*) item)->convertToMemberProperty (this);
			break;
		}
	}
}

//.............................................................................

} // namespace jnc {
