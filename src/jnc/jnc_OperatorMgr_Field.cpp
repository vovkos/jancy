#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
COperatorMgr::GetField (
	const CValue& OpValue,
	CStructField* pField,
	CMemberCoord* pCoord,
	CValue* pResultValue
	)
{
	CType* pType = OpValue.GetType ();

	if (pType->GetTypeKindFlags () & ETypeKindFlag_DataPtr)
		pType = ((CDataPtrType*) pType)->GetTargetType ();
	else if (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_ClassPtr)
		pType = ((CClassPtrType*) OpValue.GetType ())->GetTargetType ();

	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_Struct:
		return GetStructField (OpValue, pField, pCoord,	pResultValue);

	case EType_Union:
		return pCoord ?
			GetStructField (OpValue, pField, pCoord, pResultValue) :
			GetUnionField (OpValue, pField, pResultValue);

	case EType_Class:
		return GetClassField (OpValue, pField, pCoord, pResultValue);

	default:
		err::SetFormatStringError ("cannot get a field '%s' of '%s'", pField->GetName ().cc (), pType->GetTypeString ().cc ());
		return false;
	}
}

bool
COperatorMgr::GetFieldPtrImpl (
	const CValue& OpValueRaw,
	CMemberCoord* pCoord,
	CType* pResultType,
	CValue* pResultValue
	)
{
	#pragma AXL_TODO ("double check multiple levels of nested unnamed structs/unions")

	if (pCoord->m_UnionCoordArray.IsEmpty ())
	{
		m_pModule->m_LlvmIrBuilder.CreateGep (
			OpValueRaw,
			pCoord->m_LlvmIndexArray,
			pCoord->m_LlvmIndexArray.GetCount (),
			pResultType,
			pResultValue
			);

		return true;
	}

	// if LLVM were to support unions natively, the following would be not needed

	CValue OpValue = OpValueRaw;

	int32_t* pLlvmIndex = pCoord->m_LlvmIndexArray;
	int32_t* pLlvmIndexEnd = pLlvmIndex + pCoord->m_LlvmIndexArray.GetCount ();
	intptr_t UnionLevel = -1; // take into account initial 0 in LlvmIndexArray

	size_t UnionCount = pCoord->m_UnionCoordArray.GetCount ();
	TUnionCoord* pUnionCoord = pCoord->m_UnionCoordArray;
	for (size_t i = 0; i < UnionCount; i++, pUnionCoord++)
	{
		ASSERT (pUnionCoord->m_Level >= UnionLevel);
		size_t LlvmIndexDelta = pUnionCoord->m_Level - UnionLevel;

		if (LlvmIndexDelta)
		{
			m_pModule->m_LlvmIrBuilder.CreateGep (
				OpValue,
				pLlvmIndex,
				LlvmIndexDelta,
				NULL,
				&OpValue
				);
		}

		CStructField* pField = pUnionCoord->m_pType->GetFieldByIndex (pLlvmIndex [LlvmIndexDelta]);
		CType* pType = pField->GetType ()->GetDataPtrType_c ();

		m_pModule->m_LlvmIrBuilder.CreateBitCast (OpValue, pType, &OpValue);

		pLlvmIndex += LlvmIndexDelta + 1;
		UnionLevel = pUnionCoord->m_Level + 1;
	}

	if (pLlvmIndexEnd > pLlvmIndex)
	{
		ASSERT (pLlvmIndex > pCoord->m_LlvmIndexArray);
		
		pLlvmIndex--;
		*pLlvmIndex = 0; // create initial 0

		m_pModule->m_LlvmIrBuilder.CreateGep (
			OpValue,
			pLlvmIndex,
			pLlvmIndexEnd - pLlvmIndex,
			pResultType,
			pResultValue
			);
	}
	else
	{
		pResultValue->OverrideType (OpValue, pResultType);
	}

	return true;
}

