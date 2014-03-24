// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"

namespace jnc {

struct TObjHdr;

//.............................................................................
	
class CDataPtrType: public CType
{
	friend class CTypeMgr;
	
protected:
	EDataPtrType m_PtrTypeKind;
	CType* m_pTargetType;
	CNamespace* m_pAnchorNamespace; // for dual pointers

public:
	CDataPtrType ();

	EDataPtrType
	GetPtrTypeKind ()
	{
		return m_PtrTypeKind;
	}

	CType*
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

	CDataPtrType*
	GetCheckedPtrType ()
	{
		return !(m_Flags & EPtrTypeFlag_Safe) ?  
			m_pTargetType->GetDataPtrType (m_TypeKind, m_PtrTypeKind, m_Flags | EPtrTypeFlag_Safe) : 
			this;			
	}

	CDataPtrType*
	GetUnCheckedPtrType ()
	{
		return (m_Flags & EPtrTypeFlag_Safe) ?  
			m_pTargetType->GetDataPtrType (m_TypeKind, m_PtrTypeKind, m_Flags & ~EPtrTypeFlag_Safe) : 
			this;			
	}

	CDataPtrType*
	GetUnConstPtrType ()
	{
		return (m_Flags & EPtrTypeFlag_Const) ?  
			m_pTargetType->GetDataPtrType (m_TypeKind, m_PtrTypeKind, m_Flags & ~EPtrTypeFlag_Const) : 
			this;			
	}

	CStructType* 
	GetDataPtrStructType ();

	static
	rtl::CString
	CreateSignature (
		CType* pBaseType,
		EType TypeKind,
		EDataPtrType PtrTypeKind,
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

struct TDataPtrTypeTuple: rtl::TListLink
{
	CStructType* m_pPtrStructType;
	CDataPtrType* m_PtrTypeArray [2] [3] [2] [2] [2]; // ref x kind x const x volatile x safe
};

//.............................................................................

inline
bool 
IsCharPtrType (CType* pType)
{
	return 
		pType->GetTypeKind () == EType_DataPtr &&
		((CDataPtrType*) pType)->GetTargetType ()->GetTypeKind () == EType_Char;
}

//.............................................................................

// structure backing up fat data pointer, e.g.:
// int* p;

struct TDataPtr
{
	void* m_p;
	void* m_pRangeBegin;
	void* m_pRangeEnd;
	TObjHdr* m_pObject;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// structure backing up formatting literal

struct TFmtLiteral
{
	char* m_p;
	size_t m_MaxLength;
	size_t m_Length;
};

//.............................................................................

TDataPtr
StrDup (
	const char* p,
	size_t Length = -1
	);

//.............................................................................

} // namespace jnc {
