#include "pch.h"
#include "jnc_NamedType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CNamedType::CNamedType ()
{
	m_NamespaceKind = ENamespace_Type;
	m_pItemDecl = this;
}

CFunctionType*
CNamedType::GetMemberMethodType (
	CFunctionType* pShortType,
	uint_t ThisArgTypeFlags
	)
{
	return m_pModule->m_TypeMgr.GetMemberMethodType (this, pShortType, ThisArgTypeFlags);
}

CPropertyType*
CNamedType::GetMemberPropertyType (CPropertyType* pShortType)
{
	return m_pModule->m_TypeMgr.GetMemberPropertyType (this, pShortType);
}

CModuleItem*
CNamedType::FindItemTraverseImpl (
	const char* pName,
	CMemberCoord* pCoord,
	uint_t Flags
	)
{
	CModuleItem* pItem;

	if (!(Flags & ETraverse_NoThis))
	{
		pItem = FindItem (pName);
		if (pItem)
			return pItem;
	}

	if (!(Flags & ETraverse_NoExtensionNamespace) && m_pExtensionNamespace)
	{
		pItem = m_pExtensionNamespace->FindItem (pName);
		if (pItem)
			return pItem;
	}

	if (!(Flags & ETraverse_NoParentNamespace) && m_pParentNamespace)
	{
		pItem = m_pParentNamespace->FindItemTraverse (pName, pCoord, Flags & ~ETraverse_NoThis);
		if (pItem)
			return pItem;
	}

	return NULL;
}

bool
CNamedType::CalcLayout ()
{
	if (m_pExtensionNamespace)
		ApplyExtensionNamespace ();

	return true;
}

void
CNamedType::ApplyExtensionNamespace ()
{
	ASSERT (m_pExtensionNamespace);

	size_t Count = m_pExtensionNamespace->GetItemCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CModuleItem* pItem = m_pExtensionNamespace->GetItem (i);

		EModuleItem ItemKind = pItem->GetItemKind ();
		switch (ItemKind)
		{
		case EModuleItem_Function:
			if (((CFunction*) pItem)->GetStorageKind () != EStorage_Static)
				((CFunction*) pItem)->ConvertToMemberMethod (this);
			break;

		case EModuleItem_Property:
			if (((CProperty*) pItem)->GetStorageKind () != EStorage_Static)
				((CProperty*) pItem)->ConvertToMemberProperty (this);
			break;
		}
	}
}

//.............................................................................

} // namespace jnc {
