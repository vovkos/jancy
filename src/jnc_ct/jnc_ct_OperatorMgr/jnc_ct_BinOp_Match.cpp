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

	Type* charPtrType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Char)->getDataPtrType(
		DataPtrTypeKind_Normal,
		PtrTypeFlag_Const
	);

	bool result =
		castOperator(m_module, rawOpValue1, charPtrType, &opValue1) &&
		castOperator(m_module, rawOpValue2, charPtrType, &opValue2);

	if (!result)
		return false;

	Value regexValue;
	Value execValue;

	if (opValue2.getValueKind() == ValueKind_Const) { // can compile and save regex now
		re::Regex regex;
		result = regex.compile(*(char**)opValue2.getConstData());
		if (!result)
			return false;

		Variable* regexVariable = m_module->m_variableMgr.createStaticRegexVariable("regex", &regex);
		if (!regexVariable)
			return false;

		regexValue.setVariable(regexVariable);
	} else {
		err::setError("dynamic regex-es not supported yet");
		return false;
	}

	result =
		m_module->m_operatorMgr.memberOperator(regexValue, "exec", &execValue) &&
		m_module->m_operatorMgr.callOperator(
			execValue,
			opValue1,
			resultValue
		);

	if (!result)
		return false;

	ScopedCondStmt* scopedCondStmt = m_module->m_controlFlowMgr.getScopedCondStmt();
	if (scopedCondStmt)
		scopedCondStmt->m_regexStateValue = *resultValue;

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
