// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ModuleItem.h"
#include "jnc_QualifiedName.h"

namespace jnc {

class CNamespace;
class CEnumType;
class CEnumConst;
class CMemberCoord;
class CFunction;

struct TDualPtrTypeTuple;

//.............................................................................

enum ENamespace
{
	ENamespace_Undefined,
	ENamespace_Global,
	ENamespace_Scope,
	ENamespace_Type,
	ENamespace_TypeExtension,
	ENamespace_Property,
	ENamespace_PropertyTemplate,
	ENamespace__Count
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetNamespaceKindString (ENamespace NamespaceKind);

//.............................................................................

enum
{
	ETraverse_NoThis               = 0x01,
	ETraverse_NoExtensionNamespace = 0x02,
	ETraverse_NoBaseType           = 0x04,
	ETraverse_NoParentNamespace    = 0x08,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CNamespace: public CModuleItemDecl
{
	friend class CNamespaceMgr;
	friend class CTypeMgr;
	friend class CParser;

protected:
	ENamespace m_NamespaceKind;

	rtl::CArrayT <CModuleItem*> m_ItemArray;
	rtl::CStringHashTableMapT <CModuleItem*> m_ItemMap;
	rtl::CStringHashTable m_FriendSet;
	rtl::CStringHashTableMapT <TDualPtrTypeTuple*> m_DualPtrTypeTupleMap;

public:
	CNamespace ()
	{
		m_NamespaceKind = ENamespace_Undefined;
	}

	ENamespace
	GetNamespaceKind ()
	{
		return m_NamespaceKind;
	}

	rtl::CString
	CreateQualifiedName (const char* pName);

	rtl::CString
	CreateQualifiedName (const rtl::CString& Name)
	{
		return CreateQualifiedName (Name.cc ());
	}

	rtl::CString
	CreateQualifiedName (const CQualifiedName& Name)
	{
		return CreateQualifiedName (Name.GetFullName ());
	}

	bool
	IsFriend (CNamespace* pNamespace)
	{
		return m_FriendSet.Find (pNamespace->m_QualifiedName) != NULL;
	}

	CModuleItem*
	GetItemByName (const char* pName);

	CClassType*
	GetClassTypeByName (const char* pName)
	{
		return VerifyModuleItemIsClassType (GetItemByName (pName), pName);
	}

	CFunction*
	GetFunctionByName (const char* pName)
	{
		return VerifyModuleItemIsFunction (GetItemByName (pName), pName);
	}

	CProperty*
	GetPropertyByName (const char* pName)
	{
		return VerifyModuleItemIsProperty (GetItemByName (pName), pName);
	}

	CModuleItem*
	FindItem (const char* pName);

	CModuleItem*
	FindItem (const rtl::CString& Name)
	{
		return FindItem (Name.cc ());
	}

	CModuleItem*
	FindItem (const CQualifiedName& Name);

	CModuleItem*
	FindItemTraverse (
		const rtl::CString& Name,
		CMemberCoord* pCoord = NULL,
		uint_t Flags = 0
		)
	{
		return FindItemTraverseImpl (Name, pCoord, Flags);
	}

	CModuleItem*
	FindItemTraverse (
		const char* pName,
		CMemberCoord* pCoord = NULL,
		uint_t Flags = 0
		)
	{
		return FindItemTraverseImpl (pName, pCoord, Flags);
	}

	CModuleItem*
	FindItemTraverse (
		const CQualifiedName& Name,
		CMemberCoord* pCoord = NULL,
		uint_t Flags = 0
		);

	template <typename T>
	bool
	AddItem (T* pItem)
	{
		return AddItem (pItem, pItem);
	}

	bool
	AddFunction (CFunction* pFunction);

	size_t
	GetItemCount ()
	{
		return m_ItemArray.GetCount ();
	}

	CModuleItem*
	GetItem (size_t Index)
	{
		ASSERT (Index < m_ItemArray.GetCount ());
		return m_ItemArray [Index];
	}

	bool
	ExposeEnumConsts (CEnumType* pMember);

protected:
	void
	Clear ();

	bool
	AddItem (
		CModuleItem* pItem,
		CModuleItemDecl* pDecl
		);

	virtual
	CModuleItem*
	FindItemTraverseImpl (
		const char* pName,
		CMemberCoord* pCoord = NULL,
		uint_t Flags = 0
		);
};

//.............................................................................

enum EGlobalNamespaceFlag
{
	EGlobalNamespaceFlag_Sealed = 0x0100
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CGlobalNamespace:
	public CModuleItem,
	public CNamespace
{
	friend class CNamespaceMgr;

public:
	CGlobalNamespace ()
	{
		m_ItemKind = EModuleItem_Namespace;
		m_NamespaceKind = ENamespace_Global;
		m_pItemDecl = this;
	}
};

//.............................................................................

inline
err::CError
SetRedefinitionError (const char* pName)
{
	return err::SetFormatStringError ("redefinition of '%s'", pName);
}

//.............................................................................

} // namespace jnc {
