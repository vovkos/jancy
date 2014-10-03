// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_Namespace.h"

namespace jnc {

class ImportPtrType;

//.............................................................................

enum ImportTypeFlag
{
	ImportTypeFlag_ImportLoop = 0x010000, // used for detection of import loops
};

//.............................................................................

class ImportType: public Type
{
	friend class TypeMgr;

protected:
	Type* m_actualType;

public:
	ImportType ()
	{
		m_actualType = NULL;
	}

	bool
	isResolved ()
	{
		return m_actualType != NULL;
	}

	Type*
	getActualType ()
	{
		ASSERT (m_actualType);
		return m_actualType;
	}

protected:
	virtual
	void
	prepareLlvmType ()
	{
		ASSERT (false);
	}
};

//.............................................................................

class NamedImportType:
	public ImportType,
	public ModuleItemPos
{
	friend class TypeMgr;

protected:
	Namespace* m_anchorNamespace;
	QualifiedName m_name;
	rtl::String m_qualifiedName;

public:
	NamedImportType ()
	{
		m_typeKind = TypeKind_NamedImport;
		m_anchorNamespace = NULL;
	}

	Namespace*
	getAnchorNamespace ()
	{
		return m_anchorNamespace;
	}

	const QualifiedName&
	getName ()
	{
		return m_name;
	}

	rtl::String
	getQualifiedName ()
	{
		return m_qualifiedName;
	}

	ImportPtrType*
	getImportPtrType (
		uint_t typeModifiers = 0,
		uint_t flags = 0
		);

	static
	rtl::String
	createSignature (
		const QualifiedName& name,
		Namespace* anchorNamespace
		)
	{
		return rtl::String::format_s ("ZN%s", anchorNamespace->createQualifiedName (name).cc ());
	}

protected:
	virtual
	void
	prepareTypeString ()
	{
		m_typeString.format ("import %s", getQualifiedName ().cc ());
	}
};

//.............................................................................

class ImportPtrType: public ImportType
{
	friend class TypeMgr;

protected:
	NamedImportType* m_targetType;
	uint_t m_typeModifiers;

public:
	ImportPtrType ();

	NamedImportType*
	getTargetType ()
	{
		return m_targetType;
	}

	uint_t
	getTypeModifiers ()
	{
		return m_typeModifiers;
	}

	ImportPtrType*
	getCheckedPtrType ()
	{
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getImportPtrType (m_typeModifiers, m_flags | PtrTypeFlag_Safe) :
			this;
	}

	ImportPtrType*
	getUnCheckedPtrType ()
	{
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getImportPtrType (m_typeModifiers, m_flags & ~PtrTypeFlag_Safe) :
			this;
	}

	static
	rtl::String
	createSignature (
		NamedImportType* importType,
		uint_t typeModifiers,
		uint_t flags
		)
	{
		return rtl::String::format_s (
			"ZP%s:%d:%d",
			importType->getQualifiedName ().cc (),
			typeModifiers,
			flags
			);
	}

protected:
	virtual
	void
	prepareTypeString ();
};

//.............................................................................

class ImportIntModType: public ImportType
{
	friend class TypeMgr;

protected:
	NamedImportType* m_importType;
	uint_t m_typeModifiers; // unsigned, bigendian

public:
	ImportIntModType ();

	NamedImportType*
	getImportType ()
	{
		return m_importType;
	}

	uint_t
	getTypeModifiers ()
	{
		return m_typeModifiers;
	}

	static
	rtl::String
	createSignature (
		NamedImportType* importType,
		uint_t typeModifiers,
		uint_t flags
		)
	{
		return rtl::String::format_s (
			"ZI%s:%d:%d",
			importType->getQualifiedName ().cc (),
			typeModifiers,
			flags
			);
	}

protected:
	virtual
	void
	prepareTypeString ();
};

//.............................................................................

} // namespace jnc {
