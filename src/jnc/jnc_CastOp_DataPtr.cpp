#include "pch.h"
#include "jnc_CastOp_DataPtr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

ECast
CCast_DataPtr_FromArray::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	if (IsArrayRefType (OpValue.GetType ()))
	{
		CValue PtrValue = m_pModule->m_OperatorMgr.PrepareOperandType (OpValue, EOpFlag_ArrayRefToPtr);
		return m_pModule->m_OperatorMgr.GetCastKind (PtrValue, pType);
	}

	ASSERT (pType->GetTypeKind () == EType_DataPtr);
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_Array);

	CArrayType* pSrcType = (CArrayType*) OpValue.GetType ();	
	CDataPtrType* pDstType = (CDataPtrType*) pType;

	CType* pArrayElementType = pSrcType->GetElementType ();
	CType* pPtrDataType = pDstType->GetTargetType ();

	return
		pArrayElementType->Cmp (pPtrDataType) == 0 ? ECast_Implicit :
		(pArrayElementType->GetFlags () & ETypeFlag_Pod) ?
			pPtrDataType->GetTypeKind () == EType_Void ? ECast_Implicit :
			(pPtrDataType->GetFlags () & ETypeFlag_Pod) ? ECast_Explicit : ECast_None : ECast_None;
}

bool
CCast_DataPtr_FromArray::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_Array);
	ASSERT (pType->GetTypeKind () == EType_DataPtr);

	CArrayType* pSrcType = (CArrayType*) OpValue.GetType ();
	CDataPtrType* pDstType = (CDataPtrType*) pType;

	const CValue& SavedOpValue = m_pModule->m_ConstMgr.SaveValue (OpValue);
	void* p = SavedOpValue.GetConstData ();

	// #pragma AXL_TODO ("create a global constant holding the array")

	if (pDstType->GetPtrTypeKind () == EDataPtrType_Normal)
	{
		TDataPtr* pPtr = (TDataPtr*) pDst;
		pPtr->m_p = p;
		pPtr->m_pRangeBegin = p;
		pPtr->m_pRangeEnd = (char*) p + pSrcType->GetSize ();
		pPtr->m_pObject = GetStaticObjHdr ();
	}
	else // thin or lean
	{
		*(void**) pDst = p;
	}

	return true;
}

bool
CCast_DataPtr_FromArray::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	if (IsArrayRefType (OpValue.GetType ()))
	{
		CValue PtrValue;

		return 
			m_pModule->m_OperatorMgr.PrepareOperand (OpValue, &PtrValue, EOpFlag_ArrayRefToPtr) &&
			m_pModule->m_OperatorMgr.CastOperator (PtrValue, pType, pResultValue);
	}

	err::SetFormatStringError ("casting from array to pointer is currently only implemented for constants");
	return false;
}

//.............................................................................

ECast
CCast_DataPtr_Base::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr && pType->GetTypeKind () == EType_DataPtr);

	CDataPtrType* pSrcType = (CDataPtrType*) OpValue.GetType ();
	CDataPtrType* pDstType = (CDataPtrType*) pType;
	CType* pSrcDataType = pSrcType->GetTargetType ();
	CType* pDstDataType = pDstType->GetTargetType ();

	if (pSrcType->IsConstPtrType () && !pDstType->IsConstPtrType ())
		return ECast_None; // const vs non-const mismatch

#pragma AXL_TODO ("develop safe data pointer casts when non-POD is involved")
#if 0
	if ((pDstDataType->GetFlags () & ETypeFlag_Pod) != (pSrcDataType->GetFlags () & ETypeFlag_Pod))
		return ECast_None; // pod vs non-pod mismatch
#endif

	if (pSrcDataType->Cmp (pDstDataType) == 0 || pDstDataType->GetTypeKind () == EType_Void)
		return ECast_Implicit;

	if (pSrcDataType->GetTypeKind () != EType_Struct)
		return ECast_Explicit;

	return ((CStructType*) pSrcDataType)->FindBaseTypeTraverse (pDstDataType) ?
		ECast_Implicit :
		ECast_Explicit;
}

