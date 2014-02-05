#include "pch.h"
#include "jnc_CastOp.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

err::CError
SetCastError (
	const CValue& OpValue,
	CType* pDstType
	)
{
	CType* pSrcType = OpValue.GetType ();
	if (!pSrcType)
	{
		ASSERT (OpValue.GetValueKind () == EValue_Function);
		CFunction* pFunction = (CFunction*) OpValue.GetFunction ();

		return err::SetFormatStringError (
			"not enough information to select one of %d overloads of '%s'",
			pFunction->GetOverloadCount (),
			pFunction->m_Tag.cc () // thanks a lot gcc
			);
	}

	return err::SetFormatStringError (
		"cannot convert from '%s' to '%s'",
		OpValue.GetValueKind () == EValue_Null ? "null" : OpValue.GetType ()->GetTypeString ().cc (), 
		pDstType->GetTypeString ().cc ()
		);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CastOperator (
	CModule* pModule,
	const CValue& OpValue,
	CType* pType,
	CValue* pOpValue
	)
{
	return pModule->m_OperatorMgr.CastOperator (OpValue, pType, pOpValue);
}

//.............................................................................

CCastOperator::CCastOperator()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	m_OpFlags = 0;
}

bool
CCastOperator::Cast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	EValue OpValueKind = OpValue.GetValueKind ();
	if (OpValueKind != EValue_Const)
		return LlvmCast (StorageKind, OpValue, pType, pResultValue);
	
	CValue ResultValue;
	
	bool Result = 
		ResultValue.CreateConst (NULL, pType) && 
		ConstCast (OpValue, pType, ResultValue.GetConstData ());

	if (!Result)
		return false;

	*pResultValue = ResultValue;
	return true;
}

bool
CCastOperator::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	// fail by default; if const-cast is supported then override and implement

	err::SetFormatStringError (
		"cannot convert constant from '%s' to '%s'",
		OpValue.GetType ()->GetTypeString ().cc (), 
		pType->GetTypeString ().cc ()
		);

	return false;
}

//.............................................................................

bool
CCast_Copy::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	size_t SrcSize = OpValue.GetType ()->GetSize ();
	size_t DstSize = pType->GetSize ();

	ASSERT (SrcSize == DstSize);

	memcpy (pDst, OpValue.GetConstData (), DstSize);
	return true;
}

bool
CCast_Copy::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	m_pModule->m_LlvmIrBuilder.CreateBitCast (OpValue, pType, pResultValue);
	return true;
}

//.............................................................................

ECast
CCast_Master::GetCastKind (
	const CValue& RawOpValue,
	CType* pType
	)
{
	if (!RawOpValue.GetType ())
		return ECast_None;

	CCastOperator* pOperator = GetCastOperator (RawOpValue, pType);
	if (!pOperator)
		return ECast_None;

	CValue OpValue = RawOpValue;

	uint_t OpFlags = pOperator->GetOpFlags ();
	if (OpFlags != m_OpFlags)
		m_pModule->m_OperatorMgr.PrepareOperandType (&OpValue, OpFlags);

	return pOperator->GetCastKind (OpValue, pType);
}

bool
CCast_Master::ConstCast (
	const CValue& RawOpValue,
	CType* pType,
	void* pDst
	)
{
	CCastOperator* pOperator = GetCastOperator (RawOpValue, pType);
	if (!pOperator)
	{
		SetCastError (RawOpValue, pType);
		return false;
	}

	CValue OpValue = RawOpValue;

	uint_t OpFlags = pOperator->GetOpFlags ();
	if (OpFlags != m_OpFlags)
	{
		bool Result = m_pModule->m_OperatorMgr.PrepareOperand (&OpValue, OpFlags);
		if (!Result)
			return false;
	}

	return pOperator->ConstCast (OpValue, pType, pDst);
}

