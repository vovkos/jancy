#include "pch.h"
#include "jnc_CastOp_FunctionPtr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

ECast
CCast_FunctionPtr_FromMulticast::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (IsClassPtrType (OpValue.GetType (), EClassType_Multicast));
	ASSERT (pType->GetTypeKind () == EType_FunctionPtr);

	if (OpValue.GetType ()->GetFlags () & EPtrTypeFlag_Event)
		return ECast_None;

	CMulticastClassType* pMcType = (CMulticastClassType*) ((CClassPtrType*) OpValue.GetType ())->GetTargetType ();
	return m_pModule->m_OperatorMgr.GetCastKind (pMcType->GetTargetType (), pType);
}

bool
CCast_FunctionPtr_FromMulticast::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (IsClassPtrType (OpValue.GetType (), EClassType_Multicast));
	ASSERT (pType->GetTypeKind () == EType_FunctionPtr);

	CValue CallValue;

	return
		m_pModule->m_OperatorMgr.MemberOperator (OpValue, "call", &CallValue) &&
		m_pModule->m_OperatorMgr.CastOperator (CallValue, pType, pResultValue);
}

//.............................................................................

ECast
CCast_FunctionPtr_Base::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr);
	ASSERT (pType->GetTypeKind () == EType_FunctionPtr);

	if (!OpValue.GetType ())
	{
		ASSERT (OpValue.GetValueKind () == EValue_Function && OpValue.GetFunction ()->IsOverloaded ());
		return ECast_None; // choosing overload is not yet implemented
	}

	CFunctionPtrType* pSrcPtrType = (CFunctionPtrType*) OpValue.GetClosureAwareType ();
	CFunctionPtrType* pDstPtrType = (CFunctionPtrType*) pType;

	if (!pSrcPtrType)
		return ECast_None;

	return m_pModule->m_OperatorMgr.GetFunctionCastKind (
		pSrcPtrType->GetTargetType (),
		pDstPtrType->GetTargetType ()
		);
}

//.............................................................................

bool
CCast_FunctionPtr_FromFat::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr);
	ASSERT (pType->GetTypeKind () == EType_FunctionPtr);

	CFunctionPtrType* pSrcPtrType = (CFunctionPtrType*) OpValue.GetType ();
	CFunctionType* pSrcFunctionType = pSrcPtrType->GetTargetType ();

	CFunctionPtrType* pThinPtrType = pSrcFunctionType->GetStdObjectMemberMethodType ()->GetFunctionPtrType (EFunctionPtrType_Thin);

	CValue PfnValue;
	CValue ClosureObjValue;
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 0, pThinPtrType, &PfnValue);
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 1, m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr), &ClosureObjValue);

	PfnValue.SetClosure (OpValue.GetClosure ());
	PfnValue.InsertToClosureHead (ClosureObjValue);

	return m_pModule->m_OperatorMgr.CastOperator (StorageKind, PfnValue, pType, pResultValue);
}

//.............................................................................

bool
CCast_FunctionPtr_Weak2Normal::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr);
	ASSERT (pType->GetTypeKind () == EType_FunctionPtr && ((CFunctionPtrType*) pType)->GetPtrTypeKind () == EFunctionPtrType_Normal);

	CBasicBlock* pInitialBlock = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	CBasicBlock* pStrengthenBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("strengthen");
	CBasicBlock* pAliveBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("alive");
	CBasicBlock* pDeadBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("dead");
	CBasicBlock* pPhiBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("phi");

	CType* pClosureType = m_pModule->GetSimpleType (EStdType_ObjectPtr);
	CValue NullClosureValue = pClosureType->GetZeroValue ();

	CValue ClosureValue;
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 1, pClosureType, &ClosureValue);

	CValue CmpValue;
	m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Ne, ClosureValue, NullClosureValue, &CmpValue);
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pStrengthenBlock, pPhiBlock);

	CFunction* pStrengthenFunction = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_StrengthenClassPtr);

	CValue StrengthenedClosureValue;
	m_pModule->m_LlvmIrBuilder.CreateCall (
		pStrengthenFunction,
		pStrengthenFunction->GetType (),
		ClosureValue,
		&StrengthenedClosureValue
		);

	m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Ne, StrengthenedClosureValue, NullClosureValue, &CmpValue);
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pAliveBlock, pDeadBlock);
	m_pModule->m_ControlFlowMgr.Follow (pPhiBlock);

	m_pModule->m_ControlFlowMgr.SetCurrentBlock (pDeadBlock);
	m_pModule->m_ControlFlowMgr.Follow (pPhiBlock);

	CValue ValueArray [3] =
	{
		OpValue,
		OpValue,
		OpValue.GetType ()->GetZeroValue ()
	};

	CBasicBlock* BlockArray [3] =
	{
		pInitialBlock,
		pAliveBlock,
		pDeadBlock
	};

	CValue IntermediateValue;
	m_pModule->m_LlvmIrBuilder.CreatePhi (ValueArray, BlockArray, 3, &IntermediateValue);

	CFunctionPtrType* pIntermediateType = ((CFunctionPtrType*) OpValue.GetType ())->GetUnWeakPtrType ();
	IntermediateValue.OverrideType (pIntermediateType);
	return m_pModule->m_OperatorMgr.CastOperator (IntermediateValue, pType, pResultValue);
}

