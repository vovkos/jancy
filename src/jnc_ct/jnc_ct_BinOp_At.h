// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_BinOp.h"

namespace jnc {
namespace ct {

//..............................................................................

class BinOp_At: public BinaryOperator
{
public:
	BinOp_At ()
	{
		m_opKind = BinOpKind_At;
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
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
