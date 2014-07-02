#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

COperatorMgr::COperatorMgr ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	// operator tables

	memset (m_UnaryOperatorTable, 0, sizeof (m_UnaryOperatorTable));
	memset (m_BinaryOperatorTable, 0, sizeof (m_BinaryOperatorTable));
	memset (m_CastOperatorTable, 0, sizeof (m_CastOperatorTable));

	// unary arithmetics

	m_UnaryOperatorTable [EUnOp_Plus]     = &m_UnOp_Plus;
	m_UnaryOperatorTable [EUnOp_Minus]    = &m_UnOp_Minus;
	m_UnaryOperatorTable [EUnOp_BwNot]    = &m_UnOp_BwNot;
	m_UnaryOperatorTable [EUnOp_LogNot]   = &m_UnOp_LogNot;

	// pointer operators

	m_UnaryOperatorTable [EUnOp_Addr]     = &m_UnOp_Addr;
	m_UnaryOperatorTable [EUnOp_Indir]    = &m_UnOp_Indir;
	m_UnaryOperatorTable [EUnOp_Ptr]      = &m_UnOp_Indir;

	// increment operators

	m_UnOp_PreInc.m_OpKind  = EUnOp_PreInc;
	m_UnOp_PreDec.m_OpKind  = EUnOp_PreDec;
	m_UnOp_PostInc.m_OpKind = EUnOp_PostInc;
	m_UnOp_PostDec.m_OpKind = EUnOp_PostDec;

	m_UnaryOperatorTable [EUnOp_PreInc]   = &m_UnOp_PreInc;
	m_UnaryOperatorTable [EUnOp_PreDec]   = &m_UnOp_PreDec;
	m_UnaryOperatorTable [EUnOp_PostInc]  = &m_UnOp_PostInc;
	m_UnaryOperatorTable [EUnOp_PostDec]  = &m_UnOp_PostDec;

	// binary arithmetics

	m_BinaryOperatorTable [EBinOp_Add]    = &m_BinOp_Add;
	m_BinaryOperatorTable [EBinOp_Sub]    = &m_BinOp_Sub;
	m_BinaryOperatorTable [EBinOp_Mul]    = &m_BinOp_Mul;
	m_BinaryOperatorTable [EBinOp_Div]    = &m_BinOp_Div;
	m_BinaryOperatorTable [EBinOp_Mod]    = &m_BinOp_Mod;
	m_BinaryOperatorTable [EBinOp_Shl]    = &m_BinOp_Shl;
	m_BinaryOperatorTable [EBinOp_Shr]    = &m_BinOp_Shr;
	m_BinaryOperatorTable [EBinOp_BwAnd]  = &m_BinOp_BwAnd;
	m_BinaryOperatorTable [EBinOp_BwXor]  = &m_BinOp_BwXor;
	m_BinaryOperatorTable [EBinOp_BwOr]   = &m_BinOp_BwOr;

	// special operators

	m_BinaryOperatorTable [EBinOp_At]     = &m_BinOp_At;
	m_BinaryOperatorTable [EBinOp_Idx]    = &m_BinOp_Idx;

	// binary logic operators

	m_BinaryOperatorTable [EBinOp_LogAnd] = &m_BinOp_LogAnd;
	m_BinaryOperatorTable [EBinOp_LogOr]  = &m_BinOp_LogOr;

	// comparison operators

	m_BinaryOperatorTable [EBinOp_Eq]     = &m_BinOp_Eq;
	m_BinaryOperatorTable [EBinOp_Ne]     = &m_BinOp_Ne;
	m_BinaryOperatorTable [EBinOp_Lt]     = &m_BinOp_Lt;
	m_BinaryOperatorTable [EBinOp_Le]     = &m_BinOp_Le;
	m_BinaryOperatorTable [EBinOp_Gt]     = &m_BinOp_Gt;
	m_BinaryOperatorTable [EBinOp_Ge]     = &m_BinOp_Ge;

	// assignment operators

	m_BinOp_AddAssign.m_OpKind = EBinOp_AddAssign;
	m_BinOp_SubAssign.m_OpKind = EBinOp_SubAssign;
	m_BinOp_MulAssign.m_OpKind = EBinOp_MulAssign;
	m_BinOp_DivAssign.m_OpKind = EBinOp_DivAssign;
	m_BinOp_ModAssign.m_OpKind = EBinOp_ModAssign;
	m_BinOp_ShlAssign.m_OpKind = EBinOp_ShlAssign;
	m_BinOp_ShrAssign.m_OpKind = EBinOp_ShrAssign;
	m_BinOp_AndAssign.m_OpKind = EBinOp_AndAssign;
	m_BinOp_XorAssign.m_OpKind = EBinOp_XorAssign;
	m_BinOp_OrAssign.m_OpKind  = EBinOp_OrAssign;
	m_BinOp_AtAssign.m_OpKind  = EBinOp_AtAssign;

	m_BinaryOperatorTable [EBinOp_Assign]      = &m_BinOp_Assign;
	m_BinaryOperatorTable [EBinOp_RefAssign]   = &m_BinOp_RefAssign;
	m_BinaryOperatorTable [EBinOp_AddAssign]   = &m_BinOp_AddAssign;
	m_BinaryOperatorTable [EBinOp_SubAssign]   = &m_BinOp_SubAssign;
	m_BinaryOperatorTable [EBinOp_MulAssign]   = &m_BinOp_MulAssign;
	m_BinaryOperatorTable [EBinOp_DivAssign]   = &m_BinOp_DivAssign;
	m_BinaryOperatorTable [EBinOp_ModAssign]   = &m_BinOp_ModAssign;
	m_BinaryOperatorTable [EBinOp_ShlAssign]   = &m_BinOp_ShlAssign;
	m_BinaryOperatorTable [EBinOp_ShrAssign]   = &m_BinOp_ShrAssign;
	m_BinaryOperatorTable [EBinOp_AndAssign]   = &m_BinOp_AndAssign;
	m_BinaryOperatorTable [EBinOp_XorAssign]   = &m_BinOp_XorAssign;
	m_BinaryOperatorTable [EBinOp_OrAssign]    = &m_BinOp_OrAssign;
	m_BinaryOperatorTable [EBinOp_AtAssign]    = &m_BinOp_AtAssign;

	// cast operators

	m_StdCastOperatorTable [EStdCast_Copy] = &m_Cast_Copy;
	m_StdCastOperatorTable [EStdCast_SwapByteOrder] = &m_Cast_SwapByteOrder;
	m_StdCastOperatorTable [EStdCast_PtrFromInt] = &m_Cast_PtrFromInt;
	m_StdCastOperatorTable [EStdCast_Int] = &m_Cast_Int;
	m_StdCastOperatorTable [EStdCast_Fp] = &m_Cast_Fp;

	for (size_t i = 0; i < EType__Count; i++)
		m_CastOperatorTable [i] = &m_Cast_Default;

	m_CastOperatorTable [EType_Bool] = &m_Cast_Bool;

	for (size_t i = EType_Int8; i <= EType_Int64_u; i++)
		m_CastOperatorTable [i] = &m_Cast_Int;

	for (size_t i = EType_Int16_be; i <= EType_Int64_beu; i++)
		m_CastOperatorTable [i] = &m_Cast_BeInt;

	m_CastOperatorTable [EType_Float]       = &m_Cast_Fp;
	m_CastOperatorTable [EType_Double]      = &m_Cast_Fp;
	m_CastOperatorTable [EType_Array]       = &m_Cast_Array;
	m_CastOperatorTable [EType_Enum]        = &m_Cast_Enum;
	m_CastOperatorTable [EType_Struct]      = &m_Cast_Struct;
	m_CastOperatorTable [EType_DataPtr]     = &m_Cast_DataPtr;
	m_CastOperatorTable [EType_DataRef]     = &m_Cast_DataRef;
	m_CastOperatorTable [EType_ClassPtr]    = &m_Cast_ClassPtr;
	m_CastOperatorTable [EType_FunctionPtr] = &m_Cast_FunctionPtr;
	m_CastOperatorTable [EType_FunctionRef] = &m_Cast_FunctionRef;
	m_CastOperatorTable [EType_PropertyPtr] = &m_Cast_PropertyPtr;
	m_CastOperatorTable [EType_PropertyRef] = &m_Cast_PropertyRef;
}

