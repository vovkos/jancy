#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
COperatorMgr::GetLeanDataPtrObjHdr (
	const CValue& Value,
	CValue* pResultValue
	)
{
	ASSERT (Value.GetType ()->GetTypeKindFlags () & ETypeKindFlag_DataPtr);

	EValue ValueKind = Value.GetValueKind ();
	if (ValueKind == EValue_Variable)
	{	
		*pResultValue = Value.GetVariable ()->GetScopeLevelObjHdr ();
		return;
	}

	ASSERT (Value.GetLeanDataPtrValidator ());
	CValue ScopeValidatorValue = Value.GetLeanDataPtrValidator ()->GetScopeValidator ();

	if (ScopeValidatorValue.GetValueKind () == EValue_Variable)
	{
		*pResultValue = ScopeValidatorValue.GetVariable ()->GetScopeLevelObjHdr ();
		return;
	}
	
	CType* pScopeValidatorType = ScopeValidatorValue.GetType ();
	CType* pResultType = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjHdrPtr);
	if (pScopeValidatorType->Cmp (pResultType) == 0)
	{
		*pResultValue = ScopeValidatorValue;
	}
	else if (pScopeValidatorType->GetTypeKind () == EType_ClassPtr)
	{
		static int LlvmIndexArray [] = { 0, 0, 1 }; // Iface*, IfaceHdr**, ObjHdr**

		CValue ObjHdrValue;
		m_pModule->m_LlvmIrBuilder.CreateGep (ScopeValidatorValue, LlvmIndexArray, 3, NULL, &ObjHdrValue);
		m_pModule->m_LlvmIrBuilder.CreateLoad (ObjHdrValue, pResultType, pResultValue);
	}
	else
	{
		ASSERT (pScopeValidatorType->GetTypeKind () == EType_DataPtr);
		ASSERT (((CDataPtrType*) pScopeValidatorType)->GetPtrTypeKind () == EDataPtrType_Normal);
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (ScopeValidatorValue, 3, pResultType, pResultValue);
	}
}

void
COperatorMgr::GetLeanDataPtrRange (
	const CValue& Value,
	CValue* pRangeBeginValue,
	CValue* pRangeEndValue
	)
{
	ASSERT (Value.GetType ()->GetTypeKindFlags () & ETypeKindFlag_DataPtr);

	CType* pBytePtrType = m_pModule->GetSimpleType (EStdType_BytePtr);

	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "calc lean data pointer range");

	EValue ValueKind = Value.GetValueKind ();
	if (ValueKind == EValue_Variable)
	{	
		size_t Size =  Value.GetVariable ()->GetType ()->GetSize ();
		m_pModule->m_LlvmIrBuilder.CreateBitCast (Value, pBytePtrType, pRangeBeginValue);
		m_pModule->m_LlvmIrBuilder.CreateGep (*pRangeBeginValue, Size, pBytePtrType, pRangeEndValue);
		return;
	}

	CLeanDataPtrValidator* pValidator = Value.GetLeanDataPtrValidator ();
	ASSERT (pValidator);

	if (pValidator->GetValidatorKind () == ELeanDataPtrValidator_Complex)
	{
		m_pModule->m_LlvmIrBuilder.CreateBitCast (pValidator->GetRangeBegin (), pBytePtrType, pRangeBeginValue);
		m_pModule->m_LlvmIrBuilder.CreateGep (*pRangeBeginValue, pValidator->GetSizeValue (), pBytePtrType, pRangeEndValue);
		return;
	}

	ASSERT (pValidator->GetValidatorKind () == ELeanDataPtrValidator_Simple);
	CValue ValidatorValue = pValidator->GetScopeValidator ();

	if (ValidatorValue.GetValueKind () == EValue_Variable)
	{
		size_t Size = ValidatorValue.GetVariable ()->GetType ()->GetSize ();
		m_pModule->m_LlvmIrBuilder.CreateBitCast (ValidatorValue, pBytePtrType, pRangeBeginValue);
		m_pModule->m_LlvmIrBuilder.CreateGep (*pRangeBeginValue, Size, pBytePtrType, pRangeEndValue);
		return;
	}

	ASSERT (
		(ValidatorValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_DataPtr) &&
		((CDataPtrType*) ValidatorValue.GetType ())->GetPtrTypeKind () == EDataPtrType_Normal);

	m_pModule->m_LlvmIrBuilder.CreateExtractValue (ValidatorValue, 1, pBytePtrType, pRangeBeginValue);
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (ValidatorValue, 2, pBytePtrType, pRangeEndValue);		
}

