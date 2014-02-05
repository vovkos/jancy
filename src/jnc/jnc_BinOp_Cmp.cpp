#include "pch.h"
#include "jnc_BinOp_Cmp.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
GetPtrCmpOperatorOperandType (
	const CValue& OpValue1,
	const CValue& OpValue2
	)
{
	// TODO: check that we don't compare pointers of different typekinds

	return OpValue1.GetType ()->GetModule ()->GetSimpleType (EType_Int_p);
}

//.............................................................................

llvm::Value*
CBinOp_Eq::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateEq_i (OpValue1, OpValue2, pResultValue);
}

llvm::Value*
CBinOp_Eq::LlvmOpFp (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateEq_f (OpValue1, OpValue2, pResultValue);
}

//.............................................................................

llvm::Value*
CBinOp_Ne::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateNe_i (OpValue1, OpValue2, pResultValue);
}

llvm::Value*
CBinOp_Ne::LlvmOpFp (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateNe_f (OpValue1, OpValue2, pResultValue);
}
	
//.............................................................................

llvm::Value*
CBinOp_Lt::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return IsUnsigned ? 
		m_pModule->m_LlvmIrBuilder.CreateLt_u (OpValue1, OpValue2, pResultValue) :
		m_pModule->m_LlvmIrBuilder.CreateLt_i (OpValue1, OpValue2, pResultValue);
}

llvm::Value*
CBinOp_Lt::LlvmOpFp (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateLt_f (OpValue1, OpValue2, pResultValue);
}

//.............................................................................

llvm::Value*
CBinOp_Le::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return IsUnsigned ? 
		m_pModule->m_LlvmIrBuilder.CreateLe_u (OpValue1, OpValue2, pResultValue) :
		m_pModule->m_LlvmIrBuilder.CreateLe_i (OpValue1, OpValue2, pResultValue);
}

llvm::Value*
CBinOp_Le::LlvmOpFp (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateLe_f (OpValue1, OpValue2, pResultValue);
}

//.............................................................................

llvm::Value*
CBinOp_Gt::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return IsUnsigned ? 
		m_pModule->m_LlvmIrBuilder.CreateGt_u (OpValue1, OpValue2, pResultValue) :
		m_pModule->m_LlvmIrBuilder.CreateGt_i (OpValue1, OpValue2, pResultValue);
}

llvm::Value*
CBinOp_Gt::LlvmOpFp (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateGt_f (OpValue1, OpValue2, pResultValue);
}

//.............................................................................

llvm::Value*
CBinOp_Ge::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return IsUnsigned ? 
		m_pModule->m_LlvmIrBuilder.CreateGe_u (OpValue1, OpValue2, pResultValue) :
		m_pModule->m_LlvmIrBuilder.CreateGe_i (OpValue1, OpValue2, pResultValue);
}

llvm::Value*
CBinOp_Ge::LlvmOpFp (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateGe_f (OpValue1, OpValue2, pResultValue);
}

//.............................................................................

} // namespace jnc {
