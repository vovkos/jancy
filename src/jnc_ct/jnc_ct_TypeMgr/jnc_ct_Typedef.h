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

class Typedef: public ModuleItemWithDecl<> {
	friend class TypeMgr;

protected:
	Type* m_type;
	TypedefShadowType* m_shadowType;

public:
	Typedef();

	bool
	isRecursive() const;

	Type*
	getType() const {
		return m_type;
	}

	TypedefShadowType*
	getShadowType() const;

	virtual
	Type*
	getItemType() {
		return m_type;
	}

	virtual
	Namespace*
	getNamespace() {
		return m_type->ensureNoImports() ? m_type->getNamespace() : NULL;
	}

	virtual
	bool
	generateDocumentation(
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
	);

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	void
	prepareShadowType();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Typedef::Typedef() {
	m_itemKind = ModuleItemKind_Typedef;
	m_type = NULL;
	m_shadowType = NULL;
}

//
// we need to explicitly handle recursion cases like this:
//
// typedef int A;
//
// struct B<T> {
//	...
// }
//
// struct C<T> {
//	typedef A A;
//	typedef B<T> B;
//	...
// }
//
// lookup in typedef's grandparent namespace to resolve recursion
//

inline
bool
Typedef::isRecursive() const {
	return
		(m_type->getTypeKindFlags() & TypeKindFlag_Import) &&
		(m_type->getFlags() & ImportTypeFlag_InResolve);
}

inline
TypedefShadowType*
Typedef::getShadowType() const {
	if (!m_shadowType)
		((Typedef*)this)->prepareShadowType();

	return m_shadowType;
}

//..............................................................................

class TypedefShadowType: public ModuleItemWithDecl<Type> {
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
		m_signature = m_typedef->getType()->getSignature();
		m_flags |= m_typedef->getType()->getFlags() & TypeFlag_SignatureMask;
	}

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
