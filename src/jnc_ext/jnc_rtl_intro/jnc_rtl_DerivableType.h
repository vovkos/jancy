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

#include "jnc_rtl_Type.h"
#include "jnc_rtl_MemberBlock.h"
#include "jnc_ct_DerivableType.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(BaseTypeSlot)
JNC_DECLARE_OPAQUE_CLASS_TYPE(DerivableType)

//..............................................................................

class BaseTypeSlot:
	public ModuleItemBase<ct::BaseTypeSlot>,
	public ModuleItemDecl {
public:
	BaseTypeSlot(ct::BaseTypeSlot* slot):
		ModuleItemBase(slot),
		ModuleItemDecl(slot) {}

	DerivableType*
	JNC_CDECL
	getType() {
		return (DerivableType*)rtl::getType(m_item->getType());
	}

	size_t
	JNC_CDECL
	getOffset() {
		return m_item->getOffset();
	}

	size_t
	JNC_CDECL
	getVtableIndex() {
		return m_item->getVtableIndex();
	}
};

//..............................................................................

template <typename T>
class DerivableTypeBase:
	public NamedTypeBase<T>,
	public MemberBlock {
public:
	DerivableTypeBase(T* type):
		NamedTypeBase<T>(type),
		MemberBlock(type) {}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class DerivableType: public DerivableTypeBase<ct::DerivableType> {
public:
	DerivableType(ct::DerivableType* type):
		DerivableTypeBase(type) {}

	size_t
	JNC_CDECL
	getBaseTypeCount() {
		return m_item->getBaseTypeArray().getCount();
	}

	BaseTypeSlot*
	JNC_CDECL
	getBaseType(size_t index) {
		size_t count = m_item->getBaseTypeArray().getCount();
		return index < count ? rtl::getBaseTypeSlot(m_item->getBaseTypeArray()[index]) : NULL;
	}

	size_t
	JNC_CDECL
	findBaseTypeOffset(DerivableType* baseType) {
		return m_item->findBaseTypeOffset(baseType->m_item);
	}

	Function*
	JNC_CDECL
	getUnaryOperator(UnOpKind opKind) {
		return rtl::getFunction(m_item->getUnaryOperator(opKind));
	}

	Function*
	JNC_CDECL
	getBinaryOperator(BinOpKind opKind) {
		return rtl::getFunction(m_item->getBinaryOperator(opKind));
	}

	Function*
	JNC_CDECL
	getCallOperator() {
		return rtl::getFunction(m_item->getCallOperator());
	}

	size_t
	JNC_CDECL
	getCastOperatorCount() {
		return m_item->getCastOperatorArray().getCount();
	}

	Function*
	JNC_CDECL
	getCastOperator(size_t index) {
		size_t count = m_item->getCastOperatorArray().getCount();
		return index < count ? rtl::getFunction(m_item->getCastOperatorArray()[index]) : NULL;
	}
};

//..............................................................................

} // namespace rtl
} // namespace jnc
