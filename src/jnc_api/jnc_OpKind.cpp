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

#include "pch.h"
#include "jnc_OpKind.h"

//..............................................................................

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getUnOpKindString (jnc_UnOpKind opKind)
{
	static const char* stringTable [jnc_UnOpKind__Count] =
	{
		"undefined-unary-operator",  // jnc_UnOpKind_Undefined = 0,
		"+",                         // jnc_UnOpKind_Plus,
		"-",                         // jnc_UnOpKind_Minus,
		"~",                         // jnc_UnOpKind_BwNot,
		"*",                         // jnc_UnOpKind_Indir,
		"&",                         // jnc_UnOpKind_Addr,
		"!",                         // jnc_UnOpKind_LogNot,
		"++",                        // jnc_UnOpKind_PreInc,
		"--",                        // jnc_UnOpKind_PreDec,
		"postfix ++",                // jnc_UnOpKind_PostInc,
		"postfix --",                // jnc_UnOpKind_PostDec,
		"->",                        // jnc_UnOpKind_Ptr,
	};

	return (size_t) opKind < jnc_UnOpKind__Count ?
		stringTable [opKind] :
		stringTable [jnc_UnOpKind_Undefined];
}

JNC_EXTERN_C
JNC_EXPORT_O
const char*
jnc_getBinOpKindString (jnc_BinOpKind opKind)
{
	static const char* stringTable [jnc_BinOpKind__Count] =
	{
		"undefined-binary-operator",  // jnc_BinOpKind_Undefined = 0,
		"+",                          // jnc_BinOpKind_Add,
		"-",                          // jnc_BinOpKind_Sub,
		"*",                          // jnc_BinOpKind_Mul,
		"/",                          // jnc_BinOpKind_Div,
		"%",                          // jnc_BinOpKind_Mod,
		"<<",                         // jnc_BinOpKind_Shl,
		">>",                         // jnc_BinOpKind_Shr,
		"&",                          // jnc_BinOpKind_BwAnd,
		"^",                          // jnc_BinOpKind_BwXor,
		"|",                          // jnc_BinOpKind_BwOr,
		"@",                          // jnc_BinOpKind_At,
		"==",                         // jnc_BinOpKind_Eq,
		"!=",                         // jnc_BinOpKind_Ne,
		"<",                          // jnc_BinOpKind_Lt,
		">",                          // jnc_BinOpKind_Le,
		"<=",                         // jnc_BinOpKind_Gt,
		">=",                         // jnc_BinOpKind_Ge,
		"[]",                         // jnc_BinOpKind_Idx,
		"&&",                         // jnc_BinOpKind_LogAnd,
		"||",                         // jnc_BinOpKind_LogOr,
		"=",                          // jnc_BinOpKind_Assign,
		":=",                         // jnc_BinOpKind_RefAssign,
		"+=",                         // jnc_BinOpKind_AddAssign,
		"-=",                         // jnc_BinOpKind_SubAssign,
		"*=",                         // jnc_BinOpKind_MulAssign,
		"/=",                         // jnc_BinOpKind_DivAssign,
		"%=",                         // jnc_BinOpKind_ModAssign,
		"<<=",                        // jnc_BinOpKind_ShlAssign,
		">>=",                        // jnc_BinOpKind_ShrAssign,
		"&=",                         // jnc_BinOpKind_AndAssign,
		"^=",                         // jnc_BinOpKind_XorAssign,
		"|=",                         // jnc_BinOpKind_OrAssign,
		"@=",                         // jnc_BinOpKind_AtAssign,
	};

	return (size_t) opKind < jnc_BinOpKind__Count ?
		stringTable [opKind] :
		stringTable [jnc_BinOpKind_Undefined];
}

//..............................................................................