CType*
COperatorMgr::GetUnaryOperatorResultType (
	EUnOp OpKind,
	const CValue& RawOpValue
	)
{
	ASSERT ((size_t) OpKind < EUnOp__Count);

	CValue OpValue;

	PrepareOperandType (RawOpValue, &OpValue);
	if (OpValue.GetType ()->GetTypeKind () == EType_ClassPtr)
	{
		CClassType* pClassType = ((CClassPtrType*) OpValue.GetType ())->GetTargetType ();
		CFunction* pFunction = pClassType->GetUnaryOperator (OpKind);
		if (pFunction)
		{
			rtl::CBoxListT <CValue> ArgList;
			ArgList.InsertTail (RawOpValue);
			return GetCallOperatorResultType (pFunction->GetTypeOverload (), &ArgList);
		}
	}

	CUnaryOperator* pOperator = m_UnaryOperatorTable [OpKind];
	ASSERT (pOperator);

	PrepareOperandType (RawOpValue, &OpValue, pOperator->GetOpFlags ());

	return pOperator->GetResultType (OpValue);
}

bool
COperatorMgr::GetUnaryOperatorResultType (
	EUnOp OpKind,
	const CValue& RawOpValue,
	CValue* pResultValue
	)
{
	CType* pResultType = GetUnaryOperatorResultType (OpKind, RawOpValue);
	if (!pResultType)
		return false;

	pResultValue->SetType (pResultType);
	return true;
}

