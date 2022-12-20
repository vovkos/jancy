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

#include "jnc_ct_CastOp.h"

namespace jnc {
namespace ct {

//..............................................................................

class Cast_BoolTrue: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		return CastKind_Implicit;
	}

	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	) {
		*(bool*)dst = true;
		return true;
	}

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	) {
		static bool trueValue = true;
		return resultValue->createConst(&trueValue, type);
	}
};

//..............................................................................

// comparison to zero -> bool (common for both integer & fp)

class Cast_BoolFromZeroCmp: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		return CastKind_Implicit;
	}

	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};


//..............................................................................

// extract 1st element, convert to int, compare it to zero

class Cast_BoolFromPtr: public Cast_BoolFromZeroCmp {
public:
	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// bool <-> int

class Cast_IntFromBool: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		return CastKind_Implicit;
	}

	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// bool master cast

class Cast_Bool: public Cast_Master {
protected:
	Cast_BoolFromZeroCmp m_fromZeroCmp;
	Cast_BoolFromPtr m_fromPtr;
	Cast_BoolTrue m_true;

public:
	Cast_Bool() {
		m_opFlags = OpFlag_KeepBool;
	}

	virtual
	CastOperator*
	getCastOperator(
		const Value& opValue,
		Type* type
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
