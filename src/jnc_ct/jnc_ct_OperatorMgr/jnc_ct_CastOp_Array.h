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

// arr -> arr

class Cast_Array: public CastOperator {
public:
	Cast_Array() {
		m_opFlags = OpFlag_LoadArrayRef;
	}

	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
	);

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

} // namespace ct
} // namespace jnc
