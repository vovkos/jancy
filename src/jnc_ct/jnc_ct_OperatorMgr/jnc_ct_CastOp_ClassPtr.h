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

class Cast_ClassPtr: public CastOperator {
public:
	Cast_ClassPtr() {
		m_opFlags = OpFlag_EnsurePtrTargetLayout;
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

// class ref (UnOpKind_Addr => class ptr cast => UnOpKind_Indir)

class Cast_ClassRef: public CastOperator {
public:
	Cast_ClassRef() {
		m_opFlags = OpFlag_KeepRef;
	}

	virtual
	CastKind
	getCastKind(
		const Value& opValue,
		Type* type
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
