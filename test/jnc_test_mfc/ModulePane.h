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

//..............................................................................

class CModulePane : public CDockablePane
{
protected:
	CTreeCtrl m_TreeCtrl;

public:
	bool
	Build (jnc::CModule* pModule);

	void
	Clear ();

protected:
	bool
	AddItemAttributes (
		HTREEITEM hParent,
		jnc::CModuleItem* pItem
		);

	void
	AddNamespace (
		HTREEITEM hParent,
		jnc::CGlobalNamespace* pNamespace
		);

	void
	AddItem (
		HTREEITEM hParent,
		jnc::CModuleItem* pItem
		);

	void
	AddType (
		HTREEITEM hParent,
		jnc::CType* pType
		);

	void
	AddTypedef (
		HTREEITEM hParent,
		jnc::CTypedef* pTypedef
		);

	void
	AddVariable (
		HTREEITEM hParent,
		jnc::CVariable* pVariable
		);

	void
	AddEnumConst (
		HTREEITEM hParent,
		jnc::CEnumConst* pMember
		);

	void
	AddValue (
		HTREEITEM hParent,
		const char* pName,
		jnc::CType* pType,
		jnc::CModuleItem* pItem
		);

	void
	AddFunction (
		HTREEITEM hParent,
		jnc::CFunction* pFunction
		);

	void
	AddFunctionImpl (
		HTREEITEM hParent,
		jnc::CFunction* pFunction
		);

	void
	AddProperty (
		HTREEITEM hParent,
		jnc::CProperty* pProperty
		);

	void
	AddEnumTypeMembers (
		HTREEITEM hParent,
		jnc::CEnumType* pType
		);

	void
	AddStructTypeMembers (
		HTREEITEM hParent,
		jnc::CStructType* pType
		);

	void
	AddStructField (
		HTREEITEM hParent,
		jnc::CStructField* pMember
		)
	{
		AddValue (hParent, pMember->GetName (), pMember->GetType (), pMember);
	}

	void
	AddUnionTypeMembers (
		HTREEITEM hParent,
		jnc::CUnionType* pType
		);

	void
	AddClassTypeMembers (
		HTREEITEM hParent,
		jnc::CClassType* pType
		);

	void
	AddPropertyTypeMembers (
		HTREEITEM hParent,
		jnc::CPropertyType* pType
		);

	rtl::CString
	GetItemTip (jnc::CModuleItem* pItem);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblClk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

//..............................................................................
