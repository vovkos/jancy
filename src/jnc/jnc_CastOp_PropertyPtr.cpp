#include "pch.h"
#include "jnc_CastOp_PropertyPtr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

ECast
CCast_PropertyPtr_FromDataPtr::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr && pType->GetTypeKind () == EType_PropertyPtr);

	CDataPtrType* pSrcPtrType = (CDataPtrType*) OpValue.GetType ();
	CPropertyPtrType* pDstPtrType = (CPropertyPtrType*) pType;
	CPropertyType* pPropertyType = pDstPtrType->GetTargetType ();

	return !pPropertyType->IsIndexed ()  ?
		m_pModule->m_OperatorMgr.GetCastKind (pSrcPtrType->GetTargetType (), pPropertyType->GetReturnType ()) :
		ECast_None;
}

bool
CCast_PropertyPtr_FromDataPtr::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr && pType->GetTypeKind () == EType_PropertyPtr);

	CPropertyPtrType* pDstPtrType = (CPropertyPtrType*) pType;

	if (OpValue.GetValueKind () == EValue_Variable &&
		OpValue.GetVariable ()->GetStorageKind () == EStorage_Static &&
		OpValue.GetLlvmValue () == OpValue.GetVariable ()->GetLlvmValue ())
	{
		return LlvmCast_DirectThunk (OpValue.GetVariable (), pDstPtrType, pResultValue);
	}

	if (pDstPtrType->GetPtrTypeKind () == EPropertyPtrType_Thin)
	{
		SetCastError (OpValue, pType);
		return false;
	}

	return LlvmCast_FullClosure (StorageKind, OpValue, pDstPtrType, pResultValue);
}

bool
CCast_PropertyPtr_FromDataPtr::LlvmCast_DirectThunk (
	CVariable* pVariable,
	CPropertyPtrType* pDstPtrType,
	CValue* pResultValue
	)
{
	CProperty* pThunkProperty = m_pModule->m_FunctionMgr.GetDirectDataThunkProperty (
		pVariable,
		pDstPtrType->GetTargetType (),
		pDstPtrType->HasClosure ()
		);

	CValue PropertyValue = pThunkProperty;
	m_pModule->m_OperatorMgr.UnaryOperator (EUnOp_Addr, &PropertyValue);

	CValue ClosureValue;

	if (pDstPtrType->HasClosure ())
	{
		ClosureValue = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr)->GetZeroValue ();
		PropertyValue.InsertToClosureHead (ClosureValue);
	}

	return m_pModule->m_OperatorMgr.CastOperator (PropertyValue, pDstPtrType, pResultValue);
}

bool
CCast_PropertyPtr_FromDataPtr::LlvmCast_FullClosure (
	EStorage StorageKind,
	const CValue& OpValue,
	CPropertyPtrType* pDstPtrType,
	CValue* pResultValue
	)
{
	CValue ClosureValue;
	bool Result = m_pModule->m_OperatorMgr.CreateDataClosureObject (
		StorageKind,
		OpValue,
		pDstPtrType->GetTargetType (),
		&ClosureValue
		);

	if (!Result)
		return false;

	ASSERT (IsClassPtrType (ClosureValue.GetType (), EClassType_PropertyClosure));

	CPropertyClosureClassType* pClosureType = (CPropertyClosureClassType*) ((CClassPtrType*) ClosureValue.GetType ())->GetTargetType ();
	m_pModule->m_LlvmIrBuilder.CreateClosurePropertyPtr (pClosureType->GetThunkProperty (), ClosureValue, pDstPtrType, pResultValue);
	return true;
}

//.............................................................................

ECast
CCast_PropertyPtr_Base::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_PropertyPtr && pType->GetTypeKind () == EType_PropertyPtr);

	CPropertyPtrType* pSrcPtrType = (CPropertyPtrType*) OpValue.GetClosureAwareType ();
	CPropertyPtrType* pDstPtrType = (CPropertyPtrType*) pType;

	if (!pSrcPtrType)
		return ECast_None;

	if (pSrcPtrType->IsConstPtrType () && !pDstPtrType->IsConstPtrType ())
		return ECast_None;

	return m_pModule->m_OperatorMgr.GetPropertyCastKind (
		pSrcPtrType->GetTargetType (),
		pDstPtrType->GetTargetType ()
		);
}

//.............................................................................