bool
COperatorMgr::PrepareDataPtr (
	const CValue& Value,
	CValue* pResultValue
	)
{
	ASSERT (Value.GetType ()->GetTypeKind () == EType_DataPtr || Value.GetType ()->GetTypeKind () == EType_DataRef);
	CDataPtrType* pType = (CDataPtrType*) Value.GetType ();
	EDataPtrType PtrTypeKind = pType->GetPtrTypeKind ();

	CDataPtrType* pResultType = pType->GetTargetType ()->GetDataPtrType_c ();

	CValue PtrValue;
	CValue RangeBeginValue;	
	CValue RangeEndValue;	

	if (PtrTypeKind == EDataPtrType_Thin)
	{
		pResultValue->OverrideType (Value, pResultType);
		return true;
	}
	else if (PtrTypeKind == EDataPtrType_Lean)
	{
		if (pType->GetFlags () & EPtrTypeFlag_Checked)
		{
			pResultValue->OverrideType (Value, pResultType);
			return true;
		}

		PtrValue.OverrideType (Value, pResultType);
		GetLeanDataPtrRange (Value, &RangeBeginValue, &RangeEndValue);
	}
	else // EDataPtrType_Normal
	{
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (Value, 0, pResultType, &PtrValue);

		if (pType->GetFlags () & EPtrTypeFlag_Checked)
		{
			*pResultValue = PtrValue;
			return true;
		}

		m_pModule->m_LlvmIrBuilder.CreateExtractValue (Value, 1, NULL, &RangeBeginValue);
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (Value, 2, NULL, &RangeEndValue);
	}

	CheckDataPtrRange (PtrValue, pType->GetTargetType ()->GetSize (), RangeBeginValue, RangeEndValue);
	*pResultValue = PtrValue;
	return true;
}

bool
COperatorMgr::LoadDataRef (
	const CValue& OpValue,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataRef);
	
	bool Result;
	
	CDataPtrType* pType = (CDataPtrType*) OpValue.GetType ();

	CType* pTargetType = pType->GetTargetType ();

	CValue PtrValue;
	Result = PrepareDataPtr (OpValue, &PtrValue);
	if (!Result)
		return false;

	m_pModule->m_LlvmIrBuilder.CreateLoad (
		PtrValue, 
		pTargetType, 
		pResultValue, 
		(pType->GetFlags () & EPtrTypeFlag_Volatile) != 0
		);

	if (pTargetType->GetTypeKind () == EType_BitField)
	{
		Result = ExtractBitField (
			*pResultValue, 
			(CBitFieldType*) pTargetType,
			pResultValue
			);

		if (!Result)
			return false;
	}

	return true;
}

bool
COperatorMgr::StoreDataRef (
	const CValue& DstValue,
	const CValue& RawSrcValue
	)
{
	ASSERT (DstValue.GetType ()->GetTypeKind () == EType_DataRef);

	bool Result;

	CDataPtrType* pDstType = (CDataPtrType*) DstValue.GetType ();	
	if (pDstType->IsConstPtrType ())
	{
		err::SetFormatStringError ("cannot store into const location");
		return false;
	}

	CType* pTargetType = pDstType->GetTargetType ();
	EType TargetTypeKind = pTargetType->GetTypeKind ();

	CType* pCastType = (TargetTypeKind == EType_BitField) ? 
		((CBitFieldType*) pTargetType)->GetBaseType () : 
		pTargetType;

	CValue PtrValue;
	CValue SrcValue;
	CValue BfShadowValue;

	Result = 
		CheckCastKind (RawSrcValue, pCastType) &&
		CastOperator (RawSrcValue, pCastType, &SrcValue) &&
		PrepareDataPtr (DstValue, &PtrValue);

	if (!Result)
		return false;

	switch (TargetTypeKind)
	{
	case EType_DataPtr:
		Result = CheckDataPtrScopeLevel (SrcValue, DstValue);
		if (!Result)
			return false;

		break;

	case EType_ClassPtr:
		CheckClassPtrScopeLevel (SrcValue, DstValue);
		break;

	case EType_FunctionPtr:
		CheckFunctionPtrScopeLevel (SrcValue, DstValue);
		break;

	case EType_PropertyPtr:
		CheckPropertyPtrScopeLevel (SrcValue, DstValue);
		break;

	case EType_BitField:
		m_pModule->m_LlvmIrBuilder.CreateLoad (
			PtrValue, 
			pCastType,
			&BfShadowValue,
			(pDstType->GetFlags () & EPtrTypeFlag_Volatile) != 0
			);

		Result = MergeBitField (
			SrcValue,
			BfShadowValue, 
			(CBitFieldType*) pTargetType,
			&SrcValue
			);

		if (!Result)
			return false;
	}

	m_pModule->m_LlvmIrBuilder.CreateStore (
		SrcValue, 
		PtrValue, 
		(pDstType->GetFlags () & EPtrTypeFlag_Volatile) != 0
		);

	return true;
}

