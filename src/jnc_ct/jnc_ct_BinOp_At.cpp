//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_ct_BinOp_At.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

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
		((ClassType*) m_module->m_typeMgr.getStdType (StdType_Scheduler))->getClassPtrType (),
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

	Closure* resultClosure = resultValue->createClosure ();
	resultClosure->getArgValueList ()->insertTail (opValue1);
	resultClosure->getArgValueList ()->insertTail (schedulerValue);

	Closure* opClosure = opValue1.getClosure ();
	if (opClosure)
	{
		resultClosure->append (*opClosure->getArgValueList ());

		size_t thisArgIdx = opClosure->getThisArgIdx ();
		if (thisArgIdx != -1)
			resultClosure->setThisArgIdx (thisArgIdx + 2);
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
