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

//..............................................................................

class ImportType: public Type {
	friend class TypeMgr;

protected:
	Type* m_actualType;
	sl::Array<Type**> m_fixupArray;
	err::Error m_resolveError;

public:
	ImportType() {
		m_actualType = NULL;
	}

	bool
	isResolved() {
		return m_actualType != NULL;
	}

	Type*
	getActualType() {
		ASSERT(m_actualType);
		return m_actualType;
	}

	void
	addFixup(Type** type) {
		m_fixupArray.append(type);
	}

	void
	applyFixups();

	bool
	ensureResolved() {
		return
			m_actualType ? true :
			m_resolveError ? err::fail(m_resolveError) :
			resolve();
	}

protected:
	bool
	resolve();

	virtual
	void
	prepareLlvmType() {
		ASSERT(false);
	}

	virtual
	void
	prepareLlvmDiType() {
		ASSERT(false);
	}

	virtual
	bool
	calcLayout() {
		return ensureResolved() && m_actualType->ensureLayout();
	}
};

//..............................................................................

class NamedImportType:
	public ImportType,
	public ModuleItemPos {
	friend class TypeMgr;
	friend class Parser;

protected:
	QualifiedName m_name;
	QualifiedName m_baseName;

public:
	NamedImportType() {
		m_typeKind = TypeKind_NamedImport;
	}

	const QualifiedName&
	getName() {
		return m_name;
	}

	const QualifiedName&
	getBaseName() {
		return m_baseName;
	}

	static
	sl::String
	createSignature(
		Namespace* parentNamespace,
		const QualifiedName& name,
		const QualifiedName* baseName = NULL
	);

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	bool
	resolveImports();
};

//..............................................................................

class ImportPtrType: public ModType<
	ImportType,
	NamedImportType,
	TypeKind_ImportPtr,
	'PI'
> {
protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	bool
	resolveImports();
};

//..............................................................................

class ImportIntModType: public ModType<
	ImportType,
	NamedImportType,
	TypeKind_ImportIntMod,
	'II'
> {
protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	bool
	resolveImports();
};

//..............................................................................

} // namespace ct
} // namespace jnc