bool
COperatorMgr::UnaryOperator (
	EUnOp OpKind,
	const CValue& RawOpValue,
	CValue* pResultValue
	)
{
	ASSERT ((size_t) OpKind < EUnOp__Count);

	CValue OpValue;
	CValue UnusedResultValue;

	if (!pResultValue)
		pResultValue = &UnusedResultValue;

	PrepareOperandType (RawOpValue, &OpValue);
	if (OpValue.GetType ()->GetTypeKind () == EType_ClassPtr)
	{
		CClassType* pClassType = ((CClassPtrType*) OpValue.GetType ())->GetTargetType ();
		CFunction* pFunction = pClassType->GetUnaryOperator (OpKind);
		if (pFunction)
		{
			rtl::CBoxListT <CValue> ArgList;
			ArgList.InsertTail (RawOpValue);
			return CallOperator (pFunction, &ArgList, pResultValue);
		}
	}

	CUnaryOperator* pOperator = m_UnaryOperatorTable [OpKind];
	ASSERT (pOperator);

	return
		PrepareOperand (RawOpValue, &OpValue, pOperator->GetOpFlags ()) &&
		pOperator->Operator (OpValue, pResultValue);
}

CType*
COperatorMgr::GetBinaryOperatorResultType (
	EBinOp OpKind,
	const CValue& RawOpValue1,
	const CValue& RawOpValue2
	)
{
	ASSERT ((size_t) OpKind < EBinOp__Count);

	CValue OpValue1;
	CValue OpValue2;

	PrepareOperandType (RawOpValue1, &OpValue1);
	if (OpValue1.GetType ()->GetTypeKind () == EType_ClassPtr)
	{
		CClassType* pClassType = ((CClassPtrType*) OpValue1.GetType ())->GetTargetType ();
		CFunction* pFunction = pClassType->GetBinaryOperator (OpKind);

		if (pFunction)
		{
			rtl::CBoxListT <CValue> ArgList;
			ArgList.InsertTail (RawOpValue1);
			ArgList.InsertTail (RawOpValue2);
			return GetCallOperatorResultType (pFunction->GetTypeOverload (), &ArgList);
		}
	}

	CBinaryOperator* pOperator = m_BinaryOperatorTable [OpKind];
	ASSERT (pOperator);

	PrepareOperandType (RawOpValue1, &OpValue1, pOperator->GetOpFlags1 ());
	PrepareOperandType (RawOpValue2, &OpValue2, pOperator->GetOpFlags2 ());
	return pOperator->GetResultType (OpValue1, OpValue2);
}

bool
COperatorMgr::GetBinaryOperatorResultType (
	EBinOp OpKind,
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	CType* pResultType = GetBinaryOperatorResultType (OpKind, RawOpValue1, RawOpValue2);
	if (!pResultType)
		return false;

	pResultValue->SetType (pResultType);
	return true;
}

CFunction*
COperatorMgr::GetOverloadedBinaryOperator (
	EBinOp OpKind,
	const CValue& RawOpValue1,
	const CValue& RawOpValue2
	)
{
	CValue OpValue1;
	PrepareOperandType (RawOpValue1, &OpValue1);

	if (OpValue1.GetType ()->GetTypeKind () == EType_ClassPtr)
	{
		CClassPtrType* pPtrType = (CClassPtrType*) OpValue1.GetType ();
		return pPtrType->GetTargetType ()->GetBinaryOperator (OpKind);
	}

	return NULL;
}

