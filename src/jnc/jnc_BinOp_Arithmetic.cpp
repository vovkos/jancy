#include "pch.h"
#include "jnc_BinOp_Arithmetic.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................
	
bool
DataPtrIncrementOperator (
	CModule* pModule,
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	ASSERT (OpValue1.GetType ()->GetTypeKind () == EType_DataPtr);
	
	CDataPtrType* pOpType = (CDataPtrType*) OpValue1.GetType ();
	CDataPtrType* pResultType = pOpType->GetUnCheckedPtrType ();

	EDataPtrType PtrTypeKind = pOpType->GetPtrTypeKind ();
	if (PtrTypeKind == EDataPtrType_Thin)
	{
		pModule->m_LlvmIrBuilder.CreateGep (OpValue1, OpValue2, pResultType, pResultValue);
	}
	else if (PtrTypeKind == EDataPtrType_Lean)
	{
		pModule->m_LlvmIrBuilder.CreateGep (OpValue1, OpValue2, pResultType, pResultValue);
		pResultValue->SetLeanDataPtr (pResultValue->GetLlvmValue (), pResultType, OpValue1.GetLeanDataPtrValidator ());
	}
	else // EDataPtrType_Normal
	{
		CValue PtrValue;
		pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue1, 0, NULL, &PtrValue);
		pModule->m_LlvmIrBuilder.CreateGep (PtrValue, OpValue2, NULL, &PtrValue);
		pModule->m_LlvmIrBuilder.CreateInsertValue (OpValue1, PtrValue, 0, pResultType, pResultValue);
	}
	
	return true;
}	

bool
DataPtrDifferenceOperator (
	CModule* pModule,
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	ASSERT (RawOpValue1.GetType ()->GetTypeKind () == EType_DataPtr);
	ASSERT (RawOpValue2.GetType ()->GetTypeKind () == EType_DataPtr);
	
	CDataPtrType* pOpType = (CDataPtrType*) RawOpValue1.GetType ();
	CDataPtrType* pBytePtrType = (CDataPtrType*) pModule->GetSimpleType (EStdType_BytePtr);

	CValue OpValue1;
	CValue OpValue2;

	bool Result = 
		pModule->m_OperatorMgr.CastOperator (RawOpValue1, pBytePtrType, &OpValue1) &&
		pModule->m_OperatorMgr.CastOperator (RawOpValue2, pBytePtrType, &OpValue2);

	if (!Result)
		return false;

	CType* pType = pModule->GetSimpleType (EType_Int_p);

	CValue DiffValue;
	CValue SizeValue;

	size_t Size = pOpType->GetTargetType ()->GetSize ();
	SizeValue.SetConstSizeT (Size ? Size : 1);

	pModule->m_LlvmIrBuilder.CreatePtrToInt (OpValue1, pType, &OpValue1);
	pModule->m_LlvmIrBuilder.CreatePtrToInt (OpValue2, pType, &OpValue2);
	pModule->m_LlvmIrBuilder.CreateSub_i (OpValue1, OpValue2, pType, &DiffValue);
	pModule->m_LlvmIrBuilder.CreateDiv_i (DiffValue, SizeValue, pType, pResultValue);
	return true;
}
//.............................................................................

bool
CBinOp_Add::Operator (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	if (OpValue1.GetType ()->GetTypeKind () == EType_DataPtr && 
		(OpValue2.GetType ()->GetTypeKindFlags () & ETypeKindFlag_Integer))
		return DataPtrIncrementOperator (m_pModule, OpValue1, OpValue2, pResultValue);
	else if (
		OpValue2.GetType ()->GetTypeKind () == EType_DataPtr && 
		(OpValue1.GetType ()->GetTypeKindFlags () & ETypeKindFlag_Integer))
		return DataPtrIncrementOperator (m_pModule, OpValue2, OpValue1, pResultValue);
	else
		return CBinOpT_Arithmetic <CBinOp_Add>::Operator (OpValue1, OpValue2, pResultValue);
}

llvm::Value*
CBinOp_Add::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateAdd_i (OpValue1, OpValue2, pResultType, pResultValue);
}
	
llvm::Value*
CBinOp_Add::LlvmOpFp (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateAdd_f (OpValue1, OpValue2, pResultType, pResultValue);
}

//.............................................................................

