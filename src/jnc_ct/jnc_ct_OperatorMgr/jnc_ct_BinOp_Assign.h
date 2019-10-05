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

#pragma once

#include "jnc_ct_BinOp.h"

namespace jnc {
namespace ct {

//..............................................................................

class BinOp_Assign: public BinaryOperator
{
public:
	BinOp_Assign()
	{
		m_opKind = BinOpKind_Assign;
		m_opFlags1 = OpFlag_KeepRef;
		m_opFlags2 = OpFlag_KeepEnum | OpFlag_KeepBool;
	}

	virtual
	bool
	op(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		);
};

//..............................................................................

class BinOp_OpAssign: public BinaryOperator
{
public:
	BinOp_OpAssign()
	{
		m_opFlags1 = OpFlag_KeepRef;
		m_opFlags2 = OpFlag_KeepEnum;
	}

	virtual
	bool
	op(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		);
};

//..............................................................................

class BinOp_RefAssign: public BinaryOperator
{
public:
	BinOp_RefAssign()
	{
		m_opKind = BinOpKind_RefAssign;
		m_opFlags1 = OpFlag_KeepRef;
	}

	virtual
	bool
	op(
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		)
	{
		err::setFormatStringError("'%s' has no overloaded ':=' operator", opValue1.getType ()->getTypeString().sz());
		return false;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
