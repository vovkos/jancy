// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_UnOp.h"

namespace jnc {

//.............................................................................

enum BinOpKind
{
	BinOpKind_Undefined = 0,
	
	// arithmetic
	
	BinOpKind_Add,
	BinOpKind_Sub,
	BinOpKind_Mul,
	BinOpKind_Div,
	BinOpKind_Mod,	
	BinOpKind_Shl,
	BinOpKind_Shr,	
	BinOpKind_BwAnd,
	BinOpKind_BwXor,	
	BinOpKind_BwOr,

	// special ops

	BinOpKind_At,
	BinOpKind_Idx,

	// comparison

	BinOpKind_Eq,
	BinOpKind_Ne,
	BinOpKind_Lt,
	BinOpKind_Le,
	BinOpKind_Gt,
	BinOpKind_Ge,

	// logic

	BinOpKind_LogAnd,
	BinOpKind_LogOr,

	// assignment

	BinOpKind_Assign,
	BinOpKind_RefAssign,
	BinOpKind_AddAssign,
	BinOpKind_SubAssign,
	BinOpKind_MulAssign,
	BinOpKind_DivAssign,
	BinOpKind_ModAssign,
	BinOpKind_ShlAssign,
	BinOpKind_ShrAssign,
	BinOpKind_AndAssign,
	BinOpKind_XorAssign,
	BinOpKind_OrAssign,
	BinOpKind_AtAssign,

	BinOpKind__Count,
	BinOpKind__OpAssignDelta = BinOpKind_AddAssign - BinOpKind_Add,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

const char*
getBinOpKindString (BinOpKind opKind);

//.............................................................................

class BinaryOperator
{	
	friend class OperatorMgr;

protected:
	Module* m_module;
	BinOpKind m_opKind;
	uint_t m_opFlags1;
	uint_t m_opFlags2;

public:
	BinaryOperator ();

	Module*
	getModule ()
	{
		return m_module;
	}

	BinOpKind 
	getOpKind ()
	{
		return m_opKind;
	}

	int
	getOpFlags1 ()
	{
		return m_opFlags1;
	}

	int
	getOpFlags2 ()
	{
		return m_opFlags2;
	}

	bool
	getResultType (
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		);

	virtual
	Type*
	getResultType (
		const Value& opValue1,
		const Value& opValue2
		) = 0;

	virtual
	bool
	op (
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		) = 0;

	err::Error
	setOperatorError (		
		Type* opType1,
		Type* opType2
		)
	{
		return err::setFormatStringError (
			"binary '%s' cannot be applied to '%s' and '%s'",
			getBinOpKindString (m_opKind),
			opType1->getTypeString ().cc (), // thanks a lot gcc
			opType2->getTypeString ().cc ()
			);
	}

	err::Error
	setOperatorError (		
		const Value& opValue1,
		const Value& opValue2
		)
	{
		return setOperatorError (opValue1.getType (), opValue2.getType ());
	}
};

//.............................................................................

} // namespace jnc {
