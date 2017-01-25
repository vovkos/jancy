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

class UnOp_PreInc: public UnaryOperator
{
public:
	UnOp_PreInc ()
	{
		m_opFlags = OpFlag_KeepRef;
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
};

//..............................................................................

class UnOp_PostInc: public UnaryOperator
{
public:
	UnOp_PostInc ()
	{
		m_opFlags = OpFlag_KeepRef;
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
};

//..............................................................................

} // namespace ct
} // namespace jnc
