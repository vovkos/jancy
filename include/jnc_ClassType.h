// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_DerivableType.h"
#include "jnc_StructType.h"
#include "jnc_Function.h"
#include "jnc_Property.h"
#include "jnc_UnOp.h"
#include "jnc_BinOp.h"

namespace jnc {

class CClassPtrType;
struct TClassPtrTypeTuple;
struct TObjHdr;
struct TIfaceHdr;

//.............................................................................

enum EClassType
{
	EClassType_Normal = 0,
	EClassType_StdObject, // EStdType_Object
	EClassType_Box,
	EClassType_Multicast,
	EClassType_McSnapshot,
	EClassType_Reactor,
	EClassType_ReactorIface,
	EClassType_FunctionClosure,
	EClassType_PropertyClosure,
	EClassType_DataClosure,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum EClassTypeFlag
{
	EClassTypeFlag_Abstract = 0x010000,
	EClassTypeFlag_Opaque   = 0x020000,
};

//.............................................................................

enum EClassPtrType
{
	EClassPtrType_Normal = 0,
	EClassPtrType_Weak,
	EClassPtrType__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetClassPtrTypeKindString (EClassPtrType PtrTypeKind);

//............................................................................

class CClassType: public CDerivableType
{
	friend class CTypeMgr;
	friend class CParser;
	friend class CProperty;

protected:
	EClassType m_ClassTypeKind;

	CStructType* m_pIfaceStructType;
	CStructType* m_pClassStructType;

	CFunction* m_pPrimer;
	CFunction* m_pDestructor;
	CFunction* m_pOperatorNew;

	// prime arrays

	rtl::CArrayT <CBaseTypeSlot*> m_BaseTypePrimeArray;
	rtl::CArrayT <CStructField*> m_ClassMemberFieldArray;

	// destruct arrays

	rtl::CArrayT <CClassType*> m_BaseTypeDestructArray;
	rtl::CArrayT <CStructField*> m_MemberFieldDestructArray;
	rtl::CArrayT <CProperty*> m_MemberPropertyDestructArray;

	// vtable

	rtl::CArrayT <CFunction*> m_VirtualMethodArray;
	rtl::CArrayT <CFunction*> m_OverrideMethodArray;
	rtl::CArrayT <CProperty*> m_VirtualPropertyArray;

	CStructType* m_pVTableStructType;
	rtl::CArrayT <CFunction*> m_VTable;
	CValue m_VTablePtrValue;

	TClassPtrTypeTuple* m_pClassPtrTypeTuple;

public:
	CClassType ();

	bool
	IsCreatable ()
	{
		return
			m_ClassTypeKind != EClassType_StdObject &&
			!(m_Flags & (EClassTypeFlag_Abstract | EClassTypeFlag_Opaque));
	}

	EClassType
	GetClassTypeKind ()
	{
		return m_ClassTypeKind;
	}

	CStructType*
	GetIfaceStructType ()
	{
		ASSERT (m_pIfaceStructType);
		return m_pIfaceStructType;
	}

	CStructType*
	GetClassStructType ()
	{
		ASSERT (m_pClassStructType);
		return m_pClassStructType;
	}

	CClassPtrType*
	GetClassPtrType (
		CNamespace* pAnchorNamespace,
		EType TypeKind,
		EClassPtrType PtrTypeKind = EClassPtrType_Normal,
		uint_t Flags = 0
		);

	CClassPtrType*
	GetClassPtrType (
		EType TypeKind,
		EClassPtrType PtrTypeKind = EClassPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetClassPtrType (NULL, TypeKind, PtrTypeKind, Flags);
	}

	CClassPtrType*
	GetClassPtrType (
		EClassPtrType PtrTypeKind = EClassPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetClassPtrType (EType_ClassPtr, PtrTypeKind, Flags);
	}

	virtual
	CType*
	GetThisArgType (uint_t PtrTypeFlags)
	{
		return (CType*) GetClassPtrType (EClassPtrType_Normal, PtrTypeFlags);
	}

	CFunction*
	GetPrimer ()
	{
		return m_pPrimer;
	}

	CFunction*
	GetDestructor ()
	{
		return m_pDestructor;
	}

	CFunction*
	GetOperatorNew ()
	{
		return m_pOperatorNew;
	}

	rtl::CConstListT <CStructField>
	GetFieldList ()
	{
		return m_pIfaceStructType->GetFieldList ();
	}

	virtual
	CStructField*
	GetFieldByIndex (size_t Index);

