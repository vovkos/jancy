#include "pch.h"
#include "jnc_TypeMgr.h"
#include "jnc_Module.h"
#include "jnc_DeclTypeCalc.h"

namespace jnc {

//.............................................................................

CTypeMgr::CTypeMgr ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	SetupCallConvTable ();
	SetupAllPrimitiveTypes ();

	memset (m_StdTypeArray, 0, sizeof (m_StdTypeArray));
	memset (m_LazyStdTypeArray, 0, sizeof (m_LazyStdTypeArray));

	m_UnnamedEnumTypeCounter = 0;
	m_UnnamedStructTypeCounter = 0;
	m_UnnamedUnionTypeCounter = 0;
	m_UnnamedClassTypeCounter = 0;
}

void
CTypeMgr::Clear ()
{
	m_ArrayTypeList.Clear ();
	m_BitFieldTypeList.Clear ();
	m_EnumTypeList.Clear ();
	m_StructTypeList.Clear ();
	m_UnionTypeList.Clear ();
	m_ClassTypeList.Clear ();
	m_FunctionTypeList.Clear ();
	m_PropertyTypeList.Clear ();
	m_DataPtrTypeList.Clear ();
	m_ClassPtrTypeList.Clear ();
	m_FunctionPtrTypeList.Clear ();
	m_PropertyPtrTypeList.Clear ();
	m_NamedImportTypeList.Clear ();
	m_ImportPtrTypeList.Clear ();
	m_ImportIntModTypeList.Clear ();
	m_ReactorClassTypeList.Clear ();
	m_FunctionClosureClassTypeList.Clear ();
	m_PropertyClosureClassTypeList.Clear ();
	m_DataClosureClassTypeList.Clear ();
	m_MulticastClassTypeList.Clear ();
	m_McSnapshotClassTypeList.Clear ();

	m_SimplePropertyTypeTupleList.Clear ();
	m_FunctionArgTupleList.Clear ();
	m_DataPtrTypeTupleList.Clear ();
	m_ClassPtrTypeTupleList.Clear ();
	m_FunctionPtrTypeTupleList.Clear ();
	m_PropertyPtrTypeTupleList.Clear ();
	m_DualPtrTypeTupleList.Clear ();

	m_TypedefList.Clear ();
	m_LazyStdTypeList.Clear ();
	m_FunctionArgList.Clear ();

	m_TypeMap.Clear ();
	m_GcShadowStackFrameTypeArray.Clear ();

	SetupAllPrimitiveTypes ();

	memset (m_StdTypeArray, 0, sizeof (m_StdTypeArray));
	memset (m_LazyStdTypeArray, 0, sizeof (m_LazyStdTypeArray));

	m_UnnamedEnumTypeCounter = 0;
	m_UnnamedStructTypeCounter = 0;
	m_UnnamedUnionTypeCounter = 0;
	m_UnnamedClassTypeCounter = 0;
}

CType*
CTypeMgr::GetStdType (EStdType StdType)
{
	ASSERT ((size_t) StdType < EStdType__Count);
	if (m_StdTypeArray [StdType])
		return m_StdTypeArray [StdType];

	CType* pType;

	switch (StdType)
	{
	case EStdType_BytePtr:
		pType = GetPrimitiveType (EType_Int8_u)->GetDataPtrType_c ();
		break;

	case EStdType_ObjHdr:
		pType = CreateObjHdrType ();
		break;

	case EStdType_ObjHdrPtr:
		pType = GetStdType (EStdType_ObjHdr)->GetDataPtrType_c ();
		break;

	case EStdType_ObjectClass:
		pType = CreateObjectType ();
		break;

	case EStdType_ObjectPtr:
		pType = ((CClassType*) GetStdType (EStdType_ObjectClass))->GetClassPtrType ();
		break;

	case EStdType_SimpleFunction:
		pType = GetFunctionType (GetPrimitiveType (EType_Void), NULL, 0, 0);
		break;

	case EStdType_SimpleMulticast:
		pType = GetMulticastType ((CFunctionType*) GetStdType (EStdType_SimpleFunction));
		break;

	case EStdType_SimpleEventPtr:
		pType = ((CClassType*) GetStdType (EStdType_SimpleMulticast))->GetClassPtrType (EClassPtrType_Normal);
		break;

	case EStdType_ReactorBindSite:
		pType = CreateReactorBindSiteType ();
		break;

	case EStdType_Binder:
		pType = GetFunctionType (GetStdType (EStdType_SimpleEventPtr), NULL, 0);
		break;

	case EStdType_Scheduler:
		pType = CreateSchedulerType ();
		break;

	case EStdType_SchedulerPtr:
		pType = ((CClassType*) GetStdType (EStdType_Scheduler))->GetClassPtrType ();
		break;

	case EStdType_FmtLiteral:
		pType = CreateFmtLiteralType ();
		break;

	case EStdType_Guid:
		pType = CreateGuidType ();
		break;

	case EStdType_Error:
		pType = CreateErrorType ();
		break;

	default:
		ASSERT (false);
		return NULL;
	}

	m_StdTypeArray [StdType] = pType;
	return pType;
}

CLazyStdType*
CTypeMgr::GetLazyStdType (EStdType StdType)
{
	ASSERT ((size_t) StdType < EStdType__Count);

	if (m_LazyStdTypeArray [StdType])
		return m_LazyStdTypeArray [StdType];

	const char* NameTable [EStdType__Count] =
	{
		NULL, // EStdType_BytePtr,
		NULL, // EStdType_ObjHdr,
		NULL, // EStdType_ObjHdrPtr,
		NULL, // EStdType_ObjectClass,
		NULL, // EStdType_ObjectPtr,
		NULL, // EStdType_SimpleFunction,
		NULL, // EStdType_SimpleMulticast,
		NULL, // EStdType_SimpleEventPtr,
		NULL, // EStdType_Binder,
		NULL, // EStdType_ReactorBindSite,
		"Scheduler", // EStdType_Scheduler,
		NULL, // EStdType_SchedulerPtr,
		NULL, // EStdType_FmtLiteral,
		"Guid",      // EStdType_Guid
		"Error",     // EStdType_Error,
	};

	const char* pName = NameTable [StdType];
	ASSERT (pName);

	CLazyStdType* pType = AXL_MEM_NEW (CLazyStdType);
	pType->m_pModule = m_pModule;
	pType->m_Name = pName;
	pType->m_StdType = StdType;
	m_LazyStdTypeList.InsertTail (pType);
	m_LazyStdTypeArray [StdType] = pType;
	return pType;
}

void
PushImportSrcPosError (CNamedImportType* pImportType)
{
	err::PushSrcPosError (
		pImportType->GetParentUnit ()->GetFilePath (),
		*pImportType->GetPos ()
		);
}

bool
CTypeMgr::ResolveImportTypes ()
{
	char Buffer [256];
	rtl::CArrayT <CNamedImportType*> SuperImportTypeArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));

	rtl::CIteratorT <CNamedImportType> ImportType = m_NamedImportTypeList.GetHead ();
	for (; ImportType; ImportType++)
	{
		CNamedImportType* pImportType = *ImportType;
		CModuleItem* pItem = pImportType->m_pAnchorNamespace->FindItemTraverse (pImportType->m_Name);
		if (!pItem)
		{
			err::SetFormatStringError ("unresolved import '%s'", pImportType->GetTypeString ().cc ());
			PushImportSrcPosError (pImportType);
			return false;
		}

		EModuleItem ItemKind = pItem->GetItemKind ();
		switch (ItemKind)
		{
		case EModuleItem_Type:
			pImportType->m_pActualType = (CType*) pItem;
			break;

		case EModuleItem_Typedef:
			pImportType->m_pActualType = ((CTypedef*) pItem)->GetType ();
			if (pImportType->m_pActualType->GetTypeKind () == EType_NamedImport)
				SuperImportTypeArray.Append (pImportType);
			break;

		default:
			err::SetFormatStringError ("'%s' is not a type", pImportType->GetTypeString ().cc ());
			PushImportSrcPosError (pImportType);
			return false;
		}
	}

	// eliminate super-imports and detect import loops

	size_t Count = SuperImportTypeArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CNamedImportType* pSuperImportType = SuperImportTypeArray [i];
		pSuperImportType->m_Flags |= EImportTypeFlag_ImportLoop;

		CType* pType = pSuperImportType->m_pActualType;
		while (pType->m_TypeKind == EType_NamedImport)
		{
			CImportType* pImportType = (CImportType*) pType;
			if (pImportType->m_Flags & EImportTypeFlag_ImportLoop)
			{
				err::SetFormatStringError ("'%s': import loop detected", pImportType->GetTypeString ().cc ());
				PushImportSrcPosError (pSuperImportType);
				return false;
			}

			pImportType->m_Flags |= EImportTypeFlag_ImportLoop;
			pType = pImportType->m_pActualType;
		}

		CType* pExternType = pType;
		while (pType->m_TypeKind == EType_NamedImport)
		{
			CImportType* pImportType = (CImportType*) pType;
			pImportType->m_pActualType = pExternType;
			pImportType->m_Flags &= ~EImportTypeFlag_ImportLoop;
			pType = pImportType->m_pActualType;
		}
	}

	rtl::CIteratorT <CImportIntModType> ImportIntModType = m_ImportIntModTypeList.GetHead ();
	for (; ImportIntModType; ImportIntModType++)
	{
		CImportIntModType* pImportType = *ImportIntModType;

		CDeclTypeCalc TypeCalc;

		CType* pType = TypeCalc.CalcIntModType (
			pImportType->m_pImportType->m_pActualType,
			pImportType->m_TypeModifiers
			);

		if (!pType)
			return false;

		pImportType->m_pActualType = pType;
	}

	rtl::CIteratorT <CImportPtrType> ImportPtrType = m_ImportPtrTypeList.GetHead ();
	for (; ImportPtrType; ImportPtrType++)
	{
		CImportPtrType* pImportType = *ImportPtrType;

		CDeclTypeCalc TypeCalc;

		CType* pType = TypeCalc.CalcPtrType (
			pImportType->m_pTargetType->m_pActualType,
			pImportType->m_TypeModifiers
			);

		if (!pType)
			return false;

		if (pImportType->GetFlags () & EPtrTypeFlag_Safe)
			pType = GetCheckedPtrType (pType);

		pImportType->m_pActualType = pType;
	}

	return true;
}

void
CTypeMgr::UpdateTypeSignature (
	CType* pType,
	const rtl::CString& Signature
	)
{
	if (pType->m_Signature == Signature)
		return;

	if (!pType->m_TypeMapIt)
	{
		pType->m_Signature = Signature;
		return;
	}

	m_TypeMap.Delete (pType->m_TypeMapIt);
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = m_TypeMap.Goto (Signature);
	pType->m_TypeMapIt->m_Value = pType;
}

