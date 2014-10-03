// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_BinOp.h"

namespace jnc {

//.............................................................................

class BinOp_Idx: public BinaryOperator
{
public:
	BinOp_Idx ()
	{
		m_opKind = BinOpKind_Idx;
		m_opFlags1 = OpFlag_KeepPropertyRef;
	}

	virtual
	Type*
	getResultType (
		const Value& opValue1,
		const Value& opValue2
		);

	virtual
	bool
	op (
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
		);

protected:
	bool
	arrayIndexOperator (
		const Value& rawOpValue1,
		ArrayType* arrayType,
		const Value& rawOpValue2,
		Value* resultValue
		);

	bool
	propertyIndexOperator (
		const Value& rawOpValue1,
		const Value& rawOpValue2,
		Value* resultValue
		);

	Type*
	getPropertyIndexResultType (
		const Value& rawOpValue1,
		const Value& rawOpValue2
		);
};

//.............................................................................

} // namespace jnc {