bool
COperatorMgr::GetStructField (
	const CValue& OpValue,
	CStructField* pField,
	CMemberCoord* pCoord,
	CValue* pResultValue
	)
{
	CMemberCoord Coord;
	if (!pCoord)
		pCoord = &Coord;

	pCoord->m_LlvmIndexArray.Append (pField->GetLlvmIndex ());
	pCoord->m_Offset += pField->GetOffset ();

	EValue OpValueKind = OpValue.GetValueKind ();
	if (OpValueKind == EValue_Const)
	{
		pResultValue->CreateConst (
			(char*) OpValue.GetConstData () + pCoord->m_Offset,
			pField->GetType ()
			);

		return true;
	}

	if (!(OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_DataPtr))
	{
		if (!pCoord->m_UnionCoordArray.IsEmpty ())
		{
			err::SetFormatStringError ("union member operator on registers is not implemented yet");
			return false;
		}

		m_pModule->m_LlvmIrBuilder.CreateExtractValue (
			OpValue,
			pCoord->m_LlvmIndexArray,
			pCoord->m_LlvmIndexArray.GetCount (),
			pField->GetType (),
			pResultValue
			);

		return true;
	}

	CDataPtrType* pOpType = (CDataPtrType*) OpValue.GetType ();
	pCoord->m_LlvmIndexArray.Insert (0, 0);

	uint_t PtrTypeFlags = pOpType->GetFlags () | pField->GetPtrTypeFlags ();
	if (pField->GetStorageKind () == EStorage_Mutable)
		PtrTypeFlags &= ~EPtrTypeFlag_Const;

	EDataPtrType PtrTypeKind = pOpType->GetPtrTypeKind ();

	CDataPtrType* pPtrType = pField->GetType ()->GetDataPtrType (
		pField->GetParentNamespace (),
		EType_DataRef,
		PtrTypeKind == EDataPtrType_Thin ? EDataPtrType_Thin : EDataPtrType_Lean,
		PtrTypeFlags
		);

	if (PtrTypeKind == EDataPtrType_Thin)
	{
		GetFieldPtrImpl (OpValue, pCoord, pPtrType, pResultValue);
	}
	else if (PtrTypeKind == EDataPtrType_Lean)
	{
		GetFieldPtrImpl (OpValue, pCoord, pPtrType, pResultValue);

		if (OpValue.GetValueKind () == EValue_Variable)
			pResultValue->SetLeanDataPtrValidator (OpValue);
		else
			pResultValue->SetLeanDataPtrValidator (OpValue.GetLeanDataPtrValidator ());
	}
	else
	{
		CValue PtrValue;
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 0, NULL, &PtrValue);
		GetFieldPtrImpl (PtrValue, pCoord, pPtrType, pResultValue);
		pResultValue->SetLeanDataPtrValidator (OpValue);
	}

	return true;
}

bool
COperatorMgr::GetUnionField (
	const CValue& OpValue,
	CStructField* pField,
	CValue* pResultValue
	)
{
	EValue OpValueKind = OpValue.GetValueKind ();
	if (OpValueKind == EValue_Const)
	{
		pResultValue->CreateConst (OpValue.GetConstData (), pField->GetType ());
		return true;
	}

	if (OpValue.GetType ()->GetTypeKind () != EType_DataRef)
	{
		err::SetFormatStringError ("union member operator on registers is not implemented yet");
		return false;
	}

	CDataPtrType* pOpType = (CDataPtrType*) OpValue.GetType ();

	uint_t PtrTypeFlags = pOpType->GetFlags () | pField->GetPtrTypeFlags ();
	if (pField->GetStorageKind () == EStorage_Mutable)
		PtrTypeFlags &= ~EPtrTypeFlag_Const;

	EDataPtrType PtrTypeKind = pOpType->GetPtrTypeKind ();

	CDataPtrType* pPtrType = pField->GetType ()->GetDataPtrType (
		pField->GetParentNamespace (),
		EType_DataRef,
		PtrTypeKind == EDataPtrType_Thin ? EDataPtrType_Thin : EDataPtrType_Lean,
		PtrTypeFlags
		);

	if (PtrTypeKind == EDataPtrType_Thin)
	{
		m_pModule->m_LlvmIrBuilder.CreateBitCast (OpValue, pPtrType, pResultValue);
	}
	else if (PtrTypeKind == EDataPtrType_Lean)
	{
		m_pModule->m_LlvmIrBuilder.CreateBitCast (OpValue, pPtrType, pResultValue);

		if (OpValue.GetValueKind () == EValue_Variable)
			pResultValue->SetLeanDataPtrValidator (OpValue);
		else
			pResultValue->SetLeanDataPtrValidator (OpValue.GetLeanDataPtrValidator ());
	}
	else
	{
		CValue PtrValue;
		m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 0, NULL, &PtrValue);
		m_pModule->m_LlvmIrBuilder.CreateBitCast (OpValue, pField->GetType ()->GetDataPtrType_c (), &PtrValue);

		pResultValue->SetLeanDataPtr (
			PtrValue.GetLlvmValue (),
			pPtrType,
			OpValue
			);
	}

	return true;
}