bool
COperatorMgr::OverloadedBinaryOperator (
	CFunction* pFunction,
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	if (pFunction->GetFlags () & EMulticastMethodFlag_InaccessibleViaEventPtr)
	{
		CValue OpValue1;
		PrepareOperandType (RawOpValue1, &OpValue1);

		if (OpValue1.GetType ()->GetTypeKind () == EType_ClassPtr &&
			((CClassPtrType*) OpValue1.GetType ())->IsEventPtrType ())
		{
			err::SetFormatStringError ("'%s' is inaccessible via 'event' pointer", GetBinOpKindString (pFunction->GetBinOpKind ()));
			return false;
		}
	}

	rtl::CBoxListT <CValue> ArgList;
	ArgList.InsertTail (RawOpValue1);
	ArgList.InsertTail (RawOpValue2);
	return CallOperator (pFunction, &ArgList, pResultValue);
}

bool
COperatorMgr::BinaryOperator (
	EBinOp OpKind,
	const CValue& RawOpValue1,
	const CValue& RawOpValue2,
	CValue* pResultValue
	)
{
	ASSERT ((size_t) OpKind < EBinOp__Count);

	CFunction* pFunction = GetOverloadedBinaryOperator (OpKind, RawOpValue1, RawOpValue2);
	if (pFunction)
		return OverloadedBinaryOperator (pFunction, RawOpValue1, RawOpValue2, pResultValue);

	CValue OpValue1;
	CValue OpValue2;
	CValue UnusedResultValue;

	if (!pResultValue)
		pResultValue = &UnusedResultValue;

	CBinaryOperator* pOperator = m_BinaryOperatorTable [OpKind];
	ASSERT (pOperator);

	return
		PrepareOperand (RawOpValue1, &OpValue1, pOperator->GetOpFlags1 ()) &&
		PrepareOperand (RawOpValue2, &OpValue2, pOperator->GetOpFlags2 ()) &&
		pOperator->Operator (OpValue1, OpValue2, pResultValue);
}

CType*
COperatorMgr::GetConditionalOperatorResultType (
	const CValue& TrueValue,
	const CValue& FalseValue
	)
{
	CType* pResultType;

	CType* pTrueType = TrueValue.GetType ();
	CType* pFalseType = FalseValue.GetType ();

	if (pTrueType->GetTypeKind () == EType_Array)
		pTrueType = ((CArrayType*) pTrueType)->GetElementType ()->GetDataPtrType ();

	if (pFalseType->GetTypeKind () == EType_Array)
		pFalseType = ((CArrayType*) pFalseType)->GetElementType ()->GetDataPtrType ();

	if (pTrueType->Cmp (pFalseType) == 0)
	{
		pResultType = pTrueType;
	}
	else
	{
		uint_t TrueFlags = EOpFlag_KeepBool | EOpFlag_KeepEnum;
		uint_t FalseFlags = EOpFlag_KeepBool | EOpFlag_KeepEnum;
		
		if (IsArrayRefType (pTrueType))
			TrueFlags |= EOpFlag_ArrayRefToPtr;

		if (IsArrayRefType (pFalseType))
			FalseFlags |= EOpFlag_ArrayRefToPtr;

		pTrueType = PrepareOperandType (pTrueType, TrueFlags);
		pFalseType = PrepareOperandType (pFalseType, FalseFlags);

		pResultType =
			pTrueType->Cmp (pFalseType) == 0 ? pTrueType :
			(pTrueType->GetTypeKindFlags () & pFalseType->GetTypeKindFlags () & ETypeKindFlag_Numeric) ?
				GetArithmeticOperatorResultType (pTrueType, pFalseType) :
				m_pModule->m_OperatorMgr.PrepareOperandType (pTrueType);
	}

	// if it's a lean data pointer, fatten it

	if ((pResultType->GetTypeKindFlags () & ETypeKindFlag_DataPtr) &&
		((CDataPtrType*) pResultType)->GetPtrTypeKind () == EDataPtrType_Lean)
	{
		pResultType = ((CDataPtrType*) pResultType)->GetTargetType ()->GetDataPtrType (
			pResultType->GetTypeKind (),
			EDataPtrType_Normal,
			pResultType->GetFlags ()
			);
	}

	bool Result =
		CheckCastKind (TrueValue, pResultType) &&
		CheckCastKind (FalseValue, pResultType);

	return Result ? pResultType : NULL;
}