bool
CCast_PropertyPtr_FromFat::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_PropertyPtr && pType->GetTypeKind () == EType_PropertyPtr);

	CPropertyPtrType* pSrcPtrType = (CPropertyPtrType*) OpValue.GetType ();
	CPropertyType* pSrcPropertyType = pSrcPtrType->GetTargetType ();

	CPropertyPtrType* pThinPtrType = pSrcPropertyType->GetStdObjectMemberPropertyType ()->GetPropertyPtrType (EPropertyPtrType_Thin);

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
CCast_PropertyPtr_Thin2Fat::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_PropertyPtr && pType->GetTypeKind () == EType_PropertyPtr);

	CPropertyPtrType* pSrcPtrType = (CPropertyPtrType*) OpValue.GetType ();
	CPropertyPtrType* pDstPtrType = (CPropertyPtrType*) pType;

	CPropertyType* pSrcPropertyType = pSrcPtrType->GetTargetType ();
	CPropertyType* pDstPropertyType = pDstPtrType->GetTargetType ();

	CClosure* pClosure = OpValue.GetClosure ();

	CValue SimpleClosureObjValue;

	bool IsSimpleClosure = pClosure && pClosure->IsSimpleClosure ();
	if (IsSimpleClosure)
		SimpleClosureObjValue = *pClosure->GetArgValueList ()->GetHead ();

	// case 1: no conversion required, no closure object needs to be created

	if (IsSimpleClosure &&
		pSrcPropertyType->IsMemberPropertyType () &&
		pSrcPropertyType->GetShortType ()->Cmp (pDstPropertyType) == 0)
	{
		return LlvmCast_NoThunkSimpleClosure (
			OpValue,
			SimpleClosureObjValue,
			pSrcPropertyType,
			pDstPtrType,
			pResultValue
			);
	}

	if (OpValue.GetValueKind () == EValue_Property)
	{
		CProperty* pProperty = OpValue.GetProperty ();

		// case 2.1: conversion is required, but no closure object needs to be created (closure arg is null)

		if (!pClosure)
			return LlvmCast_DirectThunkNoClosure (
				pProperty,
				pDstPtrType,
				pResultValue
				);

		// case 2.2: same as above, but simple closure is passed as closure arg

		if (IsSimpleClosure && pProperty->GetType ()->IsMemberPropertyType ())
			return LlvmCast_DirectThunkSimpleClosure (
				pProperty,
				SimpleClosureObjValue,
				pDstPtrType,
				pResultValue
				);
	}

	// case 3: closure object needs to be created (so conversion is required even if Property signatures match)

	return LlvmCast_FullClosure (
		StorageKind,
		OpValue,
		pSrcPropertyType,
		pDstPtrType,
		pResultValue
		);
}

bool
CCast_PropertyPtr_Thin2Fat::LlvmCast_NoThunkSimpleClosure (
	const CValue& OpValue,
	const CValue& SimpleClosureObjValue,
	CPropertyType* pSrcPropertyType,
	CPropertyPtrType* pDstPtrType,
	CValue* pResultValue
	)
{
	CType* pThisArgType = pSrcPropertyType->GetThisArgType ();

	CValue ThisArgValue;
	bool Result = m_pModule->m_OperatorMgr.CastOperator (SimpleClosureObjValue, pThisArgType, &ThisArgValue);
	if (!Result)
		return false;

	if (OpValue.GetValueKind () == EValue_Property)
		return CreateClosurePropertyPtr (OpValue.GetProperty (), ThisArgValue, pDstPtrType, pResultValue);

	m_pModule->m_LlvmIrBuilder.CreateClosurePropertyPtr (OpValue, ThisArgValue, pDstPtrType, pResultValue);
	return true;
}

bool
CCast_PropertyPtr_Thin2Fat::LlvmCast_DirectThunkNoClosure (
	CProperty* pProperty,
	CPropertyPtrType* pDstPtrType,
	CValue* pResultValue
	)
{
	CProperty* pThunkProperty = m_pModule->m_FunctionMgr.GetDirectThunkProperty (
		pProperty,
		((CPropertyPtrType*) pDstPtrType)->GetTargetType (),
		true
		);

	CValue NullValue = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr)->GetZeroValue ();

	return CreateClosurePropertyPtr (pThunkProperty, NullValue, pDstPtrType, pResultValue);
}

