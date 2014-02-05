#include "pch.h"
#include "jnc_UnOp_Inc.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
CUnOp_PreInc::GetResultType (const CValue& OpValue)
{
	return OpValue.GetType ();
}

bool
CUnOp_PreInc::Operator (
	const CValue& OpValue,
	CValue* pResultValue
	)
{
	CValue OneValue;
	OneValue.SetConstInt32 (1);
	EBinOp BinOpKind = m_OpKind == EUnOp_PreInc ? EBinOp_AddAssign : EBinOp_SubAssign;
		
	bool Result = m_pModule->m_OperatorMgr.BinaryOperator (BinOpKind, OpValue, OneValue);
	if (!Result)
		return false;

	*pResultValue = OpValue;
	return true;
}

//.............................................................................

CType*
CUnOp_PostInc::GetResultType (const CValue& OpValue)
{
	CValue OldValue;
	m_pModule->m_OperatorMgr.PrepareOperandType (OpValue, &OldValue);
	return OldValue.GetType ();
}

bool
CUnOp_PostInc::Operator (
	const CValue& OpValue,
	CValue* pResultValue
	)
{
	bool Result;

	CValue OldValue;
	Result = m_pModule->m_OperatorMgr.PrepareOperand (OpValue, &OldValue);
	if (!Result)
		return false;

	CValue OneValue;
	OneValue.SetConstInt32 (1);
	EBinOp BinOpKind = m_OpKind == EUnOp_PostInc ? EBinOp_AddAssign : EBinOp_SubAssign;
		
	Result = m_pModule->m_OperatorMgr.BinaryOperator (BinOpKind, OpValue, OneValue);
	if (!Result)
		return false;

	*pResultValue = OldValue;
	return true;
}

//.............................................................................

} // namespace jnc {
