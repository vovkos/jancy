#include "pch.h"
#include "jnc_DeclTypeCalc.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
CDeclTypeCalc::CalcType (
	CType* pBaseType,
	CTypeModifiers* pTypeModifiers,
	const rtl::CConstListT <CDeclPointerPrefix>& PointerPrefixList,
	const rtl::CConstListT <CDeclSuffix>& SuffixList,
	CValue* pElementCountValue,
	uint_t* pFlags
	)
{
	bool Result;

	CType* pType = pBaseType;
	m_pModule = pType->GetModule ();

	rtl::CIteratorT <CDeclSuffix> FirstSuffix = SuffixList.GetHead ();
	rtl::CIteratorT <CDeclSuffix> SuffixEnd;

	// strip non-const array suffix if any

	if (pElementCountValue &&
		FirstSuffix &&
		FirstSuffix->GetSuffixKind () == EDeclSuffix_Array)
	{
		CDeclArraySuffix* pArraySuffix = (CDeclArraySuffix*) *FirstSuffix;
		rtl::CBoxListT <CToken>* pElementCountInitializer = pArraySuffix->GetElementCountInitializer ();

		if (!pElementCountInitializer->IsEmpty ())
		{
			Result = m_pModule->m_OperatorMgr.ParseExpression (
				NULL,
				*pElementCountInitializer,
				pElementCountValue
				);

			if (!Result)
				return NULL;

			SuffixEnd = pArraySuffix;
		}
		else if (pArraySuffix->GetElementCount () != -1)
		{
			pElementCountValue->SetConstSizeT (pArraySuffix->GetElementCount ());
			SuffixEnd = pArraySuffix;
		}
	}

	m_Suffix = SuffixList.GetTail ();

	// pointer prefixes

	rtl::CIteratorT <CDeclPointerPrefix> PointerPrefix = PointerPrefixList.GetHead ();
	for (; PointerPrefix; PointerPrefix++)
	{
		EType TypeKind = pType->GetTypeKind ();

		m_TypeModifiers = PointerPrefix->GetTypeModifiers ();
		if (m_TypeModifiers & ETypeModifier_Array)
		{
			CArrayType* pArrayType = GetArrayType (pType);
			if (!pArrayType)
				return NULL;

			pType = GetDataPtrType (pArrayType);
		}
		else if (m_TypeModifiers & ETypeModifier_Function)
		{
			CFunctionType* pFunctionType = GetFunctionType (pType);
			if (!pFunctionType)
				return NULL;

			pType = GetFunctionPtrType (pFunctionType);
		}
		else if (m_TypeModifiers & ETypeModifier_Property)
		{
			CPropertyType* pPropertyType = GetPropertyType (pType);
			if (!pPropertyType)
				return NULL;

			pType = GetPropertyPtrType (pPropertyType);
		}
		else if (m_TypeModifiers & (ETypeModifier_Multicast | ETypeModifier_Event | ETypeModifier_DEvent))
		{
			CClassType* pClassType = GetMulticastType (pType);
			if (!pClassType)
				return NULL;

			pType = GetClassPtrType (pClassType);
		}
		else if (m_TypeModifiers & ETypeModifier_Reactor)
		{
			CClassType* pClassType = GetReactorType (pType);
			if (!pClassType)
				return NULL;

			pType = GetClassPtrType (pClassType);
		}
		else switch (TypeKind)
		{
		case EType_Class:
			pType = GetClassPtrType ((CClassType*) pType);
			break;

		case EType_Function:
			pType = GetFunctionPtrType ((CFunctionType*) pType);
			break;

		case EType_Property:
			pType = GetPropertyPtrType ((CPropertyType*) pType);
			break;

		case EType_NamedImport:
			pType = GetImportPtrType ((CNamedImportType*) pType);
			break;

		default:
			pType = GetDataPtrType (pType);
		}

		if (!pType || !CheckUnusedModifiers ())
			return NULL;
	}

	TakeOver (pTypeModifiers);

	if (m_TypeModifiers & ETypeModifierMask_Integer)
	{
		pType = GetIntegerType (pType);
		if (!pType)
			return NULL;
	}

	if (m_TypeModifiers & ETypeModifier_Property)
	{
		pType = GetPropertyType (pType);
		if (!pType)
			return NULL;

		if (pFlags)
			*pFlags = GetPropertyFlags ();
	}
	else if (m_TypeModifiers & (ETypeModifier_Multicast | ETypeModifier_Event | ETypeModifier_DEvent))
	{
		pType = GetMulticastType (pType);
		if (!pType)
			return NULL;
	}
	else if (m_TypeModifiers & ETypeModifier_Bindable)
	{
		pType = GetBindableDataType (pType);
		if (!pType)
			return NULL;

		if (pFlags)
			*pFlags = EPropertyFlag_AutoGet | EPropertyFlag_AutoSet;
	}
	else if (m_TypeModifiers & ETypeModifier_Reactor)
	{
		pType = GetReactorType (pType);
		if (!pType)
			return NULL;
	}

	while (m_Suffix != SuffixEnd)
	{
		CDeclSuffix* pSuffix = (CDeclSuffix*) *m_Suffix;
		EDeclSuffix SuffixKind = pSuffix->GetSuffixKind ();

		switch (SuffixKind)
		{
		case EDeclSuffix_Array:
			pType = GetArrayType (pType);
			if (!pType)
				return NULL;

			break;

		case EDeclSuffix_Function:
		case EDeclSuffix_Throw:
			if (m_TypeModifiers & ETypeModifier_Reactor)
				pType = GetReactorType (pType);
			else
				pType = GetFunctionType (pType);

			if (!pType)
				return NULL;

			if (!CheckUnusedModifiers ())
				return NULL;

			break;

		default:
			ASSERT (false);
		}
	}

	if (!(pType->GetTypeKindFlags () & ETypeKindFlag_Code) && pFlags != NULL)
	{
		Result = GetPtrTypeFlags (pType, pFlags);
		if (!Result)
			return NULL;
	}

	if (!CheckUnusedModifiers ())
		return NULL;

	return pType;
}