//.............................................................................

bool
CCast_FunctionPtr_Thin2Fat::LlvmCast (
	EStorage StorageKind,
	const CValue& RawOpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (RawOpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr);
	ASSERT (pType->GetTypeKind () == EType_FunctionPtr);

	CValue OpValue = RawOpValue;

	CFunctionPtrType* pSrcPtrType = (CFunctionPtrType*) OpValue.GetType ();
	CFunctionPtrType* pDstPtrType = (CFunctionPtrType*) pType;

	CFunctionType* pSrcFunctionType = pSrcPtrType->GetTargetType ();
	CFunctionType* pDstFunctionType = pDstPtrType->GetTargetType ();

	CClosure* pClosure = OpValue.GetClosure ();

	CValue SimpleClosureValue;

	bool IsSimpleClosure = pClosure && pClosure->IsSimpleClosure ();
	if (IsSimpleClosure)
		SimpleClosureValue = *pClosure->GetArgValueList ()->GetHead ();

	if (OpValue.GetValueKind () == EValue_Function && OpValue.GetFunction ()->IsVirtual ())
	{
		bool Result = m_pModule->m_OperatorMgr.GetVirtualMethod (OpValue.GetFunction (), pClosure, &OpValue);
		if (!Result)
			return false;
	}

	// case 1: no conversion required, no closure object needs to be created

	if (IsSimpleClosure &&
		pSrcFunctionType->IsMemberMethodType () &&
		pSrcFunctionType->GetShortType ()->Cmp (pDstFunctionType) == 0)
	{
		return LlvmCast_NoThunkSimpleClosure (
			OpValue,
			SimpleClosureValue,
			pSrcFunctionType,
			pDstPtrType,
			pResultValue
			);
	}

	if (OpValue.GetValueKind () == EValue_Function)
	{
		CFunction* pFunction = OpValue.GetFunction ();
		ASSERT (!pFunction->IsVirtual ());

		// case 2.1: conversion is required, but no closure object needs to be created (closure arg is null)

		if (!pClosure)
			return LlvmCast_DirectThunkNoClosure (
				pFunction,
				pDstPtrType,
				pResultValue
				);

		// case 2.2: same as above, but simple closure is passed as closure arg

		if (IsSimpleClosure && pFunction->GetType ()->IsMemberMethodType ())
			return LlvmCast_DirectThunkSimpleClosure (
				pFunction,
				SimpleClosureValue,
				pDstPtrType,
				pResultValue
				);
	}

	// case 3: closure object needs to be created (so conversion is required even if function signatures match)

	return LlvmCast_FullClosure (
		StorageKind,
		OpValue,
		pSrcFunctionType,
		pDstPtrType,
		pResultValue
		);
}

bool
CCast_FunctionPtr_Thin2Fat::LlvmCast_NoThunkSimpleClosure (
	const CValue& OpValue,
	const CValue& SimpleClosureObjValue,
	CFunctionType* pSrcFunctionType,
	CFunctionPtrType* pDstPtrType,
	CValue* pResultValue
	)
{
	CType* pThisArgType = pSrcFunctionType->GetThisArgType ();

	CValue ThisArgValue;
	bool Result = m_pModule->m_OperatorMgr.CastOperator (SimpleClosureObjValue, pThisArgType, &ThisArgValue);
	if (!Result)
		return false;

	m_pModule->m_LlvmIrBuilder.CreateClosureFunctionPtr (OpValue, ThisArgValue, pDstPtrType, pResultValue);
	return true;
}

