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

#include "jnc_ct_BinOp.h"

namespace jnc {
namespace ct {

//..............................................................................

class BinOp_Idx: public BinaryOperator {
public:
	BinOp_Idx() {
		m_opKind = BinOpKind_Idx;
		m_opFlags1 = OpFlag_KeepPropertyRef | OpFlag_KeepVariantRef | OpFlag_EnsurePtrTargetLayout;
	}

	virtual
	bool
	op(
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
	);

protected:
	bool
	arrayIndexOperator(
		const Value& rawOpValue1,
		ArrayType* arrayType,
		const Value& rawOpValue2,
		Value* resultValue
	);

	bool
	variantIndexOperator(
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
	);

	static
	bool
	propertyIndexOperator(
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
	);

	bool
	derivableTypeIndexOperator(
		DerivableType* derivableType,
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
	);

	Property*
	getDerivableTypeIndexerProperty(
		DerivableType* derivableType,
		const Value& opValue2
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
