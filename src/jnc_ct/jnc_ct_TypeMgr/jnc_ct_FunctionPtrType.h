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
	FunctionPtrTypeKind m_ptrTypeKind;
	FunctionType* m_targetType;
	ClassType* m_multicastType;

public:
	FunctionPtrType();

	FunctionPtrTypeKind
	getPtrTypeKind() {
		return m_ptrTypeKind;
	}

	FunctionType*
	getTargetType() {
		return m_targetType;
	}

	bool
	hasClosure() {
		return m_ptrTypeKind == FunctionPtrTypeKind_Normal || m_ptrTypeKind == FunctionPtrTypeKind_Weak;
	}

	FunctionPtrType*
	getCheckedPtrType() {
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getFunctionPtrType(m_typeKind, m_ptrTypeKind, (m_flags & PtrTypeFlag__All) | PtrTypeFlag_Safe) :
			this;
	}

	FunctionPtrType*
	getUnCheckedPtrType() {
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getFunctionPtrType(m_typeKind, m_ptrTypeKind, m_flags & (PtrTypeFlag__All & ~PtrTypeFlag_Safe)) :
			this;
	}

	FunctionPtrType*
	getNormalPtrType() {
		return (m_ptrTypeKind != FunctionPtrTypeKind_Normal) ?
			m_targetType->getFunctionPtrType(FunctionPtrTypeKind_Normal, m_flags & PtrTypeFlag__All) :
			this;
	}

	FunctionPtrType*
	getWeakPtrType() {
		return (m_ptrTypeKind != FunctionPtrTypeKind_Weak) ?
			m_targetType->getFunctionPtrType(FunctionPtrTypeKind_Weak, m_flags & PtrTypeFlag__All) :
			this;
	}

	FunctionPtrType*
	getUnWeakPtrType() {
		return (m_ptrTypeKind == FunctionPtrTypeKind_Weak) ?
			m_targetType->getFunctionPtrType(FunctionPtrTypeKind_Normal, m_flags & PtrTypeFlag__All) :
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
		FunctionPtrTypeKind ptrTypeKind,
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
	calcLayout() {
		return m_targetType->ensureLayout();
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
	m_ptrTypeKind = FunctionPtrTypeKind_Normal;
	m_alignment = sizeof(void*);
	m_targetType = NULL;
	m_multicastType = NULL;
}


//..............................................................................

struct FunctionPtrTypeTuple: sl::ListLink {
	FunctionPtrType* m_ptrTypeArray[2][3][2]; // ref x kind x checked
};

//..............................................................................

} // namespace ct
} // namespace jnc
