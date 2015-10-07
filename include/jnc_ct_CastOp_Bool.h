// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_CastOp.h"

namespace jnc {
namespace ct {

//.............................................................................

// comparison to zero -> bool (common for both integer & fp)

class Cast_BoolFromZeroCmp: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		)
	{
		return CastKind_Implicit;
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

// extract 1st element, convert to int, compare it to zero

class Cast_BoolFromPtr: public Cast_BoolFromZeroCmp
{
public:
	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//.............................................................................

// bool <-> int

class Cast_IntFromBool: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		)
	{
		return CastKind_Implicit;
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

// bool master cast

class Cast_Bool: public Cast_Master
{
protected:
	Cast_BoolFromZeroCmp m_fromZeroCmp;
	Cast_BoolFromPtr m_fromPtr;

public:
	Cast_Bool ()
	{
		m_opFlags = OpFlag_KeepBool;
	}

	virtual
	CastOperator*
	getCastOperator (
		const Value& opValue,
		Type* type
		);
};

//.............................................................................

} // namespace ct
} // namespace jnc
