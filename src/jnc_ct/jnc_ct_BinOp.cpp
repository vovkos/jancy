#include "pch.h"
#include "jnc_ct_BinOp.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

const char*
getBinOpKindString (BinOpKind opKind)
{
	static const char* stringTable [BinOpKind__Count] =
	{
		"undefined-binary-operator",  // EBinOp_Undefined = 0,
		"+",                          // EBinOp_Add,
		"-",                          // EBinOp_Sub,
		"*",                          // EBinOp_Mul,
		"/",                          // EBinOp_Div,
		"%",                          // EBinOp_Mod,
		"<<",                         // EBinOp_Shl,
		">>",                         // EBinOp_Shr,
		"&",                          // EBinOp_BwAnd,
		"^",                          // EBinOp_BwXor,
		"|",                          // EBinOp_BwOr,
		"@",                          // EBinOp_At,
		"[]",                         // EBinOp_Idx,
		"==",                         // EBinOp_Eq,
		"!=",                         // EBinOp_Ne,
		"<",                          // EBinOp_Lt,
		">",                          // EBinOp_Le,
		"<=",                         // EBinOp_Gt,
		">=",                         // EBinOp_Ge,
		"&&",                         // EBinOp_LogAnd,
		"||",                         // EBinOp_LogOr,
		"=",                          // EBinOp_Assign,
		":=",                         // EBinOp_RefAssign,
		"+=",                         // EBinOp_AddAssign,
		"-=",                         // EBinOp_SubAssign,
		"*=",                         // EBinOp_MulAssign,
		"/=",                         // EBinOp_DivAssign,
		"%=",                         // EBinOp_ModAssign,
		"<<=",                        // EBinOp_ShlAssign,
		">>=",                        // EBinOp_ShrAssign,
		"&=",                         // EBinOp_AndAssign,
		"^=",                         // EBinOp_XorAssign,
		"|=",                         // EBinOp_OrAssign,
		"@=",                         // EBinOp_AtAssign,
	};

	return (size_t) opKind < BinOpKind__Count ?
		stringTable [opKind] :
		stringTable [BinOpKind_Undefined];
}

//..............................................................................

BinaryOperator::BinaryOperator()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_opKind = BinOpKind_Undefined;
	m_opFlags1 = m_opFlags2 = 0;
}

bool
BinaryOperator::getResultType (
	const Value& opValue1,
	const Value& opValue2,
	Value* resultValue
	)
{
	Type* type = getResultType (opValue1, opValue2);
	if (!type)
		return false;

	resultValue->setType (type);
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
