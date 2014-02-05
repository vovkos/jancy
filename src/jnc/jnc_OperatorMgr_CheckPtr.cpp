#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
COperatorMgr::GetDataRefObjHdr (
	const CValue& Value,
	CValue* pResultValue
	)
{
	ASSERT (Value.GetType ()->GetTypeKind () == EType_DataRef);
	CDataPtrType* pPtrType = (CDataPtrType*) Value.GetType ();
	EDataPtrType PtrTypeKind = pPtrType->GetPtrTypeKind ();

	if (PtrTypeKind == EDataPtrType_Lean)
	{
		GetLeanDataPtrObjHdr (Value, pResultValue);
	}
	else
	{
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (
			Value,
			3,
			m_pModule->m_TypeMgr.GetStdType (EStdType_ObjHdrPtr),
			pResultValue
			);
	}
}

void
COperatorMgr::CheckDataPtrRange (
	const CValue& RawPtrValue,
	size_t Size,
	const CValue& RangeBeginValue,
	const CValue& RangeEndValue
	)
{
	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "check data pointer range");

	CValue SizeValue (Size, EType_SizeT);

	CValue PtrValue;
	m_pModule->m_LlvmIrBuilder.CreateBitCast (RawPtrValue, m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr), &PtrValue);

	CValue ArgValueArray [] =
	{
		PtrValue,
		SizeValue,
		RangeBeginValue,
		RangeEndValue,
	};

	CFunction* pCheckDataPtrRange = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_CheckDataPtrRange);

	m_pModule->m_LlvmIrBuilder.CreateCall (
		pCheckDataPtrRange,
		pCheckDataPtrRange->GetType (),
		ArgValueArray,
		countof (ArgValueArray),
		NULL
		);
}

void
COperatorMgr::CheckDataPtrRange (const CValue& Value)
{
	ASSERT (Value.GetType ()->GetTypeKind () == EType_DataPtr || Value.GetType ()->GetTypeKind () == EType_DataRef);
	CDataPtrType* pType = (CDataPtrType*) Value.GetType ();
	EDataPtrType PtrTypeKind = pType->GetPtrTypeKind ();

	if (pType->GetFlags () & EPtrTypeFlag_Checked)
		return;

	CValue PtrValue;
	CValue RangeBeginValue;
	CValue RangeEndValue;

	switch (PtrTypeKind)
	{
	case EDataPtrType_Thin:
		return;

	case EDataPtrType_Lean:
		PtrValue = Value;
		GetLeanDataPtrRange (Value, &RangeBeginValue, &RangeEndValue);
		break;

	case EDataPtrType_Normal:
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (
			Value, 0, 
			pType->GetTargetType ()->GetDataPtrType_c (), 
			&PtrValue
			);

		m_pModule->m_LlvmIrBuilder.CreateExtractValue (Value, 1, NULL, &RangeBeginValue);
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (Value, 2, NULL, &RangeEndValue);
		break;

	default:
		ASSERT (false);
	}

	CheckDataPtrRange (PtrValue, pType->GetTargetType ()->GetSize (), RangeBeginValue, RangeEndValue);
}

bool
COperatorMgr::CheckDataPtrScopeLevel (
	const CValue& SrcValue,
	const CValue& DstValue
	)
{
	ASSERT (SrcValue.GetType ()->GetTypeKind () == EType_DataPtr);

	CDataPtrType* pPtrType = (CDataPtrType*) SrcValue.GetType ();
	EDataPtrType PtrTypeKind = pPtrType->GetPtrTypeKind ();
		
	if (PtrTypeKind == EDataPtrType_Thin) // in general case we can't deduce scope-level
		return true;

	if (SrcValue.GetValueKind () == EValue_Variable && DstValue.GetValueKind () == EValue_Variable)
	{
		if (SrcValue.GetVariable ()->GetScopeLevel () > DstValue.GetVariable ()->GetScopeLevel ())
		{
			err::SetFormatStringError ("data pointer scope level mismatch");
			return false;
		}

		return true;
	}

	CValue SrcObjHdrValue;

	if (PtrTypeKind == EDataPtrType_Lean)
		GetLeanDataPtrObjHdr (SrcValue, &SrcObjHdrValue);
	else
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (SrcValue, 3, m_pModule->m_TypeMgr.GetStdType (EStdType_ObjHdrPtr), &SrcObjHdrValue);

	CValue DstObjHdrValue;
	GetDataRefObjHdr (DstValue, &DstObjHdrValue);

	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "check data pointer scope level");

	CFunction* pCheckFunction = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_CheckScopeLevel);

	m_pModule->m_LlvmIrBuilder.CreateCall2 (
		pCheckFunction,
		pCheckFunction->GetType (),
		SrcObjHdrValue,
		DstObjHdrValue,
		NULL
		);

	return true;
}

