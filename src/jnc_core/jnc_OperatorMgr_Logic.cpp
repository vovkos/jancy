#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
OperatorMgr::logicalOrOperator (
	BasicBlock* opBlock1,
	BasicBlock* opBlock2,
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
	)
{
	bool result;

	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (opBlock1);

	Function* function = getOverloadedBinaryOperator (BinOpKind_LogOr, rawOpValue1);
	if (function)
	{
		m_module->m_controlFlowMgr.follow (opBlock2);
		m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);

		rtl::BoxList <Value> argList;
		argList.insertTail (rawOpValue1);
		argList.insertTail (rawOpValue2);
		return callOperator (function, &argList, resultValue);
	}

	Value unusedResultValue;
	if (!resultValue)
		resultValue = &unusedResultValue;

	BasicBlock* phiBlock = m_module->m_controlFlowMgr.createBlock ("and_phi");
	BasicBlock* falseBlock2 = m_module->m_controlFlowMgr.createBlock ("op2_false");

	Value opValue1;
	result = m_module->m_operatorMgr.castOperator (rawOpValue1, TypeKind_Bool, &opValue1);
	if (!result)
		return false;

	BasicBlock* jumpBlock1 = m_module->m_controlFlowMgr.getCurrentBlock ();
	result = m_module->m_controlFlowMgr.conditionalJump (opValue1, phiBlock, opBlock2, prevBlock);
	ASSERT (result);

	Value opValue2;
	result = m_module->m_operatorMgr.castOperator (rawOpValue2, TypeKind_Bool, &opValue2);
	if (!result)
		return false;

	BasicBlock* jumpBlock2 = m_module->m_controlFlowMgr.getCurrentBlock ();
	result = m_module->m_controlFlowMgr.conditionalJump (opValue2, phiBlock, falseBlock2, falseBlock2);
	ASSERT (result);

	Type* type = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
	Value trueValue (true, type);
	Value falseValue ((int64_t) false, type);

	Value valueArray [] =
	{
		trueValue,
		trueValue,
		falseValue,
	};

	BasicBlock* blockArray [] =
	{
		jumpBlock1,
		jumpBlock2,
		falseBlock2,
	};

	m_module->m_controlFlowMgr.follow (phiBlock);
	m_module->m_llvmIrBuilder.createPhi (valueArray, blockArray, 3, resultValue);
	return true;
}

bool
OperatorMgr::logicalAndOperator (
	BasicBlock* opBlock1,
	BasicBlock* opBlock2,
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
	)
{
	bool result;

	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (opBlock1);

	Function* function = getOverloadedBinaryOperator (BinOpKind_LogAnd, rawOpValue1);
	if (function)
	{
		m_module->m_controlFlowMgr.follow (opBlock2);
		m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);

		rtl::BoxList <Value> argList;
		argList.insertTail (rawOpValue1);
		argList.insertTail (rawOpValue2);
		return callOperator (function, &argList, resultValue);
	}

	Value unusedResultValue;
	if (!resultValue)
		resultValue = &unusedResultValue;

	BasicBlock* lastBlock = m_module->m_controlFlowMgr.getCurrentBlock ();
	BasicBlock* phiBlock = m_module->m_controlFlowMgr.createBlock ("and_phi");
	BasicBlock* trueBlock2 = m_module->m_controlFlowMgr.createBlock ("op2_true");

	Value opValue1;
	result = m_module->m_operatorMgr.castOperator (rawOpValue1, TypeKind_Bool, &opValue1);
	if (!result)
		return false;

	BasicBlock* jumpBlock1 = m_module->m_controlFlowMgr.getCurrentBlock ();
	result = m_module->m_controlFlowMgr.conditionalJump (opValue1, opBlock2, phiBlock, prevBlock);
	ASSERT (result);

	Value opValue2;
	result = m_module->m_operatorMgr.castOperator (rawOpValue2, TypeKind_Bool, &opValue2);
	if (!result)
		return false;

	BasicBlock* jumpBlock2 = m_module->m_controlFlowMgr.getCurrentBlock ();
	result = m_module->m_controlFlowMgr.conditionalJump (opValue2, trueBlock2, phiBlock);
	ASSERT (result);

	Type* type = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
	Value trueValue (true, type);
	Value falseValue ((int64_t) false, type);

	Value valueArray [] =
	{
		falseValue,
		falseValue,
		trueValue,
	};

	BasicBlock* blockArray [] =
	{
		jumpBlock1,
		jumpBlock2,
		trueBlock2,
	};

	m_module->m_controlFlowMgr.follow (phiBlock);
	m_module->m_llvmIrBuilder.createPhi (valueArray, blockArray, 3, resultValue);
	return true;
}

//.............................................................................

} // namespace jnc {
