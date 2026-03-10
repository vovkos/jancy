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

sl::String
DataPtrType::createSignature(
	Type* targetType,
	uint_t bitOffset,
	uint_t bitCount,
	TypeKind typeKind,
	uint_t flags
) {
	static const char* stringTable[] = {
		"Pn",
		"Pl",
		"Pt",
		"Rn",
		"Rl",
		"Rt"
	};

	size_t j = (flags >> PtrTypeFlag__PtrKindBit) & 3;
	size_t i = (typeKind - TypeKind_DataPtr) * 2 + j;
	ASSERT(i < countof(stringTable));

	sl::String signature(stringTable[i], 2);
	if (bitCount)
		signature.appendFormat(":%d:%d", bitOffset, bitOffset + bitCount);

	appendPtrTypeFlagSignature(&signature, flags);
	signature += '&';
	signature += targetType->getSignature();
	return signature;
}

bool
DataPtrType::calcLayout() {
	bool result = m_targetType->ensureNoImports(); // only resolve imports (calc layout could cause loops)
	if (!result)
		return false;

	m_flags |= m_targetType->getFlags() & TypeFlag_Dual;
	return true;
}

sl::StringRef
DataPtrType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		sl::String string = m_targetType->getItemString(index);
		sl::StringRef ptrTypeFlagString = getPtrTypeFlagString(m_flags);
		if (!ptrTypeFlagString.isEmpty()) {
			string += ' ';
			string += ptrTypeFlagString;
		}

		DataPtrKind ptrKind = getPtrKind();
		if (ptrKind != DataPtrKind_Normal) {
			string += ' ';
			string += getDataPtrKindString(ptrKind);
		}

		if (m_targetType->getTypeKind() == TypeKind_Array)
			string += " array";

		string += m_typeKind == TypeKind_DataRef ? "&" : "*";
		return string;
		}

	case TypeStringKind_Suffix:
	case TypeStringKind_DoxyLinkedTextSuffix:
		return m_targetType->getItemString(index);

	default:
		return Type::createItemString(index);
	}
}

void
DataPtrType::prepareLlvmType() {
	m_llvmType =
		getPtrKind() == DataPtrKind_Normal ? m_module->m_typeMgr.getStdType(StdType_DataPtrStruct)->getLlvmType() :
		m_targetType->getTypeKind() != TypeKind_Void ? llvm::PointerType::get(m_targetType->getLlvmType(), 0) :
		m_module->m_typeMgr.getStdType(StdType_ByteThinPtr)->getLlvmType();
}

void
DataPtrType::prepareLlvmDiType() {
	m_llvmDiType =
		getPtrKind() == DataPtrKind_Normal ? m_module->m_typeMgr.getStdType(StdType_DataPtrStruct)->getLlvmDiType() :
		m_targetType->getTypeKind() != TypeKind_Void && (m_targetType->getFlags() & TypeFlag_LayoutReady) ?
				m_module->m_llvmDiBuilder.createPointerType(m_targetType) :
				m_module->m_typeMgr.getStdType(StdType_ByteThinPtr)->getLlvmDiType();
}

void
DataPtrType::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
) {
	ASSERT(getPtrKind() == DataPtrKind_Normal);

	DataPtr* ptr = (DataPtr*)p;
	if (!ptr->m_validator)
		return;

	gcHeap->weakMark(ptr->m_validator->m_validatorBox);
	gcHeap->markData(ptr->m_validator->m_targetBox);
}

Type*
DataPtrType::calcFoldedDualType(
	AccessKind accessKind,
	ConstKind constKind
) {
	ASSERT(m_flags & TypeFlag_Dual);

	Type* targetType = m_targetType->getActualTypeIfDual(accessKind, constKind);
	uint_t flags = m_flags & PtrTypeFlag__All & ~(ConstKind_ReadOnly | ConstKind_AutoConst);

	ConstKind thisConstKind = getConstKind();
	if (thisConstKind == ConstKind_AutoConst)
		flags |= constKind;
	else if (thisConstKind == ConstKind_ReadOnly && accessKind == AccessKind_Public)
		flags |= ConstKind_Const;

	return targetType->getDataPtrType(m_typeKind, flags);
}

Type*
DataPtrType::mergeAutoConstTypes(Type* constType0) {
	ASSERT((m_flags & TypeFlag_LayoutReady) && (constType0->getFlags() & TypeFlag_LayoutReady));
	DataPtrType* constType = (DataPtrType*)constType0;
	if (constType->getTypeKind() != TypeKind_DataPtr || getPtrKind() != constType->getPtrKind())
		return NULL;

	Type* targetType = m_targetType->mergeAutoConstTypes(constType->m_targetType);
	if (!targetType)
		return NULL;

	AXL_TODO("deduce & apply PtrConstKind")
	return this;
}

sl::StringRef
DataPtrType::getTargetValueString(
	const void* p,
	const char* formatSpec
) {
	if (!(m_flags & (PtrTypeFlag_BigEndian | PtrTypeFlag_BitField)))
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