intptr_t
CCast_DataPtr_Base::GetOffset (
	CDataPtrType* pSrcType,
	CDataPtrType* pDstType,
	CBaseTypeCoord* pCoord
	)
{
	CType* pSrcDataType = pSrcType->GetTargetType ();
	CType* pDstDataType = pDstType->GetTargetType ();

	if (pSrcDataType->Cmp (pDstDataType) == 0 ||
		pSrcDataType->GetTypeKind () != EType_Struct ||
		pDstDataType->GetTypeKind () != EType_Struct)
	{
		return 0;
	}

	CStructType* pSrcStructType = (CStructType*) pSrcDataType;
	CStructType* pDstStructType = (CStructType*) pDstDataType;

	if (pSrcStructType->FindBaseTypeTraverse (pDstStructType, pCoord))
		return pCoord->m_Offset;

	CBaseTypeCoord Coord;
	if (pDstStructType->FindBaseTypeTraverse (pSrcStructType, &Coord))
		return -Coord.m_Offset;

	return 0;
}

intptr_t
CCast_DataPtr_Base::GetOffsetUnsafePtrValue (
	const CValue& PtrValue,
	CDataPtrType* pSrcType,
	CDataPtrType* pDstType,
	CValue* pResultValue
	)
{
	CBaseTypeCoord Coord;
	intptr_t Offset = GetOffset (pSrcType, pDstType, &Coord);

	if (!Coord.m_LlvmIndexArray.IsEmpty ())
	{
		Coord.m_LlvmIndexArray.Insert (0, 0);
		m_pModule->m_LlvmIrBuilder.CreateGep (
			PtrValue,
			Coord.m_LlvmIndexArray,
			Coord.m_LlvmIndexArray.GetCount (),
			pDstType,
			pResultValue
			);

		return Offset;
	}

	if (!Offset)
	{
		m_pModule->m_LlvmIrBuilder.CreateBitCast (PtrValue, pDstType, pResultValue);
		return Offset;
	}

	ASSERT (Offset < 0);

	CValue BytePtrValue;
	m_pModule->m_LlvmIrBuilder.CreateBitCast (PtrValue, m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr), &BytePtrValue);
	m_pModule->m_LlvmIrBuilder.CreateGep (BytePtrValue, (int32_t) Offset, NULL, &BytePtrValue);
	m_pModule->m_LlvmIrBuilder.CreateBitCast (BytePtrValue, pDstType, pResultValue);
	return Offset;
}

//.............................................................................

bool
CCast_DataPtr_Normal2Normal::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr);
	ASSERT (pType->GetTypeKind () == EType_DataPtr);

	intptr_t Offset = GetOffset ((CDataPtrType*) OpValue.GetType (), (CDataPtrType*) pType, NULL);

	TDataPtr* pDstPtr = (TDataPtr*) pDst;
	TDataPtr* pSrcPtr = (TDataPtr*) OpValue.GetConstData ();
	pDstPtr->m_p = (char*) pSrcPtr->m_p + Offset;
	pDstPtr->m_pRangeBegin = pSrcPtr->m_pRangeBegin;
	pDstPtr->m_pRangeEnd = pSrcPtr->m_pRangeEnd;
	pDstPtr->m_pObject = pSrcPtr->m_pObject;
	return true;
}

