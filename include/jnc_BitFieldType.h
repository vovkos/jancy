// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ImportType.h"

namespace jnc {

//.............................................................................

class CBitFieldType: public CType
{
	friend class CTypeMgr;

protected:
	CType* m_pBaseType;
	CImportType* m_pBaseType_i;
	size_t m_BitOffset;
	size_t m_BitCount;

public:
	CBitFieldType ();

	CType*
	GetBaseType ()
	{
		return m_pBaseType;
	}

	CImportType*
	GetBaseType_i ()
	{
		return m_pBaseType_i;
	}

	size_t
	GetBitOffset ()
	{
		return m_BitOffset;
	}

	size_t
	GetBitCount ()
	{
		return m_BitCount;
	}

	static
	rtl::CString
	CreateSignature (
		CType* pBaseType,
		size_t BitOffset,
		size_t BitCount
		)
	{
		return rtl::CString::Format_s (
			"B%s:%d:%d",
			pBaseType->GetSignature ().cc (), // thanks a lot gcc
			BitOffset,
			BitOffset + BitCount
			);
	}

protected:
	virtual
	void
	PrepareTypeString ();

	virtual
	void
	PrepareLlvmType ()
	{
		m_pLlvmType = m_pBaseType->GetLlvmType ();
	}

	virtual
	void
	PrepareLlvmDiType ()
	{
		m_LlvmDiType = m_pBaseType->GetLlvmDiType ();
	}

	virtual
	bool
	CalcLayout ();
};

//.............................................................................

} // namespace jnc {
