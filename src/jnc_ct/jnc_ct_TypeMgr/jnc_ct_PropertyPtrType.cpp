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
#include "jnc_ct_PropertyPtrType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

//..............................................................................

sl::String
PropertyPtrType::createSignature(
	PropertyType* propertyType,
	TypeKind typeKind,
	PropertyPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	static const char* stringTable[] = {
		"Pn",
		"Pw",
		"Pt",
		"Rn",
		"Rw",
		"Rt"
	};

	size_t i = (typeKind - TypeKind_PropertyPtr) * 2 + ptrTypeKind;
	ASSERT(i < countof(stringTable));

	sl::String signature = stringTable[i];
	signature += getPtrTypeFlagSignature(flags);
	signature += '&';
	signature += propertyType->getSignature();
	return signature;
}

sl::StringRef
PropertyPtrType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		Type* returnType = m_targetType->getReturnType();
		sl::String string = returnType->getItemString(index);
		sl::StringRef ptrTypeFlagString = getPtrTypeFlagString(m_flags);
		if (!ptrTypeFlagString.isEmpty()) {
			string += ' ';
			string += ptrTypeFlagString;
		}

		if (m_ptrTypeKind != PropertyPtrTypeKind_Normal) {
			string += ' ';
			string += getPropertyPtrTypeKindString(m_ptrTypeKind);
		}

		sl::StringRef modifierString = m_targetType->getTypeModifierString();
		if (!modifierString.isEmpty()) {
			string += ' ';
			string += modifierString;
		}

		string += m_typeKind == TypeKind_PropertyRef ? " property&" : " property*";
		return string;
		}

	case TypeStringKind_Suffix:
	case TypeStringKind_DoxyLinkedTextSuffix:
		return m_targetType->getGetterType()->getItemString(index);

	case TypeStringKind_DoxyTypeString: {
		sl::String string = Type::createItemString(index);
		m_targetType->getGetterType()->appendDoxyArgString(&string);
		return string;
		}

	default:
		return Type::createItemString(index);
	}
}

void
PropertyPtrType::prepareLlvmType() {
	m_llvmType = m_ptrTypeKind != PropertyPtrTypeKind_Thin ?
		m_module->m_typeMgr.getStdType(StdType_PropertyPtrStruct)->getLlvmType() :
		m_targetType->getVtableStructType()->getDataPtrType_c()->getLlvmType();
}

void
PropertyPtrType::prepareLlvmDiType() {
	m_llvmDiType = m_ptrTypeKind != PropertyPtrTypeKind_Thin ?
		m_module->m_typeMgr.getStdType(StdType_PropertyPtrStruct)->getLlvmDiType() :
		m_targetType->getVtableStructType()->getDataPtrType_c()->getLlvmDiType();
}

void
PropertyPtrType::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
) {
	ASSERT(m_ptrTypeKind == PropertyPtrTypeKind_Normal || m_ptrTypeKind == PropertyPtrTypeKind_Weak);

	PropertyPtr* ptr = (PropertyPtr*)p;
	if (!ptr->m_closure)
		return;

	Box* box = ptr->m_closure->m_box;
	if (m_ptrTypeKind == PropertyPtrTypeKind_Normal)
		gcHeap->markClass(box);
	else if (isClassType(box->m_type, ClassTypeKind_PropertyClosure))
		gcHeap->weakMarkClosureClass(box);
	else  // simple weak closure
		gcHeap->weakMark(box);
}

bool
PropertyPtrType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	if (referenceType->getTypeKind() != TypeKind_PropertyPtr) {
		setTemplateArgDeductionError(referenceType);
		return false;
	}

	return m_targetType->deduceTemplateArgs(
		templateArgTypeArray,
		((PropertyPtrType*)referenceType)->getTargetType()
	);
}

//..............................................................................

} // namespace ct
} // namespace jnc
