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

// floating point truncation

class Cast_FpTrunc: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		return CastKind_Explicit;
	}

	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	) {
		*(float*)dst = (float)*(double*)opValue.getConstData();
		return true;
	}

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// floating point extension

class Cast_FpExt: public CastOperator {
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
		*(double*)dst = *(float*)opValue.getConstData();
		return true;
	}

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

//..............................................................................

// signed integer -> floating point

class Cast_FpFromInt: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		return CastKind_ImplicitCrossFamily;
	}

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);

	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

protected:
	void
	constCast_Fp32(
		const Value& srcValue,
		float* fp32
	);

	void
	constCast_Fp64(
		const Value& srcValue,
		double* fp64
	);
};

//..............................................................................

// unsigned integer -> floating point

class Cast_FpFromInt_u: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		return CastKind_ImplicitCrossFamily;
	}

	virtual
	bool
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);

	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);

protected:
	void
	constCast_Fp32(
		const Value& srcValue,
		float* fp32
	);

	void
	constCast_Fp64(
		const Value& srcValue,
		double* fp64
	);
};

//..............................................................................

// enum -> floating point

class Cast_FpFromEnum: public Cast_SuperMaster {
public:
	virtual
	bool
	getCastOperators(
		const Value& opValue,
		Type* type,
		CastOperator** firstOperator,
		CastOperator** secondOperator,
		Type** intermediateType
	);
};

//..............................................................................

// floating point master cast

class Cast_Fp: public Cast_Master {
protected:
	Cast_FpTrunc m_trunc;
	Cast_FpExt m_ext;
	Cast_FpFromInt m_fromInt;
	Cast_FpFromInt_u m_fromInt_u;
	Cast_FpFromEnum m_fromEnum;

public:
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
