#include "pch.h"
#include "jnc_UnOp_Ptr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
CUnOp_Addr::GetResultType (const CValue& OpValue)
{
	CType* pOpType = OpValue.GetType ();
	EType OpTypeKind = pOpType->GetTypeKind ();
	switch (OpTypeKind)
	{
	case EType_DataRef:
		return ((CDataPtrType*) pOpType)->GetTargetType ()->GetDataPtrType (
			((CDataPtrType*) pOpType)->GetAnchorNamespace (),
			EType_DataPtr, 
			((CDataPtrType*) pOpType)->GetPtrTypeKind (),
			pOpType->GetFlags ()
			);

	case EType_ClassRef:
		return ((CClassPtrType*) pOpType)->GetTargetType ()->GetClassPtrType (
			((CClassPtrType*) pOpType)->GetAnchorNamespace (),
			EType_ClassPtr, 
			((CClassPtrType*) pOpType)->GetPtrTypeKind (),
			pOpType->GetFlags ()
			);

	case EType_FunctionRef:
		return ((CFunctionPtrType*) pOpType)->GetTargetType ()->GetFunctionPtrType (
			EType_FunctionPtr, 
			((CFunctionPtrType*) pOpType)->GetPtrTypeKind (),
			pOpType->GetFlags ()
			);

	case EType_PropertyRef:
		return ((CPropertyPtrType*) pOpType)->GetTargetType ()->GetPropertyPtrType (
			((CPropertyPtrType*) pOpType)->GetAnchorNamespace (),
			EType_PropertyPtr, 
			((CPropertyPtrType*) pOpType)->GetPtrTypeKind (),
			pOpType->GetFlags ()
			);

	default:
		err::SetFormatStringError ("can only apply unary '&' to a reference");
		return NULL;
	}
}

bool
CUnOp_Addr::Operator (
	const CValue& OpValue,
	CValue* pResultValue
	)
{
	CType* pResultType = GetResultType (OpValue);
	if (!pResultType)
		return false;
	
	pResultValue->OverrideType (OpValue, pResultType);
	return true;
}

//.............................................................................

CType*
CUnOp_Indir::GetResultType (const CValue& OpValue)
{
	CType* pOpType = OpValue.GetType ();
	EType OpTypeKind = pOpType->GetTypeKind ();
	switch (OpTypeKind)
	{
	case EType_DataPtr:
		return ((CDataPtrType*) pOpType)->GetTargetType ()->GetDataPtrType (
			((CDataPtrType*) pOpType)->GetAnchorNamespace (),
			EType_DataRef, 
			((CDataPtrType*) pOpType)->GetPtrTypeKind (),
			pOpType->GetFlags ()
			);

	case EType_ClassPtr:
		return ((CClassPtrType*) pOpType)->GetTargetType ()->GetClassPtrType (
			((CClassPtrType*) pOpType)->GetAnchorNamespace (),
			EType_ClassRef, 
			((CClassPtrType*) pOpType)->GetPtrTypeKind (),
			pOpType->GetFlags ()
			);

	case EType_FunctionPtr:
		return ((CFunctionPtrType*) pOpType)->GetTargetType ()->GetFunctionPtrType (
			EType_FunctionRef, 
			((CFunctionPtrType*) pOpType)->GetPtrTypeKind (),
			pOpType->GetFlags ()
			);

	case EType_PropertyPtr:
		return ((CPropertyPtrType*) pOpType)->GetTargetType ()->GetPropertyPtrType (
			((CPropertyPtrType*) pOpType)->GetAnchorNamespace (),
			EType_PropertyRef, 
			((CPropertyPtrType*) pOpType)->GetPtrTypeKind (),
			pOpType->GetFlags ()
			);

	default:
		err::SetFormatStringError ("can only apply unary '*' to a pointer");
		return NULL;
	}
}

bool
CUnOp_Indir::Operator (
	const CValue& OpValue,
	CValue* pResultValue
	)
{
	CType* pResultType = GetResultType (OpValue);
	if (!pResultType)
		return false;
	
	pResultValue->OverrideType (OpValue, pResultType);
	return true;
}

//.............................................................................

} // namespace jnc {
