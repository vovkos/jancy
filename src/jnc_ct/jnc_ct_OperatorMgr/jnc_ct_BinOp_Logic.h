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

class BinOp_LogAnd: public BinaryOperator
{
public:
	BinOp_LogAnd ()
	{
		m_opKind = BinOpKind_LogAnd;
	}

	virtual
	Type*
	getResultType (
		const Value& opValue1,
		const Value& opValue2
		);

	virtual
	bool
	op (
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
		)
	{
		ASSERT (false); // special handling in COperator::LogicalOrOperator
		return true;
	}
};

//..............................................................................

class BinOp_LogOr: public BinaryOperator
{
public:
	BinOp_LogOr ()
	{
		m_opKind = BinOpKind_LogOr;
	}

	virtual
	Type*
	getResultType (
		const Value& opValue1,
		const Value& opValue2
		);

	virtual
	bool
	op (
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
		)
	{
		ASSERT (false); // special handling in COperator::LogicalOrOperator
		return true;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
