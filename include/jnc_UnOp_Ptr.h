// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_UnOp.h"

namespace jnc {

//.............................................................................

class UnOp_Addr: public UnaryOperator
{
public:
	UnOp_Addr ()
	{
		m_opKind = UnOpKind_Addr;
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

class UnOp_Indir: public UnaryOperator
{
public:
	UnOp_Indir ()
	{
		m_opKind = UnOpKind_Indir;
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
