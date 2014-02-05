#include "pch.h"
#include "jnc_UnOp_LogNot.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
CUnOp_LogNot::GetResultType (const CValue& OpValue)
{
	return m_pModule->m_TypeMgr.GetPrimitiveType (EType_Bool);
}

bool
CUnOp_LogNot::Operator (
	const CValue& OpValue,
	CValue* pResultValue
	)
{
	EType SrcTypeKind = OpValue.GetType ()->GetTypeKind ();
	switch (SrcTypeKind)
	{
	case EType_Bool:
	case EType_Int8:
	case EType_Int8_u:
	case EType_Int16:
	case EType_Int16_u:
	case EType_Int32:
	case EType_Int32_u:
	case EType_Int64:
	case EType_Int64_u:
	case EType_Int16_be:
	case EType_Int16_beu:
	case EType_Int32_be:
	case EType_Int32_beu:
	case EType_Int64_be:
	case EType_Int64_beu:
	case EType_Float:
	case EType_Double:
	case EType_BitField:
	case EType_Enum:
		return ZeroCmpOperator (OpValue, pResultValue);

	case EType_DataPtr:
	case EType_ClassPtr:
	case EType_FunctionPtr:
	case EType_PropertyPtr:
		return PtrOperator (OpValue, pResultValue);

	default:
		SetOperatorError (OpValue);
		return false;
	}
}

bool
CUnOp_LogNot::ZeroCmpOperator (
	const CValue& OpValue,
	CValue* pResultValue
	)
{
	CValue ZeroValue = OpValue.GetType ()->GetZeroValue ();
	return m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Eq, OpValue, ZeroValue, pResultValue);
}

bool
CUnOp_LogNot::PtrOperator (
	const CValue& OpValue,
	CValue* pResultValue
	)
{
	if (OpValue.GetType ()->GetSize () == sizeof (void*))
		return ZeroCmpOperator (OpValue, pResultValue);

	CValue PtrValue;
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 0, m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr), &PtrValue);
	return ZeroCmpOperator (PtrValue, pResultValue);
}

//.............................................................................

} // namespace jnc {
