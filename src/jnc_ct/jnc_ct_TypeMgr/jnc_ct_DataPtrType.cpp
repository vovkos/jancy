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

#include "pch.h"
#include "jnc_ct_DataPtrType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

//..............................................................................

DataPtrType::DataPtrType() {
	m_typeKind = TypeKind_DataPtr;
	m_ptrTypeKind = DataPtrTypeKind_Normal;
	m_targetType = NULL;
	m_alignment = sizeof(void*);
	m_bitOffset = 0;
	m_bitCount = 0;
}

sl::String
DataPtrType::createSignature(
	Type* targetType,
	uint_t bitOffset,
	uint_t bitCount,
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	static const char* stringTable[] = {
		"P",
		"Pl",
		"Pt",
		"R",
		"Rl",
		"Rt"
	};

	size_t i = (typeKind - TypeKind_DataPtr) * 2 + ptrTypeKind;
	ASSERT(i < countof(stringTable));

	sl::String signature = stringTable[i];
	if (bitCount)
		signature.appendFormat(":%d:%d", bitOffset, bitCount);

	signature += getPtrTypeFlagSignature(flags);
	signature += targetType->getSignature();
	return signature;
}

bool
DataPtrType::calcLayout() {
	if (!(m_targetType->getTypeKindFlags() & TypeKindFlag_Import)) // already fixed up!
		return true;

	bool result = ((ImportType*)m_targetType)->ensureResolved();
	if (!result)
		return false;

	ASSERT(!(m_targetType->getTypeKindFlags() & TypeKindFlag_Import)); // should have been fixed up in resolve
	return true;
}

void
DataPtrType::prepareTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();
	sl::String string = m_targetType->getTypeString();
	appendPointerStringSuffix(&string);
	tuple->m_typeStringPrefix = string;
	tuple->m_typeStringSuffix = m_targetType->getTypeStringSuffix();
}

void
DataPtrType::prepareDoxyLinkedText() {
	TypeStringTuple* tuple = getTypeStringTuple();
	tuple->m_doxyLinkedTextPrefix = m_targetType->getDoxyLinkedTextPrefix();
	appendPointerStringSuffix(&tuple->m_doxyLinkedTextPrefix);
	tuple->m_doxyLinkedTextSuffix = m_targetType->getDoxyLinkedTextSuffix();
}

void
DataPtrType::appendPointerStringSuffix(sl::String* string) {
	sl::StringRef ptrTypeFlagString = getPtrTypeFlagString(m_flags);
	if (!ptrTypeFlagString.isEmpty()) {
		*string += ' ';
		*string += ptrTypeFlagString;
	}

	if (m_ptrTypeKind != DataPtrTypeKind_Normal) {
		*string += ' ';
		*string += getDataPtrTypeKindString(m_ptrTypeKind);
	}

	if (m_targetType->getTypeKind() == TypeKind_Array)
		*string += " array";

	*string += m_typeKind == TypeKind_DataRef ? "&" : "*";
}

void
DataPtrType::prepareLlvmType() {
	m_llvmType =
		m_ptrTypeKind == DataPtrTypeKind_Normal ? m_module->m_typeMgr.getStdType(StdType_DataPtrStruct)->getLlvmType() :
		m_targetType->getTypeKind() != TypeKind_Void ? llvm::PointerType::get(m_targetType->getLlvmType(), 0) :
		m_module->m_typeMgr.getStdType(StdType_ByteThinPtr)->getLlvmType();
}

void
DataPtrType::prepareLlvmDiType() {
	m_llvmDiType =
		m_ptrTypeKind == DataPtrTypeKind_Normal ? m_module->m_typeMgr.getStdType(StdType_DataPtrStruct)->getLlvmDiType() :
		m_targetType->getTypeKind() != TypeKind_Void && (m_targetType->getFlags() & TypeFlag_LayoutReady) ?
				m_module->m_llvmDiBuilder.createPointerType(m_targetType) :
				m_module->m_typeMgr.getStdType(StdType_ByteThinPtr)->getLlvmDiType();
}

void
DataPtrType::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
) {
	ASSERT(m_ptrTypeKind == DataPtrTypeKind_Normal);

	DataPtr* ptr = (DataPtr*)p;
	if (!ptr->m_validator)
		return;

	gcHeap->weakMark(ptr->m_validator->m_validatorBox);
	gcHeap->markData(ptr->m_validator->m_targetBox);
}

Type*
DataPtrType::calcFoldedDualType(
	bool isAlien,
	bool isContainerConst
) {
	ASSERT(isDualType(this));

	Type* targetType = (m_flags & PtrTypeFlag_DualTarget) ?
		m_module->m_typeMgr.foldDualType(m_targetType, isAlien, isContainerConst) :
		m_targetType;

	uint_t flags = m_flags & (PtrTypeFlag__All & ~(PtrTypeFlag_ReadOnly | PtrTypeFlag_CMut | PtrTypeFlag_DualTarget));

	if ((m_flags & PtrTypeFlag_ReadOnly) && isAlien)
		flags |= PtrTypeFlag_Const;

	if ((m_flags & PtrTypeFlag_CMut) && isContainerConst)
		flags |= PtrTypeFlag_Const;

	return m_module->m_typeMgr.getDataPtrType(targetType, m_typeKind, m_ptrTypeKind, flags);
}

sl::StringRef
DataPtrType::getTargetValueString(
	const void* p,
	const char* formatSpec
) {
	if (m_flags & !(PtrTypeFlag_BigEndian | PtrTypeFlag_BitField))
		return m_targetType->getValueString(p, formatSpec);

	uint64_t n = 0;
	if (m_flags & PtrTypeFlag_BigEndian)
		sl::swapByteOrder(&n, p, m_targetType->getSize());
	else
		memcpy(&n, p, m_targetType->getSize());

	if (m_flags & PtrTypeFlag_BitField) {
		n >>= m_bitOffset;
		n &= (1 << m_bitCount) - 1;
		if (!(m_targetType->getTypeKindFlags() & TypeKindFlag_Unsigned)) // extend with sign bit
			n |= ~((n & ((int64_t)1 << (m_bitCount - 1))) - 1);
	}

	return m_targetType->getValueString(&n, formatSpec);
}

bool
DataPtrType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	TypeKind typeKind = referenceType->getTypeKind();
	switch (typeKind) {
	case TypeKind_DataRef:
		if (((DataPtrType*)referenceType)->getTargetType()->getTypeKind() != TypeKind_Array)
			break;

		referenceType = m_module->m_operatorMgr.prepareArrayRefType((DataPtrType*)referenceType);
		// and fall through

	case TypeKind_DataPtr:
		return m_targetType->deduceTemplateArgs(
			templateArgTypeArray,
			((DataPtrType*)referenceType)->getTargetType()
		);
	}

	setTemplateArgDeductionError(referenceType);
	return false;
}

//..............................................................................

} // namespace ct
} // namespace jnc