bool
COperatorMgr::GetClassField (
	const CValue& RawOpValue,
	CStructField* pField,
	CMemberCoord* pCoord,
	CValue* pResultValue
	)
{
	CValue OpValue;
	bool Result = PrepareOperand (RawOpValue, &OpValue);
	if (!Result)
		return false;

	ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_ClassPtr);
	CClassPtrType* pOpType = (CClassPtrType*) OpValue.GetType ();

	CheckClassPtrNull (OpValue);

	CClassType* pClassType = (CClassType*) pField->GetParentNamespace ();

	CMemberCoord Coord;
	if (!pCoord)
		pCoord = &Coord;

	pCoord->m_LlvmIndexArray.Insert (0, 0);
	pCoord->m_LlvmIndexArray.Append (pField->GetLlvmIndex ());

	if (pField->GetType ()->GetTypeKind () == EType_Class)
		pCoord->m_LlvmIndexArray.Append (1);

	CValue PtrValue;
	m_pModule->m_LlvmIrBuilder.CreateGep (
		OpValue,
		pCoord->m_LlvmIndexArray,
		pCoord->m_LlvmIndexArray.GetCount (),
		NULL,
		&PtrValue
		);

	uint_t PtrTypeFlags = pOpType->GetFlags () | pField->GetPtrTypeFlags () | EPtrTypeFlag_Safe;
	if (pField->GetStorageKind () == EStorage_Mutable)
		PtrTypeFlags &= ~EPtrTypeFlag_Const;

	// TODO: handle dual types
	// (PtrTypeFlags & EPtrTypeFlag_ReadOnly) && m_pModule->m_NamespaceMgr.GetAccessKind (pCoord->m_pType) == EAccess_Public)

	if (pField->GetType ()->GetTypeKind () == EType_Class)
	{
		CClassPtrType* pPtrType = ((CClassType*) pField->GetType ())->GetClassPtrType (
			pField->GetParentNamespace (),
			EType_ClassRef,
			EClassPtrType_Normal,
			PtrTypeFlags
			);

		pResultValue->SetLlvmValue (PtrValue.GetLlvmValue (), pPtrType);
	}
	else
	{
		CDataPtrType* pPtrType = pField->GetType ()->GetDataPtrType (
			pField->GetParentNamespace (),
			EType_DataRef,
			EDataPtrType_Lean,
			PtrTypeFlags
			);

		pResultValue->SetLeanDataPtr (
			PtrValue.GetLlvmValue (),
			pPtrType,
			OpValue,
			PtrValue,
			pField->GetType ()->GetSize ()
			);
	}

	return true;
}

bool
COperatorMgr::GetPropertyField (
	const CValue& OpValue,
	CModuleItem* pMember,
	CValue* pResultValue
	)
{
	EModuleItem ItemKind = pMember->GetItemKind ();

	switch (ItemKind)
	{
	case EModuleItem_StructField:
		break;

	case EModuleItem_Variable:
		pResultValue->SetVariable ((CVariable*) pMember);
		return true;

	case EModuleItem_Alias:
		return EvaluateAlias (
			pMember->GetItemDecl (),
			OpValue.GetClosure (),
			((CAlias*) pMember)->GetInitializer (),
			pResultValue
			);

	default:
		ASSERT (false);
	}

	ASSERT (pMember->GetItemKind () == EModuleItem_StructField);
	CClosure* pClosure = OpValue.GetClosure ();
	ASSERT (pClosure);

	CValue ParentValue = *pClosure->GetArgValueList ()->GetHead ();
	return GetField (ParentValue, (CStructField*) pMember, pResultValue);
}

//.............................................................................

} // namespace jnc {