CType*
CDeclTypeCalc::CalcPtrType (
	CType* pType,
	uint_t TypeModifiers
	)
{
	m_pModule = pType->GetModule ();
	m_TypeModifiers = TypeModifiers;

	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_Class:
		pType = GetClassPtrType ((CClassType*) pType);
		break;

	case EType_Function:
		pType = GetFunctionPtrType ((CFunctionType*) pType);
		break;

	case EType_Property:
		pType = GetPropertyPtrType ((CPropertyType*) pType);
		break;

	default:
		pType = GetDataPtrType (pType);
	}

	if (!CheckUnusedModifiers ())
		return NULL;

	return pType;
}

CType*
CDeclTypeCalc::CalcIntModType (
	CType* pType,
	uint_t TypeModifiers
	)
{
	m_pModule = pType->GetModule ();
	m_TypeModifiers = TypeModifiers;

	pType = GetIntegerType (pType);

	if (!CheckUnusedModifiers ())
		return NULL;

	return pType;
}

CFunctionType*
CDeclTypeCalc::CalcPropertyGetterType (CDeclarator* pDeclarator)
{
	uint_t TypeModifiers = pDeclarator->GetTypeModifiers ();
	ASSERT (TypeModifiers & ETypeModifier_Property);

	CDeclFunctionSuffix* pFunctionSuffix = NULL;
	if (!(TypeModifiers & ETypeModifier_Indexed))
		pFunctionSuffix = pDeclarator->AddFunctionSuffix ();

	pDeclarator->m_TypeModifiers &= ~(
		ETypeModifier_Const |
		ETypeModifier_Property |
		ETypeModifier_Bindable |
		ETypeModifier_AutoGet |
		ETypeModifier_Indexed
		);

	CType* pType = CalcType (
		pDeclarator->GetBaseType (),
		pDeclarator,
		pDeclarator->GetPointerPrefixList (),
		pDeclarator->GetSuffixList (),
		NULL,
		NULL
		);

	pDeclarator->m_TypeModifiers = TypeModifiers;

	if (pFunctionSuffix)
		pDeclarator->DeleteSuffix (pFunctionSuffix);

	if (!pType)
		return NULL;

	ASSERT (pType->GetTypeKind () == EType_Function);
	return (CFunctionType*) pType;
}