bool
CCast_FunctionPtr_Thin2Fat::LlvmCast_DirectThunkNoClosure (
	CFunction* pFunction,
	CFunctionPtrType* pDstPtrType,
	CValue* pResultValue
	)
{
	CFunction* pThunkFunction = m_pModule->m_FunctionMgr.GetDirectThunkFunction (
		pFunction,
		((CFunctionPtrType*) pDstPtrType)->GetTargetType (),
		true
		);

	CValue NullValue = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr)->GetZeroValue ();
	m_pModule->m_LlvmIrBuilder.CreateClosureFunctionPtr (pThunkFunction, NullValue, pDstPtrType, pResultValue);
	return true;
}

bool
CCast_FunctionPtr_Thin2Fat::LlvmCast_DirectThunkSimpleClosure (
	CFunction* pFunction,
	const CValue& SimpleClosureObjValue,
	CFunctionPtrType* pDstPtrType,
	CValue* pResultValue
	)
{
	CType* pThisArgType = pFunction->GetType ()->GetThisArgType ();
	CNamedType* pThisTargetType = pFunction->GetType ()->GetThisTargetType ();

	CValue ThisArgValue;
	bool Result = m_pModule->m_OperatorMgr.CastOperator (SimpleClosureObjValue, pThisArgType, &ThisArgValue);
	if (!Result)
		return false;

	CFunction* pThunkFunction = m_pModule->m_FunctionMgr.GetDirectThunkFunction (
		pFunction,
		m_pModule->m_TypeMgr.GetMemberMethodType (pThisTargetType, pDstPtrType->GetTargetType ())
		);

	m_pModule->m_LlvmIrBuilder.CreateClosureFunctionPtr (pThunkFunction, ThisArgValue, pDstPtrType, pResultValue);
	return true;
}

bool
CCast_FunctionPtr_Thin2Fat::LlvmCast_FullClosure (
	EStorage StorageKind,
	const CValue& OpValue,
	CFunctionType* pSrcFunctionType,
	CFunctionPtrType* pDstPtrType,
	CValue* pResultValue
	)
{
	CValue ClosureValue;
	bool Result = m_pModule->m_OperatorMgr.CreateClosureObject (
		StorageKind,
		OpValue,
		pDstPtrType->GetTargetType (),
		pDstPtrType->GetPtrTypeKind () == EFunctionPtrType_Weak,
		&ClosureValue
		);

	if (!Result)
		return false;

	ASSERT (IsClassPtrType (ClosureValue.GetType (), EClassType_FunctionClosure));

	CFunctionClosureClassType* pClosureType = (CFunctionClosureClassType*) ((CClassPtrType*) ClosureValue.GetType ())->GetTargetType ();
	m_pModule->m_LlvmIrBuilder.CreateClosureFunctionPtr (pClosureType->GetThunkFunction (), ClosureValue, pDstPtrType, pResultValue);
	return true;
}

//.............................................................................

bool
CCast_FunctionPtr_Thin2Thin::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr);
	ASSERT (pType->GetTypeKind () == EType_FunctionPtr);

	if (OpValue.GetClosure ())
	{
		err::SetFormatStringError ("cannot create thin function pointer to a closure");
		return false;
	}

	CFunctionPtrType* pSrcPtrType = (CFunctionPtrType*) OpValue.GetType ();
	CFunctionPtrType* pDstPtrType = (CFunctionPtrType*) pType;

	if (pSrcPtrType->GetTargetType ()->Cmp (pDstPtrType->GetTargetType ()) == 0)
	{
		pResultValue->OverrideType (OpValue, pType);
		return true;
	}

	if (OpValue.GetValueKind () != EValue_Function)
	{
		err::SetFormatStringError ("can only create thin pointer thunk to a function, not a function pointer");
		return false;
	}

	CFunction* pThunkFunction = m_pModule->m_FunctionMgr.GetDirectThunkFunction (
		OpValue.GetFunction (),
		pDstPtrType->GetTargetType ()
		);

	pResultValue->SetFunction (pThunkFunction);
	pResultValue->OverrideType (pType);
	return true;
}

