#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
COperatorMgr::GetNamespaceMemberType (
	CNamespace* pNamespace,
	const char* pName,
	CValue* pResultValue
	)
{
	CMemberCoord Coord;
	CModuleItem* pItem = pNamespace->FindItemTraverse (pName, &Coord, ETraverse_NoParentNamespace);
	if (!pItem)
	{
		err::SetFormatStringError ("'%s' is not a member of '%s'", pName, pNamespace->GetQualifiedName ().cc ());
		return false;
	}

	CModuleItemDecl* pDecl = pItem->GetItemDecl ();
	ASSERT (pDecl);

	if (pDecl->GetAccessKind () != EAccess_Public &&
		m_pModule->m_NamespaceMgr.GetAccessKind (pNamespace) == EAccess_Public)
	{
		err::SetFormatStringError ("'%s.%s' is protected", pNamespace->GetQualifiedName ().cc (), pName);
		return false;
	}

	bool Result = true;

	EModuleItem ItemKind = pItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Namespace:
		pResultValue->SetNamespace ((CGlobalNamespace*) pItem);
		break;

	case EModuleItem_Typedef:
		pItem = ((CTypedef*) pItem)->GetType ();
		// and fall through

	case EModuleItem_Type:
		if (!(((CType*) pItem)->GetTypeKindFlags () & ETypeKindFlag_Named))
		{
			err::SetFormatStringError ("'%s' cannot be used as expression", ((CType*) pItem)->GetTypeString ().cc ());
			return false;
		}

		pResultValue->SetNamespace ((CNamedType*) pItem);
		break;

	case EModuleItem_Alias:
		pResultValue->SetType (((CAlias*) pItem)->GetType ());
		break;

	case EModuleItem_Variable:
		pResultValue->SetType (((CVariable*) pItem)->GetType ()->GetDataPtrType (EType_DataRef, EDataPtrType_Lean));
		break;

	case EModuleItem_Function:
		pResultValue->SetFunctionTypeOverload (((CFunction*) pItem)->GetTypeOverload ());

		if (((CFunction*) pItem)->IsMember ())
			Result = false;

		break;

	case EModuleItem_Property:
		pResultValue->SetType (((CProperty*) pItem)->GetType ()->GetPropertyPtrType (EType_PropertyRef, EPropertyPtrType_Thin));

		if (((CProperty*) pItem)->IsMember ())
			Result = false;

		break;

	case EModuleItem_EnumConst:
		pResultValue->SetType (((CEnumConst*) pItem)->GetParentEnumType ());
		break;

	case EModuleItem_StructField:
		pResultValue->SetField ((CStructField*) pItem, Coord.m_Offset);
		break;

	default:
		Result = false;
	};

	if (!Result)
	{
		err::SetFormatStringError ("'%s.%s' cannot be used as expression", pNamespace->GetQualifiedName ().cc (), pName);
		return false;
	}

	return true;
}

