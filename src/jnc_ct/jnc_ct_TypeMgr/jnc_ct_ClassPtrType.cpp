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

sl::String
ClassPtrType::createSignature(
	ClassType* classType,
	TypeKind typeKind,
	ClassPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	static const char* stringTable[] = {
		"Pn",
		"Pw",
		"Rn",
		"Rw"
	};

	size_t i = (typeKind - TypeKind_ClassPtr) * 2 + ptrTypeKind;
	ASSERT(i < countof(stringTable));

	sl::String signature = stringTable[i];
	signature += getPtrTypeFlagSignature(flags);
	signature += '&';
	signature += classType->getSignature();
	return signature;
}

sl::StringRef
ClassPtrType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		sl::String string = m_targetType->getItemString(index);
		sl::StringRef ptrTypeFlagString = getPtrTypeFlagString(m_flags);
		if (!ptrTypeFlagString.isEmpty()) {
			string += ' ';
			string += ptrTypeFlagString;
		}

		if (m_ptrTypeKind != ClassPtrTypeKind_Normal) {
			string += ' ';
			string += getClassPtrTypeKindString(m_ptrTypeKind);
		}

		string += m_typeKind == TypeKind_ClassRef ? "&" : "*";
		return string;
		}

	default:
		return Type::createItemString(index);
	}
}

void
ClassPtrType::prepareLlvmType() {
	m_llvmType = llvm::PointerType::get(m_targetType->getIfaceStructType()->getLlvmType(), 0);
}

void
ClassPtrType::prepareLlvmDiType() {
	m_llvmDiType = (m_targetType->getFlags() & TypeFlag_LayoutReady) ?
		m_module->m_llvmDiBuilder.createPointerType(m_targetType->getIfaceStructType()) :
		m_module->m_typeMgr.getStdType(StdType_ByteThinPtr)->getLlvmDiType();
}

void
ClassPtrType::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
) {
	IfaceHdr* iface = *(IfaceHdr**)p;
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
	uint_t ptrFlags
) {
	ASSERT(m_flags & TypeFlag_Dual);
	uint_t flags = m_flags & PtrTypeFlag__All & ~(PtrTypeFlag_ReadOnly | PtrTypeFlag_ConstIf | PtrTypeFlag_DualEvent);

	if (isAlien) {
		if (m_flags & PtrTypeFlag_ReadOnly)
			flags |= PtrTypeFlag_Const;
		else if (m_flags & PtrTypeFlag_ConstIf)
			flags |= ptrFlags;

		if (m_flags & PtrTypeFlag_DualEvent)
			flags |= PtrTypeFlag_Event;
	} else if (m_flags & PtrTypeFlag_ConstIf)
		flags |= ptrFlags;

	return m_targetType->getClassPtrType(m_typeKind, m_ptrTypeKind, flags);
}

bool
ClassPtrType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	if (referenceType->getTypeKind() != TypeKind_ClassPtr) {
		setTemplateArgDeductionError(referenceType);
		return false;
	}

	return m_targetType->deduceTemplateArgs(
		templateArgTypeArray,
		((ClassPtrType*)referenceType)->getTargetType()
	);
}

//..............................................................................

} // namespace ct
} // namespace jnc
