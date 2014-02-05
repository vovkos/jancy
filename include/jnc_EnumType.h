// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ImportType.h"
#include "jnc_NamedType.h"

namespace jnc {

class CEnumType;

//.............................................................................

enum EEnumType
{
	EEnumType_Normal,
	EEnumType_Flag,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum EEnumTypeFlag
{
	EEnumTypeFlag_Exposed = 0x010000,
};

//.............................................................................

class CEnumConst: public CUserModuleItem
{
	friend class CEnumType;
	friend class CNamespace;

protected:
	CEnumType* m_pParentEnumType;
	intptr_t m_Value;
	rtl::CBoxListT <CToken> m_Initializer;

public:
	CEnumConst ()
	{
		m_ItemKind = EModuleItem_EnumConst;
		m_pParentEnumType = NULL;
		m_Value = 0;
	}

	CEnumType*
	GetParentEnumType ()
	{
		return m_pParentEnumType;
	}

	intptr_t
	GetValue ()
	{
		return m_Value;
	}

	rtl::CConstBoxListT <CToken> 
	GetInitializer ()
	{
		return m_Initializer;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CEnumType: public CNamedType
{
	friend class CTypeMgr;
	friend class CParser;
	
protected:
	EEnumType m_EnumTypeKind;

	CType* m_pBaseType;
	CImportType* m_pBaseType_i;
	rtl::CStdListT <CEnumConst> m_ConstList;

public:
	CEnumType ();

	EEnumType 
	GetEnumTypeKind ()
	{
		return m_EnumTypeKind;
	}

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

	rtl::CConstListT <CEnumConst>
	GetConstList ()
	{
		return m_ConstList;
	}

	CEnumConst*
	CreateConst (
		const rtl::CString& Name,
		rtl::CBoxListT <CToken>* pInitializer = NULL
		);

protected:
	virtual 
	void
	PrepareTypeString ()
	{
		m_TypeString.Format (
			(m_Flags & EEnumTypeFlag_Exposed) ? 
				"cenum %s" : 
				"enum %s", 
			m_Tag.cc () // thanks a lot gcc
			);
	}

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
