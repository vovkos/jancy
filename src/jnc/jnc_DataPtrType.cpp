#include "pch.h"
#include "jnc_DataPtrType.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

const char*
GetDataPtrTypeKindString (EDataPtrType PtrTypeKind)
{
	static const char* StringTable [EDataPtrType__Count] = 
	{
		"normal", // EDataPtrType_Normal = 0,
		"lean",   // EDataPtrType_Lean,
		"thin",   // EDataPtrType_Thin,
	};
		
	return (size_t) PtrTypeKind < EDataPtrType__Count ? 
		StringTable [PtrTypeKind] : 
		"undefined-data-ptr-kind";
}

//.............................................................................

CDataPtrType::CDataPtrType ()
{
	m_TypeKind = EType_DataPtr;
	m_PtrTypeKind = EDataPtrType_Normal;
	m_pTargetType = NULL;
	m_pAnchorNamespace = NULL;
	m_Size = sizeof (TDataPtr);
}

bool
CDataPtrType::IsConstPtrType ()
{
	return 
		(m_Flags & EPtrTypeFlag_Const) != 0 || 
		(m_Flags & EPtrTypeFlag_ConstD) != 0 && 
		m_pModule->m_NamespaceMgr.GetAccessKind (m_pAnchorNamespace) == EAccess_Public;
}

CStructType* 
CDataPtrType::GetDataPtrStructType ()
{
	return m_pModule->m_TypeMgr.GetDataPtrStructType (m_pTargetType);
}

rtl::CString
CDataPtrType::CreateSignature (
	CType* pBaseType,
	EType TypeKind,
	EDataPtrType PtrTypeKind,
	uint_t Flags
	)
{
	rtl::CString Signature = TypeKind == EType_DataRef ? "RD" : "PD";

	switch (PtrTypeKind)
	{
	case EDataPtrType_Lean:
		Signature += 'l';
		break;

	case EDataPtrType_Thin:
		Signature += 't';
		break;
	}

	Signature += GetPtrTypeFlagSignature (Flags);
	Signature += pBaseType->GetSignature ();
	return Signature;
}

void
CDataPtrType::PrepareTypeString ()
{
	m_TypeString = m_pTargetType->GetTypeString ();

	if (m_Flags & EPtrTypeFlag__AllMask)
	{
		m_TypeString += ' ';
		m_TypeString += GetPtrTypeFlagString (m_Flags);
	}

	if (m_PtrTypeKind != EDataPtrType_Normal)
	{
		m_TypeString += ' ';
		m_TypeString += GetDataPtrTypeKindString (m_PtrTypeKind);
	}

	m_TypeString += m_TypeKind == EType_DataRef ? "&" : "*";
}

void
CDataPtrType::PrepareLlvmType ()
{
	m_pLlvmType = 
		m_PtrTypeKind == EDataPtrType_Normal ? GetDataPtrStructType ()->GetLlvmType () :
		m_pTargetType->GetTypeKind () != EType_Void ? llvm::PointerType::get (m_pTargetType->GetLlvmType (), 0) :
		m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr)->GetLlvmType ();
}

void
CDataPtrType::PrepareLlvmDiType ()
{
	m_LlvmDiType = 
		m_PtrTypeKind == EDataPtrType_Normal ? GetDataPtrStructType ()->GetLlvmDiType () :
		m_pTargetType->GetTypeKind () != EType_Void ? m_pModule->m_LlvmDiBuilder.CreatePointerType (m_pTargetType) :
		m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr)->GetLlvmDiType ();
}

void
CDataPtrType::GcMark (
	CRuntime* pRuntime,
	void* p
	)
{
	ASSERT (m_PtrTypeKind == EDataPtrType_Normal);

	TDataPtr* pPtr = (TDataPtr*) p;		
	TObjHdr* pObject = pPtr->m_pObject;
	if (!pObject || pObject->m_ScopeLevel)
		return;

	pObject->GcMarkData (pRuntime);
}

//.............................................................................

TDataPtr
StrDup (
	const char* p,
	size_t Length
	)
{
	if (Length == -1)
		Length = p ? strlen (p) : 0;

	char* pDst = (char*) AXL_MEM_ALLOC (Length + 1);
	memcpy (pDst, p, Length);
	pDst [Length] = 0;

	jnc::TDataPtr Ptr;
	Ptr.m_p = pDst;
	Ptr.m_pRangeBegin = pDst;
	Ptr.m_pRangeEnd = pDst + Length + 1;
	Ptr.m_pObject = jnc::GetStaticObjHdr ();
	return Ptr;
}

//.............................................................................

} // namespace jnc {

