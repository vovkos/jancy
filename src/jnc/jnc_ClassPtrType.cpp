#include "pch.h"
#include "jnc_ClassPtrType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

const char*
GetClassPtrTypeKindString (EClassPtrType PtrTypeKind)
{
	static const char* StringTable [EClassPtrType__Count] = 
	{
		"strong", // EClassPtrType_Normal = 0,
		"weak",   // EClassPtrType_Weak,
	};

	return (size_t) PtrTypeKind < EClassPtrType__Count ? 
		StringTable [PtrTypeKind] : 
		"undefined-class-ptr-kind";
}

//.............................................................................

CClassPtrType::CClassPtrType ()
{
	m_TypeKind = EType_ClassPtr;
	m_PtrTypeKind = EClassPtrType_Normal;
	m_pTargetType = NULL;
	m_pAnchorNamespace = NULL;
	m_Size = sizeof (void*);
	m_AlignFactor = sizeof (void*);
}

bool
CClassPtrType::IsConstPtrType ()
{
	return 
		(m_Flags & EPtrTypeFlag_Const) != 0 || 
		(m_Flags & EPtrTypeFlag_ConstD) != 0 && 
		m_pModule->m_NamespaceMgr.GetAccessKind (m_pAnchorNamespace) == EAccess_Public;
}

bool
CClassPtrType::IsEventPtrType ()
{
	return 
		(m_Flags & EPtrTypeFlag_Event) != 0 || 
		(m_Flags & EPtrTypeFlag_EventD) != 0 && 
		m_pModule->m_NamespaceMgr.GetAccessKind (m_pAnchorNamespace) == EAccess_Public;
}

rtl::CString
CClassPtrType::CreateSignature (
	CClassType* pClassType,
	EType TypeKind,
	EClassPtrType PtrTypeKind,
	uint_t Flags
	)
{
	rtl::CString Signature = TypeKind == EType_ClassRef ? "RC" : "PC";

	if (PtrTypeKind == EClassPtrType_Weak)
		Signature += 'w';

	Signature += GetPtrTypeFlagSignature (Flags);
	Signature += pClassType->GetSignature ();
	return Signature;
}

void
CClassPtrType::PrepareTypeString ()
{
	m_TypeString += m_pTargetType->GetTypeString ();

	if (m_Flags & EPtrTypeFlag__AllMask)
	{
		m_TypeString += ' ';
		m_TypeString += GetPtrTypeFlagString (m_Flags);
	}

	if (m_PtrTypeKind != EClassPtrType_Normal)
	{
		m_TypeString += ' ';
		m_TypeString += GetClassPtrTypeKindString (m_PtrTypeKind);
	}

	m_TypeString += m_TypeKind == EType_ClassRef ? "&" : "*";
}

void
CClassPtrType::PrepareLlvmType ()
{
	m_pLlvmType = llvm::PointerType::get (m_pTargetType->GetIfaceStructType ()->GetLlvmType (), 0);
}

void
CClassPtrType::PrepareLlvmDiType ()
{
	m_LlvmDiType = m_pModule->m_LlvmDiBuilder.CreatePointerType (m_pTargetType->GetIfaceStructType ());
}

void
CClassPtrType::GcMark (
	CRuntime* pRuntime,
	void* p
	)
{
	TIfaceHdr* pIface = *(TIfaceHdr**) p;
	if (!pIface || pIface->m_pObject->m_ScopeLevel)
		return;

	TObjHdr* pObject = pIface->m_pObject;
	if (m_PtrTypeKind == EClassPtrType_Weak)
		pObject->GcWeakMarkObject ();
	else
		pObject->GcMarkObject (pRuntime);
}

//.............................................................................

} // namespace jnc {
