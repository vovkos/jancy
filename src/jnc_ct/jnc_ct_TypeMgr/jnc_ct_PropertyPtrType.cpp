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

	size_t j = (flags >> PtrTypeFlag__PtrKindBit) & 3;
	size_t i = (typeKind - TypeKind_PropertyPtr) * 2 + j;
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

		PropertyPtrKind ptrKind = getPtrKind();
		if (ptrKind != PropertyPtrKind_Normal) {
			string += ' ';
			string += getPropertyPtrKindString(ptrKind);
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
	m_llvmType = getPtrKind() != PropertyPtrKind_Thin ?
		m_module->m_typeMgr.getStdType(StdType_PropertyPtrStruct)->getLlvmType() :
		m_targetType->getVtableStructType()->getDataPtrType(DataPtrKind_Thin)->getLlvmType();
}

void
PropertyPtrType::prepareLlvmDiType() {
	m_llvmDiType = getPtrKind() != PropertyPtrKind_Thin ?
		m_module->m_typeMgr.getStdType(StdType_PropertyPtrStruct)->getLlvmDiType() :
		m_targetType->getVtableStructType()->getDataPtrType(DataPtrKind_Thin)->getLlvmDiType();
}

void
PropertyPtrType::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
) {
	PropertyPtrKind ptrKind = getPtrKind();
	ASSERT(ptrKind == PropertyPtrKind_Normal || ptrKind == PropertyPtrKind_Weak);

	PropertyPtr* ptr = (PropertyPtr*)p;
	if (!ptr->m_closure)
		return;

	Box* box = ptr->m_closure->m_box;
	if (ptrKind == PropertyPtrKind_Normal)
		gcHeap->markClass(box);
	else if (isClassType(box->m_type, ClassTypeKind_PropertyClosure))
		gcHeap->weakMarkClosureClass(box);
	else  // simple weak closure
		gcHeap->weakMark(box);
}

bool
PropertyPtrType::calcLayout() {
	bool result = m_targetType->ensureLayout();
	if (!result)
		return false;

	m_flags |= m_targetType->getFlags() & TypeFlag_Dual;
	return true;
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

Type*
PropertyPtrType::mergeAutoConstTypes(Type* constType0) {
	ASSERT((m_flags & TypeFlag_LayoutReady) && (constType0->getFlags() & TypeFlag_LayoutReady));
	PropertyPtrType* constType = (PropertyPtrType*)constType0;
	if (constType->getTypeKind() != TypeKind_PropertyPtr || getPtrKind() != constType->getPtrKind())
		return NULL;

	PropertyType* targetType = (PropertyType*)m_targetType->mergeAutoConstTypes(constType->m_targetType);
	if (!targetType)
		return NULL;

	ASSERT(targetType->getTypeKind() == TypeKind_Property);
	return m_module->m_typeMgr.getPropertyPtrType(targetType, m_typeKind, m_flags & PtrTypeFlag__All);
}

//..............................................................................

} // namespace ct
} // namespace jnc
