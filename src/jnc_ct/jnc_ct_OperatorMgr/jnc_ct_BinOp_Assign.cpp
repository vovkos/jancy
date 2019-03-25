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
#include "jnc_ct_BinOp_Assign.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
BinOp_Assign::op(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
	)
{
	*resultValue = opValue1;

	TypeKind dstTypeKind = opValue1.getType()->getTypeKind();

	switch (dstTypeKind)
	{
	case TypeKind_DataRef:
		return m_module->m_operatorMgr.storeDataRef(opValue1, opValue2);

	case TypeKind_ClassRef:
		return m_module->m_operatorMgr.binaryOperator(BinOpKind_RefAssign, opValue1, opValue2, resultValue);

	case TypeKind_PropertyRef:
		return m_module->m_operatorMgr.setProperty(opValue1, opValue2);

	default:
		err::setFormatStringError("left operand must be l-value");
		return false;
	}
}

//..............................................................................

bool
BinOp_OpAssign::op(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
	)
{
	ASSERT(m_opKind >= BinOpKind_AddAssign && m_opKind <= BinOpKind_AtAssign);

	*resultValue = opValue1;

	BinOpKind opKind = (BinOpKind)(m_opKind - BinOpKind__OpAssignDelta);

	Value RValue;
	return
		m_module->m_operatorMgr.binaryOperator(opKind, opValue1, opValue2, &RValue) &&
		m_module->m_operatorMgr.binaryOperator(BinOpKind_Assign, opValue1, RValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
