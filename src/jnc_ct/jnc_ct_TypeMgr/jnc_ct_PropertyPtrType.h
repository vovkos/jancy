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

#include "jnc_ct_PropertyType.h"
#include "jnc_ct_FunctionPtrType.h"

namespace jnc {
namespace ct {

//..............................................................................

class PropertyPtrType: public Type {
	friend class TypeMgr;

protected:
	PropertyType* m_targetType;

public:
	PropertyPtrType();

	PropertyPtrKind
	getPtrKind() {
		return getPropertyPtrKindFromFlags(m_flags);
	}

	PropertyType*
	getTargetType() {
		return m_targetType;
	}

	bool
	hasClosure() {
		return getPtrKind() <= PropertyPtrKind_Weak;
	}

	PropertyPtrType*
	getSafePtrType() {
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getPropertyPtrType(m_typeKind, m_flags & PtrTypeFlag__All | PtrTypeFlag_Safe) :
			this;
	}

	PropertyPtrType*
	getUnsafePtrType() {
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getPropertyPtrType(m_typeKind, m_flags & PtrTypeFlag__All & ~PtrTypeFlag_Safe) :
			this;
	}

	PropertyPtrType*
	getNormalPtrType() {
		return getPtrKind() != PropertyPtrKind_Normal ?
			m_targetType->getPropertyPtrType(m_typeKind, m_flags & PtrTypeFlag__All & ~PtrTypeFlag__PtrKindMask) :
			this;
	}

	PropertyPtrType*
	getWeakPtrType() {
		return getPtrKind() != PropertyPtrKind_Weak ?
			m_targetType->getPropertyPtrType(m_typeKind, m_flags & PtrTypeFlag__All & ~PtrTypeFlag__PtrKindMask | PropertyPtrKind_Weak) :
			this;
	}

	PropertyPtrType*
	getNonWeakPtrType() {
		return getPtrKind() == PropertyPtrKind_Weak ?
			m_targetType->getPropertyPtrType(m_typeKind, m_flags & PtrTypeFlag__All & ~PtrTypeFlag__PtrKindMask) :
			this;
	}

	static
	sl::String
	createSignature(
		PropertyType* propertyType,
		TypeKind typeKind,
		uint_t flags
	);

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	);

	virtual
	bool
	deduceTemplateArgs(
		sl::Array<Type*>* templateArgTypeArray,
		Type* referenceType
	);

protected:
	virtual
	bool
	resolveImports() {
		return m_targetType->ensureNoImports();
	}

	virtual
	bool
	calcLayout();

	virtual
	Type*
	calcFoldedDualType(
		AccessKind accessKind,
		ConstKind constKind
	) {
		PropertyType* targetType = (PropertyType*)m_targetType->foldDualType(accessKind, constKind);
		return targetType->getPropertyPtrType(m_typeKind, m_flags & PtrTypeFlag__All);
	}

	virtual
	void
	prepareSignature() {
		m_signature = createSignature(m_targetType, m_typeKind, m_flags);
		m_flags |= m_targetType->getFlags() & TypeFlag_SignatureFinal;
	}

	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_PropertyPtrType);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
PropertyPtrType::PropertyPtrType() {
	m_typeKind = TypeKind_PropertyPtr;
	m_alignment = sizeof(void*);
	m_targetType = NULL;
}

//..............................................................................

struct PropertyPtrTypeTuple: sl::ListLink {
	PropertyPtrType* m_ptrTypeArray[2][PropertyPtrKind__Count][2]; // ref x ptrkind x safe
};

//..............................................................................

} // namespace ct
} // namespace jnc
