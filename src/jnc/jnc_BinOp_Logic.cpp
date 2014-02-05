#include "pch.h"
#include "jnc_BinOp_Logic.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
CBinOp_LogAnd::GetResultType (
	const CValue& OpValue1,
	const CValue& OpValue2
	)
{
	return m_pModule->m_TypeMgr.GetPrimitiveType (EType_Bool);
}

bool
CBinOp_LogAnd::Operator (
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	bool Result;

	CBasicBlock* pPhiBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("and_phi");
	CBasicBlock* pTrueBlock1 = m_pModule->m_ControlFlowMgr.CreateBlock ("op1_true");
	CBasicBlock* pTrueBlock2 = m_pModule->m_ControlFlowMgr.CreateBlock ("op2_true");

	CValue OpValue1;
	Result = m_pModule->m_OperatorMgr.CastOperator (RawOpValue1, EType_Bool, &OpValue1);
	if (!Result)
		return false;

	CBasicBlock* pJumpBlock1 = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	Result = m_pModule->m_ControlFlowMgr.ConditionalJump (OpValue1, pTrueBlock1, pPhiBlock);
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

CType*
CBinOp_LogOr::GetResultType (
	const CValue& OpValue1,
	const CValue& OpValue2
	)
{
	return m_pModule->m_TypeMgr.GetPrimitiveType (EType_Bool);
}

bool
CBinOp_LogOr::Operator (
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	bool Result;

	CBasicBlock* pPhiBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("and_phi");
	CBasicBlock* pFalseBlock1 = m_pModule->m_ControlFlowMgr.CreateBlock ("op1_false");
	CBasicBlock* pFalseBlock2 = m_pModule->m_ControlFlowMgr.CreateBlock ("op2_false");

	CValue OpValue1;
	Result = m_pModule->m_OperatorMgr.CastOperator (RawOpValue1, EType_Bool, &OpValue1);
	if (!Result)
		return false;

	CBasicBlock* pJumpBlock1 = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	Result = m_pModule->m_ControlFlowMgr.ConditionalJump (OpValue1, pPhiBlock, pFalseBlock1, pFalseBlock1);
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

//.............................................................................

} // namespace jnc {
