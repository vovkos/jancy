#include "pch.h"
#include "jnc_BinOp_Idx.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
CBinOp_Idx::GetResultType (
	const CValue& OpValue1,
	const CValue& OpValue2
	)
{
	CType* pOpType1 = OpValue1.GetType ();
	if (pOpType1->GetTypeKind () == EType_DataRef)
	{
		CDataPtrType* pPtrType = (CDataPtrType*) pOpType1;
		CType* pBaseType = pPtrType->GetTargetType ();

		if (pBaseType->GetTypeKind () == EType_Array)
			return ((CArrayType*) pBaseType)->GetElementType ()->GetDataPtrType (
				EType_DataRef, 
				pPtrType->GetPtrTypeKind (), 
				pPtrType->GetFlags ()
				);

		pOpType1 = pBaseType;
	}

	CDataPtrType* pPtrType;

	EType TypeKind = pOpType1->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_DataPtr:
		pPtrType = (CDataPtrType*) pOpType1;
		return pPtrType->GetTargetType ()->GetDataPtrType (
			EType_DataRef, 
			pPtrType->GetPtrTypeKind (), 
			pPtrType->GetFlags ()
			);

	case EType_Array:
		return ((CArrayType*) pOpType1)->GetElementType ();

	case EType_PropertyRef:
	case EType_PropertyPtr:
		return GetPropertyIndexResultType (OpValue1, OpValue2);

	default:
		err::SetFormatStringError ("cannot index '%s'", pOpType1->GetTypeString ().cc ()); // thanks a lot gcc
		return NULL;
	}
}

bool
CBinOp_Idx::Operator (
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	bool Result;

	CValue OpValue1 = RawOpValue1;
	CValue OpValue2 = RawOpValue2;

	CType* pOpType1 = RawOpValue1.GetType ();
	if (pOpType1->GetTypeKind () == EType_DataRef)
	{
		CType* pBaseType = ((CDataPtrType*) pOpType1)->GetTargetType ();

		if (pBaseType->GetTypeKind () == EType_Array)
			return 
				m_pModule->m_OperatorMgr.CastOperator (&OpValue2, EType_Int_p) &&
				ArrayIndexOperator (RawOpValue1, (CArrayType*) pBaseType, OpValue2, pResultValue);

		Result = m_pModule->m_OperatorMgr.LoadDataRef (RawOpValue1, &OpValue1);
		if (!Result)
			return false;

		pOpType1 = OpValue1.GetType ();
	}

	EType TypeKind = pOpType1->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_DataPtr:
		return 
			m_pModule->m_OperatorMgr.CastOperator (&OpValue2, EType_Int_p) &&
			m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Add, OpValue1, OpValue2, &OpValue1) &&
			m_pModule->m_OperatorMgr.UnaryOperator (EUnOp_Indir, OpValue1, pResultValue);

	case EType_Array:
		return 
			m_pModule->m_OperatorMgr.CastOperator (&OpValue2, EType_Int_p) &&
			ArrayIndexOperator (OpValue1, (CArrayType*) pOpType1, OpValue2, pResultValue);

	case EType_PropertyRef:
	case EType_PropertyPtr:
		return PropertyIndexOperator (OpValue1, OpValue2, pResultValue);

	default:
		err::SetFormatStringError ("cannot index '%s'", pOpType1->GetTypeString ().cc ()); 
		return false;
	}
}

bool
CBinOp_Idx::ArrayIndexOperator (
	const CValue& OpValue1,
	CArrayType* pArrayType,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	CType* pElementType = pArrayType->GetElementType ();

	if (OpValue1.GetValueKind () == EValue_Const && OpValue2.GetValueKind ())
	{
		void* p = (char*) OpValue1.GetConstData () + OpValue2.GetSizeT () * pElementType->GetSize ();
		pResultValue->CreateConst (p, pElementType);
		return true;
	}

	EType OpTypeKind1 = OpValue1.GetType ()->GetTypeKind ();

	if (OpTypeKind1 != EType_DataRef)
	{
		ASSERT (OpTypeKind1 == EType_Array);
		err::SetFormatStringError ("indexing register-based arrays is not supported yet");
		return false;
	}

	CDataPtrType* pOpType1 = (CDataPtrType*) OpValue1.GetType ();

	CDataPtrType* pPtrType;

	uint_t PtrTypeFlags = pOpType1->GetFlags ();

	if (PtrTypeFlags & EPtrTypeFlag_Safe)
	{
		if (OpValue2.GetValueKind () == EValue_Const)
		{			
			CValue IdxValue;
			bool Result = m_pModule->m_OperatorMgr.CastOperator (OpValue2, EType_Int_p, &IdxValue);
			if (!Result)
				return false;

			intptr_t i = IdxValue.GetSizeT ();
			if (i < 0 || i >= (intptr_t) pArrayType->GetElementCount ())
			{
				err::SetFormatStringError ("index '%d' is out of bounds in '%s'", i, pArrayType->GetTypeString ().cc ());
				return false;
			}
		}
		else
		{
			PtrTypeFlags &= ~EPtrTypeFlag_Safe;
		}
	}
	
	CValue PtrValue;

	EDataPtrType PtrTypeKind = pOpType1->GetPtrTypeKind ();
	if (PtrTypeKind == EDataPtrType_Thin)
	{
		pPtrType = pElementType->GetDataPtrType (EType_DataRef, EDataPtrType_Thin, PtrTypeFlags);
		m_pModule->m_LlvmIrBuilder.CreateGep2 (OpValue1, OpValue2, pPtrType, pResultValue);
	}
	else if (PtrTypeKind == EDataPtrType_Lean)
	{
		pPtrType = pElementType->GetDataPtrType (EType_DataRef, EDataPtrType_Lean, PtrTypeFlags);
		m_pModule->m_LlvmIrBuilder.CreateGep2 (OpValue1, OpValue2, pPtrType, pResultValue);

		if (OpValue1.GetValueKind () == EValue_Variable)
			pResultValue->SetLeanDataPtrValidator (OpValue1);
		else
			pResultValue->SetLeanDataPtrValidator (OpValue1.GetLeanDataPtrValidator ());
	}
	else // EDataPtrType_Normal
	{
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue1, 0, NULL, &PtrValue);

		pPtrType = pElementType->GetDataPtrType_c ();

		m_pModule->m_LlvmIrBuilder.CreateGep2 (PtrValue, OpValue2, NULL, &PtrValue);

		pPtrType = pElementType->GetDataPtrType (EType_DataRef, EDataPtrType_Lean, PtrTypeFlags);

		pResultValue->SetLeanDataPtr (
			PtrValue.GetLlvmValue (), 
			pPtrType,
			OpValue1
			);
	}

	return true;
}

bool
CBinOp_Idx::PropertyIndexOperator (
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	*pResultValue = RawOpValue1;
	pResultValue->InsertToClosureTail (RawOpValue2);
	return true;
}

CType*
CBinOp_Idx::GetPropertyIndexResultType (
	const CValue& RawOpValue1,
	const CValue& RawOpValue2
	)
{
	CValue ResultValue;
	PropertyIndexOperator (RawOpValue1, RawOpValue2, &ResultValue);
	return ResultValue.GetClosure ()->GetClosureType (RawOpValue1.GetType ());
}

//.............................................................................

} // namespace jnc {
