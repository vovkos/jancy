#include "pch.h"
#include "jnc_BinOp_Assign.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................
	
bool
CBinOp_Assign::Operator (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	*pResultValue = OpValue1;

	EType DstTypeKind = OpValue1.GetType ()->GetTypeKind ();
	
	switch (DstTypeKind)
	{
	case EType_DataRef:
		return m_pModule->m_OperatorMgr.StoreDataRef (OpValue1, OpValue2);

	case EType_ClassRef:
		return m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_RefAssign, OpValue1, OpValue2, pResultValue);

	case EType_PropertyRef:
		return m_pModule->m_OperatorMgr.SetProperty (OpValue1, OpValue2);

	default:
		err::SetFormatStringError ("left operand must be l-value");
		return false;
	}
}

//.............................................................................

bool
CBinOp_OpAssign::Operator (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	ASSERT (m_OpKind >= EBinOp_AddAssign && m_OpKind <= EBinOp_AtAssign);

	*pResultValue = OpValue1;

	EBinOp OpKind = (EBinOp) (m_OpKind - EBinOp__OpAssignDelta);

	CValue RValue;
	return 
		m_pModule->m_OperatorMgr.BinaryOperator (OpKind, OpValue1, OpValue2, &RValue) &&
		m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Assign, OpValue1, RValue);
}
	
//.............................................................................

} // namespace jnc {
