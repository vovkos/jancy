// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_PropertyType.h"
#include "jnc_FunctionPtrType.h"

namespace jnc {

struct TIfaceHdr;

//.............................................................................

class CPropertyPtrType: public CType
{
	friend class CTypeMgr;

protected:
	EPropertyPtrType m_PtrTypeKind;
	CPropertyType* m_pTargetType;
	CNamespace* m_pAnchorNamespace; // for dual pointers

public:
	CPropertyPtrType ();

	EPropertyPtrType
	GetPtrTypeKind ()
	{
		return m_PtrTypeKind;
	}

	CPropertyType*
	GetTargetType ()
	{
		return m_pTargetType;
	}

	CNamespace*
	GetAnchorNamespace ()
	{
		return m_pAnchorNamespace;
	}

	bool
	IsConstPtrType ();

	bool
	HasClosure ()
	{
		return m_PtrTypeKind == EPropertyPtrType_Normal || m_PtrTypeKind == EPropertyPtrType_Weak;
	}

	CPropertyPtrType*
	GetCheckedPtrType ()
	{
		return !(m_Flags & EPtrTypeFlag_Safe) ?
			m_pTargetType->GetPropertyPtrType (m_TypeKind, m_PtrTypeKind, m_Flags | EPtrTypeFlag_Safe) :
			this;
	}

	CPropertyPtrType*
	GetUnCheckedPtrType ()
	{
		return (m_Flags & EPtrTypeFlag_Safe) ?
			m_pTargetType->GetPropertyPtrType (m_TypeKind, m_PtrTypeKind, m_Flags & ~EPtrTypeFlag_Safe) :
			this;
	}

	CPropertyPtrType*
	GetNormalPtrType ()
	{
		return (m_PtrTypeKind != EPropertyPtrType_Normal) ?
			m_pTargetType->GetPropertyPtrType (EPropertyPtrType_Normal, m_Flags) :
			this;
	}

	CPropertyPtrType*
	GetWeakPtrType ()
	{
		return (m_PtrTypeKind != EPropertyPtrType_Weak) ?
			m_pTargetType->GetPropertyPtrType (EPropertyPtrType_Weak, m_Flags) :
			this;
	}

	CPropertyPtrType*
	GetUnWeakPtrType ()
	{
		return (m_PtrTypeKind == EPropertyPtrType_Weak) ?
			m_pTargetType->GetPropertyPtrType (EPropertyPtrType_Normal, m_Flags) :
			this;
	}

	CStructType*
	GetPropertyPtrStructType ();

	static
	rtl::CString
	CreateSignature (
		CPropertyType* pPropertyType,
		EType TypeKind,
		EPropertyPtrType PtrTypeKind,
		uint_t Flags
		);

	virtual
	void
	GcMark (
		CRuntime* pRuntime,
		void* p
		);

protected:
	virtual
	void
	PrepareTypeString ();

	virtual
	void
	PrepareLlvmType ();

	virtual
	void
	PrepareLlvmDiType ();
};

//.............................................................................

struct TPropertyPtrTypeTuple: rtl::TListLink
{
	CStructType* m_pPtrStructType;
	CPropertyPtrType* m_PtrTypeArray [2] [3] [3]; // ref x kind x unsafe / checked
};

//.............................................................................

inline
bool
IsBindableType (CType* pType)
{
	return
		pType->GetTypeKind () == EType_PropertyRef &&
		(((CPropertyPtrType*) pType)->GetTargetType ()->GetFlags () & EPropertyTypeFlag_Bindable) != 0;
}

//.............................................................................

// structure backing up property pointers, e.g.:
// int property* pxTest;
// int property weak* pxTest;

struct TPropertyPtr
{
	void** m_pVTable;
	TIfaceHdr* m_pClosure;
};

//.............................................................................

} // namespace jnc {
