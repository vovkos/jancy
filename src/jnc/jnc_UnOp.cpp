#include "pch.h"
#include "jnc_UnOp.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

const char*
GetUnOpKindString (EUnOp OpKind)
{
	static const char* StringTable [EUnOp__Count] = 
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

	return (size_t) OpKind < EUnOp__Count ? 
		StringTable [OpKind] : 
		StringTable [EUnOp_Undefined];
}

//.............................................................................

CUnaryOperator::CUnaryOperator ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	m_OpKind = EUnOp_Undefined;
	m_OpFlags = 0;
}

bool
CUnaryOperator::GetResultType (
	const CValue& OpValue,
	CValue* pResultValue
	)
{
	CType* pType = GetResultType (OpValue);
	if (!pType)
		return false;

	pResultValue->SetType (pType);
	return true;
}

//.............................................................................

} // namespace jnc {
