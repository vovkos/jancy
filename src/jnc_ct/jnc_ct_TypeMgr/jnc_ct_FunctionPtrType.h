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

#include "jnc_ct_FunctionType.h"

namespace jnc {
namespace ct {

//..............................................................................

class FunctionPtrType: public Type {
	friend class TypeMgr;

protected:
	FunctionType* m_targetType;
	ClassType* m_multicastType;

public:
	FunctionPtrType();

	FunctionPtrKind
	getPtrKind() {
		return getFunctionPtrKindFromFlags(m_flags);
	}

	FunctionType*
	getTargetType() {
		return m_targetType;
	}

	bool
	hasClosure() {
		return getPtrKind() <= FunctionPtrKind_Weak;
	}

	FunctionPtrType*
	getSafePtrType() {
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getFunctionPtrType(m_typeKind, m_flags & PtrTypeFlag_All | PtrTypeFlag_Safe) :
			this;
	}

	FunctionPtrType*
	getUnsafePtrType() {
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getFunctionPtrType(m_typeKind, m_flags & PtrTypeFlag_All & ~PtrTypeFlag_Safe) :
			this;
	}

	FunctionPtrType*
	getNormalPtrType() {
		return getPtrKind() != FunctionPtrKind_Normal ?
			m_targetType->getFunctionPtrType(m_typeKind, m_flags & PtrTypeFlag_All & ~PtrTypeFlag_PtrKindMask) :
			this;
	}

	FunctionPtrType*
	getWeakPtrType() {
		return getPtrKind() != FunctionPtrKind_Weak ?
			m_targetType->getFunctionPtrType(m_typeKind, m_flags & PtrTypeFlag_All & ~PtrTypeFlag_PtrKindMask | FunctionPtrKind_Weak) :
			this;
	}

	FunctionPtrType*
	getNonWeakPtrType() {
		return getPtrKind() == FunctionPtrKind_Weak ?
			m_targetType->getFunctionPtrType(m_typeKind, m_flags & PtrTypeFlag_All & ~PtrTypeFlag_PtrKindMask) :
			this;
	}

	ClassType*
	getMulticastType();

	sl::StringRef
	getTypeModifierString();

	static
	sl::String
	createSignature(
		FunctionType* functionType,
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
		m_flags |= m_targetType->getFlags() & TypeFlag_SignatureMask;
	}

	virtual
	sl::StringRef
	createItemString(size_t index);

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
		FunctionType* targetType = (FunctionType*)m_targetType->foldDualType(accessKind, constKind);
		return targetType->getFunctionPtrType(m_typeKind, m_flags & PtrTypeFlag_All);
	}

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	void
	prepareTypeVariable() {
		prepareSimpleTypeVariable(StdType_FunctionPtrType);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
FunctionPtrType::FunctionPtrType() {
	m_typeKind = TypeKind_FunctionPtr;
	m_alignment = sizeof(void*);
	m_targetType = NULL;
	m_multicastType = NULL;
}


//..............................................................................

struct FunctionPtrTypeTuple: sl::ListLink {
	FunctionPtrType* m_ptrTypeArray[2][FunctionPtrKind__Count][2]; // ref x ptrkind x safe
};

//..............................................................................

} // namespace ct
} // namespace jnc
