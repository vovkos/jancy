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

#include "jnc_ct_ClassType.h"

namespace jnc {
namespace ct {

//..............................................................................

class ClassPtrType: public Type {
	friend class TypeMgr;

protected:
	ClassPtrTypeKind m_ptrTypeKind;
	ClassType* m_targetType;

public:
	ClassPtrType();

	ClassPtrTypeKind
	getPtrTypeKind() {
		return m_ptrTypeKind;
	}

	ClassType*
	getTargetType() {
		return m_targetType;
	}

	ClassPtrType*
	getCheckedPtrType() {
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getClassPtrType(m_typeKind, m_ptrTypeKind, (m_flags & PtrTypeFlag__All) | PtrTypeFlag_Safe) :
			this;
	}

	ClassPtrType*
	getUnCheckedPtrType() {
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getClassPtrType(m_typeKind, m_ptrTypeKind, m_flags & (PtrTypeFlag__All & ~PtrTypeFlag_Safe)) :
			this;
	}

	ClassPtrType*
	getUnConstPtrType() {
		return (m_flags & PtrTypeFlag_Const) ?
			m_targetType->getClassPtrType(m_typeKind, m_ptrTypeKind, m_flags & (PtrTypeFlag__All & ~PtrTypeFlag_Const)) :
			this;
	}

	ClassPtrType*
	getNormalPtrType() {
		return (m_ptrTypeKind != ClassPtrTypeKind_Normal) ?
			m_targetType->getClassPtrType(ClassPtrTypeKind_Normal, m_flags & PtrTypeFlag__All) :
			this;
	}

	ClassPtrType*
	getWeakPtrType() {
		return (m_ptrTypeKind != ClassPtrTypeKind_Weak) ?
			m_targetType->getClassPtrType(ClassPtrTypeKind_Weak, m_flags & PtrTypeFlag__All) :
			this;
	}

	ClassPtrType*
	getUnWeakPtrType() {
		return (m_ptrTypeKind == ClassPtrTypeKind_Weak) ?
			m_targetType->getClassPtrType(ClassPtrTypeKind_Normal, m_flags & PtrTypeFlag__All) :
			this;
	}

	static
	sl::String
	createSignature(
		ClassType* classType,
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind,
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
	void
	prepareSignature() {
		m_signature = createSignature(m_targetType, m_typeKind, m_ptrTypeKind, m_flags);
		m_flags |= TypeFlag_SignatureFinal;
	}

	virtual
	void
	prepareTypeString();

	virtual
	void
	prepareDoxyLinkedText();

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_ClassPtrType);
	}

	virtual
	Type*
	calcFoldedDualType(
		bool isAlien,
		bool isContainerConst
	);

	void
	appendPointerStringSuffix(sl::String* string);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ClassPtrType::ClassPtrType() {
	m_typeKind = TypeKind_ClassPtr;
	m_ptrTypeKind = ClassPtrTypeKind_Normal;
	m_targetType = NULL;
	m_size = sizeof(void*);
	m_alignment = sizeof(void*);
}

//..............................................................................

struct ClassPtrTypeTuple: sl::ListLink {
	ClassPtrType* m_ptrTypeArray[2][2][4][2][2]; // ref x kind x const/readonly/cmut x volatile x checked
};

//..............................................................................

JNC_INLINE
bool
isClassPtrType(
	Type* type,
	ClassTypeKind classTypeKind
) {
	return
		(type->getTypeKindFlags() & TypeKindFlag_ClassPtr) &&
		((ClassPtrType*)type)->getTargetType()->getClassTypeKind() == classTypeKind;
}

JNC_INLINE
bool
isClassPtrType(
	Type* type,
	ClassType* classType
) {
	return
		(type->getTypeKindFlags() & TypeKindFlag_ClassPtr) &&
		((ClassPtrType*)type)->getTargetType()->cmp(classType) == 0;
}

JNC_INLINE
bool
isDerivedClassPtrType(
	Type* type,
	ClassType* classType
) {
	return
		(type->getTypeKindFlags() & TypeKindFlag_ClassPtr) &&
		((ClassPtrType*)type)->getTargetType()->isDerivedType(classType);
}

//..............................................................................

} // namespace ct
} // namespace jnc
