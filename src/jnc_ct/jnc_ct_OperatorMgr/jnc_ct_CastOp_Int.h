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

// integer truncation

class Cast_IntTrunc: public CastOperator {
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

// integer extensions

class Cast_IntExt: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		return opValue.getValueKind() == ValueKind_Const ? CastKind_Identity : CastKind_Implicit;
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

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_IntExt_u: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		return opValue.getValueKind() == ValueKind_Const ? CastKind_Identity : CastKind_Implicit;
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

// big endian <-> little endian

class Cast_SwapByteOrder: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		return opValue.getValueKind() == ValueKind_Const ? CastKind_Identity : CastKind_Implicit;
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

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_IntFromBeInt: public Cast_SuperMaster {
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

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_BeInt: public Cast_SuperMaster {
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

// floating point -> integer

class Cast_IntFromFp: public CastOperator {
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
	llvmCast(
		const Value& opValue,
		Type* type,
		Value* resultValue
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_IntFromFp32: public Cast_IntFromFp {
public:
	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_IntFromFp64: public Cast_IntFromFp {
public:
	virtual
	bool
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	);
};

//..............................................................................

// enum <-> integer

class Cast_IntFromEnum: public Cast_SuperMaster {
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

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_Enum: public Cast_SuperMaster {
public:
	Cast_Enum() {
		m_opFlags = OpFlag_KeepEnum;
	}

	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);

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

// pointer <-> integer

class Cast_IntFromPtr: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	) {
		ASSERT(opValue.getType()->getSize() >= sizeof(intptr_t));
		return CastKind_Explicit;
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

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_PtrFromInt: public CastOperator {
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

// integer master cast

class Cast_Int: public Cast_Master {
protected:
	Cast_IntTrunc m_trunc;
	Cast_IntExt m_ext;
	Cast_IntExt_u m_ext_u;
	Cast_IntFromBeInt m_fromBeInt;
	Cast_IntFromFp32 m_fromFp32;
	Cast_IntFromFp64 m_fromFp64;
	Cast_IntFromEnum m_fromEnum;
	Cast_IntFromPtr m_fromPtr;

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
