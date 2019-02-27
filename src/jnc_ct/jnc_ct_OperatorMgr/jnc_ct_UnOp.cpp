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
#include "jnc_ct_UnOp.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

const char*
getUnOpKindString(UnOpKind opKind)
{
	static const char* stringTable[UnOpKind__Count] =
	{
		"undefined-unary-operator",  // UnOpKind_Undefined = 0,
		"+",                         // UnOpKind_Plus,
		"-",                         // UnOpKind_Minus,
		"~",                         // UnOpKind_BwNot,
		"&",                         // UnOpKind_Addr,
		"*",                         // UnOpKind_Indir,
		"!",                         // UnOpKind_LogNot,
		"++",                        // UnOpKind_PreInc,
		"--",                        // UnOpKind_PreDec,
		"postfix ++",                // UnOpKind_PostInc,
		"postfix --",                // UnOpKind_PostDec,
		"->",                        // UnOpKind_Ptr,
	};

	return (size_t)opKind < UnOpKind__Count ?
		stringTable[opKind] :
		stringTable[UnOpKind_Undefined];
}

//..............................................................................

UnaryOperator::UnaryOperator()
{
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_opKind = UnOpKind_Undefined;
	m_opFlags = 0;
}

bool
UnaryOperator::getResultType(
	const Value& opValue,
	Value* resultValue
	)
{
	Type* type = getResultType(opValue);
	if (!type)
		return false;

	resultValue->setType(type);
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
