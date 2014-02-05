#include "pch.h"
#include "jnc_CastOp_Fp.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
CCast_FpTrunc::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	m_pModule->m_LlvmIrBuilder.CreateTrunc_f (OpValue, pType, pResultValue);
	return true;
}

//.............................................................................

bool
CCast_FpExt::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	m_pModule->m_LlvmIrBuilder.CreateExt_f (OpValue, pType, pResultValue);
	return true;
}

//.............................................................................

bool
CCast_FpFromInt::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	m_pModule->m_LlvmIrBuilder.CreateIntToFp (OpValue, pType, pResultValue);
	return true;
}

bool
CCast_FpFromInt::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	EType DstTypeKind = pType->GetTypeKind ();
	switch (DstTypeKind)
	{
	case EType_Float:
		ConstCast_Fp32 (OpValue, (float*) pDst);
		break;

	case EType_Double:
		ConstCast_Fp64 (OpValue, (double*) pDst);
		break;

	default:
		ASSERT (false);
	}

	return true;
}

void
CCast_FpFromInt::ConstCast_Fp32 (
	const CValue& OpValue,
	float* pFp32
	)
{
	const void* pSrc = OpValue.GetConstData ();
	
	size_t SrcSize = OpValue.GetType ()->GetSize ();
	switch (SrcSize)
	{
	case 1:
		*pFp32 = *(char*) pSrc;
		break;

	case 2:
		*pFp32 = *(short*) pSrc;
		break;

	case 4:
		*pFp32 = (float) *(int32_t*) pSrc;
		break;

	case 8:
		*pFp32 = (float) *(int64_t*) pSrc;
		break;

	default:
		ASSERT (false);
	}
};

void
CCast_FpFromInt::ConstCast_Fp64 (
	const CValue& OpValue,
	double* pFp64
	)
{
	const void* pSrc = OpValue.GetConstData ();
	
	size_t SrcSize = OpValue.GetType ()->GetSize ();
	switch (SrcSize)
	{
	case 1:
		*pFp64 = *(char*) pSrc;
		break;

	case 2:
		*pFp64 = *(short*) pSrc;
		break;

	case 4:
		*pFp64 = *(int32_t*) pSrc;
		break;

	case 8:
		*pFp64 = (double) *(int64_t*) pSrc;
		break;

	default:
		ASSERT (false);
	}
};

//.............................................................................

bool
CCast_FpFromInt_u::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	m_pModule->m_LlvmIrBuilder.CreateIntToFp_u (OpValue, pType, pResultValue);
	return true;
}

bool
CCast_FpFromInt_u::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	EType DstTypeKind = pType->GetTypeKind ();
	switch (DstTypeKind)
	{
	case EType_Float:
		ConstCast_Fp32 (OpValue, (float*) pDst);
		break;

	case EType_Double:
		ConstCast_Fp64 (OpValue, (double*) pDst);
		break;

	default:
		ASSERT (false);
	}

	return true;
}

void
CCast_FpFromInt_u::ConstCast_Fp32 (
	const CValue& OpValue,
	float* pFp32
	)
{
	const void* pSrc = OpValue.GetConstData ();
	
	size_t SrcSize = OpValue.GetType ()->GetSize ();
	switch (SrcSize)
	{
	case 1:
		*pFp32 = *(uint8_t*) pSrc;
		break;

	case 2:
		*pFp32 = *(uint16_t*) pSrc;
		break;

	case 4:
		*pFp32 = (float) *(uint32_t*) pSrc;
		break;

	case 8:
		*pFp32 = (float) *(uint64_t*) pSrc;
		break;

	default:
		ASSERT (false);
	}
};

void
CCast_FpFromInt_u::ConstCast_Fp64 (
	const CValue& OpValue,
	double* pFp64
	)
{
	const void* pSrc = OpValue.GetConstData ();
	
	size_t SrcSize = OpValue.GetType ()->GetSize ();
	switch (SrcSize)
	{
	case 1:
		*pFp64 = *(uint8_t*) pSrc;
		break;

	case 2:
		*pFp64 = *(uint16_t*) pSrc;
		break;

	case 4:
		*pFp64 = *(uint32_t*) pSrc;
		break;

	case 8:
		*pFp64 = (double) *(uint64_t*) pSrc;
		break;

	default:
		ASSERT (false);
	}
};

//.............................................................................

bool
CCast_FpFromBeInt::GetCastOperators (
	const CValue& OpValue,
	CType* pType,
	CCastOperator** ppFirstOperator,
	CCastOperator** ppSecondOperator,
	CType** ppIntermediateType
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_BigEndian);

	EType IntermediateTypeKind = GetLittleEndianIntegerTypeKind (OpValue.GetType ()->GetTypeKind ());

	*ppFirstOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_SwapByteOrder);
	*ppSecondOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Fp);
	*ppIntermediateType = m_pModule->m_TypeMgr.GetPrimitiveType (IntermediateTypeKind);
	return true;
}

//.............................................................................

bool
CCast_FpFromEnum::GetCastOperators (
	const CValue& OpValue,
	CType* pType,
	CCastOperator** ppFirstOperator,
	CCastOperator** ppSecondOperator,
	CType** ppIntermediateType
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_Enum);

	CType* pIntermediateType = ((CEnumType*) OpValue.GetType ())->GetBaseType ();

	*ppFirstOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Copy);
	*ppSecondOperator = m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Fp);
	*ppIntermediateType = pIntermediateType;
	return true;
}

//.............................................................................

CCastOperator*
CCast_Fp::GetCastOperator (
	const CValue& OpValue,
	CType* pType
	)
{
	CType* pSrcType = OpValue.GetType ();

	EType SrcTypeKind = pSrcType->GetTypeKind ();
	EType DstTypeKind = pType->GetTypeKind ();

	size_t SrcSize = pSrcType->GetSize ();
	size_t DstSize = pType->GetSize ();

	ASSERT (DstTypeKind == EType_Float || DstTypeKind == EType_Double);

	switch (SrcTypeKind)
	{
	case EType_Int8:
	case EType_Int16:
	case EType_Int32:
	case EType_Int64:
		return &m_FromInt;

	case EType_Bool:
	case EType_Int8_u:
	case EType_Int16_u:
	case EType_Int32_u:
	case EType_Int64_u:
		return &m_FromInt_u;

	case EType_Int16_be:
	case EType_Int16_beu:
	case EType_Int32_be:
	case EType_Int32_beu:
	case EType_Int64_be:
	case EType_Int64_beu:
		return &m_FromBeInt;

	case EType_Float:
	case EType_Double:
		return 
			SrcSize == DstSize ? m_pModule->m_OperatorMgr.GetStdCastOperator (EStdCast_Copy) : 
			SrcSize > DstSize ? (CCastOperator*) &m_Trunc : 
			(CCastOperator*) &m_Ext;

	case EType_Enum:
		return &m_FromEnum;

	default:
		return NULL;
	}
}

//.............................................................................

} // namespace jnc {