bool
CDeclTypeCalc::CheckUnusedModifiers ()
{
	if (m_TypeModifiers)
	{
		err::SetFormatStringError ("unused modifier '%s'", GetTypeModifierString (m_TypeModifiers).cc ());
		return false;
	}

	return true;
}

bool
CDeclTypeCalc::GetPtrTypeFlags (
	CType* pType,
	uint_t* pFlags
	)
{
	uint_t Flags = 0;

	if (m_TypeModifiers & ETypeModifier_Const)
		Flags |= EPtrTypeFlag_Const;

	if (m_TypeModifiers & ETypeModifier_DConst)
		Flags |= EPtrTypeFlag_ConstD;

	if (m_TypeModifiers & ETypeModifier_Volatile)
	{
		if (pType->GetTypeKindFlags () & ETypeKindFlag_Code)
		{
			err::SetFormatStringError ("'volatile' cannot be applied to '%s'", pType->GetTypeString ().cc ());
			return false;
		}

		Flags |= EPtrTypeFlag_Volatile;
	}

	if (m_TypeModifiers & (ETypeModifier_Event | ETypeModifier_DEvent)) // convert 'event' to 'devent'
	{
		ASSERT (IsClassType (pType, EClassType_Multicast));
		Flags |= EPtrTypeFlag_EventD;
	}

	if (m_TypeModifiers & ETypeModifier_Bindable)
	{
		if (!IsClassType (pType, EClassType_Multicast))
		{
			err::SetFormatStringError ("'bindable' cannot be applied to '%s'", pType->GetTypeString ().cc ());
			return false;
		}

		Flags |= EPtrTypeFlag_Bindable;
	}

	if (m_TypeModifiers & ETypeModifier_AutoGet)
		Flags |= EPtrTypeFlag_AutoGet;

	m_TypeModifiers &= ~ETypeModifierMask_DeclPtr;
	*pFlags = Flags;
	return true;
}

uint_t
CDeclTypeCalc::GetPropertyFlags ()
{
	uint_t Flags = 0;

	if (m_TypeModifiers & ETypeModifier_AutoGet)
		Flags |= EPropertyFlag_AutoGet;

	m_TypeModifiers &= ~ETypeModifier_AutoGet;
	return Flags;
}

CType*
CDeclTypeCalc::GetIntegerType (CType* pType)
{
	ASSERT (m_TypeModifiers & ETypeModifierMask_Integer);

	if (pType->GetTypeKind () == EType_NamedImport)
		return GetImportIntModType ((CNamedImportType*) pType);

	if (!(pType->GetTypeKindFlags () & ETypeKindFlag_Integer))
	{
		err::SetFormatStringError ("'%s' modifier cannot be applied to '%s'",
			GetTypeModifierString (m_TypeModifiers & ETypeModifierMask_Integer).cc (),
			pType->GetTypeString ().cc ()
			);
		return NULL;
	}

	if (m_TypeModifiers & ETypeModifier_Unsigned)
	{
		EType ModTypeKind = GetUnsignedIntegerTypeKind (pType->GetTypeKind ());
		pType = m_pModule->m_TypeMgr.GetPrimitiveType (ModTypeKind);
	}

	if (m_TypeModifiers & ETypeModifier_BigEndian)
	{
		EType ModTypeKind = GetBigEndianIntegerTypeKind (pType->GetTypeKind ());
		pType = m_pModule->m_TypeMgr.GetPrimitiveType (ModTypeKind);
	}

	m_TypeModifiers &= ~ETypeModifierMask_Integer;
	return pType;
}