bool
COperatorMgr::GetNamespaceMember (
	CNamespace* pNamespace,
	const char* pName,
	CValue* pResultValue
	)
{
	CModuleItem* pItem = pNamespace->FindItemTraverse (pName, NULL, ETraverse_NoParentNamespace);
	if (!pItem)
	{
		err::SetFormatStringError ("'%s' is not a member of '%s'", pName, pNamespace->GetQualifiedName ().cc ());
		return false;
	}

	CModuleItemDecl* pDecl = pItem->GetItemDecl ();
	ASSERT (pDecl);

	if (pDecl->GetAccessKind () != EAccess_Public &&
		m_pModule->m_NamespaceMgr.GetAccessKind (pNamespace) == EAccess_Public)
	{
		err::SetFormatStringError ("'%s.%s' is protected", pNamespace->GetQualifiedName ().cc (), pName);
		return false;
	}

	bool Result = true;

	EModuleItem ItemKind = pItem->GetItemKind ();
	switch (ItemKind)
	{
	case EModuleItem_Namespace:
		pResultValue->SetNamespace ((CGlobalNamespace*) pItem);
		break;

	case EModuleItem_Typedef:
		pItem = ((CTypedef*) pItem)->GetType ();
		// and fall through

	case EModuleItem_Type:
		if (!(((CType*) pItem)->GetTypeKindFlags () & ETypeKindFlag_Named))
		{
			err::SetFormatStringError ("'%s' cannot be used as expression", ((CType*) pItem)->GetTypeString ().cc ());
			return false;
		}

		pResultValue->SetNamespace ((CNamedType*) pItem);
		break;

	case EModuleItem_Alias:
		return EvaluateAlias (
			pItem->GetItemDecl ()->GetParentUnit (),
			((CAlias*) pItem)->GetInitializer (),
			pResultValue
			);

	case EModuleItem_Variable:
		pResultValue->SetVariable ((CVariable*) pItem);
		break;

	case EModuleItem_Function:
		pResultValue->SetFunction ((CFunction*) pItem);

		if (((CFunction*) pItem)->IsMember ())
			Result = false;

		break;

	case EModuleItem_Property:
		pResultValue->SetProperty ((CProperty*) pItem);

		if (((CProperty*) pItem)->IsMember ())
			Result = false;

		break;

	case EModuleItem_EnumConst:
		Result = ((CEnumConst*) pItem)->GetParentEnumType ()->EnsureLayout ();
		if (!Result)
			return false;

		pResultValue->SetConstInt64 (
			((CEnumConst*) pItem)->GetValue (),
			((CEnumConst*) pItem)->GetParentEnumType ()
			);
		break;

	default:
		Result = false;
	};

	if (!Result)
	{
		err::SetFormatStringError ("'%s.%s' cannot be used as expression", pNamespace->GetQualifiedName ().cc (), pName);
		return false;
	}

	return true;
}

bool
COperatorMgr::GetNamedTypeMemberType (
	const CValue& OpValue,
	CNamedType* pNamedType,
	const char* pName,
	CValue* pResultValue
	)
{
	CMemberCoord Coord;
	CModuleItem* pMember = pNamedType->FindItemTraverse (pName, &Coord, ETraverse_NoParentNamespace);
	if (!pMember)
	{
		err::SetFormatStringError ("'%s' is not a member of '%s'", pName, pNamedType->GetTypeString ().cc ());
		return false;
	}

	EModuleItem MemberKind = pMember->GetItemKind ();
	switch (MemberKind)
	{
	case EModuleItem_StructField:
		{
		CStructField* pField = (CStructField*) pMember;
		size_t BaseOffset = 0;
		if (OpValue.GetValueKind () == EValue_Field)
			BaseOffset = OpValue.GetFieldOffset ();

		if (!(OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_DataPtr))
		{
			pResultValue->SetField (pField, BaseOffset); 
			break;
		}

		CDataPtrType* pPtrType = (CDataPtrType*) OpValue.GetType ();
		EDataPtrType PtrTypeKind = pPtrType->GetPtrTypeKind ();

		CType* pResultType = pField->GetType ()->GetDataPtrType (
			EType_DataRef,
			PtrTypeKind == EDataPtrType_Thin ? EDataPtrType_Thin : EDataPtrType_Lean,
			OpValue.GetType ()->GetFlags ()
			);

		pResultValue->SetField (pField, pResultType, BaseOffset); 
		break;
		}

	case EModuleItem_Function:
		pResultValue->SetType (((CFunction*) pMember)->GetType ()->GetShortType ()->GetFunctionPtrType (
			EType_FunctionRef,
			EFunctionPtrType_Thin
			));
		break;

	case EModuleItem_Property:
		pResultValue->SetType (((CProperty*) pMember)->GetType ()->GetShortType ()->GetPropertyPtrType (
			EType_PropertyRef,
			EPropertyPtrType_Thin
			));
		break;

	default:
		err::SetFormatStringError ("invalid member kind '%s'", GetModuleItemKindString (MemberKind));
		return false;
	}

	return true;
}