CBitFieldType*
CTypeMgr::GetBitFieldType (
	CType* pBaseType,
	size_t BitOffset,
	size_t BitCount
	)
{
	rtl::CString Signature = CBitFieldType::CreateSignature (pBaseType, BitOffset, BitCount);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CBitFieldType* pType = (CBitFieldType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	CBitFieldType* pType = AXL_MEM_NEW (CBitFieldType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->m_pBaseType = pBaseType;
	pType->m_BitOffset = BitOffset;
	pType->m_BitCount = BitCount;

	m_BitFieldTypeList.InsertTail (pType);
	It->m_Value = pType;

	if (pBaseType->GetTypeKindFlags () & ETypeKindFlag_Import)
	{
		pType->m_pBaseType_i = (CImportType*) pBaseType;
		m_pModule->MarkForLayout (pType, true);
	}
	else
	{
		bool Result = pType->EnsureLayout ();
		if (!Result)
			return NULL;
	}

	return pType;
}

CArrayType*
CTypeMgr::CreateAutoSizeArrayType (CType* pElementType)
{
	CArrayType* pType = AXL_MEM_NEW (CArrayType);
	pType->m_Flags |= EArrayTypeFlag_AutoSize;
	pType->m_pModule = m_pModule;
	pType->m_pElementType = pElementType;
	m_ArrayTypeList.InsertTail (pType);

	if (pElementType->GetTypeKindFlags () & ETypeKindFlag_Import)
		pType->m_pElementType_i = (CImportType*) pElementType;

	if (!m_pModule->m_NamespaceMgr.GetCurrentScope ())
		m_pModule->MarkForLayout (pType, true); // can't calclayout yet

	return pType;
}

CArrayType*
CTypeMgr::CreateArrayType (
	CType* pElementType,
	rtl::CBoxListT <CToken>* pElementCountInitializer
	)
{
	CArrayType* pType = AXL_MEM_NEW (CArrayType);
	pType->m_pModule = m_pModule;
	pType->m_pElementType = pElementType;
	pType->m_ElementCountInitializer.TakeOver (pElementCountInitializer);
	pType->m_pParentUnit = m_pModule->m_UnitMgr.GetCurrentUnit ();
	pType->m_pParentNamespace = m_pModule->m_NamespaceMgr.GetCurrentNamespace ();
	m_ArrayTypeList.InsertTail (pType);

	if (pElementType->GetTypeKindFlags () & ETypeKindFlag_Import)
		pType->m_pElementType_i = (CImportType*) pElementType;

	if (!m_pModule->m_NamespaceMgr.GetCurrentScope ())
	{
		m_pModule->MarkForLayout (pType, true);
	}
	else
	{
		bool Result = pType->EnsureLayout ();
		if (!Result)
			return NULL;
	}

	return pType;
}

CArrayType*
CTypeMgr::GetArrayType (
	CType* pElementType,
	size_t ElementCount
	)
{
	rtl::CString Signature = CArrayType::CreateSignature (pElementType, ElementCount);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CArrayType* pType = (CArrayType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	CArrayType* pType = AXL_MEM_NEW (CArrayType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->m_pElementType = pElementType;
	pType->m_ElementCount = ElementCount;
	m_ArrayTypeList.InsertTail (pType);

	if (pElementType->GetTypeKindFlags () & ETypeKindFlag_Import)
	{
		pType->m_pElementType_i = (CImportType*) pElementType;
		m_pModule->MarkForLayout (pType, true);
	}
	else
	{
		bool Result = pType->EnsureLayout ();
		if (!Result)
			return NULL;
	}

	It->m_Value = pType;
	return pType;
}

CTypedef*
CTypeMgr::CreateTypedef (
	const rtl::CString& Name,
	const rtl::CString& QualifiedName,
	CType* pType
	)
{
	CTypedef* pTypedef = AXL_MEM_NEW (CTypedef);
	pTypedef->m_Name = Name;
	pTypedef->m_QualifiedName = QualifiedName;
	pTypedef->m_Tag = QualifiedName;
	pTypedef->m_pType = pType;
	m_TypedefList.InsertTail (pTypedef);

	return pTypedef;
}

CEnumType*
CTypeMgr::CreateEnumType (
	EEnumType EnumTypeKind,
	const rtl::CString& Name,
	const rtl::CString& QualifiedName,
	CType* pBaseType,
	uint_t Flags
	)
{
	const char* pSignaturePrefix = (Flags & EEnumTypeFlag_Exposed) ? "EC" : "EE";

	CEnumType* pType = AXL_MEM_NEW (CEnumType);

	if (Name.IsEmpty ())
	{
		m_UnnamedEnumTypeCounter++;
		pType->m_Signature.Format ("%s%d", pSignaturePrefix, m_UnnamedEnumTypeCounter);
		pType->m_Tag.Format (".UnnamedEnum%d", m_UnnamedEnumTypeCounter);
	}
	else
	{
		pType->m_Signature.Format ("%s%s", pSignaturePrefix, QualifiedName.cc ());
		pType->m_Name = Name;
		pType->m_QualifiedName = QualifiedName;
		pType->m_Tag = QualifiedName;
		pType->m_Flags |= ETypeFlag_Named;
	}

	if (!pBaseType)
		pBaseType = GetPrimitiveType (EType_Int);

	pType->m_pModule = m_pModule;
	pType->m_EnumTypeKind = EnumTypeKind;
	pType->m_pBaseType = pBaseType;
	pType->m_Flags |= Flags;
	m_EnumTypeList.InsertTail (pType);

	if (pBaseType->GetTypeKindFlags () & ETypeKindFlag_Import)
		pType->m_pBaseType_i = (CImportType*) pBaseType;

	m_pModule->MarkForLayout (pType, true);
	return pType;
}

CStructType*
CTypeMgr::CreateStructType (
	const rtl::CString& Name,
	const rtl::CString& QualifiedName,
	size_t PackFactor
	)
{
	CStructType* pType = AXL_MEM_NEW (CStructType);

	if (Name.IsEmpty ())
	{
		m_UnnamedStructTypeCounter++;
		pType->m_Signature.Format ("S%d", m_UnnamedStructTypeCounter);
		pType->m_Tag.Format (".UnnamedStruct%d", m_UnnamedStructTypeCounter);
	}
	else
	{
		pType->m_Signature.Format ("S%s", QualifiedName.cc ());
		pType->m_Name = Name;
		pType->m_QualifiedName = QualifiedName;
		pType->m_Tag = QualifiedName;
		pType->m_Flags |= ETypeFlag_Named;
	}

	pType->m_pModule = m_pModule;
	pType->m_PackFactor = PackFactor;
	m_StructTypeList.InsertTail (pType);
	m_pModule->MarkForLayout (pType, true);
	return pType;
}

CUnionType*
CTypeMgr::CreateUnionType (
	const rtl::CString& Name,
	const rtl::CString& QualifiedName,
	size_t PackFactor
	)
{
	CUnionType* pType = AXL_MEM_NEW (CUnionType);

	if (Name.IsEmpty ())
	{
		m_UnnamedUnionTypeCounter++;
		pType->m_Signature.Format ("U%d", m_UnnamedUnionTypeCounter);
		pType->m_Tag.Format (".UnamedUnion%d", m_UnnamedUnionTypeCounter);
	}
	else
	{
		pType->m_Signature.Format ("U%s", QualifiedName.cc ());
		pType->m_Name = Name;
		pType->m_QualifiedName = QualifiedName;
		pType->m_Tag = QualifiedName;
		pType->m_Flags |= ETypeFlag_Named;
	}

	m_pModule->MarkForLayout (pType, true); // before child struct

	CStructType* pUnionStructType = CreateUnnamedStructType ();
	pUnionStructType->m_pParentNamespace = pType;
	pUnionStructType->m_StructTypeKind = EStructType_UnionStruct;
	pUnionStructType->m_PackFactor = PackFactor;
	pUnionStructType->m_Tag.Format ("%s.Struct", pType->m_Tag.cc ());

	pType->m_pModule = m_pModule;
	pType->m_pStructType = pUnionStructType;
	m_UnionTypeList.InsertTail (pType);
	return pType;
}

CClassType*
CTypeMgr::CreateClassType (
	EClassType ClassTypeKind,
	const rtl::CString& Name,
	const rtl::CString& QualifiedName,
	size_t PackFactor,
	uint_t Flags
	)
{
	CClassType* pType;

	switch (ClassTypeKind)
	{
	case EClassType_Reactor:
		pType = AXL_MEM_NEW (CReactorClassType);
		m_ReactorClassTypeList.InsertTail ((CReactorClassType*) pType);
		break;

	case EClassType_FunctionClosure:
		pType = AXL_MEM_NEW (CFunctionClosureClassType);
		m_FunctionClosureClassTypeList.InsertTail ((CFunctionClosureClassType*) pType);
		break;

	case EClassType_PropertyClosure:
		pType = AXL_MEM_NEW (CPropertyClosureClassType);
		m_PropertyClosureClassTypeList.InsertTail ((CPropertyClosureClassType*) pType);
		break;

	case EClassType_DataClosure:
		pType = AXL_MEM_NEW (CDataClosureClassType);
		m_DataClosureClassTypeList.InsertTail ((CDataClosureClassType*) pType);
		break;

	case EClassType_Multicast:
		pType = AXL_MEM_NEW (CMulticastClassType);
		m_MulticastClassTypeList.InsertTail ((CMulticastClassType*) pType);
		break;

	case EClassType_McSnapshot:
		pType = AXL_MEM_NEW (CMcSnapshotClassType);
		m_McSnapshotClassTypeList.InsertTail ((CMcSnapshotClassType*) pType);
		break;

	default:
		pType = AXL_MEM_NEW (CClassType);
		m_ClassTypeList.InsertTail (pType);
	}

	if (Name.IsEmpty ())
	{
		m_UnnamedClassTypeCounter++;
		pType->m_Signature.Format ("CC%d", m_UnnamedClassTypeCounter);
		pType->m_Tag.Format (".UnnamedClass%d", m_UnnamedClassTypeCounter);
	}
	else
	{
		pType->m_Signature.Format ("CC%s", QualifiedName.cc ());
		pType->m_Name = Name;
		pType->m_QualifiedName = QualifiedName;
		pType->m_Tag = QualifiedName;
		pType->m_Flags |= ETypeFlag_Named;
	}

	m_pModule->MarkForLayout (pType, true); // before child structs

	CStructType* pVTableStructType = CreateUnnamedStructType ();
	pVTableStructType->m_Tag.Format ("%s.Vtbl", pType->m_Tag.cc ());

	CStructType* pIfaceHdrStructType = CreateUnnamedStructType (PackFactor);
	pIfaceHdrStructType->m_Tag.Format ("%s.IfaceHdr", pType->m_Tag.cc ());
	pIfaceHdrStructType->CreateField ("!m_vtbl", pVTableStructType->GetDataPtrType_c ());
	pIfaceHdrStructType->CreateField ("!m_object", GetStdType (EStdType_ObjHdrPtr));

	CStructType* pIfaceStructType = CreateUnnamedStructType (PackFactor);
	pIfaceStructType->m_StructTypeKind = EStructType_IfaceStruct;
	pIfaceStructType->m_Tag.Format ("%s.Iface", pType->m_Tag.cc ());
	pIfaceStructType->m_pParentNamespace = pType;
	pIfaceStructType->m_StorageKind = EStorage_Member;
	pIfaceStructType->m_PackFactor = PackFactor;
	pIfaceStructType->AddBaseType (pIfaceHdrStructType);

	CStructType* pClassStructType = CreateUnnamedStructType (PackFactor);
	pClassStructType->m_StructTypeKind = EStructType_ClassStruct;
	pClassStructType->m_Tag.Format ("%s.Class", pType->m_Tag.cc ());
	pClassStructType->m_pParentNamespace = pType;
	pClassStructType->CreateField ("!m_objectHdr", GetStdType (EStdType_ObjHdr));
	pClassStructType->CreateField ("!m_iface", pIfaceStructType);

	pType->m_pModule = m_pModule;
	pType->m_Flags |= Flags;
	pType->m_ClassTypeKind = ClassTypeKind;
	pType->m_pVTableStructType = pVTableStructType;
	pType->m_pIfaceStructType = pIfaceStructType;
	pType->m_pClassStructType = pClassStructType;
	return pType;
}

CClassType*
CTypeMgr::GetBoxClassType (CType* pBaseType)
{
	if (pBaseType->m_pBoxClassType)
		return pBaseType->m_pBoxClassType;

	EType BaseTypeKind = pBaseType->GetTypeKind ();
	switch (BaseTypeKind)
	{
	case EType_Void:
		err::SetFormatStringError ("cannot create a box class for 'void'");
		return NULL;

	case EType_Class:
		return (CClassType*) pBaseType;
	}

	CClassType* pType = CreateUnnamedClassType (EClassType_Box);
	pType->m_Tag.Format ("object <%s>", pBaseType->GetTypeString ().cc ());
	pType->m_Signature.Format ("CB%s", pBaseType->GetSignature ().cc ());
	pType->CreateField ("m_value", pBaseType);
	pType->EnsureLayout ();

	pBaseType->m_pBoxClassType = pType;
	return pType;
}

CFunctionArg*
CTypeMgr::CreateFunctionArg (
	const rtl::CString& Name,
	CType* pType,
	uint_t PtrTypeFlags,
	rtl::CBoxListT <CToken>* pInitializer
	)
{
	CFunctionArg* pFunctionArg = AXL_MEM_NEW (CFunctionArg);
	pFunctionArg->m_pModule = m_pModule;
	pFunctionArg->m_Name = Name;
	pFunctionArg->m_QualifiedName = Name;
	pFunctionArg->m_Tag = Name;
	pFunctionArg->m_pType = pType;
	pFunctionArg->m_PtrTypeFlags = PtrTypeFlags;

	if (pInitializer)
		pFunctionArg->m_Initializer.TakeOver (pInitializer);

	m_FunctionArgList.InsertTail (pFunctionArg);

	if (pType->GetTypeKindFlags () & ETypeKindFlag_Import)
		pFunctionArg->m_pType_i = (CImportType*) pType;

	// all this should be calculated during CFunctionType::CalcLayout

	//if (pType->GetTypeKindFlags () & ETypeKindFlag_Import)
	//{
	//	m_pModule->MarkForLayout (pFunctionArg);
	//}
	//else
	//{
	//	bool Result = pFunctionArg->EnsureLayout ();
	//	if (!Result)
	//		return NULL;
	//}

	return pFunctionArg;
}

CFunctionArg*
CTypeMgr::GetSimpleFunctionArg (
	EStorage StorageKind,
	CType* pType,
	uint_t PtrTypeFlags
	)
{
	TFunctionArgTuple* pTuple = GetFunctionArgTuple (pType);

	// this x const x volatile

	size_t i1 = StorageKind == EStorage_This;
	size_t i2 = (PtrTypeFlags & EPtrTypeFlag_Const) != 0;
	size_t i3 = (PtrTypeFlags & EPtrTypeFlag_Volatile) != 0;

	if (pTuple->m_ArgArray [i1] [i2] [i3])
		return pTuple->m_ArgArray [i1] [i2] [i3];

	CFunctionArg* pArg = CreateFunctionArg (rtl::CString (), pType, PtrTypeFlags);
	if (!pArg)
		return NULL;

	pArg->m_StorageKind = StorageKind;

	pTuple->m_ArgArray [i1] [i2] [i3] = pArg;
	return pArg;
}

CFunctionType*
CTypeMgr::GetFunctionType (
	CCallConv* pCallConv,
	CType* pReturnType,
	const rtl::CArrayT <CFunctionArg*>& ArgArray,
	uint_t Flags
	)
{
	ASSERT (pCallConv && pReturnType);

	rtl::CString Signature = CFunctionType::CreateSignature (
		pCallConv,
		pReturnType,
		ArgArray,
		ArgArray.GetCount (),
		Flags
		);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CFunctionType* pType = (CFunctionType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	if (pReturnType->m_Flags & ETypeFlag_StructRet)
		Flags |= ETypeFlag_StructRet;

	CFunctionType* pType = AXL_MEM_NEW (CFunctionType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->m_pCallConv = pCallConv;
	pType->m_pReturnType = pReturnType;
	pType->m_Flags = Flags;
	pType->m_ArgArray = ArgArray;

	m_FunctionTypeList.InsertTail (pType);

	if (pReturnType->GetTypeKindFlags () & ETypeKindFlag_Import)
		pType->m_pReturnType_i = (CImportType*) pReturnType;

	if (!m_pModule->m_NamespaceMgr.GetCurrentScope ())
	{
		m_pModule->MarkForLayout (pType, true);
	}
	else
	{
		bool Result = pType->EnsureLayout ();
		if (!Result)
			return NULL;
	}

	It->m_Value = pType;
	return pType;
}

CFunctionType*
CTypeMgr::GetFunctionType (
	CCallConv* pCallConv,
	CType* pReturnType,
	CType* const* pArgTypeArray,
	size_t ArgCount,
	uint_t Flags
	)
{
	ASSERT (pCallConv && pReturnType);

	rtl::CArrayT <CFunctionArg*> ArgArray;
	ArgArray.SetCount (ArgCount);
	for (size_t i = 0; i < ArgCount; i++)
	{
		CFunctionArg* pArg = GetSimpleFunctionArg (pArgTypeArray [i]);
		if (!pArg)
			return NULL;

		ArgArray [i] = pArg;
	}

	rtl::CString Signature = CFunctionType::CreateSignature (
		pCallConv,
		pReturnType,
		pArgTypeArray,
		ArgCount,
		Flags
		);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CFunctionType* pType = (CFunctionType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	if (pReturnType->m_Flags & ETypeFlag_StructRet)
		Flags |= ETypeFlag_StructRet;

	CFunctionType* pType = AXL_MEM_NEW (CFunctionType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->m_pCallConv = pCallConv;
	pType->m_pReturnType = pReturnType;
	pType->m_Flags = Flags;
	pType->m_ArgArray = ArgArray;

	m_FunctionTypeList.InsertTail (pType);

	if (pReturnType->GetTypeKindFlags () & ETypeKindFlag_Import)
		pType->m_pReturnType_i = (CImportType*) pReturnType;

	if (!m_pModule->m_NamespaceMgr.GetCurrentScope ())
	{
		m_pModule->MarkForLayout (pType, true);
	}
	else
	{
		bool Result = pType->EnsureLayout ();
		if (!Result)
			return NULL;
	}

	It->m_Value = pType;
	return pType;
}

CFunctionType*
CTypeMgr::CreateUserFunctionType (
	CCallConv* pCallConv,
	CType* pReturnType,
	rtl::CBoxListT <CToken>* pThrowCondition,
	const rtl::CArrayT <CFunctionArg*>& ArgArray,
	uint_t Flags
	)
{
	ASSERT (pCallConv && pReturnType);

	rtl::CString Signature = CFunctionType::CreateSignature (
		pCallConv,
		pReturnType,
		ArgArray,
		ArgArray.GetCount (),
		Flags
		);

	if (pReturnType->m_Flags & ETypeFlag_StructRet)
		Flags |= ETypeFlag_StructRet;

	CFunctionType* pType = AXL_MEM_NEW (CFunctionType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = Signature;
	pType->m_pCallConv = pCallConv;
	pType->m_pReturnType = pReturnType;
	pType->m_Flags = Flags | EModuleItemFlag_User;
	pType->m_ArgArray = ArgArray;

	if (pThrowCondition)
	{
		ASSERT (Flags & EFunctionTypeFlag_Throws);
		pType->m_ThrowCondition.TakeOver (pThrowCondition);
	}

	m_FunctionTypeList.InsertTail (pType);

	if (pReturnType->GetTypeKindFlags () & ETypeKindFlag_Import)
		pType->m_pReturnType_i = (CImportType*) pReturnType;

	if (!m_pModule->m_NamespaceMgr.GetCurrentScope ())
	{
		m_pModule->MarkForLayout (pType, true);
	}
	else
	{
		bool Result = pType->EnsureLayout ();
		if (!Result)
			return NULL;
	}

	return pType;
}

CFunctionType*
CTypeMgr::GetMemberMethodType (
	CNamedType* pParentType,
	CFunctionType* pFunctionType,
	uint_t ThisArgPtrTypeFlags
	)
{
	if (!IsClassType (pParentType, EClassType_StdObject)) // std object members are miscellaneous closures
		ThisArgPtrTypeFlags |= EPtrTypeFlag_Safe;

	CType* pThisArgType = pParentType->GetThisArgType (ThisArgPtrTypeFlags);
	CFunctionArg* pThisArg = GetSimpleFunctionArg (EStorage_This, pThisArgType);

	rtl::CArrayT <CFunctionArg*> ArgArray = pFunctionType->m_ArgArray;
	ArgArray.Insert (0, pThisArg);

	CFunctionType* pMemberMethodType;

	if (pFunctionType->m_Flags & EModuleItemFlag_User)
	{
		pMemberMethodType = CreateUserFunctionType (
			pFunctionType->m_pCallConv,
			pFunctionType->m_pReturnType,
			ArgArray,
			pFunctionType->m_Flags
			);

		pMemberMethodType->m_pShortType = pFunctionType;
	}
	else
	{
		pMemberMethodType = GetFunctionType (
			pFunctionType->m_pCallConv,
			pFunctionType->m_pReturnType,
			ArgArray,
			pFunctionType->m_Flags
			);

		pMemberMethodType->m_pShortType = pFunctionType;
	}

	return pMemberMethodType;
}

CFunctionType*
CTypeMgr::GetStdObjectMemberMethodType (CFunctionType* pFunctionType)
{
	if (pFunctionType->m_pStdObjectMemberMethodType)
		return pFunctionType->m_pStdObjectMemberMethodType;

	CClassType* pClassType = (CClassType*) GetStdType (EStdType_ObjectClass);
	pFunctionType->m_pStdObjectMemberMethodType = pClassType->GetMemberMethodType (pFunctionType);
	return pFunctionType->m_pStdObjectMemberMethodType;
}

CPropertyType*
CTypeMgr::GetPropertyType (
	CFunctionType* pGetterType,
	const CFunctionTypeOverload& SetterType,
	uint_t Flags
	)
{
	rtl::CString Signature = CPropertyType::CreateSignature (pGetterType, SetterType, Flags);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CPropertyType* pType = (CPropertyType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	if (SetterType.IsEmpty ())
		Flags |= EPropertyTypeFlag_Const;

	CPropertyType* pType = AXL_MEM_NEW (CPropertyType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->m_pGetterType = pGetterType;
	pType->m_SetterType = SetterType;
	pType->m_Flags = Flags;

	if (Flags & EPropertyTypeFlag_Bindable)
	{
		CFunctionType* pBinderType = (CFunctionType*) GetStdType (EStdType_Binder);
		if (pGetterType->IsMemberMethodType ())
			pBinderType = pBinderType->GetMemberMethodType (pGetterType->GetThisTargetType (), EPtrTypeFlag_Const);

		pType->m_pBinderType = pBinderType;
	}

	m_PropertyTypeList.InsertTail (pType);

	It->m_Value = pType;
	return pType;
}

CPropertyType*
CTypeMgr::GetSimplePropertyType (
	CCallConv* pCallConv,
	CType* pReturnType,
	uint_t Flags
	)
{
	TSimplePropertyTypeTuple* pTuple = GetSimplePropertyTypeTuple (pReturnType);

	uint_t CallConvFlags = pCallConv->GetFlags ();

	size_t i1 =
		(CallConvFlags & ECallConvFlag_Stdcall) ? 2 :
		(CallConvFlags & ECallConvFlag_Cdecl) ? 1 : 0;

	size_t i2 = (Flags & EPropertyTypeFlag_Const) != 0;
	size_t i3 = (Flags & EPropertyTypeFlag_Bindable) != 0;

	if (pTuple->m_PropertyTypeArray [i1] [i2] [i3])
		return pTuple->m_PropertyTypeArray [i1] [i2] [i3];

	CPropertyType* pPropertyType;

	CFunctionType* pGetterType = GetFunctionType (pCallConv, pReturnType, NULL, 0, 0);
	if (Flags & EPropertyTypeFlag_Const)
	{
		pPropertyType = GetPropertyType (pGetterType, NULL, Flags);
	}
	else
	{
		CType* pVoidType = &m_PrimitiveTypeArray [EType_Void];
		CFunctionType* pSetterType = GetFunctionType (pCallConv, pVoidType, &pReturnType, 1, 0);
		pPropertyType = GetPropertyType (pGetterType, pSetterType, Flags);
	}

	pTuple->m_PropertyTypeArray [i1] [i2] [i3] = pPropertyType;
	return pPropertyType;
}

CPropertyType*
CTypeMgr::GetIndexedPropertyType (
	CCallConv* pCallConv,
	CType* pReturnType,
	CType* const* pIndexArgTypeArray,
	size_t IndexArgCount,
	uint_t Flags
	)
{
	CFunctionType* pGetterType = GetFunctionType (pCallConv, pReturnType, pIndexArgTypeArray, IndexArgCount, 0);

	if (Flags & EPropertyTypeFlag_Const)
		return GetPropertyType (pGetterType, NULL, Flags);

	char Buffer [256];
	rtl::CArrayT <CType*> ArgTypeArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	ArgTypeArray.Copy (pIndexArgTypeArray, IndexArgCount);
	ArgTypeArray.Append (pReturnType);

	CType* pVoidType = &m_PrimitiveTypeArray [EType_Void];
	CFunctionType* pSetterType = GetFunctionType (pCallConv, pVoidType, ArgTypeArray, IndexArgCount + 1, 0);
	return GetPropertyType (pGetterType, pSetterType, Flags);
}

CPropertyType*
CTypeMgr::GetIndexedPropertyType (
	CCallConv* pCallConv,
	CType* pReturnType,
	const rtl::CArrayT <CFunctionArg*>& ArgArray,
	uint_t Flags
	)
{
	CFunctionType* pGetterType = GetFunctionType (pCallConv, pReturnType, ArgArray, 0);

	if (Flags & EPropertyTypeFlag_Const)
		return GetPropertyType (pGetterType, NULL, Flags);

	rtl::CArrayT <CFunctionArg*> SetterArgArray = ArgArray;
	SetterArgArray.Append (pReturnType->GetSimpleFunctionArg ());

	CType* pVoidType = &m_PrimitiveTypeArray [EType_Void];
	CFunctionType* pSetterType = GetFunctionType (pCallConv, pVoidType, SetterArgArray, 0);
	return GetPropertyType (pGetterType, pSetterType, Flags);
}

CPropertyType*
CTypeMgr::CreateIndexedPropertyType (
	CCallConv* pCallConv,
	CType* pReturnType,
	const rtl::CArrayT <CFunctionArg*>& ArgArray,
	uint_t Flags
	)
{
	CFunctionType* pGetterType = CreateUserFunctionType (pCallConv, pReturnType, ArgArray, 0);

	if (Flags & EPropertyTypeFlag_Const)
		return GetPropertyType (pGetterType, NULL, Flags);

	rtl::CArrayT <CFunctionArg*> SetterArgArray = ArgArray;
	SetterArgArray.Append (pReturnType->GetSimpleFunctionArg ());

	CType* pVoidType = &m_PrimitiveTypeArray [EType_Void];
	CFunctionType* pSetterType = CreateUserFunctionType (pCallConv, pVoidType, SetterArgArray, 0);
	return GetPropertyType (pGetterType, pSetterType, Flags);
}

CPropertyType*
CTypeMgr::GetMemberPropertyType (
	CNamedType* pParentType,
	CPropertyType* pPropertyType
	)
{
	CFunctionType* pGetterType = GetMemberMethodType (
		pParentType,
		pPropertyType->m_pGetterType,
		EPtrTypeFlag_Const
		);

	size_t SetterTypeOverloadCount = pPropertyType->m_SetterType.GetOverloadCount ();

	char Buffer [256];
	rtl::CArrayT <CFunctionType*> SetterTypeOverloadArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	SetterTypeOverloadArray.SetCount (SetterTypeOverloadCount);

	for (size_t i = 0; i < SetterTypeOverloadCount; i++)
	{
		CFunctionType* pOverloadType = pPropertyType->m_SetterType.GetOverload (i);
		SetterTypeOverloadArray [i] = GetMemberMethodType (pParentType, pOverloadType);
	}

	CPropertyType* pMemberPropertyType = GetPropertyType (
		pGetterType,
		CFunctionTypeOverload (SetterTypeOverloadArray, SetterTypeOverloadCount),
		pPropertyType->m_Flags
		);

	pMemberPropertyType->m_pShortType = pPropertyType;
	return pMemberPropertyType;
}

CPropertyType*
CTypeMgr::GetStdObjectMemberPropertyType (CPropertyType* pPropertyType)
{
	if (pPropertyType->m_pStdObjectMemberPropertyType)
		return pPropertyType->m_pStdObjectMemberPropertyType;

	CClassType* pClassType = (CClassType*) GetStdType (EStdType_ObjectClass);
	pPropertyType->m_pStdObjectMemberPropertyType = pClassType->GetMemberPropertyType (pPropertyType);
	return pPropertyType->m_pStdObjectMemberPropertyType;
}

CPropertyType*
CTypeMgr::GetShortPropertyType (CPropertyType* pPropertyType)
{
	if (pPropertyType->m_pShortType)
		return pPropertyType->m_pShortType;

	if (!pPropertyType->IsMemberPropertyType ())
	{
		pPropertyType->m_pShortType = pPropertyType;
		return pPropertyType;
	}

	CFunctionType* pGetterType = pPropertyType->m_pGetterType->GetShortType ();
	CFunctionTypeOverload SetterType;

	size_t SetterCount = pPropertyType->m_SetterType.GetOverloadCount ();
	for (size_t i = 0; i < SetterCount; i++)
	{
		CFunctionType* pSetterType = pPropertyType->m_SetterType.GetOverload (i)->GetShortType ();
		SetterType.AddOverload (pSetterType);
	}

	pPropertyType->m_pShortType = GetPropertyType (pGetterType, SetterType, pPropertyType->m_Flags);
	return pPropertyType->m_pShortType;
}

CClassType*
CTypeMgr::GetMulticastType (CFunctionPtrType* pFunctionPtrType)
{
	bool Result;

	if (pFunctionPtrType->m_pMulticastType)
		return pFunctionPtrType->m_pMulticastType;

	CType* pReturnType = pFunctionPtrType->GetTargetType ()->GetReturnType ();
	if (pReturnType->GetTypeKind () != EType_Void)
	{
		err::SetFormatStringError ("multicast cannot only return 'void', not '%s'", pReturnType->GetTypeString ().cc ());
		return NULL;
	}

	CMulticastClassType* pType = (CMulticastClassType*) CreateUnnamedClassType (EClassType_Multicast);
	pType->m_pTargetType = pFunctionPtrType;
	pType->m_Flags |= (pFunctionPtrType->m_Flags & ETypeFlag_GcRoot);

	// fields

	pType->m_FieldArray [EMulticastField_Lock] = pType->CreateField ("!m_lock", GetPrimitiveType (EType_Int_p), 0, EPtrTypeFlag_Volatile);
	pType->m_FieldArray [EMulticastField_MaxCount] = pType->CreateField ("!m_maxCount", GetPrimitiveType (EType_SizeT));
	pType->m_FieldArray [EMulticastField_Count] = pType->CreateField ("!m_count", GetPrimitiveType (EType_SizeT));
	pType->m_FieldArray [EMulticastField_PtrArray] = pType->CreateField ("!m_ptrArray", pFunctionPtrType->GetDataPtrType_c ());
	pType->m_FieldArray [EMulticastField_HandleTable] = pType->CreateField ("!m_handleTable", GetPrimitiveType (EType_Int_p));

	CType* pArgType;
	CFunction* pMethod;
	CFunctionType* pMethodType;

	bool IsThin = pFunctionPtrType->GetPtrTypeKind () == EFunctionPtrType_Thin;

	// methods

	pMethodType = GetFunctionType ();
	pMethod = pType->CreateMethod (EStorage_Member, "clear", pMethodType);
	pMethod->m_Tag = "jnc.multicastClear";
	pMethod->m_Flags |= EMulticastMethodFlag_InaccessibleViaEventPtr;
	pType->m_MethodArray [EMulticastMethod_Clear] = pMethod;

	pReturnType = GetPrimitiveType (EType_Int_p);
	pArgType = pFunctionPtrType;
	pMethodType = GetFunctionType (pReturnType, &pArgType, 1);

	pMethod = pType->CreateMethod (EStorage_Member, "set", pMethodType);
	pMethod->m_Tag = IsThin ? "jnc.multicastSet_t" : "jnc.multicastSet";
	pMethod->m_Flags |= EMulticastMethodFlag_InaccessibleViaEventPtr;
	pType->m_MethodArray [EMulticastMethod_Set] = pMethod;

	pMethod = pType->CreateMethod (EStorage_Member, "add", pMethodType);
	pMethod->m_Tag = IsThin ? "jnc.multicastAdd_t" : "jnc.multicastAdd";
	pType->m_MethodArray [EMulticastMethod_Add] = pMethod;

	pReturnType = pFunctionPtrType;
	pArgType = GetPrimitiveType (EType_Int_p);
	pMethodType = GetFunctionType (pReturnType, &pArgType, 1);
	pMethod = pType->CreateMethod (EStorage_Member, "remove", pMethodType);
	pMethod->m_Tag = IsThin ? "jnc.multicastRemove_t" : "jnc.multicastRemove";
	pType->m_MethodArray [EMulticastMethod_Remove] = pMethod;

	pReturnType = pFunctionPtrType->GetNormalPtrType ();
	pMethodType = GetFunctionType (pReturnType, NULL, 0);
	pMethod = pType->CreateMethod (EStorage_Member, "getSnapshot", pMethodType);
	pMethod->m_Tag = "jnc.multicastGetSnapshot";
	pMethod->m_Flags |= EMulticastMethodFlag_InaccessibleViaEventPtr;
	pType->m_MethodArray [EMulticastMethod_GetSnapshot] = pMethod;

	pMethodType = pFunctionPtrType->GetTargetType ();
	pMethod = pType->CreateMethod (EStorage_Member, "call", pMethodType);
	pMethod->m_Flags |= EMulticastMethodFlag_InaccessibleViaEventPtr;
	pType->m_MethodArray [EMulticastMethod_Call] = pMethod;

	// overloaded operators

	pType->m_BinaryOperatorTable.SetCount (EBinOp__Count);
	pType->m_BinaryOperatorTable [EBinOp_RefAssign] = pType->m_MethodArray [EMulticastMethod_Set];
	pType->m_BinaryOperatorTable [EBinOp_AddAssign] = pType->m_MethodArray [EMulticastMethod_Add];
	pType->m_BinaryOperatorTable [EBinOp_SubAssign] = pType->m_MethodArray [EMulticastMethod_Remove];
	pType->m_pCallOperator = pType->m_MethodArray [EMulticastMethod_Call];

	// snapshot closure (snapshot is shared between weak and normal multicasts)

	CMcSnapshotClassType* pSnapshotType = (CMcSnapshotClassType*) CreateUnnamedClassType (EClassType_McSnapshot);
	pSnapshotType->m_pTargetType = pFunctionPtrType->GetUnWeakPtrType ();
	pSnapshotType->m_Flags |= (pFunctionPtrType->m_Flags & ETypeFlag_GcRoot);

	// fields

	pSnapshotType->m_FieldArray [EMcSnapshotField_Count] = pSnapshotType->CreateField ("!m_count", GetPrimitiveType (EType_SizeT));
	pSnapshotType->m_FieldArray [EMcSnapshotField_PtrArray] = pSnapshotType->CreateField ("!m_ptrArray", pFunctionPtrType->GetDataPtrType_c ());

	// call method

	pMethodType = pFunctionPtrType->GetTargetType ();
	pSnapshotType->m_MethodArray [EMcSnapshotMethod_Call] = pSnapshotType->CreateMethod (EStorage_Member, "call", pMethodType);

	pType->m_pSnapshotType = pSnapshotType;

	if (!m_pModule->m_NamespaceMgr.GetCurrentScope ())
	{
		m_pModule->MarkForLayout (pType);
		m_pModule->MarkForLayout (pType->m_pSnapshotType);
	}
	else
	{
		Result =
			pType->CalcLayout () &&
			pType->m_pSnapshotType->CalcLayout ();

		if (!Result)
			return NULL;
	}

	m_pModule->MarkForCompile (pType);
	m_pModule->MarkForCompile (pType->m_pSnapshotType);

	pFunctionPtrType->m_pMulticastType = pType;
	return pType;
}

CClassType*
CTypeMgr::GetReactorInterfaceType (CFunctionType* pStartMethodType)
{
	CType* pReturnType = pStartMethodType->GetReturnType ();
	if (pReturnType->GetTypeKind () != EType_Void)
	{
		err::SetFormatStringError ("reactor must return 'void', not '%s'", pReturnType->GetTypeString ().cc ());
		return NULL;
	}

	if (pStartMethodType->m_pReactorInterfaceType)
		return pStartMethodType->m_pReactorInterfaceType;

	CClassType* pType = CreateUnnamedClassType (EClassType_ReactorIface);
	pType->m_Signature.Format ("CA%s", pStartMethodType->GetTypeString ().cc ());
	CFunction* pStarter = pType->CreateMethod (EStorage_Abstract, "start", pStartMethodType);
	pType->CreateMethod (EStorage_Abstract, "stop", (CFunctionType*) GetStdType (EStdType_SimpleFunction));
	pType->m_pCallOperator = pStarter;
	return pType;
}

CReactorClassType*
CTypeMgr::CreateReactorType (
	const rtl::CString& Name,
	const rtl::CString& QualifiedName,
	CClassType* pIfaceType,
	CClassType* pParentType
	)
{
	CReactorClassType* pType = (CReactorClassType*) CreateClassType (EClassType_Reactor, Name, QualifiedName);

	pType->AddBaseType (pIfaceType);

	// fields

	pType->m_FieldArray [EReactorField_Lock]  = pType->CreateField ("!m_lock", m_pModule->GetSimpleType (EType_Int_p));
	pType->m_FieldArray [EReactorField_State] = pType->CreateField ("!m_state", m_pModule->GetSimpleType (EType_Int_p));

	rtl::CArrayT <CFunction*> VirtualMethodArray = pIfaceType->GetVirtualMethodArray ();
	ASSERT (VirtualMethodArray.GetCount () == 2);

	CFunctionType* pStartMethodType = VirtualMethodArray [0]->GetType ()->GetShortType ();
	rtl::CArrayT <CFunctionArg*> ArgArray = pStartMethodType->GetArgArray ();

	size_t ArgCount = ArgArray.GetCount ();
	for (size_t i = 0; i < ArgCount; i++)
	{
		CFunctionArg* pArg = ArgArray [i];
		CStructField* pField = pType->CreateField (pArg->GetName (), pArg->GetType ());
		if (!pField)
			return NULL;

		if (i == 0)
			pType->m_FirstArgField = pField;
	}

	// constructor & destructor

	if (pParentType)
	{
		CClassPtrType* pParentPtrType = pParentType->GetClassPtrType (EClassPtrType_Normal, EPtrTypeFlag_Safe);

		pType->m_Flags |= ETypeFlag_Child;
		pType->m_FieldArray [EReactorField_Parent] = pType->CreateField ("!m_parent", pParentPtrType);

		CType* pVoidType = &m_PrimitiveTypeArray [EType_Void];
		CFunctionType* pConstructorType = GetFunctionType (pVoidType, (CType**) &pParentPtrType, 1);
		CFunction* pConstructor = m_pModule->m_FunctionMgr.CreateFunction (EFunction_Constructor, pConstructorType);
		pType->AddMethod (pConstructor);
	}
	else
	{
		pType->CreateDefaultMethod (EFunction_Constructor);
	}

	pType->CreateDefaultMethod (EFunction_Destructor);

	// methods

	pType->m_MethodArray [EReactorMethod_Start] = pType->CreateMethod (EStorage_Override, "start", pStartMethodType);
	pType->m_MethodArray [EReactorMethod_Stop]  = pType->CreateMethod (EStorage_Override, "stop", (CFunctionType*) GetStdType (EStdType_SimpleFunction));
	pType->m_pCallOperator = pType->m_MethodArray [EReactorMethod_Start];
	return pType;
}

CFunctionClosureClassType*
CTypeMgr::GetFunctionClosureClassType (
	CFunctionType* pTargetType,
	CFunctionType* pThunkType,
	CType* const* ppArgTypeArray,
	const size_t* pClosureMap,
	size_t ArgCount,
	uint64_t WeakMask
	)
{
	rtl::CString Signature = CClosureClassType::CreateSignature (
		pTargetType,
		pThunkType,
		ppArgTypeArray,
		pClosureMap,
		ArgCount,
		WeakMask
		);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CFunctionClosureClassType* pType = (CFunctionClosureClassType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	CFunctionClosureClassType* pType = (CFunctionClosureClassType*) CreateUnnamedClassType (EClassType_FunctionClosure);
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->m_ClosureMap.Copy (pClosureMap, ArgCount);
	pType->m_WeakMask = WeakMask;

	pType->CreateField ("!m_target", pTargetType->GetFunctionPtrType (EFunctionPtrType_Thin));

	rtl::CString ArgFieldName;

	for (size_t i = 0; i < ArgCount; i++)
	{
		ArgFieldName.Format ("!m_arg%d", i);

		CStructField* pField = pType->CreateField (ArgFieldName, ppArgTypeArray [i]);
		if (WeakMask & (2 << i)) // account for field #0 function ptr
			pField->m_Flags |= EStructFieldFlag_WeakMasked;
	}

	CFunction* pThunkFunction = m_pModule->m_FunctionMgr.CreateFunction (EFunction_Internal, "thunkFunction", pThunkType);
	pType->AddMethod (pThunkFunction);
	pType->m_pThunkFunction = pThunkFunction;

	pType->EnsureLayout ();
	m_pModule->MarkForCompile (pType);

	It->m_Value = pType;

	return pType;
}

CPropertyClosureClassType*
CTypeMgr::GetPropertyClosureClassType (
	CPropertyType* pTargetType,
	CPropertyType* pThunkType,
	CType* const* ppArgTypeArray,
	const size_t* pClosureMap,
	size_t ArgCount,
	uint64_t WeakMask
	)
{
	rtl::CString Signature = CClosureClassType::CreateSignature (
		pTargetType,
		pThunkType,
		ppArgTypeArray,
		pClosureMap,
		ArgCount,
		WeakMask
		);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CPropertyClosureClassType* pType = (CPropertyClosureClassType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	CPropertyClosureClassType* pType = (CPropertyClosureClassType*) CreateUnnamedClassType (EClassType_PropertyClosure);
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->m_ClosureMap.Copy (pClosureMap, ArgCount);
	pType->m_WeakMask = WeakMask;

	pType->CreateField ("!m_target", pTargetType->GetPropertyPtrType (EPropertyPtrType_Thin));

	rtl::CString ArgFieldName;

	for (size_t i = 0; i < ArgCount; i++)
	{
		ArgFieldName.Format ("m_arg%d", i);

		CStructField* pField = pType->CreateField (ArgFieldName, ppArgTypeArray [i]);
		if (WeakMask & (2 << i)) // account for field #0 property ptr
			pField->m_Flags |= EStructFieldFlag_WeakMasked;
	}

	CProperty* pThunkProperty = m_pModule->m_FunctionMgr.CreateProperty (EProperty_Normal, "m_thunkProperty");
	pType->AddProperty (pThunkProperty);
	pType->m_pThunkProperty = pThunkProperty;

	pThunkProperty->Create (pThunkType);

	pType->EnsureLayout ();
	m_pModule->MarkForCompile (pType);

	It->m_Value = pType;

	return pType;
}

CDataClosureClassType*
CTypeMgr::GetDataClosureClassType (
	CType* pTargetType,
	CPropertyType* pThunkType
	)
{
	rtl::CString Signature = CDataClosureClassType::CreateSignature (pTargetType, pThunkType);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CDataClosureClassType* pType = (CDataClosureClassType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	CDataClosureClassType* pType = (CDataClosureClassType*) CreateUnnamedClassType (EClassType_DataClosure);
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->CreateField ("!m_target", pTargetType->GetDataPtrType ());

	CProperty* pThunkProperty = m_pModule->m_FunctionMgr.CreateProperty (EProperty_Normal, "m_thunkProperty");
	pType->AddProperty (pThunkProperty);
	pType->m_pThunkProperty = pThunkProperty;

	pThunkProperty->Create (pThunkType);

	pType->EnsureLayout ();
	m_pModule->MarkForCompile (pType);

	It->m_Value = pType;

	return pType;
}

CDataPtrType*
CTypeMgr::GetDataPtrType (
	CNamespace* pAnchorNamespace,
	CType* pDataType,
	EType TypeKind,
	EDataPtrType PtrTypeKind,
	uint_t Flags
	)
{
	ASSERT ((size_t) PtrTypeKind < EDataPtrType__Count);
	ASSERT (pDataType->GetTypeKind () != EType_NamedImport); // for imports, GetImportPtrType () should be called
	ASSERT (TypeKind != EType_DataRef || pDataType->m_TypeKind != EType_DataRef); // dbl reference

	if (TypeKind == EType_DataPtr && PtrTypeKind == EDataPtrType_Normal)
		Flags |= ETypeFlag_GcRoot | ETypeFlag_StructRet;

	TDataPtrTypeTuple* pTuple;

	if (Flags & EPtrTypeFlag_ConstD)
	{
		ASSERT (pAnchorNamespace != NULL);
		pTuple = GetConstDDataPtrTypeTuple (pAnchorNamespace, pDataType);
	}
	else
	{
		pTuple = GetDataPtrTypeTuple (pDataType);
	}

	// ref x ptrkind x const x volatile x checked/markup

	size_t i1 = TypeKind == EType_DataRef;
	size_t i2 = PtrTypeKind;
	size_t i3 = (Flags & EPtrTypeFlag_Const) ? 0 : 1;
	size_t i4 = (Flags & EPtrTypeFlag_Volatile) ? 0 : 1;
	size_t i5 = (Flags & EPtrTypeFlag_Safe) ? 1 : 0;

	if (pTuple->m_PtrTypeArray [i1] [i2] [i3] [i4] [i5])
		return pTuple->m_PtrTypeArray [i1] [i2] [i3] [i4] [i5];

	size_t Size = PtrTypeKind == EDataPtrType_Normal ? sizeof (TDataPtr) : sizeof (void*);

	CDataPtrType* pType = AXL_MEM_NEW (CDataPtrType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = CDataPtrType::CreateSignature (pDataType, TypeKind, PtrTypeKind, Flags);
	pType->m_TypeKind = TypeKind;
	pType->m_PtrTypeKind = PtrTypeKind;
	pType->m_Size = Size;
	pType->m_AlignFactor = sizeof (void*);
	pType->m_pTargetType = pDataType;
	pType->m_pAnchorNamespace = (Flags & EPtrTypeFlag_ConstD) ? pAnchorNamespace : NULL;
	pType->m_Flags = Flags;

	m_DataPtrTypeList.InsertTail (pType);
	pTuple->m_PtrTypeArray [i1] [i2] [i3] [i4] [i5] = pType;
	return pType;
}

CStructType*
CTypeMgr::GetDataPtrStructType (CType* pDataType)
{
	TDataPtrTypeTuple* pTuple = GetDataPtrTypeTuple (pDataType);
	if (pTuple->m_pPtrStructType)
		return pTuple->m_pPtrStructType;

	CStructType* pType = CreateUnnamedStructType ();
	pType->m_Tag.Format ("DataPtr <%s>", pDataType->GetTypeString ().cc ());
	pType->CreateField ("!m_p", pDataType->GetDataPtrType_c ());
	pType->CreateField ("!m_rangeBegin", GetStdType (EStdType_BytePtr));
	pType->CreateField ("!m_rangeEnd", GetStdType (EStdType_BytePtr));
	pType->CreateField ("!m_object", GetStdType (EStdType_ObjHdrPtr));
	pType->EnsureLayout ();

	pTuple->m_pPtrStructType = pType;
	return pType;
}

CClassPtrType*
CTypeMgr::GetClassPtrType (
	CNamespace* pAnchorNamespace,
	CClassType* pClassType,
	EType TypeKind,
	EClassPtrType PtrTypeKind,
	uint_t Flags
	)
{
	ASSERT ((size_t) PtrTypeKind < EClassPtrType__Count);
	ASSERT (!(Flags & (EPtrTypeFlag_ConstD | EPtrTypeFlag_EventD)) || pAnchorNamespace != NULL);

	if (TypeKind == EType_ClassPtr)
		Flags |= ETypeFlag_GcRoot;

	TClassPtrTypeTuple* pTuple;

	if (Flags & EPtrTypeFlag_ConstD)
	{
		ASSERT (pAnchorNamespace != NULL);
		pTuple = GetConstDClassPtrTypeTuple (pAnchorNamespace, pClassType);
	}
	else if (Flags & EPtrTypeFlag_EventD)
	{
		ASSERT (pAnchorNamespace != NULL && pClassType->GetClassTypeKind () == EClassType_Multicast);
		pTuple = GetEventDClassPtrTypeTuple (pAnchorNamespace, (CMulticastClassType*) pClassType);
	}
	else if (Flags & EPtrTypeFlag_Event)
	{
		ASSERT (pClassType->GetClassTypeKind () == EClassType_Multicast);
		pTuple = GetEventClassPtrTypeTuple ((CMulticastClassType*) pClassType);
	}
	else
	{
		pTuple = GetClassPtrTypeTuple (pClassType);
	}

	// ref x ptrkind x const x volatile x checked

	size_t i1 = TypeKind == EType_ClassRef;
	size_t i2 = PtrTypeKind;
	size_t i3 = (Flags & EPtrTypeFlag_Const) ? 0 : 1;
	size_t i4 = (Flags & EPtrTypeFlag_Volatile) ? 0 : 1;
	size_t i5 = (Flags & EPtrTypeFlag_Safe) ? 0 : 1;

	if (pTuple->m_PtrTypeArray [i1] [i2] [i3] [i4] [i5])
		return pTuple->m_PtrTypeArray [i1] [i2] [i3] [i4] [i5];

	CClassPtrType* pType = AXL_MEM_NEW (CClassPtrType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = CClassPtrType::CreateSignature (pClassType, TypeKind, PtrTypeKind, Flags);
	pType->m_TypeKind = TypeKind;
	pType->m_PtrTypeKind = PtrTypeKind;
	pType->m_pTargetType = pClassType;
	pType->m_pAnchorNamespace = (Flags & (EPtrTypeFlag_ConstD | EPtrTypeFlag_EventD)) ? pAnchorNamespace : NULL;
	pType->m_Flags = Flags;

	m_ClassPtrTypeList.InsertTail (pType);
	pTuple->m_PtrTypeArray [i1] [i2] [i3] [i4] [i5] = pType;
	return pType;
}

CFunctionPtrType*
CTypeMgr::GetFunctionPtrType (
	CFunctionType* pFunctionType,
	EType TypeKind,
	EFunctionPtrType PtrTypeKind,
	uint_t Flags
	)
{
	ASSERT (TypeKind == EType_FunctionPtr || TypeKind == EType_FunctionRef);
	ASSERT ((size_t) PtrTypeKind < EFunctionPtrType__Count);

	if (TypeKind == EType_FunctionPtr && PtrTypeKind != EFunctionPtrType_Thin)
		Flags |= ETypeFlag_GcRoot | ETypeFlag_StructRet;

	TFunctionPtrTypeTuple* pTuple = GetFunctionPtrTypeTuple (pFunctionType);

	// ref x kind x checked

	size_t i1 = TypeKind == EType_FunctionRef;
	size_t i2 = PtrTypeKind;
	size_t i3 = (Flags & EPtrTypeFlag_Safe) ? 0 : 1;

	if (pTuple->m_PtrTypeArray [i1] [i2] [i3])
		return pTuple->m_PtrTypeArray [i1] [i2] [i3];

	size_t Size = PtrTypeKind == EFunctionPtrType_Thin ? sizeof (void*) : sizeof (TFunctionPtr);

	CFunctionPtrType* pType = AXL_MEM_NEW (CFunctionPtrType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = CFunctionPtrType::CreateSignature (pFunctionType, TypeKind, PtrTypeKind, Flags);
	pType->m_TypeKind = TypeKind;
	pType->m_PtrTypeKind = PtrTypeKind;
	pType->m_Size = Size;
	pType->m_AlignFactor = sizeof (void*);
	pType->m_pTargetType = pFunctionType;
	pType->m_Flags = Flags;

	m_FunctionPtrTypeList.InsertTail (pType);
	pTuple->m_PtrTypeArray [i1] [i2] [i3] = pType;
	return pType;
}

CStructType*
CTypeMgr::GetFunctionPtrStructType (CFunctionType* pFunctionType)
{
	TFunctionPtrTypeTuple* pTuple = GetFunctionPtrTypeTuple (pFunctionType);
	if (pTuple->m_pPtrStructType)
		return pTuple->m_pPtrStructType;

	rtl::CString Signature;
	Signature.Format ("SP%s", pFunctionType->GetSignature ().cc ());

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CStructType* pType = (CStructType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	CFunctionType* pStdObjectMemberMethodType = pFunctionType->GetStdObjectMemberMethodType ();

	CStructType* pType = CreateUnnamedStructType ();
	pType->m_Tag.Format ("FunctionPtr <%s>", pFunctionType->GetTypeString ().cc ());
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->CreateField ("!m_p", pStdObjectMemberMethodType->GetFunctionPtrType (EFunctionPtrType_Thin));
	pType->CreateField ("!m_closure", GetStdType (EStdType_ObjectPtr));
	pType->EnsureLayout ();

	pTuple->m_pPtrStructType = pType;
	It->m_Value = pType;
	return pType;
}

CPropertyPtrType*
CTypeMgr::GetPropertyPtrType (
	CNamespace* pAnchorNamespace,
	CPropertyType* pPropertyType,
	EType TypeKind,
	EPropertyPtrType PtrTypeKind,
	uint_t Flags
	)
{
	ASSERT (TypeKind == EType_PropertyPtr || TypeKind == EType_PropertyRef);
	ASSERT ((size_t) PtrTypeKind < EPropertyPtrType__Count);

	if (TypeKind == EType_PropertyPtr && PtrTypeKind != EPropertyPtrType_Thin)
		Flags |= ETypeFlag_GcRoot | ETypeFlag_StructRet;

	TPropertyPtrTypeTuple* pTuple;

	if (Flags & EPtrTypeFlag_ConstD)
	{
		ASSERT (pAnchorNamespace != NULL);
		pTuple = GetConstDPropertyPtrTypeTuple (pAnchorNamespace, pPropertyType);
	}
	else
	{
		pTuple = GetPropertyPtrTypeTuple (pPropertyType);
	}

	// ref x kind x checked

	size_t i1 = TypeKind == EType_PropertyRef;
	size_t i2 = PtrTypeKind;
	size_t i3 = (Flags & EPtrTypeFlag_Safe) ? 0 : 1;

	if (pTuple->m_PtrTypeArray [i1] [i2] [i3])
		return pTuple->m_PtrTypeArray [i1] [i2] [i3];

	size_t Size = PtrTypeKind == EPropertyPtrType_Thin ? sizeof (void*) : sizeof (TPropertyPtr);

	CPropertyPtrType* pType = AXL_MEM_NEW (CPropertyPtrType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = CPropertyPtrType::CreateSignature (pPropertyType, TypeKind, PtrTypeKind, Flags);
	pType->m_TypeKind = TypeKind;
	pType->m_PtrTypeKind = PtrTypeKind;
	pType->m_Size = Size;
	pType->m_AlignFactor = sizeof (void*);
	pType->m_pTargetType = pPropertyType;
	pType->m_pAnchorNamespace = (Flags & EPtrTypeFlag_ConstD) ? pAnchorNamespace : NULL;
	pType->m_Flags = Flags;

	m_PropertyPtrTypeList.InsertTail (pType);
	pTuple->m_PtrTypeArray [i1] [i2] [i3] = pType;
	return pType;
}

CStructType*
CTypeMgr::GetPropertyVTableStructType (CPropertyType* pPropertyType)
{
	if (pPropertyType->m_pVTableStructType)
		return pPropertyType->m_pVTableStructType;

	CStructType* pType = CreateUnnamedStructType ();
	pType->m_Tag.Format ("%s.Vtbl", pPropertyType->GetTypeString ().cc ());

	if (pPropertyType->GetFlags () & EPropertyTypeFlag_Bindable)
		pType->CreateField ("!m_binder", pPropertyType->m_pBinderType->GetFunctionPtrType (EFunctionPtrType_Thin));

	pType->CreateField ("!m_getter", pPropertyType->m_pGetterType->GetFunctionPtrType (EFunctionPtrType_Thin));

	rtl::CString SetterFieldName;

	size_t SetterTypeOverloadCount = pPropertyType->m_SetterType.GetOverloadCount ();
	for (size_t i = 0; i < SetterTypeOverloadCount; i++)
	{
		SetterFieldName.Format ("!m_setter%d", i);

		CFunctionType* pSetterType = pPropertyType->m_SetterType.GetOverload (i);
		pType->CreateField (SetterFieldName, pSetterType->GetFunctionPtrType (EFunctionPtrType_Thin));
	}

	pType->EnsureLayout ();

	pPropertyType->m_pVTableStructType = pType;
	return pType;
}

CStructType*
CTypeMgr::GetPropertyPtrStructType (CPropertyType* pPropertyType)
{
	TPropertyPtrTypeTuple* pTuple = GetPropertyPtrTypeTuple (pPropertyType);
	if (pTuple->m_pPtrStructType)
		return pTuple->m_pPtrStructType;

	rtl::CString Signature;
	Signature.Format ("SP%s", pPropertyType->GetSignature ().cc ());

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CStructType* pType = (CStructType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	CPropertyType* pStdObjectMemberPropertyType = pPropertyType->GetStdObjectMemberPropertyType ();

	CStructType* pType = CreateUnnamedStructType ();
	pType->m_Tag.Format ("PropertyPtr <%s>", pPropertyType->GetTypeString ().cc ());
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->CreateField ("!m_p", pStdObjectMemberPropertyType->GetVTableStructType ()->GetDataPtrType_c ());
	pType->CreateField ("!m_closure", GetStdType (EStdType_ObjectPtr));
	pType->EnsureLayout ();

	pTuple->m_pPtrStructType = pType;
	It->m_Value = pType;
	return pType;
}

CNamedImportType*
CTypeMgr::GetNamedImportType (
	const CQualifiedName& Name,
	CNamespace* pAnchorNamespace
	)
{
	rtl::CString Signature = CNamedImportType::CreateSignature (Name, pAnchorNamespace);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CNamedImportType* pType = (CNamedImportType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	CNamedImportType* pType = AXL_MEM_NEW (CNamedImportType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->m_Name = Name;
	pType->m_QualifiedName = pAnchorNamespace->CreateQualifiedName (Name);
	pType->m_pAnchorNamespace = pAnchorNamespace;
	pType->m_pModule = m_pModule;

	m_NamedImportTypeList.InsertTail (pType);
	It->m_Value = pType;

	return pType;
}

CImportPtrType*
CTypeMgr::GetImportPtrType (
	CNamedImportType* pNamedImportType,
	uint_t TypeModifiers,
	uint_t Flags
	)
{
	rtl::CString Signature = CImportPtrType::CreateSignature (
		pNamedImportType,
		TypeModifiers,
		Flags
		);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CImportPtrType* pType = (CImportPtrType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	CImportPtrType* pType = AXL_MEM_NEW (CImportPtrType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->m_pTargetType = pNamedImportType;
	pType->m_TypeModifiers = TypeModifiers;
	pType->m_Flags = Flags;

	m_ImportPtrTypeList.InsertTail (pType);
	It->m_Value = pType;

	return pType;
}

CImportIntModType*
CTypeMgr::GetImportIntModType (
	CNamedImportType* pNamedImportType,
	uint_t TypeModifiers,
	uint_t Flags
	)
{
	rtl::CString Signature = CImportIntModType::CreateSignature (
		pNamedImportType,
		TypeModifiers,
		Flags
		);

	rtl::CStringHashTableMapIteratorT <CType*> It = m_TypeMap.Goto (Signature);
	if (It->m_Value)
	{
		CImportIntModType* pType = (CImportIntModType*) It->m_Value;
		ASSERT (pType->m_Signature == Signature);
		return pType;
	}

	CImportIntModType* pType = AXL_MEM_NEW (CImportIntModType);
	pType->m_pModule = m_pModule;
	pType->m_Signature = Signature;
	pType->m_TypeMapIt = It;
	pType->m_pImportType = pNamedImportType;
	pType->m_TypeModifiers = TypeModifiers;
	pType->m_Flags = Flags;

	m_ImportIntModTypeList.InsertTail (pType);
	It->m_Value = pType;

	return pType;
}

CType*
CTypeMgr::GetCheckedPtrType (CType* pType)
{
	EType TypeKind = pType->GetTypeKind ();
	switch (TypeKind)
	{
	case EType_DataPtr:
		return ((CDataPtrType*) pType)->GetCheckedPtrType ();

	case EType_ClassPtr:
		return ((CClassPtrType*) pType)->GetCheckedPtrType ();

	case EType_FunctionPtr:
		return ((CFunctionPtrType*) pType)->GetCheckedPtrType ();

	case EType_PropertyPtr:
		return ((CPropertyPtrType*) pType)->GetCheckedPtrType ();

	case EType_ImportPtr:
		return ((CImportPtrType*) pType)->GetCheckedPtrType ();

	default:
		ASSERT (false);
		return pType;
	}
}

CStructType*
CTypeMgr::GetGcShadowStackFrameMapType (size_t RootCount)
{
	ASSERT (RootCount);

	size_t Count = m_GcShadowStackFrameTypeArray.GetCount ();
	if (RootCount >= Count)
		m_GcShadowStackFrameTypeArray.SetCount (RootCount + 1);

	TGcShadowStackFrameTypePair* pPair = &m_GcShadowStackFrameTypeArray [RootCount];
	if (pPair->m_pGcShadowStackFrameMapType)
		return pPair->m_pGcShadowStackFrameMapType;

	CArrayType* pArrayType = GetArrayType (GetStdType (EStdType_BytePtr), RootCount);
	CStructType* pType = CreateStructType ("GcShadowStackFrameMap", "jnc.GcShadowStackFrameMap");
	pType->CreateField ("!m_count", GetPrimitiveType (EType_SizeT));
	pType->CreateField ("!m_gcRootTypeArray", pArrayType);
	pType->EnsureLayout ();

	pPair->m_pGcShadowStackFrameMapType = pType;
	return pType;
}

CStructType*
CTypeMgr::GetGcShadowStackFrameType (size_t RootCount)
{
	CStructType* pMapType = GetGcShadowStackFrameMapType (RootCount);

	ASSERT (RootCount < m_GcShadowStackFrameTypeArray.GetCount ());

	TGcShadowStackFrameTypePair* pPair = &m_GcShadowStackFrameTypeArray [RootCount];
	if (pPair->m_pGcShadowStackFrameType)
		return pPair->m_pGcShadowStackFrameType;

	CArrayType* pArrayType = GetArrayType (GetStdType (EStdType_BytePtr), RootCount);
	CStructType* pType = CreateStructType ("GcShadowStackFrame", "jnc.GcShadowStackFrame");
	pType->CreateField ("!m_prev", GetStdType (EStdType_BytePtr));
	pType->CreateField ("!m_map", pMapType->GetDataPtrType_c ());
	pType->CreateField ("!m_gcRootArray", pArrayType);
	pType->EnsureLayout ();

	pPair->m_pGcShadowStackFrameType = pType;
	return pType;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

TDualPtrTypeTuple*
CTypeMgr::GetDualPtrTypeTuple (
	CNamespace* pAnchorNamespace,
	CType* pType
	)
{
	rtl::CString Signature = pType->GetSignature ();
	rtl::CStringHashTableMapIteratorT <TDualPtrTypeTuple*> It = pAnchorNamespace->m_DualPtrTypeTupleMap.Goto (Signature);
	if (It->m_Value)
		return It->m_Value;

	TDualPtrTypeTuple* pDualPtrTypeTuple = AXL_MEM_NEW (TDualPtrTypeTuple);
	m_DualPtrTypeTupleList.InsertTail (pDualPtrTypeTuple);
	It->m_Value = pDualPtrTypeTuple;
	return pDualPtrTypeTuple;
}

TSimplePropertyTypeTuple*
CTypeMgr::GetSimplePropertyTypeTuple (CType* pType)
{
	if (pType->m_pSimplePropertyTypeTuple)
		return pType->m_pSimplePropertyTypeTuple;

	TSimplePropertyTypeTuple* pTuple = AXL_MEM_NEW (TSimplePropertyTypeTuple);
	pType->m_pSimplePropertyTypeTuple = pTuple;
	m_SimplePropertyTypeTupleList.InsertTail (pTuple);
	return pTuple;
}

TFunctionArgTuple*
CTypeMgr::GetFunctionArgTuple (CType* pType)
{
	if (pType->m_pFunctionArgTuple)
		return pType->m_pFunctionArgTuple;

	TFunctionArgTuple* pTuple = AXL_MEM_NEW (TFunctionArgTuple);
	pType->m_pFunctionArgTuple = pTuple;
	m_FunctionArgTupleList.InsertTail (pTuple);
	return pTuple;
}

TDataPtrTypeTuple*
CTypeMgr::GetDataPtrTypeTuple (CType* pType)
{
	if (pType->m_pDataPtrTypeTuple)
		return pType->m_pDataPtrTypeTuple;

	TDataPtrTypeTuple* pTuple = AXL_MEM_NEW (TDataPtrTypeTuple);
	pType->m_pDataPtrTypeTuple = pTuple;
	m_DataPtrTypeTupleList.InsertTail (pTuple);
	return pTuple;
}

TDataPtrTypeTuple*
CTypeMgr::GetConstDDataPtrTypeTuple (
	CNamespace* pAnchorNamespace,
	CType* pType
	)
{
	TDualPtrTypeTuple* pDualPtrTypeTuple = GetDualPtrTypeTuple (pAnchorNamespace, pType);
	if (pDualPtrTypeTuple->m_pConstDDataPtrTypeTuple)
		return pDualPtrTypeTuple->m_pConstDDataPtrTypeTuple;

	TDataPtrTypeTuple* pTuple = AXL_MEM_NEW (TDataPtrTypeTuple);
	pDualPtrTypeTuple->m_pConstDDataPtrTypeTuple = pTuple;
	m_DataPtrTypeTupleList.InsertTail (pTuple);
	return pTuple;
}

TClassPtrTypeTuple*
CTypeMgr::GetClassPtrTypeTuple (CClassType* pClassType)
{
	if (pClassType->m_pClassPtrTypeTuple)
		return pClassType->m_pClassPtrTypeTuple;

	TClassPtrTypeTuple* pTuple = AXL_MEM_NEW (TClassPtrTypeTuple);
	pClassType->m_pClassPtrTypeTuple = pTuple;
	m_ClassPtrTypeTupleList.InsertTail (pTuple);
	return pTuple;
}

TClassPtrTypeTuple*
CTypeMgr::GetConstDClassPtrTypeTuple (
	CNamespace* pAnchorNamespace,
	CClassType* pClassType
	)
{
	TDualPtrTypeTuple* pDualPtrTypeTuple = GetDualPtrTypeTuple (pAnchorNamespace, pClassType);
	if (pDualPtrTypeTuple->m_pConstDClassPtrTypeTuple)
		return pDualPtrTypeTuple->m_pConstDClassPtrTypeTuple;

	TClassPtrTypeTuple* pTuple = AXL_MEM_NEW (TClassPtrTypeTuple);
	pDualPtrTypeTuple->m_pConstDClassPtrTypeTuple = pTuple;
	m_ClassPtrTypeTupleList.InsertTail (pTuple);
	return pTuple;
}

TClassPtrTypeTuple*
CTypeMgr::GetEventClassPtrTypeTuple (CMulticastClassType* pClassType)
{
	if (pClassType->m_pEventClassPtrTypeTuple)
		return pClassType->m_pEventClassPtrTypeTuple;

	TClassPtrTypeTuple* pTuple = AXL_MEM_NEW (TClassPtrTypeTuple);
	pClassType->m_pEventClassPtrTypeTuple = pTuple;
	m_ClassPtrTypeTupleList.InsertTail (pTuple);
	return pTuple;
}

TClassPtrTypeTuple*
CTypeMgr::GetEventDClassPtrTypeTuple (
	CNamespace* pAnchorNamespace,
	CMulticastClassType* pClassType
	)
{
	TDualPtrTypeTuple* pDualPtrTypeTuple = GetDualPtrTypeTuple (pAnchorNamespace, pClassType);
	if (pDualPtrTypeTuple->m_pEventDClassPtrTypeTuple)
		return pDualPtrTypeTuple->m_pEventDClassPtrTypeTuple;

	TClassPtrTypeTuple* pTuple = AXL_MEM_NEW (TClassPtrTypeTuple);
	pDualPtrTypeTuple->m_pEventDClassPtrTypeTuple = pTuple;
	m_ClassPtrTypeTupleList.InsertTail (pTuple);
	return pTuple;
}

TFunctionPtrTypeTuple*
CTypeMgr::GetFunctionPtrTypeTuple (CFunctionType* pFunctionType)
{
	if (pFunctionType->m_pFunctionPtrTypeTuple)
		return pFunctionType->m_pFunctionPtrTypeTuple;

	TFunctionPtrTypeTuple* pTuple = AXL_MEM_NEW (TFunctionPtrTypeTuple);
	pFunctionType->m_pFunctionPtrTypeTuple = pTuple;
	m_FunctionPtrTypeTupleList.InsertTail (pTuple);
	return pTuple;
}

TPropertyPtrTypeTuple*
CTypeMgr::GetPropertyPtrTypeTuple (CPropertyType* pPropertyType)
{
	if (pPropertyType->m_pPropertyPtrTypeTuple)
		return pPropertyType->m_pPropertyPtrTypeTuple;

	TPropertyPtrTypeTuple* pTuple = AXL_MEM_NEW (TPropertyPtrTypeTuple);
	pPropertyType->m_pPropertyPtrTypeTuple = pTuple;
	m_PropertyPtrTypeTupleList.InsertTail (pTuple);
	return pTuple;
}

TPropertyPtrTypeTuple*
CTypeMgr::GetConstDPropertyPtrTypeTuple (
	CNamespace* pAnchorNamespace,
	CPropertyType* pPropertyType
	)
{
	TDualPtrTypeTuple* pDualPtrTypeTuple = GetDualPtrTypeTuple (pAnchorNamespace, pPropertyType);
	if (pDualPtrTypeTuple->m_pConstDPropertyPtrTypeTuple)
		return pDualPtrTypeTuple->m_pConstDPropertyPtrTypeTuple;

	TPropertyPtrTypeTuple* pTuple = AXL_MEM_NEW (TPropertyPtrTypeTuple);
	pDualPtrTypeTuple->m_pConstDPropertyPtrTypeTuple = pTuple;
	m_PropertyPtrTypeTupleList.InsertTail (pTuple);
	return pTuple;
}


//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
CTypeMgr::SetupAllPrimitiveTypes ()
{
	SetupPrimitiveType (EType_Void,      0, "v");
	SetupPrimitiveType (EType_Variant,   sizeof (TVariant), "z");
	SetupPrimitiveType (EType_Bool,      1, "b");
	SetupPrimitiveType (EType_Int8,      1, "is1");
	SetupPrimitiveType (EType_Int8_u,    1, "iu1");
	SetupPrimitiveType (EType_Int16,     2, "is2");
	SetupPrimitiveType (EType_Int16_u,   2, "iu2");
	SetupPrimitiveType (EType_Int32,     4, "is4");
	SetupPrimitiveType (EType_Int32_u,   4, "iu4");
	SetupPrimitiveType (EType_Int64,     8, "is8");
	SetupPrimitiveType (EType_Int64_u,   8, "iu8");
	SetupPrimitiveType (EType_Int16_be,  2, "ibs2");
	SetupPrimitiveType (EType_Int16_beu, 2, "ibu2");
	SetupPrimitiveType (EType_Int32_be,  4, "ibs4");
	SetupPrimitiveType (EType_Int32_beu, 4, "ibu4");
	SetupPrimitiveType (EType_Int64_be,  8, "ibs8");
	SetupPrimitiveType (EType_Int64_beu, 8, "ibu8");
	SetupPrimitiveType (EType_Float,     4, "f4");
	SetupPrimitiveType (EType_Double,    8, "f8");
}

void
CTypeMgr::SetupPrimitiveType (
	EType TypeKind,
	size_t Size,
	const char* pSignature
	)
{
	ASSERT (TypeKind < EType__PrimitiveTypeCount);

	CType* pType = &m_PrimitiveTypeArray [TypeKind];
	pType->m_pModule = m_pModule;
	pType->m_TypeKind = TypeKind;
	pType->m_Flags = ETypeFlag_Pod | EModuleItemFlag_LayoutReady;
	pType->m_Size = Size;
	pType->m_AlignFactor = Size;
	pType->m_Signature = pSignature;
	pType->m_pLlvmType = NULL;
	pType->m_LlvmDiType = llvm::DIType ();
	pType->m_pSimplePropertyTypeTuple = NULL;
	pType->m_pFunctionArgTuple = NULL;
	pType->m_pDataPtrTypeTuple = NULL;
	pType->m_pBoxClassType = NULL;
}

void
CTypeMgr::SetupCallConvTable ()
{
	m_CallConvTable [ECallConv_Jnccall_msc32] = &m_JnccallCallConv_msc32;
	m_CallConvTable [ECallConv_Jnccall_msc64] = &m_JnccallCallConv_msc64;
	m_CallConvTable [ECallConv_Jnccall_gcc32] = &m_JnccallCallConv_gcc32;
	m_CallConvTable [ECallConv_Jnccall_gcc64] = &m_JnccallCallConv_gcc64;
	m_CallConvTable [ECallConv_Cdecl_msc32]   = &m_CdeclCallConv_msc32;
	m_CallConvTable [ECallConv_Cdecl_msc64]   = &m_CdeclCallConv_msc64;
	m_CallConvTable [ECallConv_Cdecl_gcc32]   = &m_CdeclCallConv_gcc32;
	m_CallConvTable [ECallConv_Cdecl_gcc64]   = &m_CdeclCallConv_gcc64;
	m_CallConvTable [ECallConv_Stdcall_msc32] = &m_StdcallCallConv_msc32;
	m_CallConvTable [ECallConv_Stdcall_gcc32] = &m_StdcallCallConv_gcc32;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

CStructType*
CTypeMgr::CreateObjHdrType ()
{
	CStructType* pType = CreateStructType ("ObjHdr", "jnc.ObjHdr");
	pType->CreateField ("!m_scopeLevel", GetPrimitiveType (EType_SizeT));
	pType->CreateField ("!m_root", pType->GetDataPtrType_c ());
	pType->CreateField ("!m_type", GetStdType (EStdType_BytePtr));
	pType->CreateField ("!m_flags", GetPrimitiveType (EType_Int_p));
	pType->EnsureLayout ();
	return pType;
}

CClassType*
CTypeMgr::CreateObjectType ()
{
	CClassType* pType = CreateUnnamedClassType (EClassType_StdObject);
	pType->m_Tag = "object";
	pType->m_Signature = "CO"; // special signature to ensure type equality between modules
	pType->EnsureLayout ();
	return pType;
}

CStructType*
CTypeMgr::CreateReactorBindSiteType ()
{
	CStructType* pType = CreateStructType ("ReactorBindSite", "jnc.ReactorBindSite");
	pType->CreateField ("!m_event", GetStdType (EStdType_SimpleEventPtr));
	pType->CreateField ("!m_cookie", GetPrimitiveType (EType_Int_p));
	pType->EnsureLayout ();
	return pType;
}

CClassType*
CTypeMgr::CreateSchedulerType ()
{
	CFunctionType* pFunctionType = (CFunctionType*) GetStdType (EStdType_SimpleFunction);
	CType* pReturnType = GetPrimitiveType (EType_Void);
	CType* pArgType = pFunctionType->GetFunctionPtrType ();
	CFunctionType* pScheduleType = GetFunctionType (pReturnType, &pArgType, 1);

	CClassType* pType = CreateClassType ("Scheduler", "jnc.Scheduler");
	pType->CreateMethod (EStorage_Abstract, "schedule", pScheduleType);
	pType->EnsureLayout ();
	return pType;
}

CStructType*
CTypeMgr::CreateFmtLiteralType ()
{
	CStructType* pType = CreateStructType ("FmtLiteral", "jnc.FmtLiteral");
	pType->CreateField ("!m_p", GetPrimitiveType (EType_Char)->GetDataPtrType_c ());
	pType->CreateField ("!m_maxLength", GetPrimitiveType (EType_SizeT));
	pType->CreateField ("!m_length", GetPrimitiveType (EType_SizeT));
	pType->EnsureLayout ();
	return pType;
}

CStructType*
CTypeMgr::CreateGuidType ()
{
	CStructType* pType = CreateStructType ("Guid", "jnc.Guid");
	pType->CreateField ("m_data1", GetPrimitiveType (EType_Int32_u));
	pType->CreateField ("m_data2", GetPrimitiveType (EType_Int16_u));
	pType->CreateField ("m_data3", GetPrimitiveType (EType_Int16_u));
	pType->CreateField ("m_data4", GetPrimitiveType (EType_Int8_u)->GetArrayType (8));
	pType->EnsureLayout ();
	return pType;
}

CStructType*
CTypeMgr::CreateErrorType ()
{
	CStructType* pType = CreateStructType ("Error", "jnc.Error");
	pType->CreateField ("m_size", GetPrimitiveType (EType_Int32_u));
	pType->CreateField ("m_guid", GetStdType (EStdType_Guid));
	pType->CreateField ("m_code", GetPrimitiveType (EType_Int32_u));

	CProperty* pDescription = m_pModule->m_FunctionMgr.CreateProperty ("m_description", "jnc.Error.m_description");
	pType->AddProperty (pDescription);

	CType* pReturnType = GetPrimitiveType (EType_Char)->GetDataPtrType (EDataPtrType_Normal, EPtrTypeFlag_Const);
	CPropertyType* pPropertyType = GetSimplePropertyType (pReturnType, EPropertyTypeFlag_Const);
	pDescription->Create (pPropertyType);

	pType->EnsureLayout ();
	return pType;
}

//.............................................................................

} // namespace jnc {
