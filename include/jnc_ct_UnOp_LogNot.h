// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_UnOp.h"

namespace jnc {
namespace ct {

//.............................................................................

class UnOp_LogNot: public UnaryOperator
{
public:
	UnOp_LogNot ()
	{
		m_opKind = UnOpKind_LogNot;
	}

	virtual
	Type*
	getResultType (const Value& opValue);

	virtual
	bool
	op (
		const Value& opValue,
		Value* resultValue
		);

protected:
	bool
	zeroCmpOperator (
		const Value& opValue,
		Value* resultValue
		);

	bool
	ptrOperator (
		const Value& opValue,
		Value* resultValue
		);

	bool
	variantOperator (
		const Value& opValue,
		Value* resultValue
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
