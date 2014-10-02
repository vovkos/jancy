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
		StorageKind storageKind,
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//.............................................................................

} // namespace jnc {
