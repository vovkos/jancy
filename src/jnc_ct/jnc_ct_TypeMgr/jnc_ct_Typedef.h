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

namespace jnc {
namespace ct {

class TypedefShadowType;

//..............................................................................

class Typedef:
	public ModuleItem,
	public ModuleItemDecl {
	friend class TypeMgr;

protected:
	Type* m_type;
	TypedefShadowType* m_shadowType;

public:
	Typedef();

	Type*
	getType() {
		return m_type;
	}

	TypedefShadowType*
	getShadowType();

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);
};

//..............................................................................

class TypedefShadowType:
	public Type,
	public ModuleItemDecl {
	friend class TypeMgr;

protected:
	Typedef* m_typedef;

public:
	TypedefShadowType() {
		m_typeKind = TypeKind_TypedefShadow;
		m_typedef = NULL;
	}

	Typedef*
	getTypedef() {
		return m_typedef;
	}

	Type*
	getActualType() {
		return m_typedef->getType();
	}

protected:
	virtual
	void
	prepareSignature() {
		m_signature = "T" + m_typedef->getQualifiedName();
		m_flags |= TypeFlag_SignatureFinal;
	}

	virtual
	void
	prepareTypeString() {
		getTypeStringTuple()->m_typeStringPrefix = getQualifiedName();
	}

	virtual
	void
	prepareDoxyLinkedText();

	virtual
	void
	prepareLlvmType() {
		m_llvmType = m_typedef->getType()->getLlvmType();
	}

	virtual
	void
	prepareLlvmDiType() {
		m_llvmDiType = m_typedef->getType()->getLlvmDiType();
	}

	virtual
	bool
	resolveImports() {
		return m_typedef->getType()->ensureNoImports();
	}

	virtual
	bool
	calcLayout();
};

//..............................................................................

} // namespace ct
} // namespace jnc
