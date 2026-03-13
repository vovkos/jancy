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
	uint_t flags
) {
	static const char* stringTable[] = {
		"Pn",
		"Pw",
		"Rn",
		"Rw"
	};

	size_t j = (flags >> PtrTypeFlag__PtrKindBit) & 1;
	size_t i = (typeKind - TypeKind_ClassPtr) * 2 + j;
	ASSERT(i < countof(stringTable));

	sl::String signature(stringTable[i], 2);
	appendPtrTypeFlagSignature(&signature, flags);
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

		ClassPtrKind ptrKind = getPtrKind();
		if (ptrKind != ClassPtrKind_Normal) {
			string += ' ';
			string += getClassPtrKindString(ptrKind);
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

	if (getPtrKind() == ClassPtrKind_Weak)
		gcHeap->weakMark(iface->m_box);
	else
		gcHeap->markClass(iface->m_box);
}

Type*
ClassPtrType::calcFoldedDualType(
	AccessKind accessKind,
	ConstKind constKind
) {
	ASSERT(m_flags & TypeFlag_Dual);
	uint_t flags = m_flags & PtrTypeFlag_All & ~(ConstKind_ReadOnly | ConstKind_AutoConst | PtrTypeFlag_AutoEvent);

	ConstKind thisConstKind = getConstKind();
	if (thisConstKind == ConstKind_AutoConst)
		flags |= constKind;
	else if (thisConstKind == ConstKind_ReadOnly) {
		if (accessKind == AccessKind_Public)
			flags |= ConstKind_Const;
	} else {
		ASSERT(m_flags & PtrTypeFlag_AutoEvent); // class can't be dual, so it must be this pointer
		if (accessKind == AccessKind_Public)
			flags |= PtrTypeFlag_Event;
	}

	return m_targetType->getClassPtrType(m_typeKind, flags);
}

Type*
ClassPtrType::calcDualConstType(
	Type* ctype0,
	ConstKind constKind
) {
	ASSERT((m_flags & TypeFlag_LayoutReady) && (ctype0->getFlags() & TypeFlag_LayoutReady));

	uint_t ptrFlags;
	ClassPtrType* ctype = (ClassPtrType*)ctype0;
	if (ctype->getTypeKind() != TypeKind_ClassPtr ||
		getPtrKind() != ctype->getPtrKind() ||
		!m_targetType->isEqual(ctype->m_targetType) ||
		(ptrFlags = calcDualConstPtrTypeFlags(m_flags, ctype->getFlags(), constKind)) == -1
	) {
		setAutoConstError(this, ctype);
		return NULL;
	}

	return m_targetType->getClassPtrType(m_typeKind, ptrFlags);
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
