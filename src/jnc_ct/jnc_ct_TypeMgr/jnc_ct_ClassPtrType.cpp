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
#include "jnc_ct_ClassPtrType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

//..............................................................................

ClassPtrType::ClassPtrType() {
	m_typeKind = TypeKind_ClassPtr;
	m_ptrTypeKind = ClassPtrTypeKind_Normal;
	m_targetType = NULL;
	m_size = sizeof(void*);
	m_alignment = sizeof(void*);
}

sl::String
ClassPtrType::createSignature(
	ClassType* classType,
	TypeKind typeKind,
	ClassPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	sl::String signature = typeKind == TypeKind_ClassRef ? "RC" : "PC";

	if (ptrTypeKind == ClassPtrTypeKind_Weak)
		signature += 'w';

	signature += getPtrTypeFlagSignature(flags);
	signature += classType->getSignature();
	return signature;
}

void
ClassPtrType::appendPointerStringSuffix(sl::String* string) {
	sl::StringRef ptrTypeFlagString = getPtrTypeFlagString(m_flags);
	if (!ptrTypeFlagString.isEmpty()) {
		*string += ' ';
		*string += ptrTypeFlagString;
	}

	if (m_ptrTypeKind != ClassPtrTypeKind_Normal) {
		*string += ' ';
		*string += getClassPtrTypeKindString(m_ptrTypeKind);
	}

	*string += m_typeKind == TypeKind_ClassRef ? "&" : "*";
}

void
ClassPtrType::prepareTypeString() {
	sl::String string = m_targetType->getTypeString();
	appendPointerStringSuffix(&string);
	getTypeStringTuple()->m_typeStringPrefix = string;
}

void
ClassPtrType::prepareDoxyLinkedText() {
	getTypeStringTuple()->m_doxyLinkedTextPrefix = m_targetType->getDoxyLinkedTextPrefix();
	appendPointerStringSuffix(&getTypeStringTuple()->m_doxyLinkedTextPrefix);
}

void
ClassPtrType::prepareLlvmType() {
	m_llvmType = llvm::PointerType::get(m_targetType->getIfaceStructType()->getLlvmType(), 0);
}

void
ClassPtrType::prepareLlvmDiType() {
	m_llvmDiType = (m_targetType->getFlags() & ModuleItemFlag_LayoutReady) ?
		m_module->m_llvmDiBuilder.createPointerType(m_targetType->getIfaceStructType()) :
		m_module->m_typeMgr.getStdType(StdType_BytePtr)->getLlvmDiType();
}

void
ClassPtrType::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
) {
	IfaceHdr* iface = *(IfaceHdr**) p;
	if (!iface)
		return;

	if (m_ptrTypeKind == ClassPtrTypeKind_Weak)
		gcHeap->weakMark(iface->m_box);
	else
		gcHeap->markClass(iface->m_box);
}

Type*
ClassPtrType::calcFoldedDualType(
	bool isAlien,
	bool isContainerConst
) {
	ASSERT(isDualType(this));

	uint_t flags = m_flags & ~(PtrTypeFlag_ReadOnly | PtrTypeFlag_CMut | PtrTypeFlag_DualEvent);

	if (isAlien) {
		if (m_flags & PtrTypeFlag_ReadOnly)
			flags |= PtrTypeFlag_Const;

		if (m_flags & PtrTypeFlag_DualEvent)
			flags |= PtrTypeFlag_Event;
	}

	if ((m_flags & PtrTypeFlag_CMut) && isContainerConst)
		flags |= PtrTypeFlag_Const;

	return m_module->m_typeMgr.getClassPtrType(m_targetType, m_typeKind, m_ptrTypeKind, flags);
}

//..............................................................................

} // namespace ct
} // namespace jnc