bool
COperatorMgr::GetConditionalOperatorResultType (
	const CValue& TrueValue,
	const CValue& FalseValue,
	CValue* pResultValue
	)
{
	CType* pResultType = GetConditionalOperatorResultType (TrueValue, FalseValue);
	if (!pResultType)
		return false;

	pResultValue->SetType (pResultType);
	return true;
}

bool
COperatorMgr::ConditionalOperator (
	const CValue& RawTrueValue,
	const CValue& RawFalseValue,
	CBasicBlock* pThenBlock,
	CBasicBlock* pPhiBlock,
	CValue* pResultValue
	)
{
	bool Result;

	CValue TrueValue;
	CValue FalseValue;

	CType* pResultType = GetConditionalOperatorResultType (RawTrueValue, RawFalseValue);
	if (!pResultType)
		return false;

	Result = CastOperator (RawFalseValue, pResultType, &FalseValue);
	if (!Result)
		return false;

	CBasicBlock* pElseBlock = m_pModule->m_ControlFlowMgr.GetCurrentBlock (); // might have changed

	m_pModule->m_ControlFlowMgr.Jump (pPhiBlock, pThenBlock);

	Result = CastOperator (RawTrueValue, pResultType, &TrueValue);
	if (!Result)
		return false;

	pThenBlock = m_pModule->m_ControlFlowMgr.GetCurrentBlock (); // might have changed

	m_pModule->m_ControlFlowMgr.Follow (pPhiBlock);
	m_pModule->m_LlvmIrBuilder.CreatePhi (TrueValue, pThenBlock, FalseValue, pElseBlock, pResultValue);
	return true;
}

bool
COperatorMgr::CastOperator (
	EStorage StorageKind,
	const CValue& RawOpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	bool Result;

	if (RawOpValue.GetValueKind () == EValue_Null)
	{
		if ((pType->GetTypeKindFlags () & ETypeKindFlag_Ptr) && (pType->GetFlags () & EPtrTypeFlag_Safe))
		{
			SetCastError (RawOpValue, pType);
			return false;
		}

		if (pType->GetTypeKind () == EType_Void)
			pResultValue->SetNull ();
		else
			*pResultValue = pType->GetZeroValue ();
		return true;
	}

	EType TypeKind = pType->GetTypeKind ();
	ASSERT ((size_t) TypeKind < EType__Count);

	CCastOperator* pOperator = m_CastOperatorTable [TypeKind];
	ASSERT (pOperator); // there is always a default

	CValue OpValue;
	CValue UnusedResultValue;

	if (!pResultValue)
		pResultValue = &UnusedResultValue;

	Result = PrepareOperand (RawOpValue, &OpValue, pOperator->GetOpFlags ());
	if (!Result)
		return false;

	if (OpValue.GetType ()->Cmp (pType) == 0) // identity, try to shortcut
	{
		if (OpValue.HasLlvmValue ())
		{
			*pResultValue = OpValue;
			return true;
		}

		if (OpValue.GetValueKind () == EValue_Property)
		{
			ASSERT (pType->GetTypeKind () == EType_PropertyPtr);
			return GetPropertyThinPtr (OpValue.GetProperty (), OpValue.GetClosure (), (CPropertyPtrType*) pType, pResultValue);
		}

		// nope, need to go through full cast
	}

	return pOperator->Cast (StorageKind, OpValue, pType, pResultValue);
}

bool
COperatorMgr::CastOperator (
	EStorage StorageKind,
	const CValue& OpValue,
	EType TypeKind,
	CValue* pResultValue
	)
{
	CType* pType = m_pModule->m_TypeMgr.GetPrimitiveType (TypeKind);
	return CastOperator (StorageKind, OpValue, pType, pResultValue);
}

ECast
COperatorMgr::GetCastKind (
	const CValue& RawOpValue,
	CType* pType
	)
{
	if (RawOpValue.GetValueKind () == EValue_Null)
		return
			(pType->GetTypeKindFlags () & ETypeKindFlag_Ptr) ? ECast_Implicit :
			(pType->GetTypeKindFlags () & ETypeKindFlag_Integer) ? ECast_Explicit : ECast_None;

	EType TypeKind = pType->GetTypeKind ();
	ASSERT ((size_t) TypeKind < EType__Count);

	CCastOperator* pOperator = m_CastOperatorTable [TypeKind];
	ASSERT (pOperator); // there is always a default

	CValue OpValue;
	PrepareOperandType (
		RawOpValue,
		&OpValue,
		pOperator->GetOpFlags ()
		);

	if (OpValue.GetType ()->Cmp (pType) == 0) // identity!
		return ECast_Identitiy;

	return pOperator->GetCastKind (OpValue, pType);
}

