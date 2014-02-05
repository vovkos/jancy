// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

//.............................................................................

enum EUnOp
{
	EUnOp_Undefined = 0,
	EUnOp_Plus,
	EUnOp_Minus,
	EUnOp_BwNot,	
	EUnOp_Addr,
	EUnOp_Indir,	
	EUnOp_LogNot,
	EUnOp_PreInc,
	EUnOp_PreDec,
	EUnOp_PostInc,
	EUnOp_PostDec,	
	EUnOp_Ptr,
	EUnOp__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
GetUnOpKindString (EUnOp OpKind);

//.............................................................................

enum EOpFlag
{
	EOpFlag_KeepDataRef      = 0x01,
	EOpFlag_KeepClassRef	 = 0x02,
	EOpFlag_KeepFunctionRef  = 0x04,
	EOpFlag_KeepPropertyRef  = 0x08,
	EOpFlag_KeepArrayRef     = 0x10,
	EOpFlag_KeepBool         = 0x20,
	EOpFlag_KeepEnum         = 0x40,
	
	EOpFlag_KeepRef          = 
		EOpFlag_KeepDataRef | 
		EOpFlag_KeepClassRef | 
		EOpFlag_KeepFunctionRef | 
		EOpFlag_KeepPropertyRef,
};

//.............................................................................

class CUnaryOperator
{
	friend class COperatorMgr;

protected:
	CModule* m_pModule;
	EUnOp m_OpKind;
	uint_t m_OpFlags;

public:
	CUnaryOperator ();

	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	EUnOp 
	GetOpKind ()
	{
		return m_OpKind;
	}

	int 
	GetOpFlags ()
	{
		return m_OpFlags;
	}

	bool
	GetResultType (
		const CValue& OpValue,
		CValue* pResultValue
		);

	virtual
	CType*
	GetResultType (const CValue& OpValue) = 0;

	virtual
	bool
	Operator (
		const CValue& OpValue,
		CValue* pResultValue
		) = 0;

	err::CError
	SetOperatorError (CType* pOpType)
	{
		return err::SetFormatStringError (
			"unary '%s' cannot be applied to '%s'",
			GetUnOpKindString (m_OpKind),
			pOpType->GetTypeString ().cc () // thanks a lot gcc
			);
	}

	err::CError
	SetOperatorError (const CValue& OpValue)
	{
		return SetOperatorError (OpValue.GetType ());
	}
};

//.............................................................................

} // namespace jnc {