bool
COperatorMgr::ExtractBitField (
	const CValue& RawValue,
	CBitFieldType* pBitFieldType,
	CValue* pResultValue
	)
{
	bool Result;

	CType* pBaseType = pBitFieldType->GetBaseType ();
	size_t BitOffset = pBitFieldType->GetBitOffset ();
	size_t BitCount = pBitFieldType->GetBitCount ();

	EType TypeKind = pBaseType->GetSize () <= 4 ? EType_Int32_u : EType_Int64_u;
	int64_t Mask = ((int64_t) 1 << BitCount) - 1;

	CValue Value (RawValue, pBaseType);
	CValue MaskValue (Mask, TypeKind);
	CValue OffsetValue (BitOffset, TypeKind);

	Result = 
		BinaryOperator (EBinOp_Shr, &Value, OffsetValue) &&
		BinaryOperator (EBinOp_BwAnd, &Value, MaskValue);

	if (!Result)
		return false;

	if (!(pBaseType->GetTypeKindFlags () & ETypeKindFlag_Unsigned)) // extend with sign bit
	{
		int64_t SignBit = (int64_t) 1 << (BitCount - 1);

		CValue SignBitValue (SignBit, TypeKind);
		CValue OneValue (1, TypeKind);

		CValue SignExtValue;
		Result = 
			BinaryOperator (EBinOp_BwAnd, &SignBitValue, Value) &&
			BinaryOperator (EBinOp_Sub, SignBitValue, OneValue, &SignExtValue) &&
			UnaryOperator (EUnOp_BwNot, &SignExtValue) &&
			BinaryOperator (EBinOp_BwOr, &Value, SignExtValue);

		if (!Result)
			return false;
	}

	return CastOperator (Value, pBaseType, pResultValue);
}
	
bool
COperatorMgr::MergeBitField (
	const CValue& RawValue,
	const CValue& RawShadowValue,
	CBitFieldType* pBitFieldType,
	CValue* pResultValue
	)
{
	bool Result;

	CType* pBaseType = pBitFieldType->GetBaseType ();
	size_t BitOffset = pBitFieldType->GetBitOffset ();
	size_t BitCount = pBitFieldType->GetBitCount ();

	EType TypeKind = pBaseType->GetSize () <= 4 ? EType_Int32_u : EType_Int64_u;
	int64_t Mask = (((int64_t) 1 << BitCount) - 1) << BitOffset;

	CValue Value (RawValue, pBaseType);
	CValue ShadowValue (RawShadowValue, pBaseType);
	CValue MaskValue (Mask, TypeKind);
	CValue OffsetValue (BitOffset, TypeKind);

	Result = 
		BinaryOperator (EBinOp_Shl, &Value, OffsetValue) &&
		BinaryOperator (EBinOp_BwAnd, Value, MaskValue, pResultValue);

	if (!Result)
		return false;

	Mask = ~((((uint64_t) 1 << BitCount) - 1) << BitOffset);	
	MaskValue.SetConstInt64 (Mask, TypeKind);

	return 
		BinaryOperator (EBinOp_BwAnd, &ShadowValue, MaskValue) &&
		BinaryOperator (EBinOp_BwOr, &Value, ShadowValue) &&
		CastOperator (Value, pBaseType, pResultValue);
}

//.............................................................................

} // namespace jnc {
