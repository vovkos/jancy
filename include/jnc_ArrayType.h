// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ImportType.h"
#include "jnc_DataPtrType.h"

namespace jnc {

//.............................................................................

enum EArrayTypeFlag
{
	EArrayTypeFlag_AutoSize = 0x010000,
};

//.............................................................................

class CArrayType: public CType
{
	friend class CTypeMgr;
	friend class CParser;

protected:
	CType* m_pElementType;
	CImportType* m_pElementType_i;
	CType* m_pRootType;
	size_t m_ElementCount;

	rtl::CBoxListT <CToken> m_ElementCountInitializer;
	CUnit* m_pParentUnit;
	CNamespace* m_pParentNamespace;

public:
	CArrayType ();

	CType*
	GetElementType ()
	{
		return m_pElementType;
	}

	CImportType*
	GetElementType_i ()
	{
		return m_pElementType_i;
	}

	CType*
	GetRootType ();

	size_t
	GetElementCount ()
	{
		return m_ElementCount;
	}

	rtl::CConstBoxListT <CToken>
	GetElementCountInitializer ()
	{
		return m_ElementCountInitializer;
	}

	static
	rtl::CString
	CreateSignature (
		CType* pElementType,
		size_t ElementCount
		)
	{
		return rtl::CString::Format_s (
			"A%d%s",
			ElementCount,
			pElementType->GetSignature ().cc () // thanks a lot gcc
			);
	}

	virtual
	void
	GcMark (
		CRuntime* pRuntime,
		void* p
		);


protected:
	virtual
	bool
	CalcLayout ();

	virtual
	void
	PrepareTypeString ();

	virtual
	void
	PrepareLlvmType ()
	{
		ASSERT (m_ElementCount != -1);
		m_pLlvmType = llvm::ArrayType::get (m_pElementType->GetLlvmType (), m_ElementCount);
	}

	virtual
	void
	PrepareLlvmDiType ();
};

//.............................................................................

inline
bool
IsAutoSizeArrayType (CType* pType)
{
	return
		pType->GetTypeKind () == EType_Array &&
		(pType->GetFlags () & EArrayTypeFlag_AutoSize) != 0;
}

inline
bool
IsCharArrayType (CType* pType)
{
	return
		pType->GetTypeKind () == EType_Array &&
		((CArrayType*) pType)->GetElementType ()->GetTypeKind () == EType_Char;
}

inline
bool
IsCharArrayRefType (CType* pType)
{
	return
		pType->GetTypeKind () == EType_DataRef &&
		IsCharArrayType (((CDataPtrType*) pType)->GetTargetType ());
}

//.............................................................................

} // namespace jnc {
