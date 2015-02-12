// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CastOp.h"

namespace jnc {

//.............................................................................

// arr -> arr

class Cast_Array: public CastOperator
{
public:
	Cast_Array ()
	{
		// m_opFlags = OpFlag_LoadArrayRef;
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

} // namespace jnc {
