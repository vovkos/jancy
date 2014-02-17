// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_BinOp.h"

namespace jnc {

//.............................................................................

class CBinOp_LogAnd: public CBinaryOperator
{
public:
	CBinOp_LogAnd ()
	{
		m_OpKind = EBinOp_LogAnd;
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
		)
	{
		ASSERT (false); // special handling in COperator::LogicalOrOperator
		return true;
	}
};

//.............................................................................

class CBinOp_LogOr: public CBinaryOperator
{
public:
	CBinOp_LogOr ()
	{
		m_OpKind = EBinOp_LogOr;
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
		)
	{
		ASSERT (false); // special handling in COperator::LogicalOrOperator
		return true;
	}
};

//.............................................................................

} // namespace jnc {