//.............................................................................

CCast_FunctionPtr::CCast_FunctionPtr ()
{
	memset (m_OperatorTable, 0, sizeof (m_OperatorTable));

	m_OperatorTable [EFunctionPtrType_Normal] [EFunctionPtrType_Normal] = &m_FromFat;
	m_OperatorTable [EFunctionPtrType_Normal] [EFunctionPtrType_Weak]   = &m_FromFat;
	m_OperatorTable [EFunctionPtrType_Weak] [EFunctionPtrType_Normal]   = &m_Weak2Normal;
	m_OperatorTable [EFunctionPtrType_Weak] [EFunctionPtrType_Weak]     = &m_FromFat;
	m_OperatorTable [EFunctionPtrType_Thin] [EFunctionPtrType_Normal]   = &m_Thin2Fat;
	m_OperatorTable [EFunctionPtrType_Thin] [EFunctionPtrType_Weak]     = &m_Thin2Fat;
	m_OperatorTable [EFunctionPtrType_Thin] [EFunctionPtrType_Thin]     = &m_Thin2Thin;
}

CCastOperator*
CCast_FunctionPtr::GetCastOperator (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (pType->GetTypeKind () == EType_FunctionPtr);

	CFunctionPtrType* pDstPtrType = (CFunctionPtrType*) pType;
	EFunctionPtrType DstPtrTypeKind = pDstPtrType->GetPtrTypeKind ();

	CType* pSrcType = OpValue.GetType ();
	if (!pSrcType)
	{
		ASSERT (OpValue.GetValueKind () == EValue_Function && OpValue.GetFunction ()->IsOverloaded ());
		ASSERT (DstPtrTypeKind >= 0 && DstPtrTypeKind < 2);

		return m_OperatorTable [EFunctionPtrType_Thin] [DstPtrTypeKind];
	}

	if (IsClassPtrType (pSrcType, EClassType_Multicast))
	{
		return &m_FromMulticast;
	}
	else if (!(pSrcType->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr))
	{
		return NULL;
	}

	CFunctionPtrType* pSrcPtrType = (CFunctionPtrType*) pSrcType;
	EFunctionPtrType SrcPtrTypeKind = pSrcPtrType->GetPtrTypeKind ();

	ASSERT ((size_t) SrcPtrTypeKind < EFunctionPtrType__Count);
	ASSERT ((size_t) DstPtrTypeKind < EFunctionPtrType__Count);

	return m_OperatorTable [SrcPtrTypeKind] [DstPtrTypeKind];
}

//.............................................................................

ECast
CCast_FunctionRef::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (pType->GetTypeKind () == EType_FunctionRef);

	CType* pIntermediateSrcType = m_pModule->m_OperatorMgr.GetUnaryOperatorResultType (EUnOp_Addr, OpValue);
	if (!pIntermediateSrcType)
		return ECast_None;

	CFunctionPtrType* pPtrType = (CFunctionPtrType*) pType;
	CFunctionPtrType* pIntermediateDstType = pPtrType->GetTargetType ()->GetFunctionPtrType (
		EType_FunctionPtr,
		pPtrType->GetPtrTypeKind (),
		pPtrType->GetFlags ()
		);

	return m_pModule->m_OperatorMgr.GetCastKind (pIntermediateSrcType, pIntermediateDstType);
}

bool
CCast_FunctionRef::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (pType->GetTypeKind () == EType_FunctionRef);

	CFunctionPtrType* pPtrType = (CFunctionPtrType*) pType;
	CFunctionPtrType* pIntermediateType = pPtrType->GetTargetType ()->GetFunctionPtrType (
		EType_FunctionPtr,
		pPtrType->GetPtrTypeKind (),
		pPtrType->GetFlags ()
		);

	CValue IntermediateValue;

	return
		m_pModule->m_OperatorMgr.UnaryOperator (EUnOp_Addr, OpValue, &IntermediateValue) &&
		m_pModule->m_OperatorMgr.CastOperator (&IntermediateValue, pIntermediateType) &&
		m_pModule->m_OperatorMgr.UnaryOperator (EUnOp_Indir, IntermediateValue, pResultValue);
}

//.............................................................................

} // namespace jnc {
