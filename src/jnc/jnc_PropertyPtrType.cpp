#include "pch.h"
#include "jnc_PropertyPtrType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

const char*
GetPropertyPtrTypeKindString (EPropertyPtrType PtrTypeKind)
{
	static const char* StringTable [EPropertyPtrType__Count] =
	{
		"closure",  // EPropertyPtrType_Normal = 0,
		"weak",     // EPropertyPtrType_Weak,
		"thin",     // EPropertyPtrType_Thin,
	};

	return (size_t) PtrTypeKind < EPropertyPtrType__Count ?
		StringTable [PtrTypeKind] :
		"undefined-property-ptr-kind";
}

//.............................................................................

CPropertyPtrType::CPropertyPtrType ()
{
	m_TypeKind = EType_PropertyPtr;
	m_PtrTypeKind = EPropertyPtrType_Normal;
	m_Size = sizeof (TPropertyPtr);
	m_pTargetType = NULL;
	m_pAnchorNamespace = NULL;
}

bool
CPropertyPtrType::IsConstPtrType ()
{
	return
		m_pTargetType->IsReadOnly () ||
		(m_Flags & EPtrTypeFlag_Const) != 0 ||
		(m_Flags & EPtrTypeFlag_ConstD) != 0 &&
		m_pModule->m_NamespaceMgr.GetAccessKind (m_pAnchorNamespace) == EAccess_Public;
}

CStructType*
CPropertyPtrType::GetPropertyPtrStructType ()
{
	return m_pModule->m_TypeMgr.GetPropertyPtrStructType (m_pTargetType);
}

rtl::CString
CPropertyPtrType::CreateSignature (
	CPropertyType* pPropertyType,
	EType TypeKind,
	EPropertyPtrType PtrTypeKind,
	uint_t Flags
	)
{
	rtl::CString Signature = TypeKind == EType_PropertyRef ? "RX" : "PX";

	switch (PtrTypeKind)
	{
	case EPropertyPtrType_Thin:
		Signature += 't';
		break;

	case EPropertyPtrType_Weak:
		Signature += 'w';
		break;
	}

	Signature += GetPtrTypeFlagSignature (Flags);
	Signature += pPropertyType->GetSignature ();
	return Signature;
}

void
CPropertyPtrType::PrepareTypeString ()
{
	m_TypeString = m_pTargetType->GetReturnType ()->GetTypeString ();
	m_TypeString += ' ';
	m_TypeString += m_pTargetType->GetTypeModifierString ();

	if (m_Flags & EPtrTypeFlag__AllMask)
	{
		m_TypeString += GetPtrTypeFlagString (m_Flags);
		m_TypeString += ' ';
	}

	if (m_PtrTypeKind != EPropertyPtrType_Normal)
	{
		m_TypeString += GetPropertyPtrTypeKindString (m_PtrTypeKind);
		m_TypeString += ' ';
	}

	m_TypeString += m_TypeKind == EType_PropertyRef ? "property&" : "property*";

	if (m_pTargetType->IsIndexed ())
	{
		m_TypeString += ' ';
		m_TypeString += m_pTargetType->GetGetterType ()->GetArgString ();
	}
}

void
CPropertyPtrType::PrepareLlvmType ()
{
	m_pLlvmType =
		m_PtrTypeKind != EPropertyPtrType_Thin ? GetPropertyPtrStructType ()->GetLlvmType () :
		m_pTargetType->GetVTableStructType ()->GetDataPtrType_c ()->GetLlvmType ();
}

void
CPropertyPtrType::PrepareLlvmDiType ()
{
	m_LlvmDiType =
		m_PtrTypeKind != EPropertyPtrType_Thin ? GetPropertyPtrStructType ()->GetLlvmDiType () :
		m_pTargetType->GetVTableStructType ()->GetDataPtrType_c ()->GetLlvmDiType ();
}

void
CPropertyPtrType::GcMark (
	CRuntime* pRuntime,
	void* p
	)
{
	ASSERT (m_PtrTypeKind == EPropertyPtrType_Normal || m_PtrTypeKind == EPropertyPtrType_Weak);

	TPropertyPtr* pPtr = (TPropertyPtr*) p;
	if (!pPtr->m_pClosure || pPtr->m_pClosure->m_pObject->m_ScopeLevel)
		return;

	TObjHdr* pObject = pPtr->m_pClosure->m_pObject;
	if (m_PtrTypeKind == EPropertyPtrType_Normal)
		pObject->GcMarkObject (pRuntime);
	else if (pObject->m_pClassType->GetClassTypeKind () == EClassType_FunctionClosure)
		pObject->GcWeakMarkClosureObject (pRuntime);
	else  // simple weak closure
		pObject->GcWeakMarkObject ();
}

//.............................................................................

} // namespace jnc {
