#include "pch.h"
#include "jnc_FunctionPtrType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

const char*
getFunctionPtrTypeKindString (FunctionPtrTypeKind ptrTypeKind)
{
	static const char* stringTable [FunctionPtrTypeKind__Count] = 
	{
		"closure",  // EFunctionPtrType_Normal = 0,
		"weak",     // EFunctionPtrType_Weak,
		"thin",     // EFunctionPtrType_Thin,
	};

	return (size_t) ptrTypeKind < FunctionPtrTypeKind__Count ? 
		stringTable [ptrTypeKind] : 
		"undefined-function-ptr-kind";
}

//.............................................................................

FunctionPtrType::FunctionPtrType ()
{
	m_typeKind = TypeKind_FunctionPtr;
	m_ptrTypeKind = FunctionPtrTypeKind_Normal;
	m_size = sizeof (FunctionPtr);
	m_targetType = NULL;
	m_multicastType = NULL;
}

ClassType* 
FunctionPtrType::getMulticastType ()
{
	return m_module->m_typeMgr.getMulticastType (this);
}

rtl::String
FunctionPtrType::createSignature (
	FunctionType* functionType,
	TypeKind typeKind,
	FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	rtl::String signature = typeKind == TypeKind_FunctionRef ? "RF" : "PF";

	switch (ptrTypeKind)
	{
	case FunctionPtrTypeKind_Thin:
		signature += 't';
		break;

	case FunctionPtrTypeKind_Weak:
		signature += 'w';
		break;
	}

	signature += getPtrTypeFlagSignature (flags);
	signature += functionType->getSignature ();
	return signature;
}

rtl::String
FunctionPtrType::getTypeModifierString ()
{
	if (!m_typeModifierString.isEmpty ())
		return m_typeModifierString;

	if (m_flags & PtrTypeFlag__AllMask)
	{
		m_typeModifierString += getPtrTypeFlagString (m_flags);
		m_typeModifierString += ' ';
	}

	if (m_ptrTypeKind != FunctionPtrTypeKind_Normal)
	{
		m_typeModifierString += getFunctionPtrTypeKindString (m_ptrTypeKind);
		m_typeModifierString += ' ';
	}

	m_typeModifierString += m_targetType->getTypeModifierString ();

	return m_typeModifierString;
}

void
FunctionPtrType::prepareTypeString ()
{
	m_typeString = m_targetType->getReturnType ()->getTypeString ();
	m_typeString += ' ';
	m_typeString += getTypeModifierString ();
	m_typeString += m_typeKind == TypeKind_FunctionRef ? "function& " : "function* ";
	m_typeString += m_targetType->getArgString ();
}

void
FunctionPtrType::prepareLlvmType ()
{
	m_llvmType = m_ptrTypeKind != FunctionPtrTypeKind_Thin ? 
		m_module->m_typeMgr.getStdType (StdType_FunctionPtrStruct)->getLlvmType () :
		llvm::PointerType::get (m_targetType->getLlvmType (), 0);
}

void
FunctionPtrType::prepareLlvmDiType ()
{
	m_llvmDiType = m_ptrTypeKind != FunctionPtrTypeKind_Thin ? 
		m_module->m_typeMgr.getStdType (StdType_FunctionPtrStruct)->getLlvmDiType () :
		m_module->m_llvmDiBuilder.createPointerType (m_targetType);
}

void
FunctionPtrType::markGcRoots (
	const void* p,
	GcHeap* gcHeap
	)
{
	ASSERT (m_ptrTypeKind == FunctionPtrTypeKind_Normal || m_ptrTypeKind == FunctionPtrTypeKind_Weak);

	FunctionPtr* ptr = (FunctionPtr*) p;
	if (!ptr->m_closure)
		return;

	Box* box = ptr->m_closure->m_box;
	if (m_ptrTypeKind == FunctionPtrTypeKind_Normal)
		gcHeap->markClass (box);
	else if (isClassType (box->m_type, ClassTypeKind_FunctionClosure))
		gcHeap->weakMarkClosureClass (box);
	else  // simple weak closure
		gcHeap->weakMark (box);
}

//.............................................................................

} // namespace jnc {
