#include "pch.h"
#include "jnc_FunctionPtrType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

const char*
GetFunctionPtrTypeKindString (EFunctionPtrType PtrTypeKind)
{
	static const char* StringTable [EFunctionPtrType__Count] = 
	{
		"closure",  // EFunctionPtrType_Normal = 0,
		"weak",     // EFunctionPtrType_Weak,
		"thin",     // EFunctionPtrType_Thin,
	};

	return (size_t) PtrTypeKind < EFunctionPtrType__Count ? 
		StringTable [PtrTypeKind] : 
		"undefined-function-ptr-kind";
}

//.............................................................................

CFunctionPtrType::CFunctionPtrType ()
{
	m_TypeKind = EType_FunctionPtr;
	m_PtrTypeKind = EFunctionPtrType_Normal;
	m_Size = sizeof (TFunctionPtr);
	m_pTargetType = NULL;
	m_pMulticastType = NULL;
}

CStructType* 
CFunctionPtrType::GetFunctionPtrStructType ()
{
	return m_pModule->m_TypeMgr.GetFunctionPtrStructType (m_pTargetType);
}

CClassType* 
CFunctionPtrType::GetMulticastType ()
{
	return m_pModule->m_TypeMgr.GetMulticastType (this);
}

rtl::CString
CFunctionPtrType::CreateSignature (
	CFunctionType* pFunctionType,
	EType TypeKind,
	EFunctionPtrType PtrTypeKind,
	uint_t Flags
	)
{
	rtl::CString Signature = TypeKind == EType_FunctionRef ? "RF" : "PF";

	switch (PtrTypeKind)
	{
	case EFunctionPtrType_Thin:
		Signature += 't';
		break;

	case EFunctionPtrType_Weak:
		Signature += 'w';
		break;
	}

	Signature += GetPtrTypeFlagSignature (Flags);
	Signature += pFunctionType->GetSignature ();
	return Signature;
}

rtl::CString
CFunctionPtrType::GetTypeModifierString ()
{
	if (!m_TypeModifierString.IsEmpty ())
		return m_TypeModifierString;

	if (m_Flags & EPtrTypeFlag__AllMask)
	{
		m_TypeModifierString += GetPtrTypeFlagString (m_Flags);
		m_TypeModifierString += ' ';
	}

	if (m_PtrTypeKind != EFunctionPtrType_Normal)
	{
		m_TypeModifierString += GetFunctionPtrTypeKindString (m_PtrTypeKind);
		m_TypeModifierString += ' ';
	}

	m_TypeModifierString += m_pTargetType->GetTypeModifierString ();

	return m_TypeModifierString;
}

void
CFunctionPtrType::PrepareTypeString ()
{
	m_TypeString = m_pTargetType->GetReturnType ()->GetTypeString ();
	m_TypeString += ' ';
	m_TypeString += GetTypeModifierString ();
	m_TypeString += m_TypeKind == EType_FunctionRef ? "function& " : "function* ";
	m_TypeString += m_pTargetType->GetArgString ();
}

void
CFunctionPtrType::PrepareLlvmType ()
{
	m_pLlvmType = 
		m_PtrTypeKind != EFunctionPtrType_Thin ? GetFunctionPtrStructType ()->GetLlvmType () :
		llvm::PointerType::get (m_pTargetType->GetLlvmType (), 0);
}

void
CFunctionPtrType::PrepareLlvmDiType ()
{
	m_LlvmDiType = 
		m_PtrTypeKind != EFunctionPtrType_Thin ? GetFunctionPtrStructType ()->GetLlvmDiType () :
		m_pModule->m_LlvmDiBuilder.CreatePointerType (m_pTargetType);
}

void
CFunctionPtrType::GcMark (
	CRuntime* pRuntime,
	void* p
	)
{
	ASSERT (m_PtrTypeKind == EFunctionPtrType_Normal || m_PtrTypeKind == EFunctionPtrType_Weak);

	TFunctionPtr* pPtr = (TFunctionPtr*) p;
	if (!pPtr->m_pClosure || pPtr->m_pClosure->m_pObject->m_ScopeLevel)
		return;

	TObjHdr* pObject = pPtr->m_pClosure->m_pObject;
	if (m_PtrTypeKind == EFunctionPtrType_Normal)
		pObject->GcMarkObject (pRuntime);
	else if (pObject->m_pClassType->GetClassTypeKind () == EClassType_FunctionClosure)
		pObject->GcWeakMarkClosureObject (pRuntime);
	else  // simple weak closure
		pObject->GcWeakMarkObject ();
}

//.............................................................................

} // namespace jnc {
