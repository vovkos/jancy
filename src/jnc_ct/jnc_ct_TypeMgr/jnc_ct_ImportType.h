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
#include "jnc_ct_Decl.h"

namespace jnc {
namespace ct {

class ImportPtrType;

//..............................................................................

enum ImportTypeFlag
{
	ImportTypeFlag_InResolve = 0x010000, // used for detection of import/typedef loops
};

//..............................................................................

class ImportType: public Type
{
	friend class TypeMgr;

protected:
	Type* m_actualType;
	sl::Array<Type**> m_fixupArray;

public:
	ImportType()
	{
		m_actualType = NULL;
	}

	bool
	isResolved()
	{
		return m_actualType != NULL;
	}

	Type*
	getActualType()
	{
		ASSERT(m_actualType);
		return m_actualType;
	}

	sl::Array<Type**>
	getFixupArray()
	{
		return m_fixupArray;
	}

	void
	addFixup(Type** type)
	{
		m_fixupArray.append(type);
	}

	void
	applyFixups();

	bool
	ensureResolved();

protected:
	virtual
	void
	prepareLlvmType()
	{
		ASSERT(false);
	}

	virtual
	void
	prepareLlvmDiType()
	{
		ASSERT(false);
	}

	virtual
	bool
	calcLayout()
	{
		return ensureResolved() && m_actualType->ensureLayout();
	}

	virtual
	bool
	resolve() = 0;
};

//..............................................................................

class NamedImportType:
	public ImportType,
	public ModuleItemPos
{
	friend class TypeMgr;
	friend class Parser;

protected:
	QualifiedName m_name;
	Namespace* m_anchorNamespace;
	QualifiedName m_anchorName;
	sl::String m_qualifiedName;

public:
	NamedImportType();

	const QualifiedName&
	getName()
	{
		return m_name;
	}

	Namespace*
	getAnchorNamespace()
	{
		return m_anchorNamespace;
	}

	const QualifiedName&
	getAnchorName()
	{
		return m_anchorName;
	}

	const sl::String&
	getQualifiedName()
	{
		return m_qualifiedName;
	}

	ImportPtrType*
	getImportPtrType(uint_t typeModifiers);

	static
	sl::String
	createSignature(
		const QualifiedName& name,
		Namespace* anchorNamespace,
		const QualifiedName& orphanName
		);

protected:
	virtual
	void
	prepareTypeString()
	{
		getTypeStringTuple()->m_typeStringPrefix.format("import %s", getQualifiedName().sz());
	}

	virtual
	bool
	resolve();
};

//..............................................................................

class ImportPtrType: public ImportType
{
	friend class TypeMgr;

protected:
	NamedImportType* m_targetType;
	uint_t m_typeModifiers;

public:
	ImportPtrType();

	NamedImportType*
	getTargetType()
	{
		return m_targetType;
	}

	uint_t
	getTypeModifiers()
	{
		return m_typeModifiers;
	}

	static
	sl::String
	createSignature(
		NamedImportType* importType,
		uint_t typeModifiers
		)
	{
		return sl::formatString(
			"ZP%s:%d",
			importType->getQualifiedName().sz(),
			typeModifiers
			);
	}

protected:
	virtual
	void
	prepareTypeString();

	virtual
	bool
	resolve();
};

//..............................................................................

class ImportIntModType: public ImportType
{
	friend class TypeMgr;

protected:
	NamedImportType* m_importType;
	uint_t m_typeModifiers; // unsigned, bigendian

public:
	ImportIntModType();

	NamedImportType*
	getImportType()
	{
		return m_importType;
	}

	uint_t
	getTypeModifiers()
	{
		return m_typeModifiers;
	}

	static
	sl::String
	createSignature(
		NamedImportType* importType,
		uint_t typeModifiers
		)
	{
		return sl::formatString(
			"ZI%s:%d:%d",
			importType->getQualifiedName().sz(),
			typeModifiers
			);
	}

protected:
	virtual
	void
	prepareTypeString();

	virtual
	bool
	resolve();
};

//..............................................................................

} // namespace ct
} // namespace jnc