bool
CCast_DataPtr_Normal2Normal::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr);
	ASSERT (pType->GetTypeKind () == EType_DataPtr);

	if (pType->GetFlags () & EPtrTypeFlag_Safe)
		m_pModule->m_OperatorMgr.CheckDataPtrRange (OpValue);

	CValue PtrValue;
	CValue RangeBeginValue;
	CValue RangeEndValue;
	CValue ScopeLevelValue;

	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 0, NULL, &PtrValue);
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 1, NULL, &RangeBeginValue);
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 2, NULL, &RangeEndValue);
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 3, NULL, &ScopeLevelValue);

	CDataPtrType* pUnsafePtrType = ((CDataPtrType*) pType)->GetTargetType ()->GetDataPtrType_c ();
	GetOffsetUnsafePtrValue (PtrValue, (CDataPtrType*) OpValue.GetType (), pUnsafePtrType, &PtrValue);

	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "create safe data pointer");

	CValue ResultValue = pType->GetUndefValue ();
	m_pModule->m_LlvmIrBuilder.CreateInsertValue (ResultValue, PtrValue, 0, NULL, &ResultValue);
	m_pModule->m_LlvmIrBuilder.CreateInsertValue (ResultValue, RangeBeginValue, 1, NULL, &ResultValue);
	m_pModule->m_LlvmIrBuilder.CreateInsertValue (ResultValue, RangeEndValue, 2, NULL, &ResultValue);
	m_pModule->m_LlvmIrBuilder.CreateInsertValue (ResultValue, ScopeLevelValue, 3, pType, pResultValue);
	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CCast_DataPtr_Lean2Normal::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr);
	ASSERT (pType->GetTypeKind () == EType_DataPtr);

	EDataPtrType SrcPtrTypeKind = ((CDataPtrType*) OpValue.GetType ())->GetPtrTypeKind ();
	ASSERT (SrcPtrTypeKind == EDataPtrType_Lean);

	intptr_t Offset = GetOffset ((CDataPtrType*) OpValue.GetType (), (CDataPtrType*) pType, NULL);

	TDataPtr* pDstPtr = (TDataPtr*) pDst;
	const void* pSrc = OpValue.GetConstData ();

	pDstPtr->m_p = (char*) pSrc + Offset;
	pDstPtr->m_pRangeBegin = NULL;
	pDstPtr->m_pRangeEnd = (void*) -1;
	pDstPtr->m_pObject = GetStaticObjHdr ();
	return true;
}

bool
CCast_DataPtr_Lean2Normal::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr);
	ASSERT (pType->GetTypeKind () == EType_DataPtr);

	EDataPtrType SrcPtrTypeKind = ((CDataPtrType*) OpValue.GetType ())->GetPtrTypeKind ();
	ASSERT (SrcPtrTypeKind == EDataPtrType_Lean);

	CValue PtrValue;
	CDataPtrType* pUnsafePtrType = ((CDataPtrType*) pType)->GetTargetType ()->GetDataPtrType_c ();
	GetOffsetUnsafePtrValue (OpValue, (CDataPtrType*) OpValue.GetType (), pUnsafePtrType, &PtrValue);

	CValue RangeBeginValue;
	CValue RangeEndValue;
	CValue ObjHdrValue;

	m_pModule->m_OperatorMgr.GetLeanDataPtrRange (OpValue, &RangeBeginValue, &RangeEndValue);

	if (pType->GetFlags () & EPtrTypeFlag_Safe)
		m_pModule->m_OperatorMgr.CheckDataPtrRange (OpValue);

	m_pModule->m_OperatorMgr.GetLeanDataPtrObjHdr (OpValue, &ObjHdrValue);

	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "create safe data pointer");

	CValue ResultValue = pType->GetUndefValue ();
	m_pModule->m_LlvmIrBuilder.CreateInsertValue (ResultValue, PtrValue, 0, NULL, &ResultValue);
	m_pModule->m_LlvmIrBuilder.CreateInsertValue (ResultValue, RangeBeginValue, 1, NULL, &ResultValue);
	m_pModule->m_LlvmIrBuilder.CreateInsertValue (ResultValue, RangeEndValue, 2, NULL, &ResultValue);
	m_pModule->m_LlvmIrBuilder.CreateInsertValue (ResultValue, ObjHdrValue, 3, pType, pResultValue);
	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CCast_DataPtr_Normal2Thin::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr);
	ASSERT (pType->GetTypeKind () == EType_DataPtr);

	intptr_t Offset = GetOffset ((CDataPtrType*) OpValue.GetType (), (CDataPtrType*) pType, NULL);
	*(char**) pDst = *(char**) OpValue.GetConstData () + Offset;
	return true;
}

bool
CCast_DataPtr_Normal2Thin::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr);
	ASSERT (pType->GetTypeKind () == EType_DataPtr);

	CValue PtrValue;
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 0, NULL, &PtrValue);
	GetOffsetUnsafePtrValue (PtrValue, (CDataPtrType*) OpValue.GetType (), (CDataPtrType*) pType, pResultValue);
	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CCast_DataPtr_Lean2Thin::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr);
	ASSERT (pType->GetTypeKind () == EType_DataPtr);

	GetOffsetUnsafePtrValue (OpValue, (CDataPtrType*) OpValue.GetType (), (CDataPtrType*) pType, pResultValue);
	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CCast_DataPtr_Thin2Thin::ConstCast (
	const CValue& OpValue,
	CType* pType,
	void* pDst
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKind () == EType_DataPtr);
	ASSERT (pType->GetTypeKind () == EType_DataPtr);

	intptr_t Offset = GetOffset ((CDataPtrType*) OpValue.GetType (), (CDataPtrType*) pType, NULL);
	*(char**) pDst = *(char**) OpValue.GetConstData () + Offset;
	return true;
}

