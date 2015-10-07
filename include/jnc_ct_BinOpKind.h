// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {
namespace ct {

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

} // namespace ct
} // namespace jnc