bool
CCast_PropertyPtr_Thin2Fat::LlvmCast_DirectThunkSimpleClosure (
	CProperty* pProperty,
	const CValue& SimpleClosureObjValue,
	CPropertyPtrType* pDstPtrType,
	CValue* pResultValue
	)
{
	CType* pThisArgType = pProperty->GetType ()->GetThisArgType ();
	CNamedType* pThisTargetType = pProperty->GetType ()->GetThisTargetType ();

	CValue ThisArgValue;
	bool Result = m_pModule->m_OperatorMgr.CastOperator (SimpleClosureObjValue, pThisArgType, &ThisArgValue);
	if (!Result)
		return false;

	CProperty* pThunkProperty = m_pModule->m_FunctionMgr.GetDirectThunkProperty (
		pProperty,
		m_pModule->m_TypeMgr.GetMemberPropertyType (pThisTargetType, pDstPtrType->GetTargetType ())
		);

	return CreateClosurePropertyPtr (pThunkProperty, ThisArgValue, pDstPtrType, pResultValue);
}

bool
CCast_PropertyPtr_Thin2Fat::LlvmCast_FullClosure (
	EStorage StorageKind,
	const CValue& OpValue,
	CPropertyType* pSrcPropertyType,
	CPropertyPtrType* pDstPtrType,
	CValue* pResultValue
	)
{
	CValue ClosureValue;
	bool Result = m_pModule->m_OperatorMgr.CreateClosureObject (
		StorageKind,
		OpValue,
		pDstPtrType->GetTargetType (),
		pDstPtrType->GetPtrTypeKind () == EPropertyPtrType_Weak,
		&ClosureValue
		);

	if (!Result)
		return false;

	ASSERT (IsClassPtrType (ClosureValue.GetType (), EClassType_PropertyClosure));

	CPropertyClosureClassType* pClosureType = (CPropertyClosureClassType*) ((CClassPtrType*) ClosureValue.GetType ())->GetTargetType ();
	return CreateClosurePropertyPtr (pClosureType->GetThunkProperty (), ClosureValue, pDstPtrType, pResultValue);
}

bool
CCast_PropertyPtr_Thin2Fat::CreateClosurePropertyPtr (
	CProperty* pProperty,
	const CValue& ClosureValue,
	CPropertyPtrType* pPtrType,
	CValue* pResultValue
	)
{
	CValue ThinPtrValue;
	bool Result = m_pModule->m_OperatorMgr.GetPropertyThinPtr (pProperty, NULL, &ThinPtrValue);
	if (!Result)
		return false;

	m_pModule->m_LlvmIrBuilder.CreateClosurePropertyPtr (ThinPtrValue, ClosureValue, pPtrType, pResultValue);
	return true;
}

//.............................................................................

bool
CCast_PropertyPtr_Weak2Normal::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_PropertyPtr);
	ASSERT (pType->GetTypeKind () == EType_PropertyPtr && ((CPropertyPtrType*) pType)->GetPtrTypeKind () == EPropertyPtrType_Normal);

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

	CPropertyPtrType* pIntermediateType = ((CPropertyPtrType*) OpValue.GetType ())->GetUnWeakPtrType ();
	IntermediateValue.OverrideType (pIntermediateType);
	return m_pModule->m_OperatorMgr.CastOperator (IntermediateValue, pType, pResultValue);}

//.............................................................................

bool
CCast_PropertyPtr_Thin2Thin::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_PropertyPtr);
	ASSERT (pType->GetTypeKind () == EType_PropertyPtr);

	if (OpValue.GetClosure ())
	{
		err::SetFormatStringError ("cannot create thin property pointer to a closure");
		return false;
	}

	if (OpValue.GetValueKind () != EValue_Property)
	{
		err::SetFormatStringError ("can only create thin pointer thunk to a property, not a property pointer");
		return false;
	}

	CPropertyPtrType* pPtrType = (CPropertyPtrType*) pType;
	CPropertyType* pTargetType = pPtrType->GetTargetType ();
	CProperty* pProperty = OpValue.GetProperty ();

	if (pProperty->GetType ()->Cmp (pTargetType) == 0)
		return m_pModule->m_OperatorMgr.GetPropertyThinPtr (pProperty, NULL, pPtrType, pResultValue);

	if (pProperty->GetFlags () & EPropertyTypeFlag_Bindable)
	{
		err::SetFormatStringError ("bindable properties are not supported yet");
		return false;
	}

	CProperty* pThunkProperty = m_pModule->m_FunctionMgr.GetDirectThunkProperty (pProperty, pTargetType);
	return m_pModule->m_OperatorMgr.GetPropertyThinPtr (pThunkProperty, NULL, pPtrType, pResultValue);
}

