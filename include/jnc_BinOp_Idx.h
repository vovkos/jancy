// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_BinOp.h"

namespace jnc {

//.............................................................................

class CBinOp_Idx: public CBinaryOperator
{
public:
	CBinOp_Idx ()
	{
		m_OpKind = EBinOp_Idx;
		m_OpFlags1 = EOpFlag_KeepPropertyRef;
	}

	virtual
	CType*
	GetResultType (
		const CValue& OpValue1,
		const CValue& OpValue2
		);

	virtual
	bool
	Operator (
		const CValue& RawOpValue1,
		const CValue& RawOpValue2,
		CValue* pResultValue
		);

protected:
	bool
	ArrayIndexOperator (
		const CValue& RawOpValue1,
		CArrayType* pArrayType,
		const CValue& RawOpValue2,
		CValue* pResultValue
		);

	bool
	PropertyIndexOperator (
		const CValue& RawOpValue1,
		const CValue& RawOpValue2,
		CValue* pResultValue
		);

	CType*
	GetPropertyIndexResultType (
		const CValue& RawOpValue1,
		const CValue& RawOpValue2
		);
};

//.............................................................................

} // namespace jnc {