ECast
COperatorMgr::GetArgCastKind (
	CFunctionType* pFunctionType,
	CFunctionArg* const* pActualArgArray,
	size_t ActualArgCount
	)
{
	rtl::CArrayT <CFunctionArg*> FormalArgArray = pFunctionType->GetArgArray ();
	size_t FormalArgCount = FormalArgArray.GetCount ();

	if (ActualArgCount > FormalArgCount && !(pFunctionType->GetFlags () & EFunctionTypeFlag_VarArg))
		return ECast_None;

	size_t ArgCount = FormalArgCount;
	while (ActualArgCount < ArgCount)
	{
		if (FormalArgArray [ArgCount - 1]->GetInitializer ().IsEmpty ())
			return ECast_None;

		ArgCount--;
	}

	ECast WorstCastKind = ECast_Identitiy;

	for (size_t i = 0; i < ArgCount; i++)
	{
		CType* pFormalArgType = FormalArgArray [i]->GetType ();
		CType* pActualArgType = pActualArgArray [i]->GetType ();

		ECast CastKind = GetCastKind (pActualArgType, pFormalArgType);
		if (!CastKind)
			return ECast_None;

		if (CastKind < WorstCastKind)
			WorstCastKind = CastKind;
	}

	return WorstCastKind;
}

ECast
COperatorMgr::GetArgCastKind (
	CFunctionType* pFunctionType,
	const rtl::CConstBoxListT <CValue>& ArgList
	)
{
	size_t ActualArgCount = ArgList.GetCount ();

	rtl::CArrayT <CFunctionArg*> FormalArgArray = pFunctionType->GetArgArray ();
	size_t FormalArgCount = FormalArgArray.GetCount ();

	if (ActualArgCount > FormalArgCount && !(pFunctionType->GetFlags () & EFunctionTypeFlag_VarArg))
		return ECast_None;

	size_t ArgCount = FormalArgCount;
	while (ActualArgCount < ArgCount)
	{
		if (FormalArgArray [ArgCount - 1]->GetInitializer ().IsEmpty ())
			return ECast_None;

		ArgCount--;
	}

	ECast WorstCastKind = ECast_Identitiy;

	rtl::CBoxIteratorT <CValue> Arg = ArgList.GetHead ();
	for (size_t i = 0; i < ArgCount; i++, Arg++)
	{
		CType* pFormalArgType = FormalArgArray [i]->GetType ();

		ECast CastKind = GetCastKind (*Arg, pFormalArgType);
		if (!CastKind)
			return ECast_None;

		if (CastKind < WorstCastKind)
			WorstCastKind = CastKind;
	}

	return WorstCastKind;
}

ECast
COperatorMgr::GetFunctionCastKind (
	CFunctionType* pSrcType,
	CFunctionType* pDstType
	)
{
	ECast ArgCastKind = GetArgCastKind (pSrcType, pDstType->GetArgArray ());
	if (!ArgCastKind)
		return ECast_None;

	CType* pSrcReturnType = pSrcType->GetReturnType ();
	CType* pDstReturnType = pDstType->GetReturnType ();

	if (pDstReturnType->GetTypeKind () == EType_Void)
		return ArgCastKind;

	ECast ReturnCastKind = GetCastKind (pSrcReturnType, pDstReturnType);
	return AXL_MIN (ArgCastKind, ReturnCastKind);
}

ECast
COperatorMgr::GetPropertyCastKind (
	CPropertyType* pSrcType,
	CPropertyType* pDstType
	)
{
	ECast CastKind = GetFunctionCastKind (pSrcType->GetGetterType (), pDstType->GetGetterType ());
	if (!CastKind)
		return ECast_None;

	CFunctionTypeOverload* pSrcSetterType = pSrcType->GetSetterType ();
	CFunctionTypeOverload* pDstSetterType = pDstType->GetSetterType ();

	ECast WorstCastKind = CastKind;

	size_t Count = pDstSetterType->GetOverloadCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CFunctionType* pDstOverload = pDstSetterType->GetOverload (i);

		size_t j = pSrcSetterType->ChooseOverload (pDstOverload->GetArgArray (), &CastKind);
		if (j == -1)
			return ECast_None;

		if (CastKind < WorstCastKind)
			WorstCastKind = CastKind;
	}

	return WorstCastKind;
}

