// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_UnOp.h"

namespace jnc {

//.............................................................................

class CUnOp_Addr: public CUnaryOperator
{
public:
	CUnOp_Addr ()
	{
		m_OpKind = EUnOp_Addr;
		m_OpFlags = EOpFlag_KeepRef;
	}

	virtual
	CType*
	GetResultType (const CValue& OpValue);

	virtual
	bool
	Operator (
		const CValue& OpValue,
		CValue* pResultValue
		);
};

//.............................................................................

class CUnOp_Indir: public CUnaryOperator
{
public:
	CUnOp_Indir ()
	{
		m_OpKind = EUnOp_Indir;
	}

	virtual
	CType*
	GetResultType (const CValue& OpValue);

	virtual
	bool
	Operator (
		const CValue& OpValue,
		CValue* pResultValue
		);
};

//.............................................................................

} // namespace jnc {
