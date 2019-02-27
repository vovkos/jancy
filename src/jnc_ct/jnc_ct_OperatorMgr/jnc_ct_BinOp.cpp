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
#include "jnc_ct_BinOp.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

const char*
getBinOpKindString(BinOpKind opKind)
{
	static const char* stringTable[BinOpKind__Count] =
	{
		"undefined-binary-operator",  // BinOpKind_Undefined = 0,
		"+",                          // BinOpKind_Add,
		"-",                          // BinOpKind_Sub,
		"*",                          // BinOpKind_Mul,
		"/",                          // BinOpKind_Div,
		"%",                          // BinOpKind_Mod,
		"<<",                         // BinOpKind_Shl,
		">>",                         // BinOpKind_Shr,
		"&",                          // BinOpKind_BwAnd,
		"^",                          // BinOpKind_BwXor,
		"|",                          // BinOpKind_BwOr,
		"@",                          // BinOpKind_At,
		"[]",                         // BinOpKind_Idx,
		"==",                         // BinOpKind_Eq,
		"!=",                         // BinOpKind_Ne,
		"<",                          // BinOpKind_Lt,
		">",                          // BinOpKind_Le,
		"<=",                         // BinOpKind_Gt,
		">=",                         // BinOpKind_Ge,
		"&&",                         // BinOpKind_LogAnd,
		"||",                         // BinOpKind_LogOr,
		"=",                          // BinOpKind_Assign,
		":=",                         // BinOpKind_RefAssign,
		"+=",                         // BinOpKind_AddAssign,
		"-=",                         // BinOpKind_SubAssign,
		"*=",                         // BinOpKind_MulAssign,
		"/=",                         // BinOpKind_DivAssign,
		"%=",                         // BinOpKind_ModAssign,
		"<<=",                        // BinOpKind_ShlAssign,
		">>=",                        // BinOpKind_ShrAssign,
		"&=",                         // BinOpKind_AndAssign,
		"^=",                         // BinOpKind_XorAssign,
		"|=",                         // BinOpKind_OrAssign,
		"@=",                         // BinOpKind_AtAssign,
	};

	return (size_t)opKind < BinOpKind__Count ?
		stringTable[opKind] :
		stringTable[BinOpKind_Undefined];
}

//..............................................................................

BinaryOperator::BinaryOperator()
{
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_opKind = BinOpKind_Undefined;
	m_opFlags1 = m_opFlags2 = 0;
}

bool
BinaryOperator::getResultType(
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
	)
{
	Type* type = getResultType(opValue1, opValue2);
	if (!type)
		return false;

	resultValue->setType(type);
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
