#include "pch.h"
#include "jnc_Namespace.h"
#include "jnc_Module.h"
#include "jnc_StructType.h"
#include "jnc_ClassType.h"

namespace jnc {

//.............................................................................

CNamespace*
GetItemNamespace (CModuleItem* pItem)
{
	EModuleItem ItemKind = pItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Namespace:
		return (CGlobalNamespace*) pItem;

	case EModuleItem_Property:
		return (CProperty*) pItem;

	case EModuleItem_Type:
		break;

	case EModuleItem_Typedef:
		pItem = ((CTypedef*) pItem)->GetType ();
		break;

	default:
		return NULL;
	}

	CType* pType = (CType*) pItem;
	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_Enum:
	case EType_Struct:
	case EType_Union:
	case EType_Class:
		return ((CNamedType*) pType);

	default:
		return NULL;
	}
}

//.............................................................................

const char*
GetNamespaceKindString (ENamespace NamespaceKind)
{
	static const char* StringTable [ENamespace__Count] =
	{
		"undefined-namespace-kind",  // ENamespace_Undefined = 0,
		"global",                    // ENamespace_Global,
		"scope",                     // ENamespace_Scope,
		"named-type",                // ENamespace_Type,
		"named-type-extension",      // ENamespace_TypeExtension,
		"property",                  // ENamespace_Property,
		"property-template",         // ENamespace_PropertyTemplate,
	};

	return (size_t) NamespaceKind < ENamespace__Count ?
		StringTable [NamespaceKind] :
		StringTable [ENamespace_Undefined];
}

//.............................................................................

void
CNamespace::Clear ()
{
	m_ItemArray.Clear ();
	m_ItemMap.Clear ();
}

rtl::CString
CNamespace::CreateQualifiedName (const char* pName)
{
	if (m_QualifiedName.IsEmpty ())
		return pName;

	rtl::CString QualifiedName = m_QualifiedName;
	QualifiedName.Append ('.');
	QualifiedName.Append (pName);
	return QualifiedName;
}

CModuleItem*
CNamespace::GetItemByName (const char* pName)
{
	CModuleItem* pItem;

	if (!strchr (pName, '.'))
	{
		pItem = FindItem (pName);
	}
	else
	{
		CQualifiedName Name;
		Name.Parse (pName);
		pItem = FindItem (Name);
	}

	if (!pItem)
	{
		err::SetFormatStringError ("'%s' not found", pName);
		return NULL;
	}

	return pItem;
}

CModuleItem*
CNamespace::FindItem (const char* pName)
{
	rtl::CStringHashTableMapIteratorT <CModuleItem*> It = m_ItemMap.Find (pName);
	if (!It)
		return NULL;

	CModuleItem* pItem = It->m_Value;
	if (pItem->GetItemKind () != EModuleItem_Lazy)
		return pItem;

	CLazyModuleItem* pLazyItem = (CLazyModuleItem*) pItem;
	ASSERT (!(pLazyItem->m_Flags & ELazyModuleItemFlag_Touched));

	pLazyItem->m_Flags |= ELazyModuleItemFlag_Touched;
	pItem = pLazyItem->GetActualItem ();
	m_ItemArray.Append (pItem);
	It->m_Value = pItem;
	return pItem;
}

CModuleItem*
CNamespace::FindItem (const CQualifiedName& Name)
{
	CModuleItem* pItem = FindItem (Name.GetFirstName ());
	if (!pItem)
		return NULL;

	rtl::CBoxIteratorT <rtl::CString> NameIt = Name.GetNameList ().GetHead ();
	for (; NameIt; NameIt++)
	{
		CNamespace* pNamespace = GetItemNamespace (pItem);
		if (!pNamespace)
			return NULL;

		pItem = pNamespace->FindItem (*NameIt);
		if (!pItem)
			return NULL;
	}

	return pItem;
}

CModuleItem*
CNamespace::FindItemTraverse (
	const CQualifiedName& Name,
	CMemberCoord* pCoord,
	uint_t Flags
	)
{
	CModuleItem* pItem = FindItemTraverse (Name.GetFirstName (), pCoord, Flags);
	if (!pItem)
		return NULL;

	rtl::CBoxIteratorT <rtl::CString> NameIt = Name.GetNameList ().GetHead ();
	for (; NameIt; NameIt++)
	{
		CNamespace* pNamespace = GetItemNamespace (pItem);
		if (!pNamespace)
			return NULL;

		pItem = pNamespace->FindItem (*NameIt);
		if (!pItem)
			return NULL;
	}

	return pItem;
}

CModuleItem*
CNamespace::FindItemTraverseImpl (
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

	if (!(Flags & ETraverse_NoParentNamespace) && m_pParentNamespace)
	{
		pItem = m_pParentNamespace->FindItemTraverse (pName, pCoord, Flags & ~ETraverse_NoThis);
		if (pItem)
			return pItem;
	}

	return NULL;
}

bool
CNamespace::AddItem (
	CModuleItem* pItem,
	CModuleItemDecl* pDecl
	)
{
	rtl::CStringHashTableMapIteratorT <CModuleItem*> It = m_ItemMap.Goto (pDecl->m_Name);
	if (It->m_Value)
	{
		SetRedefinitionError (pDecl->m_Name);
		return false;
	}

	if (pItem->GetItemKind () != EModuleItem_Lazy)
		m_ItemArray.Append (pItem);

	It->m_Value = pItem;
	return true;
}

bool
CNamespace::AddFunction (CFunction* pFunction)
{
	CModuleItem* pOldItem = FindItem (pFunction->GetName ());
	if (!pOldItem)
		return AddItem (pFunction);

	if (pOldItem->GetItemKind () != EModuleItem_Function)
	{
		SetRedefinitionError (pFunction->GetName ());
		return false;
	}

	CFunction* pFunctionOverload = (CFunction*) pOldItem;
	return pFunctionOverload->AddOverload (pFunction);
}

bool
CNamespace::ExposeEnumConsts (CEnumType* pType)
{
	bool Result;

	rtl::CIteratorT <CEnumConst> Const = pType->GetConstList ().GetHead ();
	for (; Const; Const++)
	{
		Result = AddItem (*Const);
		if (!Result)
			return false;
	}

	return true;
}

//.............................................................................

} // namespace jnc {
