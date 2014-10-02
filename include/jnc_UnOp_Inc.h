// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_UnOp.h"

namespace jnc {

//.............................................................................

class UnOp_PreInc: public UnaryOperator
{
public:
	UnOp_PreInc ()
	{
		m_opFlags = OpFlagKind_KeepRef;
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

//.............................................................................

class UnOp_PostInc: public UnaryOperator
{
public:
	UnOp_PostInc ()
	{
		m_opFlags = OpFlagKind_KeepRef;
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

//.............................................................................

} // namespace jnc {
