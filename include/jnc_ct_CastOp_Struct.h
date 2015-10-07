// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_CastOp.h"

namespace jnc {
namespace ct {

//.............................................................................

class Cast_Struct: public CastOperator
{
protected:
	bool m_recursionStopper;

public:
	Cast_Struct ()
	{
		m_recursionStopper = false;
	}

	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		);

	virtual
	bool
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		);

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
