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
#include "jnc_String.h"

namespace jnc {
namespace ct {

//..............................................................................

class Cast_StringBase: public CastOperator {
public:
	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);

protected:
	bool
	preparePtr(
		const Value& opValue,
		Value* resultValue
	);

	bool
	finalizeString(
		String* string,
		const char* p,
		size_t length,
		DataPtrValidator* validator
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_String_FromStringable: public CastOperator {
public:
	Cast_String_FromStringable() {
		m_opFlags = OpFlag_KeepDerivableRef;
	}

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
	constCast(
		const Value& opValue,
		Type* type,
		void* dst
	) {
		return err::fail("can't call 'toString()' of a constant");
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

class Cast_String_FromPtr: public Cast_StringBase {
public:
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

class Cast_String_FromArray: public Cast_StringBase {
public:
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

class Cast_String: public Cast_Master {
protected:
	Cast_String_FromStringable m_fromStringable;
	Cast_String_FromPtr m_fromPtr;
	Cast_String_FromArray m_fromArray;

public:
	Cast_String() {
		m_opFlags = OpFlag_KeepDerivableRef;
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
