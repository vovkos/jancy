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

class ClassPtrType: public Type
{
	friend class TypeMgr;

protected:
	ClassPtrTypeKind m_ptrTypeKind;
	ClassType* m_targetType;

public:
	ClassPtrType();

	ClassPtrTypeKind
	getPtrTypeKind()
	{
		return m_ptrTypeKind;
	}

	ClassType*
	getTargetType()
	{
		return m_targetType;
	}

	ClassPtrType*
	getCheckedPtrType()
	{
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getClassPtrType(m_typeKind, m_ptrTypeKind, m_flags | PtrTypeFlag_Safe) :
			this;
	}

	ClassPtrType*
	getUnCheckedPtrType()
	{
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getClassPtrType(m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlag_Safe) :
			this;
	}

	ClassPtrType*
	getUnConstPtrType()
	{
		return (m_flags & PtrTypeFlag_Const) ?
			m_targetType->getClassPtrType(m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlag_Const) :
			this;
	}

	ClassPtrType*
	getNormalPtrType()
	{
		return (m_ptrTypeKind != ClassPtrTypeKind_Normal) ?
			m_targetType->getClassPtrType(ClassPtrTypeKind_Normal, m_flags) :
			this;
	}

	ClassPtrType*
	getWeakPtrType()
	{
		return (m_ptrTypeKind != ClassPtrTypeKind_Weak) ?
			m_targetType->getClassPtrType(ClassPtrTypeKind_Weak, m_flags) :
			this;
	}

	ClassPtrType*
	getUnWeakPtrType()
	{
		return (m_ptrTypeKind == ClassPtrTypeKind_Weak) ?
			m_targetType->getClassPtrType(ClassPtrTypeKind_Normal, m_flags) :
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

protected:
	virtual
	void
	prepareTypeString()
	{
		getTypeStringTuple()->m_typeStringPrefix = m_targetType->getTypeString() + getPointerStringSuffix();
	}

	virtual
	void
	prepareDoxyLinkedText()
	{
		getTypeStringTuple()->m_doxyLinkedTextPrefix = m_targetType->getDoxyLinkedTextPrefix() + getPointerStringSuffix();
	}

	virtual
	void
	prepareLlvmType();

	virtual
	void
	prepareLlvmDiType();

	virtual
	Type*
	calcFoldedDualType(
		bool isAlien,
		bool isContainerConst
		);

	sl::String
	getPointerStringSuffix();
};

//..............................................................................

struct ClassPtrTypeTuple: sl::ListLink
{
	ClassPtrType* m_ptrTypeArray[2] [2] [3] [2] [2]; // ref x kind x const x volatile x checked
};

//..............................................................................

JNC_INLINE
bool
isClassPtrType(
	Type* type,
	ClassTypeKind classTypeKind
	)
{
	return
		(type->getTypeKindFlags() & TypeKindFlag_ClassPtr) &&
		((ClassPtrType*)type)->getTargetType()->getClassTypeKind() == classTypeKind;
}

//..............................................................................

} // namespace ct
} // namespace jnc
