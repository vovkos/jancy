#include "pch.h"
#include "jnc_UnOp_Arithmetic.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
GetArithmeticOperatorResultType (CType* pOpType)
{
	EType TypeKind = pOpType->GetTypeKind ();

	switch (TypeKind)
	{
	case EType_Int8:
	case EType_Int16:
	case EType_Int16_be:
	case EType_Int32_be:
		TypeKind = EType_Int32;
		break;

	case EType_Int8_u:
	case EType_Int16_u:
	case EType_Int16_beu:
	case EType_Int32_beu:
		TypeKind = EType_Int32_u;
		break;
	
	case EType_Int64_be:
		TypeKind = EType_Int64;
		break;

	case EType_Int64_beu:
		TypeKind = EType_Int64_u;
		break;

	case EType_Int32:
	case EType_Int32_u:
	case EType_Int64:
	case EType_Int64_u:
	case EType_Float:
	case EType_Double:
		// no change
		break;

	default:
		return NULL;
	}

	return pOpType->GetModule ()->m_TypeMgr.GetPrimitiveType (TypeKind);
}

//.............................................................................

llvm::Value*
CUnOp_Minus::LlvmOpInt (
	const CValue& OpValue,
	CType* pResultType,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateNeg_i (OpValue, pResultType, pResultValue);
}

llvm::Value*
CUnOp_Minus::LlvmOpFp (
	const CValue& OpValue,
	CType* pResultType,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateNeg_f (OpValue, pResultType, pResultValue);
}

//.............................................................................

llvm::Value*
CUnOp_BwNot::LlvmOpInt (
	const CValue& OpValue,
	CType* pResultType,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateNot (OpValue, pResultType, pResultValue);
}

//.............................................................................

} // namespace jnc {
