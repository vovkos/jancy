// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_FunctionType.h"

namespace jnc {

struct TIfaceHdr;

//.............................................................................

class CFunctionPtrType: public CType
{
	friend class CTypeMgr;

protected:
	EFunctionPtrType m_PtrTypeKind;
	CFunctionType* m_pTargetType;
	CClassType* m_pMulticastType;
	rtl::CString m_TypeModifierString;

public:
	CFunctionPtrType ();

	EFunctionPtrType
	GetPtrTypeKind ()
	{
		return m_PtrTypeKind;
	}

	CFunctionType*
	GetTargetType ()
	{
		return m_pTargetType;
	}

	bool
	HasClosure ()
	{
		return m_PtrTypeKind == EFunctionPtrType_Normal || m_PtrTypeKind == EFunctionPtrType_Weak;
	}

	CFunctionPtrType*
	GetCheckedPtrType ()
	{
		return !(m_Flags & EPtrTypeFlag_Safe) ?
			m_pTargetType->GetFunctionPtrType (m_TypeKind, m_PtrTypeKind, m_Flags | EPtrTypeFlag_Safe) :
			this;
	}

	CFunctionPtrType*
	GetUnCheckedPtrType ()
	{
		return (m_Flags & EPtrTypeFlag_Safe) ?
			m_pTargetType->GetFunctionPtrType (m_TypeKind, m_PtrTypeKind, m_Flags & ~EPtrTypeFlag_Safe) :
			this;
	}

	CFunctionPtrType*
	GetNormalPtrType ()
	{
		return (m_PtrTypeKind != EFunctionPtrType_Normal) ?
			m_pTargetType->GetFunctionPtrType (EFunctionPtrType_Normal, m_Flags) :
			this;
	}

	CFunctionPtrType*
	GetWeakPtrType ()
	{
		return (m_PtrTypeKind != EFunctionPtrType_Weak) ?
			m_pTargetType->GetFunctionPtrType (EFunctionPtrType_Weak, m_Flags) :
			this;
	}

	CFunctionPtrType*
	GetUnWeakPtrType ()
	{
		return (m_PtrTypeKind == EFunctionPtrType_Weak) ?
			m_pTargetType->GetFunctionPtrType (EFunctionPtrType_Normal, m_Flags) :
			this;
	}

	CClassType*
	GetMulticastType ();

	CStructType*
	GetFunctionPtrStructType ();

	rtl::CString
	GetTypeModifierString ();

	static
	rtl::CString
	CreateSignature (
		CFunctionType* pFunctionType,
		EType TypeKind,
		EFunctionPtrType PtrTypeKind,
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

	virtual
	bool
	CalcLayout ()
	{
		return m_pTargetType->EnsureLayout ();	
	}
};

//.............................................................................

struct TFunctionPtrTypeTuple: rtl::TListLink
{
	CStructType* m_pPtrStructType;
	CFunctionPtrType* m_PtrTypeArray [2] [3] [2]; // ref x kind x checked
};

//.............................................................................

// structure backing up fat function pointers, e.g.:
// int function* pfTest (int, int);
// int function weak* pfTest (int, int);

struct TFunctionPtr
{
	void* m_pf;
	TIfaceHdr* m_pClosure;
};

//.............................................................................

} // namespace jnc {
