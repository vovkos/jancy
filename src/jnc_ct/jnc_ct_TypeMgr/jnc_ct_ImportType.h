//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_ct_Type.h"
#include "jnc_ct_Namespace.h"

namespace jnc {
namespace ct {

class ImportPtrType;

//..............................................................................

enum ImportTypeFlag
{
	ImportTypeFlag_ImportLoop = 0x010000, // used for detection of import loops
};

//..............................................................................

class ImportType: public Type
{
	friend class TypeMgr;

protected:
	Type* m_actualType;
	sl::Array <Type**> m_fixupArray;

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

	sl::Array <Type**>
	getFixupArray ()
	{
		return m_fixupArray;
	}

	void
	addFixup (Type** type)
	{
		m_fixupArray.append (type);
	}

	void
	applyFixups ();

protected:
	virtual
	void
	prepareLlvmType ()
	{
		ASSERT (false);
	}

	virtual
	void
	prepareLlvmDiType ()
	{
		ASSERT (false);
	}

	virtual
	bool
	calcLayout ()
	{
		ASSERT (false);
		return true;
	}
};

//..............................................................................

class NamedImportType:
	public ImportType,
	public ModuleItemPos
{
	friend class TypeMgr;

protected:
	Namespace* m_anchorNamespace;
	QualifiedName m_name;
	sl::String m_qualifiedName;

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

	const sl::String&
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
	sl::String
	createSignature (
		const QualifiedName& name,
		Namespace* anchorNamespace
		)
	{
		return sl::formatString ("ZN%s", anchorNamespace->createQualifiedName (name).sz ());
	}

protected:
	virtual
	void
	prepareTypeString ()
	{
		getTypeStringTuple ()->m_typeStringPrefix.format ("import %s", getQualifiedName ().sz ());
	}
};

//..............................................................................

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
	sl::String
	createSignature (
		NamedImportType* importType,
		uint_t typeModifiers,
		uint_t flags
		)
	{
		return sl::formatString (
			"ZP%s:%d:%d",
			importType->getQualifiedName ().sz (),
			typeModifiers,
			flags
			);
	}

protected:
	virtual
	void
	prepareTypeString ();
};

//..............................................................................

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
	sl::String
	createSignature (
		NamedImportType* importType,
		uint_t typeModifiers,
		uint_t flags
		)
	{
		return sl::formatString (
			"ZI%s:%d:%d",
			importType->getQualifiedName ().sz (),
			typeModifiers,
			flags
			);
	}

protected:
	virtual
	void
	prepareTypeString ();
};

//..............................................................................

} // namespace ct
} // namespace jnc