bool
CCast_Master::LlvmCast (
	EStorage StorageKind,
	const CValue& RawOpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	CCastOperator* pOperator = GetCastOperator (RawOpValue, pType);
	if (!pOperator)
	{
		SetCastError (RawOpValue, pType);
		return false;
	}

	CValue OpValue = RawOpValue;

	uint_t OpFlags = pOperator->GetOpFlags ();
	if (OpFlags != m_OpFlags)
	{
		bool Result = m_pModule->m_OperatorMgr.PrepareOperand (&OpValue, OpFlags);
		if (!Result)
			return false;
	}
		
	return pOperator->LlvmCast (StorageKind, OpValue, pType, pResultValue);
}

//.............................................................................

ECast
CCast_SuperMaster::GetCastKind (
	const CValue& RawOpValue,
	CType* pType
	)
{
	if (!RawOpValue.GetType ())
		return ECast_None;

	CCastOperator* pOperator1 = NULL;
	CCastOperator* pOperator2 = NULL;
	CType* pIntermediateType = NULL;

	bool Result = GetCastOperators (
		RawOpValue, 
		pType,
		&pOperator1, 
		&pOperator2, 
		&pIntermediateType
		);

	if (!Result)
		return ECast_None;

	ASSERT (pOperator1);

	CValue OpValue = RawOpValue;

	uint_t OpFlags1 = pOperator1->GetOpFlags ();
	if (OpFlags1 != m_OpFlags)
		m_pModule->m_OperatorMgr.PrepareOperandType (&OpValue, OpFlags1);

	if (!pOperator2) 
		return pOperator1->GetCastKind (OpValue, pType);
	
	ECast CastKind1 = pOperator1->GetCastKind (OpValue, pIntermediateType);
	ECast CastKind2 = pOperator2->GetCastKind (pIntermediateType, pType);
	return AXL_MIN (CastKind1, CastKind2);
}

bool
CCast_SuperMaster::ConstCast (
	const CValue& RawOpValue,
	CType* pType,
	void* pDst
	)
{
	CCastOperator* pOperator1 = NULL;
	CCastOperator* pOperator2 = NULL;
	CType* pIntermediateType = NULL;

	bool Result = GetCastOperators (
		RawOpValue, 
		pType,
		&pOperator1, 
		&pOperator2, 
		&pIntermediateType
		);

	if (!Result)
	{
		SetCastError (RawOpValue, pType);
		return false;
	}

	ASSERT (pOperator1);

	CValue SrcValue = RawOpValue;

	uint_t OpFlags1 = pOperator1->GetOpFlags ();
	if (OpFlags1 != m_OpFlags)
	{
		bool Result = m_pModule->m_OperatorMgr.PrepareOperand (&SrcValue, OpFlags1);
		if (!Result)
			return false;
	}

	if (!pOperator2) 
		return pOperator1->ConstCast (SrcValue, pType, pDst);

	CValue TmpValue;
	return 
		TmpValue.CreateConst (NULL, pIntermediateType) &&
		pOperator1->ConstCast (SrcValue, pIntermediateType, TmpValue.GetConstData ()) &&
		pOperator2->ConstCast (TmpValue, pType, pDst);
}

bool
CCast_SuperMaster::LlvmCast (
	EStorage StorageKind,
	const CValue& RawOpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	CCastOperator* pOperator1 = NULL;
	CCastOperator* pOperator2 = NULL;
	CType* pIntermediateType = NULL;

	bool Result = GetCastOperators (
		RawOpValue, 
		pType,
		&pOperator1, 
		&pOperator2, 
		&pIntermediateType
		);

	if (!Result)
	{
		SetCastError (RawOpValue, pType);
		return false;
	}

	ASSERT (pOperator1);

	CValue OpValue = RawOpValue;

	uint_t OpFlags1 = pOperator1->GetOpFlags ();
	if (OpFlags1 != m_OpFlags)
	{
		bool Result = m_pModule->m_OperatorMgr.PrepareOperand (&OpValue, OpFlags1);
		if (!Result)
			return false;
	}

	if (!pOperator2) 
		return pOperator1->LlvmCast (StorageKind, OpValue, pType, pResultValue);

	CValue TmpValue;
	return 
		pOperator1->LlvmCast (StorageKind, OpValue, pIntermediateType, &TmpValue) &&
		pOperator2->LlvmCast (StorageKind, TmpValue, pType, pResultValue);
}

//.............................................................................

} // namespace jnc {