bool
COperatorMgr::GetNamedTypeMember (
	const CValue& OpValue,
	CNamedType* pNamedType,
	const char* pName,
	CValue* pResultValue
	)
{
	CMemberCoord Coord;
	CModuleItem* pMember = pNamedType->FindItemTraverse (pName, &Coord, ETraverse_NoParentNamespace);
	if (!pMember)
	{
		err::SetFormatStringError ("'%s' is not a member of '%s'", pName, pNamedType->GetTypeString ().cc ());
		return false;
	}

	CModuleItemDecl* pDecl = pMember->GetItemDecl ();
	ASSERT (pDecl);

	if (pDecl->GetAccessKind () != EAccess_Public &&
		m_pModule->m_NamespaceMgr.GetAccessKind (Coord.m_pType) == EAccess_Public)
	{
		err::SetFormatStringError ("'%s.%s' is protected", Coord.m_pType->GetQualifiedName ().cc (), pName);
		return false;
	}

	EModuleItem MemberKind = pMember->GetItemKind ();
	switch (MemberKind)
	{
	case EModuleItem_StructField:
		return GetField (OpValue, (CStructField*) pMember, &Coord, pResultValue);

	case EModuleItem_Function:
		pResultValue->SetFunction ((CFunction*) pMember);
		break;

	case EModuleItem_Property:
		pResultValue->SetProperty ((CProperty*) pMember);
		break;

	default:
		err::SetFormatStringError ("invalid class member kind");
		return false;
	}

	if (pDecl->GetStorageKind () == EStorage_Static)
		return true;

	#pragma AXL_TODO ("remove explicit addr operator and instead allow implicit cast named_type& -> named_type*")

	CValue ThisArgValue = OpValue;
	if (pNamedType->GetTypeKind () != EType_Class)
	{
		bool Result = UnaryOperator (EUnOp_Addr, &ThisArgValue);
		if (!Result)
			return false;
	}

	if (IsClassType (pNamedType, EClassType_Multicast))
	{
		ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_ClassPtr);
		if ((pMember->GetFlags () & EMulticastMethodFlag_InaccessibleViaEventPtr) &&
			((CClassPtrType*) OpValue.GetType ())->IsEventPtrType ())
		{
			err::SetFormatStringError ("'%s' is inaccessible via 'event' pointer", pName);
			return false;
		}
	}

	pResultValue->InsertToClosureHead (ThisArgValue);
	return true;
}

bool
COperatorMgr::GetMemberOperatorResultType (
	const CValue& RawOpValue,
	const char* pName,
	CValue* pResultValue
	)
{
	if (RawOpValue.GetValueKind () == EValue_Namespace)
		return GetNamespaceMemberType (RawOpValue.GetNamespace (), pName, pResultValue);

	CValue OpValue;
	PrepareOperandType (RawOpValue, &OpValue, EOpFlag_KeepDataRef);

	CType* pType = OpValue.GetType ();
	if (pType->GetTypeKind () == EType_DataRef)
		pType = ((CDataPtrType*) pType)->GetTargetType ();

	if (pType->GetTypeKind () == EType_DataPtr)
	{
		pType = ((CDataPtrType*) pType)->GetTargetType ();

		bool Result = GetUnaryOperatorResultType (EUnOp_Indir, &OpValue);
		if (!Result)
			return false;
	}

	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_Struct:
	case EType_Union:
		return GetNamedTypeMemberType (OpValue, (CNamedType*) pType, pName, pResultValue);

	case EType_ClassPtr:
		PrepareOperandType (&OpValue);
		return GetNamedTypeMemberType (OpValue, ((CClassPtrType*) pType)->GetTargetType (), pName, pResultValue);

	default:
		err::SetFormatStringError ("member operator cannot be applied to '%s'", pType->GetTypeString ().cc ());
		return false;
	}
}

bool
COperatorMgr::MemberOperator (
	const CValue& RawOpValue,
	size_t Index,
	CValue* pResultValue
	)
{
	CValue OpValue;
	bool Result = PrepareOperand (RawOpValue, &OpValue, EOpFlag_KeepDataRef | EOpFlag_KeepClassRef);
	if (!Result)
		return false;

	CType* pType = OpValue.GetType ();
	EType TypeKind = pType->GetTypeKind ();

	CStructField* pField;

	switch (TypeKind)
	{
	case EType_DataRef:
		pType = ((CDataPtrType*) pType)->GetTargetType ();
		TypeKind = pType->GetTypeKind ();
		switch (TypeKind)
		{
		case EType_Array:
			return BinaryOperator (EBinOp_Idx, OpValue, CValue (Index, EType_SizeT), pResultValue);

		case EType_Struct:
			pField = ((CStructType*) pType)->GetFieldByIndex (Index);
			return pField && GetStructField (OpValue, pField, NULL, pResultValue);

		case EType_Union:
			pField = ((CUnionType*) pType)->GetFieldByIndex (Index);
			return pField && GetUnionField (OpValue, pField, pResultValue);

		default:
			err::SetFormatStringError ("indexed member operator cannot be applied to '%s'", pType->GetTypeString ().cc ());
			return false;
		}

	case EType_ClassRef:
		pType = ((CClassPtrType*) pType)->GetTargetType ();
		pField = ((CClassType*) pType)->GetFieldByIndex (Index);
		return pField && GetClassField (OpValue, pField, NULL, pResultValue);

	default:
		err::SetFormatStringError ("indexed member operator cannot be applied to '%s'", pType->GetTypeString ().cc ());
		return false;
	}
}

