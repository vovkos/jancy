#include "pch.h"
#include "jnc_CastOp_Int.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
CCast_IntTrunc::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	size_t SrcSize = OpValue.GetType ()->GetSize ();
	size_t DstSize = pType->GetSize ();

	ASSERT (SrcSize > DstSize);

	memcpy (pDst, OpValue.GetConstData (), DstSize);
	return true;
}

bool
CCast_IntTrunc::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	m_pModule->m_LlvmIrBuilder.CreateTrunc_i (OpValue, pType, pResultValue);
	return true;
}

//.............................................................................

bool
CCast_IntExt::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	size_t SrcSize = OpValue.GetType ()->GetSize ();
	size_t DstSize = pType->GetSize ();

	ASSERT (SrcSize < DstSize);

	char* pSrc = (char*) OpValue.GetConstData ();

	if (pSrc [SrcSize - 1] < 0)
		memset (pDst, -1, DstSize);
	else
		memset (pDst, 0, DstSize);

	memcpy (pDst, pSrc, SrcSize);
	return true;
}

bool
CCast_IntExt::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	m_pModule->m_LlvmIrBuilder.CreateExt_i (OpValue, pType, pResultValue);
	return true;
}

//.............................................................................

bool
CCast_IntExt_u::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	size_t SrcSize = OpValue.GetType ()->GetSize ();
	size_t DstSize = pType->GetSize ();

	ASSERT (SrcSize < DstSize);

	char* pSrc = (char*) OpValue.GetConstData ();

	memset (pDst, 0, DstSize);
	memcpy (pDst, pSrc, SrcSize);
	return true;
}

bool
CCast_IntExt_u::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	m_pModule->m_LlvmIrBuilder.CreateExt_u (OpValue, pType, pResultValue);
	return true;
}

//.............................................................................

bool
CCast_SwapByteOrder::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	size_t SrcSize = OpValue.GetType ()->GetSize ();
	size_t DstSize = pType->GetSize ();

	ASSERT (SrcSize == DstSize);

	char* pSrc = (char*) OpValue.GetConstData ();

	rtl::SwapByteOrder (pDst, pSrc, SrcSize);
	return true;
}

bool
CCast_SwapByteOrder::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	llvm::Type* pLlvmType = pType->GetLlvmType ();

	llvm::Function* pLlvmSwap = llvm::Intrinsic::getDeclaration (
		m_pModule->GetLlvmModule (),
		llvm::Intrinsic::bswap,
		llvm::ArrayRef <llvm::Type*> (pLlvmType)
		);

	CValue SwapFunctionValue;
	SwapFunctionValue.SetLlvmValue (pLlvmSwap, NULL);
	m_pModule->m_LlvmIrBuilder.CreateCall (
		SwapFunctionValue,
		m_pModule->m_TypeMgr.GetCallConv (ECallConv_Default),
		&OpValue, 1,
		pType,
		pResultValue
		);

	return true;
}

//.............................................................................

bool
CCast_IntFromBeInt::GetCastOperators (
	const CValue& OpValue,
	CType* pType,
	CCastOperator** ppFirstOperator,
	CCastOperator** ppSecondOperator,
	CType** ppIntermediateType
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_BigEndian);

	EType IntermediateTypeKind = GetLittleEndianIntegerTypeKind (OpValue.GetType ()->GetTypeKind ());

	if (IsEquivalentIntegerTypeKind (pType->GetTypeKind (), IntermediateTypeKind))
	{
		*ppFirstOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_SwapByteOrder);
		return true;
	}

	*ppFirstOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_SwapByteOrder);
	*ppSecondOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Int);
	*ppIntermediateType = m_pModule->m_TypeMgr.GetPrimitiveType (IntermediateTypeKind);
	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CCast_BeInt::GetCastOperators (
	const CValue& OpValue,
	CType* pType,
	CCastOperator** ppFirstOperator,
	CCastOperator** ppSecondOperator,
	CType** ppIntermediateType
	)
{
	ASSERT (pType->GetTypeKindFlags () & ETypeKindFlag_BigEndian);

	EType IntermediateTypeKind = GetLittleEndianIntegerTypeKind (pType->GetTypeKind ());

	if (IsEquivalentIntegerTypeKind (OpValue.GetType ()->GetTypeKind (), IntermediateTypeKind))
	{
		*ppFirstOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_SwapByteOrder);
		return true;
	}

	*ppFirstOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Int);
	*ppSecondOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_SwapByteOrder);
	*ppIntermediateType = m_pModule->m_TypeMgr.GetPrimitiveType (IntermediateTypeKind);
	return true;
}

