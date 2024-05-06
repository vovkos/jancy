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

class BinOp_Match: public BinaryOperator {
public:
	BinOp_Match() {
		m_opKind = BinOpKind_Match;
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

class BinOp_NotMatch: public BinaryOperator {
public:
	BinOp_NotMatch() {
		m_opKind = BinOpKind_NotMatch;
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
