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
getFunctionPtrTypeKindString(FunctionPtrTypeKind ptrTypeKind)
{
	static const char* stringTable[FunctionPtrTypeKind__Count] =
	{
		"closure",  // FunctionPtrTypeKind_Normal = 0,
		"weak",     // FunctionPtrTypeKind_Weak,
		"thin",     // FunctionPtrTypeKind_Thin,
	};

	return (size_t)ptrTypeKind < FunctionPtrTypeKind__Count ?
		stringTable[ptrTypeKind] :
		"undefined-function-ptr-kind";
}

//..............................................................................

FunctionPtrType::FunctionPtrType()
{
	m_typeKind = TypeKind_FunctionPtr;
	m_ptrTypeKind = FunctionPtrTypeKind_Normal;
	m_size = sizeof(FunctionPtr);
	m_targetType = NULL;
	m_multicastType = NULL;
}

ClassType*
FunctionPtrType::getMulticastType()
{
	return m_module->m_typeMgr.getMulticastType(this);
}

sl::String
FunctionPtrType::createSignature(
	FunctionType* functionType,
	TypeKind typeKind,
	FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	sl::String signature = typeKind == TypeKind_FunctionRef ? "RF" : "PF";

	switch (ptrTypeKind)
	{
	case FunctionPtrTypeKind_Thin:
		signature += 't';
		break;

	case FunctionPtrTypeKind_Weak:
		signature += 'w';
		break;
	}

	signature += getPtrTypeFlagSignature(flags);
	signature += functionType->getSignature();
	return signature;
}

sl::String
FunctionPtrType::getTypeModifierString()
{
	sl::String string;

	sl::String ptrTypeFlagString = getPtrTypeFlagString(m_flags);
	if (!ptrTypeFlagString.isEmpty())
	{
		string += ' ';
		string += ptrTypeFlagString;
	}

	if (m_ptrTypeKind != FunctionPtrTypeKind_Normal)
	{
		string += getFunctionPtrTypeKindString(m_ptrTypeKind);
		string += ' ';
	}

	if (!string.isEmpty())
		string.chop(1);

	return string;
}

void
FunctionPtrType::prepareTypeString()
{
	TypeStringTuple* tuple = getTypeStringTuple();
	Type* returnType = m_targetType->getReturnType();

	tuple->m_typeStringPrefix = returnType->getTypeStringPrefix();

	sl::String modifierString = getTypeModifierString();
	if (!modifierString.isEmpty())
	{
		tuple->m_typeStringPrefix += ' ';
		tuple->m_typeStringPrefix += modifierString;
	}

	tuple->m_typeStringPrefix += m_typeKind == TypeKind_FunctionRef ? " function&" : " function*";

	tuple->m_typeStringSuffix = m_targetType->getTypeStringSuffix();
	tuple->m_typeStringSuffix += returnType->getTypeStringSuffix();
}

void
FunctionPtrType::prepareDoxyLinkedText()
{
	TypeStringTuple* tuple = getTypeStringTuple();
	Type* returnType = m_targetType->getReturnType();

	tuple->m_doxyLinkedTextPrefix = returnType->getDoxyLinkedTextPrefix();

	sl::String modifierString = getTypeModifierString();
	if (!modifierString.isEmpty())
	{
		tuple->m_typeStringPrefix += ' ';
		tuple->m_typeStringPrefix += modifierString;
	}

	tuple->m_doxyLinkedTextPrefix += m_typeKind == TypeKind_FunctionRef ? " function&" : " function*";

	tuple->m_doxyLinkedTextSuffix = m_targetType->getDoxyLinkedTextSuffix();
	tuple->m_doxyLinkedTextSuffix += returnType->getDoxyLinkedTextSuffix();
}

void
FunctionPtrType::prepareDoxyTypeString()
{
	Type::prepareDoxyTypeString();
	getTypeStringTuple()->m_doxyTypeString += m_targetType->getDoxyArgString();
}

void
FunctionPtrType::prepareLlvmType()
{
	m_llvmType = m_ptrTypeKind != FunctionPtrTypeKind_Thin ?
		m_module->m_typeMgr.getStdType(StdType_FunctionPtrStruct)->getLlvmType() :
		llvm::PointerType::get(m_targetType->getLlvmType(), 0);
}

void
FunctionPtrType::prepareLlvmDiType()
{
	m_llvmDiType = m_ptrTypeKind != FunctionPtrTypeKind_Thin ?
		m_module->m_typeMgr.getStdType(StdType_FunctionPtrStruct)->getLlvmDiType() :
		m_module->m_llvmDiBuilder.createPointerType(m_targetType);
}

void
FunctionPtrType::markGcRoots(
	const void* p,
	rt::GcHeap* gcHeap
	)
{
	ASSERT(m_ptrTypeKind == FunctionPtrTypeKind_Normal || m_ptrTypeKind == FunctionPtrTypeKind_Weak);

	FunctionPtr* ptr = (FunctionPtr*)p;
	if (!ptr->m_closure)
		return;

	Box* box = ptr->m_closure->m_box;
	if (m_ptrTypeKind == FunctionPtrTypeKind_Normal)
		gcHeap->markClass(box);
	else if (isClassType(box->m_type, ClassTypeKind_FunctionClosure))
		gcHeap->weakMarkClosureClass(box);
	else  // simple weak closure
		gcHeap->weakMark(box);
}

bool
FunctionPtrType::calcLayout()
{
	bool result = m_targetType->ensureLayout();
	if (!result)
		return false;

	// update signature

	sl::String signature = createSignature(
		m_targetType,
		m_typeKind,
		m_ptrTypeKind,
		m_flags
		);

	m_module->m_typeMgr.updateTypeSignature(this, signature);
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
