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

class BinOp_At: public BinaryOperator
{
public:
	BinOp_At()
	{
		m_opKind = BinOpKind_At;
	}

	virtual
	bool
	op(
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