void
COperatorMgr::CheckClassPtrScopeLevel (
	const CValue& SrcValue,
	const CValue& DstValue
	)
{
	ASSERT (SrcValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_ClassPtr);

	CValue DstObjHdrValue;
	GetDataRefObjHdr (DstValue, &DstObjHdrValue);

	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "check class scope level");

	CValue IfaceValue;
	m_pModule->m_LlvmIrBuilder.CreateBitCast (SrcValue, m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr), &IfaceValue);

	CFunction* pCheckFunction = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_CheckClassPtrScopeLevel);

	m_pModule->m_LlvmIrBuilder.CreateCall2 (
		pCheckFunction,
		pCheckFunction->GetType (),
		IfaceValue,
		DstObjHdrValue,
		NULL
		);
}

void
COperatorMgr::CheckClassPtrNull (const CValue& Value)
{
	ASSERT (Value.GetType ()->GetTypeKindFlags () & ETypeKindFlag_ClassPtr);

	CClassPtrType* pPtrType = (CClassPtrType*) Value.GetType ();
	EClassPtrType PtrTypeKind = pPtrType->GetPtrTypeKind ();

	if (pPtrType->GetFlags () & EPtrTypeFlag_Checked)
		return;

	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "check null class pointer");

	CValue PtrValue;
	m_pModule->m_LlvmIrBuilder.CreateBitCast (Value, m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr), &PtrValue);

	CFunction* pCheckFunction = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_CheckNullPtr);

	m_pModule->m_LlvmIrBuilder.CreateCall2 (
		pCheckFunction,
		pCheckFunction->GetType (),
		PtrValue,
		CValue (ERuntimeError_NullClassPtr, EType_Int),
		NULL
		);
}

void
COperatorMgr::CheckFunctionPtrNull (const CValue& Value)
{
	ASSERT (Value.GetType ()->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr);

	CFunctionPtrType* pPtrType = (CFunctionPtrType*) Value.GetType ();
	EFunctionPtrType PtrTypeKind = pPtrType->GetPtrTypeKind ();

	if (pPtrType->GetFlags () & EPtrTypeFlag_Checked)
		return;

	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "check null function pointer");

	CValue PtrValue;

	if (PtrTypeKind == EFunctionPtrType_Thin)
		PtrValue = Value;
	else
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (Value, 0, NULL, &PtrValue);

	m_pModule->m_LlvmIrBuilder.CreateBitCast (PtrValue, m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr), &PtrValue);

	CFunction* pCheckFunction = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_CheckNullPtr);

	m_pModule->m_LlvmIrBuilder.CreateCall2 (
		pCheckFunction,
		pCheckFunction->GetType (),
		PtrValue,
		CValue (ERuntimeError_NullFunctionPtr, EType_Int),
		NULL
		);
}

void
COperatorMgr::CheckFunctionPtrScopeLevel (
	const CValue& SrcValue,
	const CValue& DstValue
	)
{
	ASSERT (SrcValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr);
	CFunctionPtrType* pPtrType = (CFunctionPtrType*) SrcValue.GetType ();

	if (!pPtrType->HasClosure ())
		return;

	CValue ClosureValue;
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (SrcValue, 1, m_pModule->GetSimpleType (EStdType_ObjectPtr), &ClosureValue);
	CheckClassPtrScopeLevel (ClosureValue, DstValue);
}

void
COperatorMgr::CheckPropertyPtrNull (const CValue& Value)
{
	ASSERT (Value.GetType ()->GetTypeKindFlags () & ETypeKindFlag_PropertyPtr);

	CPropertyPtrType* pPtrType = (CPropertyPtrType*) Value.GetType ();
	EPropertyPtrType PtrTypeKind = pPtrType->GetPtrTypeKind ();

	if (pPtrType->GetFlags () & EPtrTypeFlag_Checked)
		return;

	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "check null property pointer");

	CValue PtrValue;

	if (PtrTypeKind == EPropertyPtrType_Thin)
		PtrValue = Value;
	else
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (Value, 0, NULL, &PtrValue);

	m_pModule->m_LlvmIrBuilder.CreateBitCast (PtrValue, m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr), &PtrValue);

	CFunction* pCheckFunction = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_CheckNullPtr);
	
	m_pModule->m_LlvmIrBuilder.CreateCall2 (
		pCheckFunction,
		pCheckFunction->GetType (),
		PtrValue,
		CValue (ERuntimeError_NullPropertyPtr, EType_Int),
		NULL
		);
}

void
COperatorMgr::CheckPropertyPtrScopeLevel (
	const CValue& SrcValue,
	const CValue& DstValue
	)
{
	ASSERT (SrcValue.GetType ()->GetTypeKind () == EType_PropertyPtr);
	CPropertyPtrType* pPtrType = (CPropertyPtrType*) SrcValue.GetType ();

	if (!pPtrType->HasClosure ())
		return;

	CValue ClosureValue;
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (SrcValue, 1, m_pModule->GetSimpleType (EStdType_ObjectPtr), &ClosureValue);
	CheckClassPtrScopeLevel (ClosureValue, DstValue);
}

//.............................................................................

} // namespace jnc {
