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

class UnOp_Addr: public UnaryOperator
{
public:
	UnOp_Addr()
	{
		m_opKind = UnOpKind_Addr;
		m_opFlags = OpFlag_KeepRef;
	}

	static
	Type*
	getResultType(const Value& opValue);

	virtual
	bool
	op(
		const Value& opValue,
		Value* resultValue
		);
};

//..............................................................................

class UnOp_Indir: public UnaryOperator
{
public:
	UnOp_Indir()
	{
		m_opKind = UnOpKind_Indir;
	}

	static
	Type*
	getResultType(const Value& opValue);

	virtual
	bool
	op(
		const Value& opValue,
		Value* resultValue
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
