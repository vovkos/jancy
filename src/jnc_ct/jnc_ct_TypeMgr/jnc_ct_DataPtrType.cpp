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
}

sl::String
DataPtrType::createSignature(
	Type* targetType,
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	sl::String signature = typeKind == TypeKind_DataRef ? "RD" : "PD";

	switch (ptrTypeKind) {
	case DataPtrTypeKind_Lean:
		signature += 'l';
		break;

	case DataPtrTypeKind_Thin:
		signature += 't';
		break;
	}

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
		m_module->m_typeMgr.getStdType(StdType_BytePtr)->getLlvmType();
}

void
DataPtrType::prepareLlvmDiType() {
	m_llvmDiType =
		m_ptrTypeKind == DataPtrTypeKind_Normal ? m_module->m_typeMgr.getStdType(StdType_DataPtrStruct)->getLlvmDiType() :
		m_targetType->getTypeKind() != TypeKind_Void && (m_targetType->getFlags() & ModuleItemFlag_LayoutReady) ?
				m_module->m_llvmDiBuilder.createPointerType(m_targetType) :
				m_module->m_typeMgr.getStdType(StdType_BytePtr)->getLlvmDiType();
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

	uint_t flags = m_flags & ~(PtrTypeFlag_ReadOnly | PtrTypeFlag_CMut | PtrTypeFlag_DualTarget);

	if ((m_flags & PtrTypeFlag_ReadOnly) && isAlien)
		flags |= PtrTypeFlag_Const;

	if ((m_flags & PtrTypeFlag_CMut) && isContainerConst)
		flags |= PtrTypeFlag_Const;

	return m_module->m_typeMgr.getDataPtrType(targetType, m_typeKind, m_ptrTypeKind, flags);
}

//..............................................................................

} // namespace ct
} // namespace jnc
