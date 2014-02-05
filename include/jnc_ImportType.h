// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_Namespace.h"

namespace jnc {

class CImportPtrType;

//.............................................................................

enum EImportTypeFlag
{
	EImportTypeFlag_ImportLoop = 0x010000, // used for detection of import loops
};

//.............................................................................

class CImportType: public CType
{
	friend class CTypeMgr;

protected:
	CType* m_pActualType;

public:
	CImportType ()
	{
		m_pActualType = NULL;
	}

	bool
	IsResolved ()
	{
		return m_pActualType != NULL;
	}

	CType*
	GetActualType ()
	{
		ASSERT (m_pActualType);
		return m_pActualType;
	}

protected:
	virtual
	void
	PrepareLlvmType ()
	{
		ASSERT (false);
	}
};

//.............................................................................

class CNamedImportType:
	public CImportType,
	public CModuleItemPos
{
	friend class CTypeMgr;

protected:
	CNamespace* m_pAnchorNamespace;
	CQualifiedName m_Name;
	rtl::CString m_QualifiedName;

public:
	CNamedImportType ()
	{
		m_TypeKind = EType_NamedImport;
		m_pAnchorNamespace = NULL;
	}

	CNamespace*
	GetAnchorNamespace ()
	{
		return m_pAnchorNamespace;
	}

	const CQualifiedName&
	GetName ()
	{
		return m_Name;
	}

	rtl::CString
	GetQualifiedName ()
	{
		return m_QualifiedName;
	}

	CImportPtrType*
	GetImportPtrType (
		uint_t TypeModifiers = 0,
		uint_t Flags = 0
		);

	static
	rtl::CString
	CreateSignature (
		const CQualifiedName& Name,
		CNamespace* pAnchorNamespace
		)
	{
		return rtl::CString::Format_s ("ZN%s", pAnchorNamespace->CreateQualifiedName (Name).cc ());
	}

protected:
	virtual
	void
	PrepareTypeString ()
	{
		m_TypeString.Format ("import %s", GetQualifiedName ().cc ());
	}
};

//.............................................................................

class CImportPtrType: public CImportType
{
	friend class CTypeMgr;

protected:
	CNamedImportType* m_pTargetType;
	uint_t m_TypeModifiers;

public:
	CImportPtrType ();

	CNamedImportType*
	GetTargetType ()
	{
		return m_pTargetType;
	}

	uint_t
	GetTypeModifiers ()
	{
		return m_TypeModifiers;
	}

	CImportPtrType*
	GetCheckedPtrType ()
	{
		return !(m_Flags & EPtrTypeFlag_Checked) ?
			m_pTargetType->GetImportPtrType (m_TypeModifiers, m_Flags | EPtrTypeFlag_Checked) :
			this;
	}

	CImportPtrType*
	GetUnCheckedPtrType ()
	{
		return (m_Flags & EPtrTypeFlag_Checked) ?
			m_pTargetType->GetImportPtrType (m_TypeModifiers, m_Flags & ~EPtrTypeFlag_Checked) :
			this;
	}

	static
	rtl::CString
	CreateSignature (
		CNamedImportType* pImportType,
		uint_t TypeModifiers,
		uint_t Flags
		)
	{
		return rtl::CString::Format_s (
			"ZP%s:%d:%d",
			pImportType->GetQualifiedName ().cc (),
			TypeModifiers,
			Flags
			);
	}

protected:
	virtual
	void
	PrepareTypeString ();
};

//.............................................................................

class CImportIntModType: public CImportType
{
	friend class CTypeMgr;

protected:
	CNamedImportType* m_pImportType;
	uint_t m_TypeModifiers; // unsigned, bigendian

public:
	CImportIntModType ();

	CNamedImportType*
	GetImportType ()
	{
		return m_pImportType;
	}

	uint_t
	GetTypeModifiers ()
	{
		return m_TypeModifiers;
	}

	static
	rtl::CString
	CreateSignature (
		CNamedImportType* pImportType,
		uint_t TypeModifiers,
		uint_t Flags
		)
	{
		return rtl::CString::Format_s (
			"ZI%s:%d:%d",
			pImportType->GetQualifiedName ().cc (),
			TypeModifiers,
			Flags
			);
	}

protected:
	virtual
	void
	PrepareTypeString ();
};

//.............................................................................

} // namespace jnc {
