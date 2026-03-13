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
	ClassType* m_targetType;

public:
	ClassPtrType();

	ClassPtrKind
	getPtrKind() {
		return getClassPtrKindFromFlags(m_flags);
	}

	ConstKind
	getConstKind() {
		return getConstKindFromFlags(m_flags);
	}

	ClassType*
	getTargetType() {
		return m_targetType;
	}

	ClassPtrType*
	getSafePtrType() {
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getClassPtrType(m_typeKind, m_flags & PtrTypeFlag_All | PtrTypeFlag_Safe) :
			this;
	}

	ClassPtrType*
	getUnsafePtrType() {
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getClassPtrType(m_typeKind, m_flags & PtrTypeFlag_All & ~PtrTypeFlag_Safe) :
			this;
	}

	ClassPtrType*
	getNonConstPtrType() {
		return (m_flags & PtrTypeFlag_ConstKindMask) ?
			m_targetType->getClassPtrType(m_typeKind, m_flags & PtrTypeFlag_All & ~PtrTypeFlag_ConstKindMask) :
			this;
	}

	ClassPtrType*
	getNormalPtrType() {
		return getPtrKind() != ClassPtrKind_Normal ?
			m_targetType->getClassPtrType(m_typeKind, m_flags & PtrTypeFlag_All & ~PtrTypeFlag_PtrKindMask) :
			this;
	}

	ClassPtrType*
	getWeakPtrType() {
		return getPtrKind() != ClassPtrKind_Weak ?
			m_targetType->getClassPtrType(m_typeKind, m_flags & PtrTypeFlag_All & ~PtrTypeFlag_PtrKindMask | ClassPtrKind_Weak) :
			this;
	}

	static
	sl::String
	createSignature(
		ClassType* classType,
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
	void
	prepareSignature() {
		m_signature = createSignature(m_targetType, m_typeKind, m_flags);
		m_flags |= TypeFlag_SignatureFinal;
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
		prepareSimpleTypeVariable(StdType_ClassPtrType);
	}

	virtual
	Type*
	calcFoldedDualType(
		AccessKind accessKind,
		ConstKind constKind
	);

	virtual
	Type*
	calcDualConstType(
		Type* ctype,
		ConstKind constKind
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ClassPtrType::ClassPtrType() {
	m_typeKind = TypeKind_ClassPtr;
	m_targetType = NULL;
	m_size = sizeof(void*);
	m_alignment = sizeof(void*);
}

//..............................................................................

struct ClassPtrTypeTuple: sl::ListLink {
	ClassPtrType* m_ptrTypeArray[2][ClassPtrKind__Count][ConstKind__Count][2][2]; // ref x ptrkind x constkind/event x volatile x safe
};

//..............................................................................

inline
bool
isClassPtrType(
	Type* type,
	ClassTypeKind classTypeKind
) {
	return
		(type->getTypeKindFlags() & TypeKindFlag_ClassPtr) &&
		((ClassPtrType*)type)->getTargetType()->getClassTypeKind() == classTypeKind;
}

inline
bool
isClassPtrType(
	Type* type,
	ClassType* classType
) {
	return
		(type->getTypeKindFlags() & TypeKindFlag_ClassPtr) &&
		((ClassPtrType*)type)->getTargetType()->isEqual(classType);
}

inline
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
