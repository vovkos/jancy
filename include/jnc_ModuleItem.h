// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Lexer.h"

namespace jnc {

class CModule;
class CUnit;
class CNamespace;
class CAttributeBlock;
class CClassType;
class CFunction;
class CProperty;

//.............................................................................

enum EModuleItem
{
	EModuleItem_Undefined = 0,
	EModuleItem_Namespace,
	EModuleItem_Scope,
	EModuleItem_Type,
	EModuleItem_Typedef,
	EModuleItem_Alias,
	EModuleItem_Const,
	EModuleItem_Variable,
	EModuleItem_FunctionArg,
	EModuleItem_Function,
	EModuleItem_Property,
	EModuleItem_PropertyTemplate,
	EModuleItem_EnumConst,
	EModuleItem_StructField,
	EModuleItem_BaseTypeSlot,
	EModuleItem_Orphan,
	EModuleItem_Lazy,
	EModuleItem__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetModuleItemKindString (EModuleItem ItemKind);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum EModuleItemFlag
{
	EModuleItemFlag_User         = 0x01,
	EModuleItemFlag_NeedLayout   = 0x02,
	EModuleItemFlag_NeedCompile  = 0x04,
	EModuleItemFlag_InCalcLayout = 0x10,
	EModuleItemFlag_LayoutReady  = 0x20,
	EModuleItemFlag_Constructed  = 0x40, // fields, properties, base type slots
};


//.............................................................................

enum EStorage
{
	EStorage_Undefined = 0,
	EStorage_Alias,
	EStorage_Typedef,
	EStorage_Static,
	EStorage_Thread,
	EStorage_Stack,
	EStorage_Heap,
	EStorage_UHeap,
	EStorage_Member,
	EStorage_Abstract,
	EStorage_Virtual,
	EStorage_Override,
	EStorage_Mutable,
	EStorage_Nullable,
	EStorage_This,
	EStorage__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetStorageKindString (EStorage StorageKind);

//.............................................................................

enum EAccess
{
	EAccess_Undefined = 0,
	EAccess_Public,
	EAccess_Protected,
	EAccess__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetAccessKindString (EAccess AccessKind);

//.............................................................................

class CModuleItemPos
{
	friend class CParser;

protected:
	CUnit* m_pParentUnit;
	CToken::CPos m_Pos;

public:
	CModuleItemPos ()
	{
		m_pParentUnit = NULL;
	}

	CUnit*
	GetParentUnit ()
	{
		return m_pParentUnit;
	}

	const CToken::CPos*
	GetPos ()
	{
		return &m_Pos;
	}
};

//.............................................................................

class CModuleItemInitializer
{
	friend class CParser;

protected:
	rtl::CBoxListT <CToken> m_Initializer;
	rtl::CString m_InitializerString;

public:
	rtl::CConstBoxListT <CToken>
	GetInitializer ()
	{
		return m_Initializer;
	}

	rtl::CString
	GetInitializerString ();
};

//.............................................................................

class CModuleItemDecl: public CModuleItemPos
{
	friend class CParser;
	friend class CNamespace;
	friend class CControlFlowMgr;
	friend class COrphan;

protected:
	EStorage m_StorageKind;
	EAccess m_AccessKind;
	rtl::CString m_Name;
	rtl::CString m_QualifiedName;
	CNamespace* m_pParentNamespace;
	CAttributeBlock* m_pAttributeBlock;

public:
	CModuleItemDecl ();

	EStorage
	GetStorageKind ()
	{
		return m_StorageKind;
	}

	EAccess
	GetAccessKind ()
	{
		return m_AccessKind;
	}

	bool
	IsNamed ()
	{
		return !m_Name.IsEmpty ();
	}

	rtl::CString
	GetName ()
	{
		return m_Name;
	}

	rtl::CString
	GetQualifiedName ()
	{
		return m_QualifiedName;
	}

	CNamespace*
	GetParentNamespace ()
	{
		return m_pParentNamespace;
	}

	CAttributeBlock*
	GetAttributeBlock ()
	{
		return m_pAttributeBlock;
	}
};

//.............................................................................

class CModuleItem: public rtl::TListLink
{
	friend class CModule;
	friend class CParser;

protected:
	CModule* m_pModule;
	EModuleItem m_ItemKind;
	uint_t m_Flags;
	CModuleItemDecl* m_pItemDecl;

public:
	rtl::CString m_Tag;

public:
	CModuleItem ();

	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	EModuleItem
	GetItemKind ()
	{
		return m_ItemKind;
	}

	uint_t
	GetFlags ()
	{
		return m_Flags;
	}

	CModuleItemDecl*
	GetItemDecl ()
	{
		return m_pItemDecl;
	}

	bool
	EnsureLayout ();

	virtual
	bool
	Compile ()
	{
		ASSERT (false);
		return true;
	}

protected:
	virtual
	bool
	CalcLayout ()
	{
		ASSERT (false);
		return true;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CUserModuleItem:
	public CModuleItem,
	public CModuleItemDecl
{
public:
	CUserModuleItem ()
	{
		m_pItemDecl = this;
	}
};

//.............................................................................

enum ELazyModuleItemFlag
{
	ELazyModuleItemFlag_Touched = 0x010000,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CLazyModuleItem: public CUserModuleItem
{
	friend class CNamespace;

public:
	CLazyModuleItem ()
	{
		m_ItemKind = EModuleItem_Lazy;
	}

	virtual
	CModuleItem*
	GetActualItem () = 0;
};

//.............................................................................

CClassType*
VerifyModuleItemIsClassType (
	CModuleItem* pItem,
	const char* pName
	);

CFunction*
VerifyModuleItemIsFunction (
	CModuleItem* pItem,
	const char* pName
	);

CProperty*
VerifyModuleItemIsProperty (
	CModuleItem* pItem,
	const char* pName
	);

//.............................................................................

} // namespace jnc {
