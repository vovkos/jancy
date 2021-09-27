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

#include "jnc_ct_UnOp.h"

namespace jnc {
namespace ct {

//..............................................................................

class UnOp_LogNot: public UnaryOperator {
public:
	UnOp_LogNot() {
		m_opKind = UnOpKind_LogNot;
	}

	virtual
	bool
	op(
		const Value& opValue,
		Value* resultValue
	);

protected:
	bool
	zeroCmpOperator(
		const Value& opValue,
		Value* resultValue
	);

	bool
	ptrOperator(
		const Value& opValue,
		Value* resultValue
	);

	bool
	variantOperator(
		const Value& opValue,
		Value* resultValue
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