//.............................................................................

CCast_DataPtr::CCast_DataPtr ()
{
	memset (m_OperatorTable, 0, sizeof (m_OperatorTable));

	m_OperatorTable [EDataPtrType_Normal] [EDataPtrType_Normal] = &m_Normal2Normal;
	m_OperatorTable [EDataPtrType_Normal] [EDataPtrType_Thin]   = &m_Normal2Thin;
	m_OperatorTable [EDataPtrType_Lean] [EDataPtrType_Normal]   = &m_Lean2Normal;
	m_OperatorTable [EDataPtrType_Lean] [EDataPtrType_Thin]     = &m_Lean2Thin;
	m_OperatorTable [EDataPtrType_Thin] [EDataPtrType_Thin]     = &m_Thin2Thin;
}

CCastOperator*
CCast_DataPtr::GetCastOperator (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (pType->GetTypeKind () == EType_DataPtr);

	CDataPtrType* pDstPtrType = (CDataPtrType*) pType;
	EDataPtrType DstPtrTypeKind = pDstPtrType->GetPtrTypeKind ();

	CType* pSrcType = OpValue.GetType ();
	EType SrcTypeKind = pSrcType->GetTypeKind ();

	if (IsArrayRefType (pSrcType) || SrcTypeKind == EType_Array)
	{
		return &m_FromArray;
	}
	else if (SrcTypeKind != EType_DataPtr)
	{
		return NULL;
	}

	CDataPtrType* pSrcPtrType = (CDataPtrType*) pSrcType;
	EDataPtrType SrcPtrTypeKind = pSrcPtrType->GetPtrTypeKind ();

	if (DstPtrTypeKind == EDataPtrType_Normal)
	{
#pragma AXL_TODO ("develop safe data pointer casts when non-POD is involved")
#if 0
		if ((pSrcPtrType->GetTargetType ()->GetFlags () & ETypeFlag_Pod) !=
			(pDstPtrType->GetTargetType ()->GetFlags () & ETypeFlag_Pod))
			return NULL;
#endif

		if (pSrcPtrType->IsConstPtrType () && !pDstPtrType->IsConstPtrType ())
			return NULL;
	}

	ASSERT ((size_t) SrcPtrTypeKind < EDataPtrType__Count);
	ASSERT ((size_t) DstPtrTypeKind < EDataPtrType__Count);

	return m_OperatorTable [SrcPtrTypeKind] [DstPtrTypeKind];
}

//.............................................................................

ECast
CCast_DataRef::GetCastKind (
	const CValue& OpValue,
	CType* pType
	)
{
	ASSERT (pType->GetTypeKind () == EType_DataRef);

	CType* pIntermediateSrcType = m_pModule->m_OperatorMgr.GetUnaryOperatorResultType (EUnOp_Addr, OpValue);
	if (!pIntermediateSrcType)
		return ECast_None;

	CDataPtrType* pPtrType = (CDataPtrType*) pType;
	CDataPtrType* pIntermediateDstType = pPtrType->GetTargetType ()->GetDataPtrType (
		EType_DataPtr,
		pPtrType->GetPtrTypeKind (),
		pPtrType->GetFlags ()
		);

	return m_pModule->m_OperatorMgr.GetCastKind (pIntermediateSrcType, pIntermediateDstType);
}

bool
CCast_DataRef::LlvmCast (
	EStorage StorageKind,
	const CValue& OpValue,
	CType* pType,
	CValue* pResultValue
	)
{
	ASSERT (pType->GetTypeKind () == EType_DataRef);

	CDataPtrType* pPtrType = (CDataPtrType*) pType;
	CDataPtrType* pIntermediateType = pPtrType->GetTargetType ()->GetDataPtrType (
		EType_DataPtr,
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
