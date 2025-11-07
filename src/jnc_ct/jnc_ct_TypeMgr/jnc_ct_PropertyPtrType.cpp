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
		"P",
		"Pw",
		"Pt",
		"R",
		"Rw",
		"Rt"
	};

	size_t i = (typeKind - TypeKind_PropertyPtr) * 2 + ptrTypeKind;
	ASSERT(i < countof(stringTable));

	sl::String signature = stringTable[i];
	signature += getPtrTypeFlagSignature(flags);
	signature += propertyType->getSignature();
	return signature;
}

void
PropertyPtrType::prepareTypeString() {
	TypeStringTuple* tuple = getTypeStringTuple();
	Type* returnType = m_targetType->getReturnType();

	sl::String string = returnType->getTypeStringPrefix();
	sl::StringRef ptrTypeFlagString = getPtrTypeFlagString(m_flags);
	if (!ptrTypeFlagString.isEmpty()) {
		string += ' ';
		string += ptrTypeFlagString;
	}

	if (m_ptrTypeKind != PropertyPtrTypeKind_Normal) {
		string += ' ';
		string += getPropertyPtrTypeKindString(m_ptrTypeKind);
	}

	string += m_typeKind == TypeKind_PropertyRef ? " property&" : " property*";
	tuple->m_typeStringPrefix = string;

	if (m_targetType->isIndexed())
		tuple->m_typeStringSuffix += m_targetType->getGetterType()->getTypeStringSuffix();

	tuple->m_typeStringSuffix += returnType->getTypeStringSuffix();
}

void
PropertyPtrType::prepareDoxyLinkedText() {
	TypeStringTuple* tuple = getTypeStringTuple();
	Type* returnType = m_targetType->getReturnType();

	tuple->m_doxyLinkedTextPrefix = returnType->getDoxyLinkedTextPrefix();

	sl::StringRef ptrTypeFlagString = getPtrTypeFlagString(m_flags);
	if (!ptrTypeFlagString.isEmpty()) {
		tuple->m_doxyLinkedTextPrefix += ' ';
		tuple->m_doxyLinkedTextPrefix += ptrTypeFlagString;
	}

	if (m_ptrTypeKind != PropertyPtrTypeKind_Normal) {
		tuple->m_doxyLinkedTextPrefix += ' ';
		tuple->m_doxyLinkedTextPrefix += getPropertyPtrTypeKindString(m_ptrTypeKind);
	}

	tuple->m_doxyLinkedTextPrefix += m_typeKind == TypeKind_PropertyRef ? " property&" : " property*";

	if (m_targetType->isIndexed())
		tuple->m_doxyLinkedTextSuffix += m_targetType->getGetterType()->getDoxyLinkedTextSuffix();

	tuple->m_doxyLinkedTextSuffix += returnType->getDoxyLinkedTextSuffix();
}

void
PropertyPtrType::prepareDoxyTypeString() {
	Type::prepareDoxyTypeString();

	if (m_targetType->isIndexed())
		m_targetType->getGetterType()->appendDoxyArgString(&getTypeStringTuple()->m_doxyTypeString);
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
