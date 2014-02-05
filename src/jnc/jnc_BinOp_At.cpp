#include "pch.h"
#include "jnc_BinOp_At.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
CBinOp_At::GetResultType (
	const CValue& OpValue1,
	const CValue& OpValue2
	)
{
	return OpValue1.GetType ();
}

bool
CBinOp_At::Operator (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	bool Result;

	CValue SchedulerValue;
	Result = m_pModule->m_OperatorMgr.CastOperator (
		OpValue2,
		m_pModule->m_TypeMgr.GetStdType (EStdType_SchedulerPtr),
		&SchedulerValue
		);

	if (!Result)
		return false;

	EType OpType1 = OpValue1.GetType ()->GetTypeKind ();
	if (OpType1 != EType_FunctionPtr && OpType1 != EType_FunctionRef)
	{
		SetOperatorError (OpValue1, OpValue2);
		return false;
	}

	CFunctionPtrType* pTargetPtrType = (CFunctionPtrType*) OpValue1.GetType (); // not closure-aware!

	CFunction* pLauncher = m_pModule->m_FunctionMgr.GetScheduleLauncherFunction (pTargetPtrType);
	if (!pLauncher)
		return false;

	pResultValue->SetFunction (pLauncher);

	CClosure* pClosure = pResultValue->CreateClosure ();
	pClosure->GetArgValueList ()->InsertTail (OpValue1);
	pClosure->GetArgValueList ()->InsertTail (SchedulerValue);

	if (OpValue1.GetClosure ())
		pClosure->Append (*OpValue1.GetClosure ()->GetArgValueList ());

	return true;
}

//.............................................................................

} // namespace jnc {
