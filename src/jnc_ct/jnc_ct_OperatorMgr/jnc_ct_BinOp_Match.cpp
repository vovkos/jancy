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
#include "jnc_ct_BinOp_Match.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
BinOp_Match::op(
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
) {
	Value opValue1;
	Value opValue2;

	Type* charPtrType = m_module->m_typeMgr.getStdType(StdType_CharConstPtr);

	bool result =
		castOperator(m_module, rawOpValue1, charPtrType, &opValue1) &&
		castOperator(m_module, rawOpValue2, charPtrType, &opValue2);

	if (!result)
		return false;

	RegexCondStmt* regexCondStmt = m_module->m_controlFlowMgr.getRegexCondStmt();

	Value regexValue;
	uint_t regexFlags = regexCondStmt ? regexCondStmt->m_regexFlags : 0;

	if (opValue2.getValueKind() == ValueKind_Const) { // can compile and save regex now
		re2::Regex regex;
		result = regex.compile(*(char**)opValue2.getConstData(), regexFlags);
		if (!result)
			return false;

		Variable* regexVariable = m_module->m_variableMgr.createStaticRegexVariable(regex);
		if (!regexVariable)
			return false;

		regexValue.setVariable(regexVariable);
	} else {
		BasicBlock* throwBlock = m_module->m_controlFlowMgr.createBlock("throw_block");
		BasicBlock* followBlock = m_module->m_controlFlowMgr.createBlock("follow_block");

		ClassType* regexType = (ClassType*)m_module->m_typeMgr.getStdType(StdType_Regex);

		Value compileValue;
		Value compileResultValue;

		result =
			m_module->m_operatorMgr.newOperator(regexType, &regexValue) &&
			m_module->m_operatorMgr.memberOperator(regexValue, "compile", &compileValue) &&
			m_module->m_operatorMgr.callOperator(compileValue, opValue2, &compileResultValue) &&
			m_module->m_controlFlowMgr.conditionalJump(compileResultValue, followBlock, throwBlock, throwBlock);

		if (!result)
			return false;

		m_module->m_controlFlowMgr.throwException();
		m_module->m_controlFlowMgr.setCurrentBlock(followBlock);
	}

	ClassType* stateType = (ClassType*)m_module->m_typeMgr.getStdType(StdType_RegexState);
	Type* flagsType = (ClassType*)m_module->m_typeMgr.getStdType(StdType_RegexExecFlags);
	sl::BoxList<Value> argValueList;
	argValueList.insertTail(Value(regexFlags, flagsType));

	Value stateValue;
	Value execValue;

	return
		m_module->m_operatorMgr.newOperator(stateType, &argValueList, &stateValue) &&
		m_module->m_operatorMgr.memberOperator(regexValue, "execEof", &execValue) &&
		m_module->m_operatorMgr.callOperator(execValue, stateValue, opValue1, NULL) &&
		m_module->m_operatorMgr.memberOperator(stateValue, "m_match", resultValue) &&
		m_module->m_operatorMgr.storeDataRef(m_module->m_variableMgr.getRegexMatchVariable(), *resultValue);
}

//..............................................................................

bool
BinOp_NotMatch::op(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
) {
	return
		m_module->m_operatorMgr.binaryOperator(BinOpKind_Match, opValue1, opValue2, resultValue) &&
		m_module->m_operatorMgr.unaryOperator(UnOpKind_LogNot, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