bool
COperatorMgr::CheckCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ECast CastKind = GetCastKind (OpValue, pType);
	switch (CastKind)
	{
	case ECast_Explicit:
		err::SetFormatStringError (
			"conversion from '%s' to '%s' requires explicit cast",
			OpValue.GetType ()->GetTypeString ().cc (), // thanks a lot gcc
			pType->GetTypeString ().cc ()
			);
		return false;

	case ECast_None:
		SetCastError (OpValue, pType);
		return false;
	}

	return true;
}

void
COperatorMgr::PrepareOperandType (
	const CValue& OpValue,
	CValue* pOpValue,
	uint_t OpFlags
	)
{
	if (OpValue.IsEmpty ())
	{
		*pOpValue = OpValue;
		return;
	}

	CValue Value = OpValue;

	for (;;)
	{
		CType* pType = Value.GetType ();
		CType* pPrevType = pType;

		EType TypeKind = pType->GetTypeKind ();
		switch (TypeKind)
		{
		case EType_DataRef:
			if (!(OpFlags & EOpFlag_KeepDataRef))
			{
				CDataPtrType* pPtrType = (CDataPtrType*) pType;
				CType* pTargetType = pPtrType->GetTargetType ();
				EType TargetTypeKind = pTargetType->GetTypeKind ();

				if (TargetTypeKind == EType_BitField)
				{
					CBitFieldType* pBitFieldType = (CBitFieldType*) pTargetType;
					Value = pBitFieldType->GetBaseType ();
				}
				else if (TargetTypeKind != EType_Array)
				{
					Value = ((CDataPtrType*) pType)->GetTargetType ();
				}
				else if (OpFlags & EOpFlag_ArrayRefToPtr)
				{
					EDataPtrType PtrTypeKind = pPtrType->GetPtrTypeKind ();

					CArrayType* pArrayType = (CArrayType*) pTargetType;
					Value = pArrayType->GetElementType ()->GetDataPtrType (
						pPtrType->GetAnchorNamespace (),
						EType_DataPtr,
						PtrTypeKind == EDataPtrType_Thin ? EDataPtrType_Thin : EDataPtrType_Lean,
						pPtrType->GetFlags ()
						);
				}
			}

			break;

		case EType_ClassRef:
			if (!(OpFlags & EOpFlag_KeepClassRef))
			{
				CClassPtrType* pPtrType = (CClassPtrType*) pType;
				CClassType* pTargetType = pPtrType->GetTargetType ();
				Value = pTargetType->GetClassPtrType (
					pPtrType->GetAnchorNamespace (),
					EType_ClassPtr,
					pPtrType->GetPtrTypeKind (),
					pPtrType->GetFlags ()
					);
			}

			break;

		case EType_FunctionRef:
			if (!(OpFlags & EOpFlag_KeepFunctionRef))
			{
				CFunctionPtrType* pPtrType = (CFunctionPtrType*) Value.GetClosureAwareType (); // important: take closure into account!
				if (!pPtrType)
					break;

				CFunctionType* pTargetType = pPtrType->GetTargetType ();
				Value = pTargetType->GetFunctionPtrType (pPtrType->GetPtrTypeKind (), pPtrType->GetFlags ());
			}

			break;

		case EType_PropertyRef:
			if (!(OpFlags & EOpFlag_KeepPropertyRef))
			{
				CPropertyPtrType* pPtrType = (CPropertyPtrType*) Value.GetClosureAwareType ();
				if (!pPtrType)
					break;

				CPropertyType* pTargetType = pPtrType->GetTargetType ();
				if (!pTargetType->IsIndexed ())
					Value = pTargetType->GetReturnType ();
			}

			break;

		case EType_Bool:
			if (!(OpFlags & EOpFlag_KeepBool))
				Value = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int8);

			break;

		case EType_Enum:
			if (!(OpFlags & EOpFlag_KeepEnum))
				Value = ((CEnumType*) pType)->GetBaseType ();

			break;
		}

		if (Value.GetType () == pType)
			break;
	}

	*pOpValue = Value;
}

