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

enum ImportTypeFlag {
	ImportTypeFlag_InResolve = 0x010000, // used for detection of import/typedef loops
};

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

	sl::Array<Type**>
	getFixupArray() {
		return m_fixupArray;
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
	Namespace* m_anchorNamespace;
	QualifiedName m_anchorName;
	sl::StringRef m_qualifiedName;

public:
	NamedImportType();

	const QualifiedName&
	getName() {
		return m_name;
	}

	Namespace*
	getAnchorNamespace() {
		return m_anchorNamespace;
	}

	const QualifiedName&
	getAnchorName() {
		return m_anchorName;
	}

	const sl::StringRef&
	getQualifiedName() {
		return m_qualifiedName;
	}

	static
	sl::String
	createSignature(
		const QualifiedName& name,
		Namespace* anchorNamespace,
		const QualifiedName& anchorName
	);

protected:
	virtual
	void
	prepareSignature() {
		m_signature = createSignature(m_name, m_anchorNamespace, m_anchorName);
		m_flags |= TypeFlag_SignatureReady;
	}

	virtual
	void
	prepareTypeString() {
		getTypeStringTuple()->m_typeStringPrefix = sl::formatString("import %s", getQualifiedName().sz());
	}

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
	void
	prepareTypeString();

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
	void
	prepareTypeString();

	virtual
	bool
	resolveImports();
};

//..............................................................................

} // namespace ct
} // namespace jnc
