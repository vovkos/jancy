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
#include "jnc_ct_TypeName.h"
#include "jnc_ct_Namespace.h"

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

struct ImportTypeNameAnchor {
	Namespace* m_namespace;
	size_t m_linkId;

	ImportTypeNameAnchor() {
		m_namespace = NULL;
		m_linkId = 0;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class ImportTypeName:
	public ImportType,
	public TypeName {
	friend class TypeMgr;
	friend class Parser;

protected:
	ImportTypeNameAnchor* m_anchor;

public:
	ImportTypeName() {
		m_typeKind = TypeKind_ImportTypeName;
		m_anchor = NULL;
	}

	const ImportTypeNameAnchor*
	getAnchor() const {
		return m_anchor;
	}

	static
	sl::String
	createSignature(
		Namespace* parentNamespace,
		const QualifiedName& name,
		const ImportTypeNameAnchor* anchor = NULL
	);

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	bool
	resolveImports();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
sl::String
ImportTypeName::createSignature(
	Namespace* parentNamespace,
	const QualifiedName& name,
	const ImportTypeNameAnchor* anchor
) {
	sl::String signature = TypeName::createSignature('IN', parentNamespace, name);
	if (anchor)
		signature.appendFormat("-%d", anchor->m_linkId);

	return signature;
}

//..............................................................................

class ImportPtrType: public ModType<
	ImportType,
	ImportTypeName,
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
	ImportTypeName,
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
