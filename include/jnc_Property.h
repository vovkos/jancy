// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_PropertyType.h"
#include "jnc_PropertyVerifier.h"
#include "jnc_Function.h"

namespace jnc {

//.............................................................................

enum EProperty
{
	EProperty_Undefined = 0,
	EProperty_Normal,
	EProperty_Thunk,
	EProperty_DataThunk,
	EProperty__Count
};

//.............................................................................

enum EPropertyFlag
{
	EPropertyFlag_Const    = 0x010000,
	EPropertyFlag_Bindable = 0x020000,
	EPropertyFlag_Throws   = 0x040000,
	EPropertyFlag_AutoGet  = 0x100000,
	EPropertyFlag_AutoSet  = 0x200000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CProperty: 
	public CModuleItem,
	public CNamespace
{
	friend class CTypeMgr;
	friend class CDerivableType;
	friend class CClassType;
	friend class CFunctionMgr;
	friend class CParser;

protected:
	EProperty m_PropertyKind;

	CPropertyType* m_pType;

	// construction / destruction / accessors

	CFunction* m_pPreConstructor;
	CFunction* m_pConstructor;
	CFunction* m_pDefaultConstructor;
	CFunction* m_pStaticConstructor;
	CFunction* m_pDestructor;
	CFunction* m_pStaticDestructor;

	CFunction* m_pGetter;
	CFunction* m_pSetter;
	CFunction* m_pBinder;

	// member data is CStructField or CVariable
	
	CModuleItem* m_pOnChanged;
	CModuleItem* m_pAutoGetValue;

	// parent type

	CNamedType* m_pParentType;
	size_t m_ParentClassVTableIndex;

	rtl::CArrayT <CStructField*> m_MemberFieldConstructArray;
	rtl::CArrayT <CStructField*> m_MemberFieldDestructArray;
	rtl::CArrayT <CProperty*> m_MemberPropertyConstructArray;
	rtl::CArrayT <CProperty*> m_MemberPropertyDestructArray;

	// vtable

	rtl::CArrayT <CFunction*> m_VTable;
	CValue m_VTablePtrValue;

	CPropertyVerifier m_Verifier;

public:
	CProperty ();

	EProperty 
	GetPropertyKind ()
	{
		return m_PropertyKind;
	}

	CPropertyType* 
	GetType ()
	{
		return m_pType;
	}

	CFunction* 
	GetPreConstructor ()
	{
		return m_pPreConstructor;
	}

	CFunction* 
	GetConstructor ()
	{
		return m_pConstructor;
	}

	CFunction* 
	GetStaticConstructor ()
	{
		return m_pStaticConstructor;
	}

	CFunction* 
	GetDefaultConstructor ();

	CFunction* 
	GetDestructor ()
	{
		return m_pDestructor;
	}

	CFunction* 
	GetStaticDestructor ()
	{
		return m_pStaticDestructor;
	}

	CFunction* 
	GetGetter ()
	{
		return m_pGetter;
	}

	CFunction* 
	GetSetter ()
	{
		return m_pSetter;
	}

	CFunction* 
	GetBinder ()
	{
		return m_pBinder;
	}

	CModuleItem*
	GetOnChanged ()
	{
		return m_pOnChanged;
	}

	bool
	SetOnChanged (CModuleItem* pItem);

	bool
	CreateOnChanged ();

	CModuleItem*
	GetAutoGetValue ()
	{
		return m_pAutoGetValue;
	}

	bool
	SetAutoGetValue (CModuleItem* pItem); // struct-field or variable

	bool
	CreateAutoGetValue (CType* pType);

	CNamedType* 
	GetParentType ()
	{
		return m_pParentType;
	}

	bool
	IsMember ()
	{
		return m_StorageKind >= EStorage_Member && m_StorageKind <= EStorage_Override;
	}

	bool
	IsVirtual ()
	{
		return m_StorageKind >= EStorage_Abstract && m_StorageKind <= EStorage_Override;
	}

	size_t 
	GetParentClassVTableIndex ()
	{
		return m_ParentClassVTableIndex;
	}

	bool
	Create (CPropertyType* pType);

	void
	ConvertToMemberProperty (CNamedType* pParentType);

	CStructField*
	CreateField (
		const rtl::CString& Name,
		CType* pType,
		size_t BitCount = 0,
		uint_t PtrTypeFlags = 0,
		rtl::CBoxListT <CToken>* pConstructor = NULL,
		rtl::CBoxListT <CToken>* pInitializer = NULL
		);

	CStructField*
	CreateField (
		CType* pType,
		size_t BitCount = 0,
		uint_t PtrTypeFlags = 0
		)
	{
		return CreateField (rtl::CString (), pType, BitCount, PtrTypeFlags);
	}

	bool
	AddMethod (CFunction* pFunction);

	bool
	AddProperty (CProperty* pProperty);

	bool
	CallMemberFieldConstructors (const CValue& ThisValue);

	bool
	CallMemberPropertyConstructors (const CValue& ThisValue);

	bool
	CallMemberDestructors (const CValue& ThisValue);

	CValue
	GetVTablePtrValue ()
	{
		return m_VTablePtrValue;
	}

	virtual 
	bool
	Compile ();

protected:
	virtual
	bool
	CalcLayout ();

	void
	CreateVTablePtr ();

	CValue
	GetAutoAccessorPropertyValue ();

	bool 
	CompileAutoGetter ();

	bool 
	CompileAutoSetter ();

	bool 
	CompileBinder ();

	bool
	CallMemberFieldDestructors (const CValue& ThisValue);

	bool
	CallMemberPropertyDestructors (const CValue& ThisValue);
};

//.............................................................................

} // namespace jnc {
