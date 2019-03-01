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

class FunctionPtrType: public Type
{
	friend class TypeMgr;

protected:
	FunctionPtrTypeKind m_ptrTypeKind;
	FunctionType* m_targetType;
	ClassType* m_multicastType;

public:
	FunctionPtrType();

	FunctionPtrTypeKind
	getPtrTypeKind()
	{
		return m_ptrTypeKind;
	}

	FunctionType*
	getTargetType()
	{
		return m_targetType;
	}

	bool
	hasClosure()
	{
		return m_ptrTypeKind == FunctionPtrTypeKind_Normal || m_ptrTypeKind == FunctionPtrTypeKind_Weak;
	}

	FunctionPtrType*
	getCheckedPtrType()
	{
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getFunctionPtrType(m_typeKind, m_ptrTypeKind, m_flags | PtrTypeFlag_Safe) :
			this;
	}

	FunctionPtrType*
	getUnCheckedPtrType()
	{
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getFunctionPtrType(m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlag_Safe) :
			this;
	}

	FunctionPtrType*
	getNormalPtrType()
	{
		return (m_ptrTypeKind != FunctionPtrTypeKind_Normal) ?
			m_targetType->getFunctionPtrType(FunctionPtrTypeKind_Normal, m_flags) :
			this;
	}

	FunctionPtrType*
	getWeakPtrType()
	{
		return (m_ptrTypeKind != FunctionPtrTypeKind_Weak) ?
			m_targetType->getFunctionPtrType(FunctionPtrTypeKind_Weak, m_flags) :
			this;
	}

	FunctionPtrType*
	getUnWeakPtrType()
	{
		return (m_ptrTypeKind == FunctionPtrTypeKind_Weak) ?
			m_targetType->getFunctionPtrType(FunctionPtrTypeKind_Normal, m_flags) :
			this;
	}

	ClassType*
	getMulticastType();

	sl::String
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

protected:
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
	bool
	calcLayout();
};

//..............................................................................

struct FunctionPtrTypeTuple: sl::ListLink
{
	FunctionPtrType* m_ptrTypeArray[2][3][2]; // ref x kind x checked
};

//..............................................................................

} // namespace ct
} // namespace jnc