CArrayType*
CDeclTypeCalc::GetArrayType (CType* pElementType)
{
	if (!m_Suffix || m_Suffix->GetSuffixKind () != EDeclSuffix_Array)
	{
		err::SetFormatStringError ("missing array suffix");
		return NULL;
	}

	CDeclArraySuffix* pSuffix = (CDeclArraySuffix*) *m_Suffix--;

	EType TypeKind = pElementType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_Void:
	case EType_Class:
	case EType_Function:
	case EType_Property:
		err::SetFormatStringError ("cannot create array of '%s'", pElementType->GetTypeString ().cc () );
		return NULL;

	default:
		if (IsAutoSizeArrayType (pElementType))
		{
			err::SetFormatStringError ("cannot create array of auto-size-array '%s'", pElementType->GetTypeString ().cc () );
			return NULL;
		}

		if (m_TypeModifiers & ETypeModifierMask_Integer)
		{
			pElementType = GetIntegerType (pElementType);
			if (!pElementType)
				return NULL;
		}
	}

	m_TypeModifiers &= ~ETypeModifier_Array;

	rtl::CBoxListT <CToken>* pElementCountInitializer = pSuffix->GetElementCountInitializer ();
	if (!pElementCountInitializer->IsEmpty ())
		return m_pModule->m_TypeMgr.CreateArrayType (pElementType, pElementCountInitializer);

	size_t ElementCount = pSuffix->GetElementCount ();
	return ElementCount == -1 ?
		m_pModule->m_TypeMgr.CreateAutoSizeArrayType (pElementType) :
		m_pModule->m_TypeMgr.GetArrayType (pElementType, ElementCount);
}

CType*
CDeclTypeCalc::PrepareReturnType (CType* pType)
{
	while (m_Suffix && m_Suffix->GetSuffixKind () == EDeclSuffix_Array)
	{
		pType = GetArrayType (pType);
		if (!pType)
			return NULL;
	}

	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_Class:
	case EType_Function:
	case EType_Property:
		err::SetFormatStringError (
			"function cannot return '%s'",
			pType->GetTypeString ().cc ()
			);
		return NULL;

	default:
		if (IsAutoSizeArrayType (pType))
		{
			err::SetFormatStringError ("function cannot return auto-size-array '%s'", pType->GetTypeString ().cc () );
			return NULL;
		}

		if (m_TypeModifiers & ETypeModifierMask_Integer)
			return GetIntegerType (pType);
	}

	return pType;
}

CFunctionType*
CDeclTypeCalc::GetFunctionType (CType* pReturnType)
{
	uint_t TypeFlags = 0;
	rtl::CBoxListT <CToken>* pThrowCondition = NULL;

	if (m_Suffix && m_Suffix->GetSuffixKind () == EDeclSuffix_Throw)
	{
		CDeclThrowSuffix* pSuffix = (CDeclThrowSuffix*) *m_Suffix--;

		TypeFlags |= EFunctionTypeFlag_Throws;
		pThrowCondition = pSuffix->GetThrowCondition ();
	}

	pReturnType = PrepareReturnType (pReturnType);
	if (!pReturnType)
		return NULL;

	if ((TypeFlags & EFunctionTypeFlag_Throws) && pReturnType->GetTypeKind () == EType_Void)
	{
		err::SetFormatStringError ("void function cannot throw");
		return NULL;
	}

	if (!m_Suffix || m_Suffix->GetSuffixKind () != EDeclSuffix_Function)
	{
		err::SetFormatStringError ("missing function suffix");
		return NULL;
	}

	CDeclFunctionSuffix* pSuffix = (CDeclFunctionSuffix*) *m_Suffix--;
	TypeFlags |= pSuffix->GetFunctionTypeFlags ();

	ECallConv CallConvKind = GetCallConvKindFromModifiers (m_TypeModifiers);
	CCallConv* pCallConv = m_pModule->m_TypeMgr.GetCallConv (CallConvKind);

	if (TypeFlags & EFunctionTypeFlag_VarArg)
	{
		uint_t CallConvFlags = pCallConv->GetFlags ();

		if (CallConvFlags & ECallConvFlag_NoVarArg)
		{
			err::SetFormatStringError ("vararg cannot be used with '%s'", pCallConv->GetCallConvDisplayString ());
			return NULL;
		}

		if (!(CallConvFlags & ECallConvFlag_UnsafeVarArg))
		{
			err::SetFormatStringError ("only 'cdecl' vararg is currently supported");
			return NULL;
		}
	}

	m_TypeModifiers &= ~ETypeModifierMask_Function;

	return m_pModule->m_TypeMgr.CreateUserFunctionType (
		pCallConv,
		pReturnType,
		pThrowCondition,
		pSuffix->GetArgArray (),
		TypeFlags
		);
}

