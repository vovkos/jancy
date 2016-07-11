// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_CastOp.h"

namespace jnc {
namespace ct {

//.............................................................................

class Cast_Variant: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		)
	{
		return CastKind_ImplicitCrossFamily;
	}

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

class Cast_FromVariant: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		)
	{
		return CastKind_ImplicitCrossFamily;
	}

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