bool
COperatorMgr::MemberOperator (
	const CValue& RawOpValue,
	const char* pName,
	CValue* pResultValue
	)
{
	if (RawOpValue.GetValueKind () == EValue_Namespace)
		return GetNamespaceMember (RawOpValue.GetNamespace (), pName, pResultValue);

	CValue OpValue;
	bool Result = PrepareOperand (RawOpValue, &OpValue, EOpFlag_KeepDataRef);
	if (!Result)
		return false;

	CType* pType = OpValue.GetType ();

	if (pType->GetTypeKind () == EType_DataRef)
		pType = ((CDataPtrType*) pType)->GetTargetType ();

	if (pType->GetTypeKind () == EType_DataPtr)
	{
		pType = ((CDataPtrType*) pType)->GetTargetType ();

		Result = UnaryOperator (EUnOp_Indir, &OpValue);
		if (!Result)
			return false;
	}

	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_Struct:
	case EType_Union:
		return GetNamedTypeMember (OpValue, (CNamedType*) pType, pName, pResultValue);

	case EType_ClassPtr:
		return
			PrepareOperand (&OpValue) &&
			GetNamedTypeMember (OpValue, ((CClassPtrType*) pType)->GetTargetType (), pName, pResultValue);

	default:
		err::SetFormatStringError ("member operator cannot be applied to '%s'", pType->GetTypeString ().cc ());
		return false;
	}
}

CClassPtrType*
COperatorMgr::GetWeakenOperatorResultType (const CValue& OpValue)
{
	CType* pOpType = PrepareOperandType (OpValue);
	if (pOpType->GetTypeKind () != EType_ClassPtr)
	{
		err::SetFormatStringError ("'weak member' operator cannot be applied to '%s'", pOpType->GetTypeString ().cc ());
		return NULL;
	}

	CClassPtrType* pResultType = ((CClassPtrType*) pOpType)->GetWeakPtrType ();
	return pResultType;
}

bool
COperatorMgr::GetWeakenOperatorResultType (
	const CValue& OpValue,
	CValue* pResultValue
	)
{
	CType* pType = GetWeakenOperatorResultType (OpValue);
	if (!pType)
		return false;

	pResultValue->SetType (pType);
	return true;
}

bool
COperatorMgr::WeakenOperator (
	const CValue& RawOpValue,
	CValue* pResultValue
	)
{
	CValue OpValue;
	bool Result = PrepareOperand (RawOpValue, &OpValue);
	if (!Result)
		return false;

	CType* pOpType = OpValue.GetType ();
	if (pOpType->GetTypeKind () != EType_ClassPtr)
	{
		err::SetFormatStringError ("'weak member' operator cannot be applied to '%s'", pOpType->GetTypeString ().cc ());
		return false;
	}

	CClassPtrType* pResultType = ((CClassPtrType*) pOpType)->GetWeakPtrType ();
	pResultValue->OverrideType (OpValue, pResultType);
	return true;
}

bool
COperatorMgr::GetOffsetOf (
	const CValue& Value,
	CValue* pResultValue
	)
{
	if (Value.GetValueKind () != EValue_Field)
	{
		err::SetFormatStringError ("'offsetof' can only be applied to fields");
		return false;
	}

	pResultValue->SetConstSizeT (Value.GetFieldOffset ());
	return true;
}

//.............................................................................

} // namespace jnc {


