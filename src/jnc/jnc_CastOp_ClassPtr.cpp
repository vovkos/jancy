#include "pch.h"
#include "jnc_CastOp_ClassPtr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
IsMulticastToMulticast (
	CClassPtrType* pSrcType,
	CClassPtrType* pDstType
	)
{
	if (pSrcType->GetTargetType ()->GetClassTypeKind () != EClassType_Multicast ||
		pDstType->GetTargetType ()->GetClassTypeKind () != EClassType_Multicast)
		return false;

	// event -> multicast is never ok

	if ((pSrcType->GetFlags () & EPtrTypeFlag_Event) && !(pDstType->GetFlags () & EPtrTypeFlag_Event))
		return false;

	CMulticastClassType* pSrcMcType = (CMulticastClassType*) pSrcType->GetTargetType ();
	CMulticastClassType* pDstMcType = (CMulticastClassType*) pDstType->GetTargetType ();

	return pSrcMcType->GetTargetType ()->Cmp (pDstMcType->GetTargetType ()) == 0;
}

//.............................................................................

ECast
CCast_ClassPtr::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (pType->GetTypeKind () == EType_ClassPtr);

	if (OpValue.GetType ()->GetTypeKind () != EType_ClassPtr)
		return ECast_None; // TODO: user conversions later via constructors

	CClassPtrType* pSrcType = (CClassPtrType*) OpValue.GetType ();
	CClassPtrType* pDstType = (CClassPtrType*) pType;

	if (pSrcType->IsConstPtrType () && !pDstType->IsConstPtrType ()) 
		return ECast_None; // const vs non-const mismatch

	CClassType* pSrcClassType = pSrcType->GetTargetType ();
	CClassType* pDstClassType = pDstType->GetTargetType ();

	return 
		(pDstClassType->GetClassTypeKind () == EClassType_StdObject) ||	
		pSrcClassType->Cmp (pDstClassType) == 0 || 
		IsMulticastToMulticast (pSrcType, pDstType) ||
		pSrcClassType->FindBaseTypeTraverse (pDstClassType) ? 
		ECast_Implicit : 
		ECast_Explicit;
}

bool
CCast_ClassPtr::LlvmCast (
	EStorage StorageKind,
	const CValue& RawOpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (pType->GetTypeKind () == EType_ClassPtr);

	bool Result;

	if (RawOpValue.GetType ()->GetTypeKind () != EType_ClassPtr)
	{
		SetCastError (RawOpValue, pType);
		return false; // TODO: user conversions via constructors -- only if target ptr is EPtrTypeFlag_Const
	}

	CValue OpValue = RawOpValue;

	CClassPtrType* pSrcType = (CClassPtrType*) RawOpValue.GetType ();
	CClassPtrType* pDstType = (CClassPtrType*) pType;

	if (pSrcType->GetPtrTypeKind () == EClassPtrType_Weak &&
		pDstType->GetPtrTypeKind () != EClassPtrType_Weak)
	{
		CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "strengthen class pointer");

		CFunction* pStrengthen = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_StrengthenClassPtr);

		m_pModule->m_LlvmIrBuilder.CreateBitCast (OpValue, m_pModule->GetSimpleType (EStdType_ObjectPtr), &OpValue);
		m_pModule->m_LlvmIrBuilder.CreateCall (
			pStrengthen,
			pStrengthen->GetType (),
			OpValue,
			&OpValue
			);

		m_pModule->m_LlvmIrBuilder.CreateBitCast (OpValue, pSrcType, &OpValue);
	}

	CClassType* pSrcClassType = pSrcType->GetTargetType ();
	CClassType* pDstClassType = pDstType->GetTargetType ();

	if (pDstType->GetFlags () & EPtrTypeFlag_Checked)
		m_pModule->m_OperatorMgr.CheckClassPtrNull (OpValue);

	if (pDstClassType->GetClassTypeKind () == EClassType_StdObject ||
		IsMulticastToMulticast (pSrcType, pDstType))
	{
		m_pModule->m_LlvmIrBuilder.CreateBitCast (OpValue, pDstType, pResultValue);
		return true;
	}

	if (pSrcClassType->Cmp (pDstClassType) == 0)
	{
		pResultValue->OverrideType (OpValue, pType);
		return true;
	}

	CBaseTypeCoord Coord;
	Result = pSrcClassType->FindBaseTypeTraverse (pDstClassType, &Coord);
	if (!Result)
	{
		CValue PtrValue;
		m_pModule->m_LlvmIrBuilder.CreateBitCast (OpValue, m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr), &PtrValue);

		CValue TypeValue (&pDstClassType, m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr));

		CFunction* pDynamicCastClassPtr = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_DynamicCastClassPtr);
		m_pModule->m_LlvmIrBuilder.CreateCall2 (
			pDynamicCastClassPtr,
			pDynamicCastClassPtr->GetType (),
			PtrValue,
			TypeValue,
			&PtrValue
			);

		m_pModule->m_LlvmIrBuilder.CreateBitCast (PtrValue, pDstType, pResultValue);
		return true;
	}

	CValue SrcNullValue = pSrcType->GetZeroValue ();
	CValue DstNullValue = pDstType->GetZeroValue ();

	CBasicBlock* pCmpBlock = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	CBasicBlock* pPhiBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("iface_phi");
	CBasicBlock* pNoNullBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("iface_nonull");

	CValue CmpValue;
	Result = 
		m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Eq, OpValue, SrcNullValue, &CmpValue) &&
		m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pPhiBlock, pNoNullBlock, pNoNullBlock);

	if (!Result)
		return false;
	
	Coord.m_LlvmIndexArray.Insert (0, 0);

	CValue PtrValue;
	m_pModule->m_LlvmIrBuilder.CreateGep (
		OpValue, 
		Coord.m_LlvmIndexArray,
		Coord.m_LlvmIndexArray.GetCount (),
		NULL, 
		&PtrValue
		);		

	m_pModule->m_ControlFlowMgr.Follow (pPhiBlock);

	m_pModule->m_LlvmIrBuilder.CreatePhi (PtrValue, pNoNullBlock, DstNullValue, pCmpBlock, pResultValue);
	pResultValue->OverrideType (pDstType);
	return true;
}

//.............................................................................

} // namespace jnc {
