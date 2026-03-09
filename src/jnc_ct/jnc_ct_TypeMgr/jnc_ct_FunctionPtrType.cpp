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
#include "jnc_ct_FunctionPtrType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"

namespace jnc {
namespace ct {

//..............................................................................

const char*
getFunctionPtrKindString(FunctionPtrKind ptrKind) {
	static const char* stringTable[FunctionPtrKind__Count] = {
		"closure",  // FunctionPtrKind_Normal = 0,
		"weak",     // FunctionPtrKind_Weak,
		"thin",     // FunctionPtrKind_Thin,
	};

	size_t i = ptrKind >> jnc_PtrTypeFlag__PtrKindBit;
	return i < FunctionPtrKind__Count ?
		stringTable[i] :
		"undefined-function-ptr-kind";
}

//..............................................................................

sl::String
FunctionPtrType::createSignature(
	FunctionType* functionType,
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
	size_t i = (typeKind - TypeKind_FunctionPtr) * 2 + j;
	ASSERT(i < countof(stringTable));

	sl::String signature = stringTable[i];
	signature += getPtrTypeFlagSignature(flags);
	signature += '&';
	signature += functionType->getSignature();
	return signature;
}

sl::StringRef
FunctionPtrType::getTypeModifierString() {
	sl::String string;

	sl::StringRef ptrTypeFlagString = getPtrTypeFlagString(m_flags);
	if (!ptrTypeFlagString.isEmpty()) {
		string += ptrTypeFlagString;
		string += ' ';
	}

	FunctionPtrKind ptrKind = getPtrKind();
	if (ptrKind != FunctionPtrKind_Normal) {
		string += getFunctionPtrKindString(ptrKind);
		string += ' ';
	}

	if (!string.isEmpty())
		string.chop(1);

	return string;
}

sl::StringRef
FunctionPtrType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix: {
		sl::String string = m_targetType->getItemString(index);
		sl::StringRef modifierString = getTypeModifierString();
		if (!modifierString.isEmpty()) {
			string += ' ';
			string += modifierString;
		}

		string += m_typeKind == TypeKind_FunctionRef ? " function&" : " function*";
		return string;
		}

	case TypeStringKind_Suffix:
	case TypeStringKind_DoxyLinkedTextSuffix:
		return m_targetType->getItemString(index);

	case TypeStringKind_DoxyTypeString: {
		sl::String string = Type::createItemString(index);
		m_targetType->appendDoxyArgString(&string);
		return string;
		}

	default:
		return Type::createItemString(index);
	}
}

void
FunctionPtrType::prepareLlvmType() {
	m_llvmType = getPtrKind() != FunctionPtrKind_Thin ?
		m_module->m_typeMgr.getStdType(StdType_FunctionPtrStruct)->getLlvmType() :
		llvm::PointerType::get(m_targetType->getLlvmType(), 0);
}

void
FunctionPtrType::prepareLlvmDiType() {
	m_llvmDiType = getPtrKind() != FunctionPtrKind_Thin ?
		m_module->m_typeMgr.getStdType(StdType_FunctionPtrStruct)->getLlvmDiType() :
		(m_targetType->getFlags() & TypeFlag_LayoutReady) ?
			m_module->m_llvmDiBuilder.createPointerType(m_targetType) :
			m_module->m_llvmDiBuilder.createPointerType(m_module->m_typeMgr.getStdType(StdType_SimpleFunction));
}

void
FunctionPtrType::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
) {
	FunctionPtrKind ptrKind = getPtrKind();
	ASSERT(ptrKind == FunctionPtrKind_Normal || ptrKind == FunctionPtrKind_Weak);

	FunctionPtr* ptr = (FunctionPtr*)p;
	if (!ptr->m_closure)
		return;

	Box* box = ptr->m_closure->m_box;
	if (ptrKind == FunctionPtrKind_Normal)
		gcHeap->markClass(box);
	else if (isClassType(box->m_type, ClassTypeKind_FunctionClosure))
		gcHeap->weakMarkClosureClass(box);
	else  // simple weak closure
		gcHeap->weakMark(box);
}

bool
FunctionPtrType::calcLayout() {
	bool result = m_targetType->ensureLayout();
	if (!result)
		return false;

	m_flags |= m_targetType->getFlags() & TypeFlag_Dual;
	return true;
}

bool
FunctionPtrType::deduceTemplateArgs(
	sl::Array<Type*>* templateArgTypeArray,
	Type* referenceType
) {
	if (referenceType->getTypeKind() != TypeKind_FunctionPtr) {
		setTemplateArgDeductionError(referenceType);
		return false;
	}

	return m_targetType->deduceTemplateArgs(
		templateArgTypeArray,
		((FunctionPtrType*)referenceType)->getTargetType()
	);
}

Type*
FunctionPtrType::mergeAutoConstTypes(Type* constType0) {
	ASSERT((m_flags & TypeFlag_LayoutReady) && (constType0->getFlags() & TypeFlag_LayoutReady));
	FunctionPtrType* constType = (FunctionPtrType*)constType0;
	if (constType->getTypeKind() != TypeKind_FunctionPtr || getPtrKind() != constType->getPtrKind())
		return NULL;

	FunctionType* targetType = (FunctionType*)m_targetType->mergeAutoConstTypes(constType->m_targetType);
	if (!targetType)
		return NULL;

	ASSERT(targetType->getTypeKind() == TypeKind_Function);
	return m_module->m_typeMgr.getFunctionPtrType(targetType, m_typeKind, m_flags & PtrTypeFlag__All);
}

//..............................................................................

} // namespace ct
} // namespace jnc