bool
CBinOp_Sub::Operator (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CValue* pResultValue
	)
{
	if (OpValue1.GetType ()->GetTypeKind () == EType_DataPtr && 
		(OpValue2.GetType ()->GetTypeKindFlags () & ETypeKindFlag_Integer))
	{
		CValue NegOpValue2;
		return 
			m_pModule->m_OperatorMgr.UnaryOperator (EUnOp_Minus, OpValue2, &NegOpValue2) &&
			DataPtrIncrementOperator (m_pModule, OpValue1, NegOpValue2, pResultValue);
	}

	if (OpValue1.GetType ()->GetTypeKind () == EType_DataPtr && OpValue2.GetType ()->GetTypeKind () == EType_DataPtr)
		return DataPtrDifferenceOperator (m_pModule, OpValue1, OpValue2, pResultValue);
	else
		return CBinOpT_Arithmetic <CBinOp_Sub>::Operator (OpValue1, OpValue2, pResultValue);

}

llvm::Value*
CBinOp_Sub::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateSub_i (OpValue1, OpValue2, pResultType, pResultValue);
}

	
llvm::Value*
CBinOp_Sub::LlvmOpFp (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateSub_f (OpValue1, OpValue2, pResultType, pResultValue);
}

//.............................................................................

llvm::Value*
CBinOp_Mul::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateMul_i (OpValue1, OpValue2, pResultType, pResultValue);
}
	
llvm::Value*
CBinOp_Mul::LlvmOpFp (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateMul_f (OpValue1, OpValue2, pResultType, pResultValue);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::Value*
CBinOp_Div::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return IsUnsigned ?
		m_pModule->m_LlvmIrBuilder.CreateDiv_u (OpValue1, OpValue2, pResultType, pResultValue) :
		m_pModule->m_LlvmIrBuilder.CreateDiv_i (OpValue1, OpValue2, pResultType, pResultValue);
}

	
llvm::Value*
CBinOp_Div::LlvmOpFp (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateDiv_f (OpValue1, OpValue2, pResultType, pResultValue);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::Value*
CBinOp_Mod::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return IsUnsigned ?
		m_pModule->m_LlvmIrBuilder.CreateMod_u (OpValue1, OpValue2, pResultType, pResultValue) :
		m_pModule->m_LlvmIrBuilder.CreateMod_i (OpValue1, OpValue2, pResultType, pResultValue);
}

//.............................................................................

llvm::Value*
CBinOp_Shl::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateShl (OpValue1, OpValue2, pResultType, pResultValue);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::Value*
CBinOp_Shr::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateShr (OpValue1, OpValue2, pResultType, pResultValue);
}

//.............................................................................

llvm::Value*
CBinOp_BwAnd::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateAnd (OpValue1, OpValue2, pResultType, pResultValue);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

CBinOp_BwOr::CBinOp_BwOr ()
{
	m_OpKind = EBinOp_BwOr;
	m_OpFlags1 = EOpFlag_KeepEnum;
	m_OpFlags2 = EOpFlag_KeepEnum;
}

bool
CBinOp_BwOr::Operator (
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	CValue OpValue1;
	CValue OpValue2;

	if (!IsFlagEnumOpType (RawOpValue1, RawOpValue2))
	{
		return 
			m_pModule->m_OperatorMgr.PrepareOperand (RawOpValue1, &OpValue1) &&
			m_pModule->m_OperatorMgr.PrepareOperand (RawOpValue2, &OpValue2) &&
			CBinOpT_IntegerOnly <CBinOp_BwOr>::Operator (OpValue1, OpValue2, pResultValue);
	}

	CEnumType* pEnumType = (CEnumType*) RawOpValue1.GetType ();

	CValue TmpValue;

	OpValue1.OverrideType (RawOpValue1, pEnumType->GetBaseType ());
	OpValue2.OverrideType (RawOpValue2, pEnumType->GetBaseType ());

	return 
		CBinOpT_IntegerOnly <CBinOp_BwOr>::Operator (OpValue1, OpValue2, &TmpValue) &&
		m_pModule->m_OperatorMgr.CastOperator (TmpValue, pEnumType, pResultValue);
}

llvm::Value*
CBinOp_BwOr::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateOr (OpValue1, OpValue2, pResultType, pResultValue);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::Value*
CBinOp_BwXor::LlvmOpInt (
	const CValue& OpValue1,
	const CValue& OpValue2,
	CType* pResultType,
	CValue* pResultValue,
	bool IsUnsigned
	)
{
	return m_pModule->m_LlvmIrBuilder.CreateXor (OpValue1, OpValue2, pResultType, pResultValue);
}

//.............................................................................

} // namespace jnc {