	virtual
	bool
	AddMethod (CFunction* pFunction);

	virtual
	bool
	AddProperty (CProperty* pProperty);

	bool
	HasVTable ()
	{
		return !m_VTable.IsEmpty ();
	}

	rtl::CArrayT <CBaseTypeSlot*>
	GetBaseTypePrimeArray ()
	{
		return m_BaseTypePrimeArray;
	}

	rtl::CArrayT <CStructField*>
	GetClassMemberFieldArray ()
	{
		return m_ClassMemberFieldArray;
	}

	rtl::CArrayT <CFunction*>
	GetVirtualMethodArray ()
	{
		return m_VirtualMethodArray;
	}

	rtl::CArrayT <CProperty*>
	GetVirtualPropertyArray ()
	{
		return m_VirtualPropertyArray;
	}

	CStructType*
	GetVTableStructType ()
	{
		ASSERT (m_pVTableStructType);
		return m_pVTableStructType;
	}

	CValue
	GetVTablePtrValue ()
	{
		return m_VTablePtrValue;
	}

	virtual
	bool
	Compile ();

	virtual
	void
	GcMark (
		CRuntime* pRuntime,
		void* p
		);

	bool
	CallBaseTypeDestructors (const CValue& ThisValue);

	bool
	CallMemberFieldDestructors (const CValue& ThisValue);

	bool
	CallMemberPropertyDestructors (const CValue& ThisValue);

protected:
	void
	EnumGcRootsImpl (
		CRuntime* pRuntime,
		TIfaceHdr* pInterface
		);

	virtual
	CStructField*
	CreateFieldImpl (
		const rtl::CString& Name,
		CType* pType,
		size_t BitCount = 0,
		uint_t PtrTypeFlags = 0,
		rtl::CBoxListT <CToken>* pConstructor = NULL,
		rtl::CBoxListT <CToken>* pInitializer = NULL
		);

	virtual
	void
	PrepareTypeString ()
	{
		m_TypeString.Format ("class %s", m_Tag.cc ());
	}

	virtual
	void
	PrepareLlvmType ()
	{
		m_pLlvmType = GetClassStructType ()->GetLlvmType ();
	}

	virtual
	void
	PrepareLlvmDiType ()
	{
		m_LlvmDiType = GetClassStructType ()->GetLlvmDiType ();
	}

	virtual
	bool
	CalcLayout ();

	void
	AddVirtualFunction (CFunction* pFunction);

	bool
	OverrideVirtualFunction (CFunction* pFunction);

	void
	CreateVTablePtr ();

	void
	CreatePrimer ();

	void
	PrimeObject (
		CClassType* pClassType,
		const CValue& OpValue,
		const CValue& ScopeLevelValue,
		const CValue& RootValue,
		const CValue& FlagsValue
		);

	void
	PrimeInterface (
		CClassType* pClassType,
		const CValue& OpValue,
		const CValue& VTableValue,
		const CValue& ObjectValue,
		const CValue& ScopeLevelValue,
		const CValue& RootValue,
		const CValue& FlagsValue
		);

	bool
	CompileDefaultPreConstructor ();

	bool
	CompileDefaultDestructor ();

	bool
	CompilePrimer ();
};

//.............................................................................

inline
bool
IsClassType (
	CType* pType,
	EClassType ClassTypeKind
	)
{
	return
		pType->GetTypeKind () == EType_Class &&
		((CClassType*) pType)->GetClassTypeKind () == ClassTypeKind;
}

inline
bool
IsOpaqueClassType (CType* pType)
{
	return
		pType->GetTypeKind () == EType_Class &&
		(pType->GetFlags () & EClassTypeFlag_Opaque);
}

//.............................................................................

// header of class interface

struct TIfaceHdr
{
	void* m_pVTable;
	TObjHdr* m_pObject; // back pointer to master header

	// followed by parents, then by interface data fields
};

// TIfaceHdrT is a simple trick for allowing multiple inheritance in implementation classes

template <typename T>
struct TIfaceHdrT: TIfaceHdr
{
}; 

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// interface with master header

template <typename T>
class CObjBoxT:
	public TObjHdr,
	public T
{
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef
void
FObject_Prime (
	TObjHdr* pObject,
	size_t ScopeLevel,
	TObjHdr* pRoot,
	uintptr_t Flags
	);

typedef
void
FObject_Construct (TIfaceHdr* pInterface);

typedef
void
FObject_Destruct (TIfaceHdr* pInterface);

//.............................................................................

} // namespace jnc {
