#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
COperatorMgr::LogicalOrOperator (
	CBasicBlock* pOpBlock1,
	CBasicBlock* pOpBlock2,
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	bool Result;

	CBasicBlock* pPrevBlock = m_pModule->m_ControlFlowMgr.SetCurrentBlock (pOpBlock1);

	CFunction* pFunction = GetOverloadedBinaryOperator (EBinOp_LogOr, RawOpValue1, RawOpValue2);
	if (pFunction)
	{
		m_pModule->m_ControlFlowMgr.Follow (pOpBlock2);
		m_pModule->m_ControlFlowMgr.SetCurrentBlock (pPrevBlock);
		return OverloadedBinaryOperator (pFunction, RawOpValue1, RawOpValue2, pResultValue);
	}

	CValue UnusedResultValue;
	if (!pResultValue)
		pResultValue = &UnusedResultValue;

	CBasicBlock* pPhiBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("and_phi");
	CBasicBlock* pFalseBlock2 = m_pModule->m_ControlFlowMgr.CreateBlock ("op2_false");

	CValue OpValue1;
	Result = m_pModule->m_OperatorMgr.CastOperator (RawOpValue1, EType_Bool, &OpValue1);
	if (!Result)
		return false;

	CBasicBlock* pJumpBlock1 = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	Result = m_pModule->m_ControlFlowMgr.ConditionalJump (OpValue1, pPhiBlock, pOpBlock2, pPrevBlock);
	ASSERT (Result);

	CValue OpValue2;
	Result = m_pModule->m_OperatorMgr.CastOperator (RawOpValue2, EType_Bool, &OpValue2);
	if (!Result)
		return false;

	CBasicBlock* pJumpBlock2 = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	Result = m_pModule->m_ControlFlowMgr.ConditionalJump (OpValue2, pPhiBlock, pFalseBlock2, pFalseBlock2);
	ASSERT (Result);

	CType* pType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Bool);
	CValue TrueValue (true, pType);
	CValue FalseValue ((int64_t) false, pType);

	CValue ValueArray [] =
	{
		TrueValue,
		TrueValue,
		FalseValue,
	};

	CBasicBlock* BlockArray [] =
	{
		pJumpBlock1,
		pJumpBlock2,
		pFalseBlock2,
	};

	m_pModule->m_ControlFlowMgr.Follow (pPhiBlock);
	m_pModule->m_LlvmIrBuilder.CreatePhi (ValueArray, BlockArray, 3, pResultValue);
	return true;
}

bool
COperatorMgr::LogicalAndOperator (
	CBasicBlock* pOpBlock1,
	CBasicBlock* pOpBlock2,
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	bool Result;

	CBasicBlock* pPrevBlock = m_pModule->m_ControlFlowMgr.SetCurrentBlock (pOpBlock1);

	CFunction* pFunction = GetOverloadedBinaryOperator (EBinOp_LogAnd, RawOpValue1, RawOpValue2);
	if (pFunction)
	{
		m_pModule->m_ControlFlowMgr.Follow (pOpBlock2);
		m_pModule->m_ControlFlowMgr.SetCurrentBlock (pPrevBlock);
		return OverloadedBinaryOperator (pFunction, RawOpValue1, RawOpValue2, pResultValue);
	}

	CValue UnusedResultValue;
	if (!pResultValue)
		pResultValue = &UnusedResultValue;

	CBasicBlock* pLastBlock = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	CBasicBlock* pPhiBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("and_phi");
	CBasicBlock* pTrueBlock2 = m_pModule->m_ControlFlowMgr.CreateBlock ("op2_true");

	CValue OpValue1;
	Result = m_pModule->m_OperatorMgr.CastOperator (RawOpValue1, EType_Bool, &OpValue1);
	if (!Result)
		return false;

	CBasicBlock* pJumpBlock1 = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	Result = m_pModule->m_ControlFlowMgr.ConditionalJump (OpValue1, pOpBlock2, pPhiBlock, pPrevBlock);
	ASSERT (Result);

	CValue OpValue2;
	Result = m_pModule->m_OperatorMgr.CastOperator (RawOpValue2, EType_Bool, &OpValue2);
	if (!Result)
		return false;

	CBasicBlock* pJumpBlock2 = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	Result = m_pModule->m_ControlFlowMgr.ConditionalJump (OpValue2, pTrueBlock2, pPhiBlock);
	ASSERT (Result);

	CType* pType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Bool);
	CValue TrueValue (true, pType);
	CValue FalseValue ((int64_t) false, pType);

	CValue ValueArray [] =
	{
		FalseValue,
		FalseValue,
		TrueValue,
	};

	CBasicBlock* BlockArray [] =
	{
		pJumpBlock1,
		pJumpBlock2,
		pTrueBlock2,
	};

	m_pModule->m_ControlFlowMgr.Follow (pPhiBlock);
	m_pModule->m_LlvmIrBuilder.CreatePhi (ValueArray, BlockArray, 3, pResultValue);
	return true;
}

//.............................................................................

} // namespace jnc {
