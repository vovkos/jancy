#include "pch.h"
#include "jnc_BinOp_At.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

Type*
BinOp_At::getResultType (
	const Value& opValue1,
	const Value& opValue2
	)
{
	return opValue1.getType ();
}

bool
BinOp_At::op (
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
	)
{
	bool result;

	Value schedulerValue;
	result = m_module->m_operatorMgr.castOperator (
		opValue2,
		m_module->m_typeMgr.getStdType (StdTypeKind_SchedulerPtr),
		&schedulerValue
		);

	if (!result)
		return false;

	TypeKind opType1 = opValue1.getType ()->getTypeKind ();
	if (opType1 != TypeKind_FunctionPtr && opType1 != TypeKind_FunctionRef)
	{
		setOperatorError (opValue1, opValue2);
		return false;
	}

	FunctionPtrType* targetPtrType = (FunctionPtrType*) opValue1.getType (); // not closure-aware!

	Function* launcher = m_module->m_functionMgr.getScheduleLauncherFunction (targetPtrType);
	if (!launcher)
		return false;

	resultValue->setFunction (launcher);

	Closure* closure = resultValue->createClosure ();
	closure->getArgValueList ()->insertTail (opValue1);
	closure->getArgValueList ()->insertTail (schedulerValue);

	if (opValue1.getClosure ())
		closure->append (*opValue1.getClosure ()->getArgValueList ());

	return true;
}

//.............................................................................

} // namespace jnc {