CPropertyType*
CDeclTypeCalc::GetPropertyType (CType* pReturnType)
{
	uint_t TypeFlags = 0;
	if (m_Suffix && m_Suffix->GetSuffixKind () == EDeclSuffix_Throw)
	{
		CDeclThrowSuffix* pSuffix = (CDeclThrowSuffix*) *m_Suffix--;
		if (!pSuffix->GetThrowCondition ()->IsEmpty ())
		{
			err::SetFormatStringError ("property cannot have a throw condition");
			return NULL;
		}

		TypeFlags |= EPropertyTypeFlag_Throws;
	}

	pReturnType = PrepareReturnType (pReturnType);
	if (!pReturnType)
		return NULL;

	if (pReturnType->GetTypeKind () == EType_Void)
	{
		err::SetFormatStringError ("property cannot return 'void'");
		return NULL;
	}

	ECallConv CallConvKind = GetCallConvKindFromModifiers (m_TypeModifiers);
	CCallConv* pCallConv = m_pModule->m_TypeMgr.GetCallConv (CallConvKind);

	if (m_TypeModifiers & ETypeModifier_Const)
	{
		if (TypeFlags & EPropertyTypeFlag_Throws)
		{
			err::SetFormatStringError ("const property cannot throw");
			return NULL;
		}

		TypeFlags |= EPropertyTypeFlag_Const;
	}

	if (m_TypeModifiers & ETypeModifier_Bindable)
		TypeFlags |= EPropertyTypeFlag_Bindable;

	bool IsIndexed = (m_TypeModifiers & ETypeModifier_Indexed) != 0;
	m_TypeModifiers &= ~ETypeModifierMask_Property;

	if (!IsIndexed)
		return m_pModule->m_TypeMgr.GetSimplePropertyType (
			pCallConv,
			pReturnType,
			TypeFlags
			);

	// indexed property

	if (!m_Suffix || m_Suffix->GetSuffixKind () != EDeclSuffix_Function)
	{
		err::SetFormatStringError ("missing indexed property suffix");
		return NULL;
	}

	CDeclFunctionSuffix* pSuffix = (CDeclFunctionSuffix*) *m_Suffix--;
	return m_pModule->m_TypeMgr.CreateIndexedPropertyType (
		pCallConv,
		pReturnType,
		pSuffix->GetArgArray (),
		TypeFlags
		);
}

CPropertyType*
CDeclTypeCalc::GetBindableDataType (CType* pDataType)
{
	pDataType = PrepareReturnType (pDataType);
	if (!pDataType)
		return NULL;

	if (pDataType->GetTypeKind () == EType_Void)
	{
		err::SetFormatStringError ("bindable data cannot be 'void'");
		return NULL;
	}

	if (m_TypeModifiers & ETypeModifier_Indexed)
	{
		err::SetFormatStringError ("bindable data cannot be 'indexed'");
		return NULL;
	}

	ECallConv CallConvKind = GetCallConvKindFromModifiers (m_TypeModifiers);
	CCallConv* pCallConv = m_pModule->m_TypeMgr.GetCallConv (CallConvKind);

	m_TypeModifiers &= ~ETypeModifierMask_Property;
	return m_pModule->m_TypeMgr.GetSimplePropertyType (pCallConv, pDataType, EPropertyTypeFlag_Bindable);
}

CClassType*
CDeclTypeCalc::GetReactorType (CType* pReturnType)
{
	CFunctionType* pStartMethodType = GetFunctionType (pReturnType);
	if (!pStartMethodType)
		return NULL;

	m_TypeModifiers &= ~ETypeModifier_Reactor;
	return m_pModule->m_TypeMgr.GetReactorInterfaceType (pStartMethodType);
}

CClassType*
CDeclTypeCalc::GetMulticastType (CType* pLeftType)
{
	CFunctionPtrType* pPtrType;

	EType TypeKind = pLeftType->GetTypeKind ();
	if (TypeKind == EType_FunctionPtr)
	{
		pPtrType = (CFunctionPtrType*) pLeftType;
	}
	else if (TypeKind == EType_Function)
	{
		pPtrType = GetFunctionPtrType ((CFunctionType*) pLeftType);
		if (!pPtrType)
			return NULL;
	}
	else
	{
		CFunctionType* pFunctionType = GetFunctionType (pLeftType);
		if (!pFunctionType)
			return NULL;

		pPtrType = GetFunctionPtrType (pFunctionType);
		if (!pPtrType)
			return NULL;
	}

	m_TypeModifiers &= ~ETypeModifier_Multicast;
	return m_pModule->m_TypeMgr.GetMulticastType (pPtrType);
}

