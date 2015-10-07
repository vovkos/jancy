// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_BinOp.h"

namespace jnc {
namespace ct {

//.............................................................................

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

//.............................................................................

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

//.............................................................................

} // namespace ct
} // namespace jnc
