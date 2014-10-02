#include "pch.h"
#include "jnc_UnOp.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

const char*
getUnOpKindString (UnOpKind opKind)
{
	static const char* stringTable [UnOpKind__Count] = 
	{
		"undefined-unary-operator",  // EUnOp_Undefined = 0,
		"+",                         // EUnOp_Plus,
		"-",                         // EUnOp_Minus,
		"~",                         // EUnOp_BwNot,	
		"&",                         // EUnOp_Addr,
		"*",                         // EUnOp_Indir,	
		"!",                         // EUnOp_LogNot,
		"++",                        // EUnOp_PreInc,
		"--",                        // EUnOp_PreDec,
		"postfix ++",                // EUnOp_PostInc,
		"postfix --",                // EUnOp_PostDec,	
		"->",                        // EUnOp_Ptr,
	};

	return (size_t) opKind < UnOpKind__Count ? 
		stringTable [opKind] : 
		stringTable [UnOpKind_Undefined];
}

//.............................................................................

UnaryOperator::UnaryOperator ()
{
	m_module = getCurrentThreadModule ();
	ASSERT (m_module);

	m_opKind = UnOpKind_Undefined;
	m_opFlags = 0;
}

bool
UnaryOperator::getResultType (
	const Value& opValue,
	Value* resultValue
	)
{
	Type* type = getResultType (opValue);
	if (!type)
		return false;

	resultValue->setType (type);
	return true;
}

//.............................................................................

} // namespace jnc {