//.............................................................................
/*
bool
CCast_PropertyPtr_Thin2Weak::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	#pragma AXL_TODO ("will only work for simple closures. redesign Thin2Normal to support 'weak'")

	ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_PropertyPtr);
	ASSERT (pType->GetTypeKind () == EType_PropertyPtr);

	if (OpValue.GetClosure () && !OpValue.GetClosure ()->IsSimpleClosure ())
	{
		err::SetFormatStringError ("full weak closures are not implemented yet");
		return false;
	}

	CPropertyPtrType* pIntermediateType = ((CPropertyPtrType*) pType)->GetTargetType ()->GetPropertyPtrType (EPropertyPtrType_Normal);
	bool Result = m_pModule->m_OperatorMgr.CastOperator (StorageKind, OpValue, pIntermediateType, pResultValue);
	if (!Result)
		return false;

	pResultValue->OverrideType (pType);
	return true;
}
*/

//.............................................................................

CCast_PropertyPtr::CCast_PropertyPtr ()
{
	memset (m_OperatorTable, 0, sizeof (m_OperatorTable));

	m_OperatorTable [EPropertyPtrType_Normal] [EPropertyPtrType_Normal] = &m_FromFat;
	m_OperatorTable [EPropertyPtrType_Normal] [EPropertyPtrType_Weak]   = &m_FromFat;
	m_OperatorTable [EPropertyPtrType_Weak] [EPropertyPtrType_Normal]   = &m_Weak2Normal;
	m_OperatorTable [EPropertyPtrType_Weak] [EPropertyPtrType_Weak]     = &m_FromFat;
	m_OperatorTable [EPropertyPtrType_Thin] [EPropertyPtrType_Normal]   = &m_Thin2Fat;
	m_OperatorTable [EPropertyPtrType_Thin] [EPropertyPtrType_Weak]     = &m_Thin2Fat;
	m_OperatorTable [EPropertyPtrType_Thin] [EPropertyPtrType_Thin]     = &m_Thin2Thin;
}

CCastOperator*
CCast_PropertyPtr::GetCastOperator (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (pType->GetTypeKind () == EType_PropertyPtr);

	CPropertyPtrType* pDstPtrType = (CPropertyPtrType*) pType;
	EPropertyPtrType DstPtrTypeKind = pDstPtrType->GetPtrTypeKind ();
	ASSERT ((size_t) DstPtrTypeKind < EPropertyPtrType__Count);

	EType SrcTypeKind = OpValue.GetType ()->GetTypeKind ();
	switch (SrcTypeKind)
	{
	case EType_DataPtr:
		return &m_FromDataPtr;

	case EType_PropertyPtr:
		{
		EPropertyPtrType SrcPtrTypeKind = ((CPropertyPtrType*) OpValue.GetType ())->GetPtrTypeKind ();
		ASSERT ((size_t) SrcPtrTypeKind < EPropertyPtrType__Count);

		return m_OperatorTable [SrcPtrTypeKind] [DstPtrTypeKind];
		}

	default:
		return NULL;
	};
}

//.............................................................................

ECast
CCast_PropertyRef::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (pType->GetTypeKind () == EType_PropertyRef);

	CType* pIntermediateSrcType = m_pModule->m_OperatorMgr.GetUnaryOperatorResultType (EUnOp_Addr, OpValue);
	if (!pIntermediateSrcType)
		return ECast_None;

	CPropertyPtrType* pPtrType = (CPropertyPtrType*) pType;
	CPropertyPtrType* pIntermediateDstType = pPtrType->GetTargetType ()->GetPropertyPtrType (
		EType_PropertyPtr,
		pPtrType->GetPtrTypeKind (),
		pPtrType->GetFlags ()
		);

	return m_pModule->m_OperatorMgr.GetCastKind (pIntermediateSrcType, pIntermediateDstType);
}

bool
CCast_PropertyRef::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (pType->GetTypeKind () == EType_PropertyRef);

	CPropertyPtrType* pPtrType = (CPropertyPtrType*) pType;
	CPropertyPtrType* pIntermediateType = pPtrType->GetTargetType ()->GetPropertyPtrType (
		EType_PropertyPtr,
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
