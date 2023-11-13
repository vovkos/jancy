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
	PropertyPtrTypeKind m_ptrTypeKind;
	PropertyType* m_targetType;

public:
	PropertyPtrType();

	PropertyPtrTypeKind
	getPtrTypeKind() {
		return m_ptrTypeKind;
	}

	PropertyType*
	getTargetType() {
		return m_targetType;
	}

	bool
	hasClosure() {
		return m_ptrTypeKind == PropertyPtrTypeKind_Normal || m_ptrTypeKind == PropertyPtrTypeKind_Weak;
	}

	PropertyPtrType*
	getCheckedPtrType() {
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getPropertyPtrType(m_typeKind, m_ptrTypeKind, (m_flags & PtrTypeFlag__All) | PtrTypeFlag_Safe) :
			this;
	}

	PropertyPtrType*
	getUnCheckedPtrType() {
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getPropertyPtrType(m_typeKind, m_ptrTypeKind, m_flags & (PtrTypeFlag__All & ~PtrTypeFlag_Safe)) :
			this;
	}

	PropertyPtrType*
	getNormalPtrType() {
		return (m_ptrTypeKind != PropertyPtrTypeKind_Normal) ?
			m_targetType->getPropertyPtrType(PropertyPtrTypeKind_Normal, m_flags & PtrTypeFlag__All) :
			this;
	}

	PropertyPtrType*
	getWeakPtrType() {
		return (m_ptrTypeKind != PropertyPtrTypeKind_Weak) ?
			m_targetType->getPropertyPtrType(PropertyPtrTypeKind_Weak, m_flags & PtrTypeFlag__All) :
			this;
	}

	PropertyPtrType*
	getUnWeakPtrType() {
		return (m_ptrTypeKind == PropertyPtrTypeKind_Weak) ?
			m_targetType->getPropertyPtrType(PropertyPtrTypeKind_Normal, m_flags & PtrTypeFlag__All) :
			this;
	}

	static
	sl::String
	createSignature(
		PropertyType* propertyType,
		TypeKind typeKind,
		PropertyPtrTypeKind ptrTypeKind,
		uint_t flags
	);

	virtual
	void
	markGcRoots(
		const void* p,
		rt::GcHeap* gcHeap
	);

protected:
	virtual
	bool
	resolveImports() {
		return m_targetType->ensureNoImports();
	}

	virtual
	void
	prepareSignature() {
		m_signature = createSignature(m_targetType, m_typeKind, m_ptrTypeKind, m_flags);
		m_flags |= m_targetType->getFlags() & TypeFlag_SignatureFinal;
	}

	virtual
	void
	prepareTypeString();

	virtual
	void
	prepareDoxyLinkedText();

	virtual
	void
	prepareDoxyTypeString();

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

//..............................................................................

struct PropertyPtrTypeTuple: sl::ListLink {
	PropertyPtrType* m_ptrTypeArray[2][3][3]; // ref x kind x unsafe / checked
};

//..............................................................................

JNC_INLINE
bool
isBindableType(Type* type) {
	return
		type->getTypeKind() == TypeKind_PropertyRef &&
		(((PropertyPtrType*)type)->getTargetType()->getFlags() & PropertyTypeFlag_Bindable) != 0;
}

//..............................................................................

} // namespace ct
} // namespace jnc