CType*
COperatorMgr::PrepareOperandType (
	const CValue& OpValue,
	uint_t OpFlags
	)
{
	CValue ResultValue;
	PrepareOperandType (OpValue, &ResultValue, OpFlags);
	return ResultValue.GetType ();
}

bool
COperatorMgr::PrepareOperand (
	const CValue& OpValue,
	CValue* pOpValue,
	uint_t OpFlags
	)
{
	bool Result;

	if (OpValue.IsEmpty ())
	{
		*pOpValue = OpValue;
		return true;
	}

	CValue Value = OpValue;
	for (;;)
	{
		CType* pType = Value.GetType ();

		EType TypeKind = pType->GetTypeKind ();
		switch (TypeKind)
		{
		case EType_DataRef:
			if (!(OpFlags & EOpFlag_KeepDataRef))
			{
				CDataPtrType* pPtrType = (CDataPtrType*) pType;
				if (pPtrType->GetTargetType ()->GetTypeKind () != EType_Array)
				{
					Result = LoadDataRef (&Value);
					if (!Result)
						return false;
				}
				else if (OpFlags & EOpFlag_ArrayRefToPtr)
				{
					EDataPtrType PtrTypeKind = pPtrType->GetPtrTypeKind ();

					CArrayType* pArrayType = (CArrayType*) pPtrType->GetTargetType ();
					pType = pArrayType->GetElementType ()->GetDataPtrType (
						pPtrType->GetAnchorNamespace (),
						EType_DataPtr,
						PtrTypeKind == EDataPtrType_Thin ? EDataPtrType_Thin : EDataPtrType_Lean,
						pPtrType->GetFlags ()
						);

					CValue PrevValue = Value;
					m_pModule->m_LlvmIrBuilder.CreateGep2 (Value, 0, pType, &Value);

					if (PtrTypeKind != EDataPtrType_Thin)
					{
						if (PtrTypeKind == EDataPtrType_Normal)
							Value.SetLeanDataPtrValidator (PrevValue);
						else if (PrevValue.GetValueKind () == EValue_Variable) // EDataPtrType_Lean
							Value.SetLeanDataPtrValidator (PrevValue);
						else
							Value.SetLeanDataPtrValidator (PrevValue.GetLeanDataPtrValidator ());
					}
				}
			}

			break;

		case EType_ClassRef:
			if (!(OpFlags & EOpFlag_KeepClassRef))
			{
				CClassPtrType* pPtrType = (CClassPtrType*) pType;
				CClassType* pTargetType = pPtrType->GetTargetType ();
				Value.OverrideType (pTargetType->GetClassPtrType (
					pPtrType->GetAnchorNamespace (),
					EType_ClassPtr,
					pPtrType->GetPtrTypeKind (),
					pPtrType->GetFlags ())
					);
			}

			break;

		case EType_FunctionRef:
			if (!(OpFlags & EOpFlag_KeepFunctionRef))
			{
				CFunctionPtrType* pPtrType = (CFunctionPtrType*) pType;
				CFunctionType* pTargetType = pPtrType->GetTargetType ();
				Value.OverrideType (pTargetType->GetFunctionPtrType (pPtrType->GetPtrTypeKind (), pPtrType->GetFlags ()));
			}

			break;

		case EType_PropertyRef:
			if (!(OpFlags & EOpFlag_KeepPropertyRef))
			{
				CPropertyPtrType* pPtrType = (CPropertyPtrType*) Value.GetClosureAwareType ();
				if (!pPtrType)
					return false;

				CPropertyType* pTargetType = pPtrType->GetTargetType ();
				if (!pTargetType->IsIndexed ())
				{
					Result = GetProperty (Value, &Value);
					if (!Result)
						return false;
				}
			}

			break;

		case EType_Bool:
			if (!(OpFlags & EOpFlag_KeepBool))
			{
				Result = m_CastIntFromBool.Cast (EStorage_Heap, Value, m_pModule->GetSimpleType (EType_Int8), &Value);
				if (!Result)
					return false;
			}

			break;

		case EType_Enum:
			if (!(OpFlags & EOpFlag_KeepEnum))
				Value.OverrideType (((CEnumType*) pType)->GetBaseType ());

			break;
		}

		if (Value.GetType () == pType)
			break;
	}

	*pOpValue = Value;
	return true;
}

//.............................................................................

} // namespace jnc {