//.............................................................................

bool
CCast_IntFromFp::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	m_pModule->m_LlvmIrBuilder.CreateFpToInt (OpValue, pType, pResultValue);
	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CCast_IntFromFp32::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_Float);

	float Fp32 = *(float*) OpValue.GetConstData ();

	size_t DstSize = pType->GetSize ();
	switch (DstSize)
	{
	case 1:
		*(int8_t*) pDst = (int8_t) Fp32;
		break;

	case 2:
		*(int16_t*) pDst = (int16_t) Fp32;
		break;

	case 4:
		*(int32_t*) pDst = (int32_t) Fp32;
		break;

	case 8:
		*(int64_t*) pDst = (int64_t) Fp32;
		break;

	default:
		ASSERT (false);
	}

	return true;
};

//.............................................................................

bool
CCast_IntFromFp64::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_Double);

	double Fp64 = *(double*) OpValue.GetConstData ();

	size_t DstSize = pType->GetSize ();
	switch (DstSize)
	{
	case 1:
		*(int8_t*) pDst = (int8_t) Fp64;
		break;

	case 2:
		*(int16_t*) pDst = (int16_t) Fp64;
		break;

	case 4:
		*(int32_t*) pDst = (int32_t) Fp64;
		break;

	case 8:
		*(int64_t*) pDst = (int64_t) Fp64;
		break;

	default:
		ASSERT (false);
	}

	return true;
};

//.............................................................................

bool
CCast_IntFromPtr::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	ASSERT (OpValue.GetType ()->GetSize () >= sizeof (intptr_t));

	size_t Size = pType->GetSize ();
	if (Size > sizeof (intptr_t))
		Size = sizeof (intptr_t);

	memcpy (pDst, OpValue.GetConstData (), Size);
	return true;
}

bool
CCast_IntFromPtr::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	CValue PtrValue;

	if (OpValue.GetType ()->GetSize () > sizeof (intptr_t))
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 0, NULL, &PtrValue);
	else
		PtrValue = OpValue;

	m_pModule->m_LlvmIrBuilder.CreatePtrToInt (PtrValue, m_pModule->GetSimpleType (EType_Int_p), &PtrValue);
	return m_pModule->m_OperatorMgr.CastOperator (PtrValue, pType, pResultValue);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CCast_PtrFromInt::ConstCast (
	const CValue& RawOpValue,
	CType* pType,
	void* pDst
	)
{
	CValue OpValue;
	bool Result = m_pModule->m_OperatorMgr.CastOperator (RawOpValue, EType_Int_p, &OpValue);
	if (!Result)
		return false;

	ASSERT (OpValue.GetType ()->GetSize () == sizeof (intptr_t));
	ASSERT (pType->GetSize () == sizeof (intptr_t));

	*(intptr_t*) pDst = *(intptr_t*) OpValue.GetConstData ();
	return true;
}

bool
CCast_PtrFromInt::LlvmCast (
	EStorage StorageKind,
	const CValue& RawOpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	CValue OpValue;
	bool Result = m_pModule->m_OperatorMgr.CastOperator (RawOpValue, EType_Int_p, &OpValue);
	if (!Result)
		return false;

	m_pModule->m_LlvmIrBuilder.CreateIntToPtr (OpValue, pType, pResultValue);
	return true;
}