CDataPtrType*
CDeclTypeCalc::GetDataPtrType (CType* pDataType)
{
	if (m_TypeModifiers & ETypeModifierMask_Integer)
	{
		pDataType = GetIntegerType (pDataType);
		if (!pDataType)
			return NULL;
	}

	EDataPtrType PtrTypeKind = EDataPtrType_Normal;

	if (m_TypeModifiers & ETypeModifier_Thin)
	{
		if (m_TypeModifiers & ETypeModifier_Safe)
		{
			err::SetFormatStringError ("'thin' data pointer cannot be 'safe'");
			return NULL;
		}

		PtrTypeKind = EDataPtrType_Thin;
	}
	
	uint_t TypeFlags = GetPtrTypeFlagsFromModifiers (m_TypeModifiers);

	m_TypeModifiers &= ~ETypeModifierMask_DataPtr;
	return pDataType->GetDataPtrType (
		m_pModule->m_NamespaceMgr.GetCurrentNamespace (),
		EType_DataPtr,
		PtrTypeKind,
		TypeFlags
		);
}

CClassPtrType*
CDeclTypeCalc::GetClassPtrType (CClassType* pClassType)
{
	EClassPtrType PtrTypeKind = (m_TypeModifiers & ETypeModifier_Weak) ? EClassPtrType_Weak : EClassPtrType_Normal;
	uint_t TypeFlags = GetPtrTypeFlagsFromModifiers (m_TypeModifiers);

	m_TypeModifiers &= ~ETypeModifierMask_ClassPtr;
	return pClassType->GetClassPtrType (
		m_pModule->m_NamespaceMgr.GetCurrentNamespace (),
		EType_ClassPtr,
		PtrTypeKind,
		TypeFlags
		);
}

CFunctionPtrType*
CDeclTypeCalc::GetFunctionPtrType (CFunctionType* pFunctionType)
{
	EFunctionPtrType PtrTypeKind =
		(m_TypeModifiers & ETypeModifier_Weak) ? EFunctionPtrType_Weak :
		(m_TypeModifiers & ETypeModifier_Thin) ? EFunctionPtrType_Thin : EFunctionPtrType_Normal;

	uint_t TypeFlags = GetPtrTypeFlagsFromModifiers (m_TypeModifiers);

	m_TypeModifiers &= ~ETypeModifierMask_FunctionPtr;
	return pFunctionType->GetFunctionPtrType (PtrTypeKind);
}

CPropertyPtrType*
CDeclTypeCalc::GetPropertyPtrType (CPropertyType* pPropertyType)
{
	EPropertyPtrType PtrTypeKind =
		(m_TypeModifiers & ETypeModifier_Weak) ? EPropertyPtrType_Weak :
		(m_TypeModifiers & ETypeModifier_Thin) ? EPropertyPtrType_Thin : EPropertyPtrType_Normal;

	uint_t TypeFlags = GetPtrTypeFlagsFromModifiers (m_TypeModifiers);

	m_TypeModifiers &= ~ETypeModifierMask_PropertyPtr;
	return pPropertyType->GetPropertyPtrType (PtrTypeKind);
}

CImportPtrType*
CDeclTypeCalc::GetImportPtrType (CNamedImportType* pImportType)
{
	uint_t TypeModifiers = m_TypeModifiers & ETypeModifierMask_ImportPtr;
	m_TypeModifiers &= ~ETypeModifierMask_ImportPtr;
	return m_pModule->m_TypeMgr.GetImportPtrType (pImportType, TypeModifiers);
}

CImportIntModType*
CDeclTypeCalc::GetImportIntModType (CNamedImportType* pImportType)
{
	uint_t TypeModifiers = m_TypeModifiers & ETypeModifierMask_Integer;
	m_TypeModifiers &= ~ETypeModifierMask_Integer;
	return m_pModule->m_TypeMgr.GetImportIntModType (pImportType, TypeModifiers);
}

//.............................................................................

} // namespace jnc {
