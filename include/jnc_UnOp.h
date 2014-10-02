// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

//.............................................................................

enum UnOpKind
{
	UnOpKind_Undefined = 0,
	UnOpKind_Plus,
	UnOpKind_Minus,
	UnOpKind_BwNot,	
	UnOpKind_Addr,
	UnOpKind_Indir,	
	UnOpKind_LogNot,
	UnOpKind_PreInc,
	UnOpKind_PreDec,
	UnOpKind_PostInc,
	UnOpKind_PostDec,	
	UnOpKind_Ptr,
	UnOpKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getUnOpKindString (UnOpKind opKind);

//.............................................................................

enum OpFlagKind
{
	OpFlagKind_KeepDataRef      = 0x01,
	OpFlagKind_KeepClassRef	 = 0x02,
	OpFlagKind_KeepFunctionRef  = 0x04,
	OpFlagKind_KeepPropertyRef  = 0x08,
	OpFlagKind_KeepBool         = 0x10,
	OpFlagKind_KeepEnum         = 0x20,
	
	OpFlagKind_ArrayRefToPtr    = 0x40,
	
	OpFlagKind_KeepRef          = 
		OpFlagKind_KeepDataRef | 
		OpFlagKind_KeepClassRef | 
		OpFlagKind_KeepFunctionRef | 
		OpFlagKind_KeepPropertyRef,
};

//.............................................................................

class UnaryOperator
{
	friend class OperatorMgr;

protected:
	Module* m_module;
	UnOpKind m_opKind;
	uint_t m_opFlags;

public:
	UnaryOperator ();

	Module*
	getModule ()
	{
		return m_module;
	}

	UnOpKind 
	getOpKind ()
	{
		return m_opKind;
	}

	int 
	getOpFlags ()
	{
		return m_opFlags;
	}

	bool
	getResultType (
		const Value& opValue,
		Value* resultValue
		);

	virtual
	Type*
	getResultType (const Value& opValue) = 0;

	virtual
	bool
	op (
		const Value& opValue,
		Value* resultValue
		) = 0;

	err::Error
	setOperatorError (Type* opType)
	{
		return err::setFormatStringError (
			"unary '%s' cannot be applied to '%s'",
			getUnOpKindString (m_opKind),
			opType->getTypeString ().cc () // thanks a lot gcc
			);
	}

	err::Error
	setOperatorError (const Value& opValue)
	{
		return setOperatorError (opValue.getType ());
	}
};

//.............................................................................

} // namespace jnc {