//.............................................................................

bool
CCast_IntFromEnum::GetCastOperators (
	const CValue& OpValue,
	CType* pType,
	CCastOperator** ppFirstOperator,
	CCastOperator** ppSecondOperator,
	CType** ppIntermediateType
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_Enum);

	CType* pIntermediateType = ((CEnumType*) OpValue.GetType ())->GetBaseType ();

	if (IsEquivalentIntegerTypeKind (pType->GetTypeKind (), pIntermediateType->GetTypeKind ()))
	{
		*ppFirstOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Copy);
		return true;
	}

	*ppFirstOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Copy);
	*ppSecondOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Int);
	*ppIntermediateType = pIntermediateType;
	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

ECast
CCast_Enum::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (pType->GetTypeKind () == EType_Enum);
	ASSERT (pType->Cmp (OpValue.GetType ()) != 0); // identity should have been handled earlier

	// 0 could be put to flag enum

	return
		((CEnumType*) pType)->GetEnumTypeKind () == EEnumType_Flag &&
		OpValue.GetValueKind () == EValue_Const &&
		OpValue.GetType ()->GetTypeKind () == EType_Int8_u &&
		*(char*) OpValue.GetConstData () == 0 ? ECast_Implicit : ECast_Explicit;
}

bool
CCast_Enum::GetCastOperators (
	const CValue& OpValue,
	CType* pType,
	CCastOperator** ppFirstOperator,
	CCastOperator** ppSecondOperator,
	CType** ppIntermediateType
	)
{
	ASSERT (pType->GetTypeKind () == EType_Enum);

	CType* pIntermediateType = ((CEnumType*) pType)->GetBaseType ();

	if (IsEquivalentIntegerTypeKind (OpValue.GetType ()->GetTypeKind (), pIntermediateType->GetTypeKind ()))
	{
		*ppFirstOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Copy);
		return true;
	}

	*ppFirstOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Int);
	*ppSecondOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Copy);
	*ppIntermediateType = pIntermediateType;
	return true;
}

//.............................................................................

CCastOperator*
CCast_Int::GetCastOperator (
	const CValue& OpValue,
	CType* pType
	)
{
	CType* pSrcType = OpValue.GetType ();

	EType SrcTypeKind = pSrcType->GetTypeKind ();
	EType DstTypeKind = pType->GetTypeKind ();

	size_t SrcSize = pSrcType->GetSize ();
	size_t DstSize = pType->GetSize ();

	ASSERT (DstTypeKind >= EType_Int8 && DstTypeKind <= EType_Int64_u);

	switch (SrcTypeKind)
	{
	case EType_Bool:
		return &m_Ext_u; // 1 bit -- could only be extended

	case EType_Int8:
	case EType_Int8_u:
	case EType_Int16:
	case EType_Int16_u:
	case EType_Int32:
	case EType_Int32_u:
	case EType_Int64:
	case EType_Int64_u:
		return
			SrcSize == DstSize ? m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Copy) :
			SrcSize > DstSize ? (CCastOperator*) &m_Trunc :
			(GetTypeKindFlags (SrcTypeKind) & ETypeKindFlag_Unsigned) ?
				(CCastOperator*) &m_Ext_u :
				(CCastOperator*) &m_Ext;

	case EType_Int16_be:
	case EType_Int16_beu:
	case EType_Int32_be:
	case EType_Int32_beu:
	case EType_Int64_be:
	case EType_Int64_beu:
		return &m_FromBeInt;

	case EType_Float:
		return &m_FromFp32;

	case EType_Double:
		return &m_FromFp64;

	case EType_Enum:
		return &m_FromEnum;

	case EType_DataPtr:
	case EType_ClassPtr:
	case EType_FunctionPtr:
	case EType_FunctionRef:
	case EType_PropertyPtr:
		return &m_FromPtr;

	default:
		return NULL;
	}
}

//.............................................................................

} // namespace jnc {
