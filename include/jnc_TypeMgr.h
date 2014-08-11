// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"
#include "jnc_VariantType.h"
#include "jnc_ArrayType.h"
#include "jnc_BitFieldType.h"
#include "jnc_EnumType.h"
#include "jnc_StructType.h"
#include "jnc_UnionType.h"
#include "jnc_ClassType.h"
#include "jnc_FunctionType.h"
#include "jnc_PropertyType.h"
#include "jnc_DataPtrType.h"
#include "jnc_ClassPtrType.h"
#include "jnc_FunctionPtrType.h"
#include "jnc_PropertyPtrType.h"
#include "jnc_ImportType.h"
#include "jnc_ReactorClassType.h"
#include "jnc_ClosureClassType.h"
#include "jnc_McSnapshotClassType.h"
#include "jnc_MulticastClassType.h"
#include "jnc_JnccallCallConv.h"
#include "jnc_CallConv_msc32.h"
#include "jnc_CallConv_gcc32.h"
#include "jnc_CdeclCallConv_gcc64.h"
#include "jnc_CdeclCallConv_msc64.h"

namespace jnc {

class CModule;

//.............................................................................

struct TDualPtrTypeTuple: rtl::TListLink
{
	union
	{
		TDataPtrTypeTuple* m_pConstDDataPtrTypeTuple;
		TClassPtrTypeTuple* m_pConstDClassPtrTypeTuple;
		TPropertyPtrTypeTuple* m_pConstDPropertyPtrTypeTuple;
	};

	TClassPtrTypeTuple* m_pEventDClassPtrTypeTuple;
};

//.............................................................................

class CTypeMgr
{
	friend class CModule;

protected:
	struct TGcShadowStackFrameTypePair
	{
		CStructType* m_pGcShadowStackFrameType;
		CStructType* m_pGcShadowStackFrameMapType;
	};

protected:
	CModule* m_pModule;

	CType m_PrimitiveTypeArray [EType__PrimitiveTypeCount];
	CType* m_StdTypeArray [EStdType__Count];
	CLazyStdType* m_LazyStdTypeArray [EStdType__Count];

	CJnccallCallConv_msc32 m_JnccallCallConv_msc32;
	CJnccallCallConv_msc64 m_JnccallCallConv_msc64;
	CJnccallCallConv_gcc32 m_JnccallCallConv_gcc32;
	CJnccallCallConv_gcc64 m_JnccallCallConv_gcc64;
	CCdeclCallConv_msc32 m_CdeclCallConv_msc32;
	CCdeclCallConv_msc64 m_CdeclCallConv_msc64;
	CCdeclCallConv_gcc32 m_CdeclCallConv_gcc32;
	CCdeclCallConv_gcc64 m_CdeclCallConv_gcc64;
	CStdcallCallConv_msc32 m_StdcallCallConv_msc32;
	CStdcallCallConv_gcc32 m_StdcallCallConv_gcc32;
	CThiscallCallConv_msc32 m_ThiscallCallConv_msc32;

	CCallConv* m_CallConvTable [ECallConv__Count];

	rtl::CStdListT <CArrayType> m_ArrayTypeList;
	rtl::CStdListT <CBitFieldType> m_BitFieldTypeList;
	rtl::CStdListT <CEnumType> m_EnumTypeList;
	rtl::CStdListT <CStructType> m_StructTypeList;
	rtl::CStdListT <CUnionType> m_UnionTypeList;
	rtl::CStdListT <CClassType> m_ClassTypeList;
	rtl::CStdListT <CFunctionType> m_FunctionTypeList;
	rtl::CStdListT <CPropertyType> m_PropertyTypeList;
	rtl::CStdListT <CDataPtrType> m_DataPtrTypeList;
	rtl::CStdListT <CClassPtrType> m_ClassPtrTypeList;
	rtl::CStdListT <CFunctionPtrType> m_FunctionPtrTypeList;
	rtl::CStdListT <CPropertyPtrType> m_PropertyPtrTypeList;
	rtl::CStdListT <CNamedImportType> m_NamedImportTypeList;
	rtl::CStdListT <CImportPtrType> m_ImportPtrTypeList;
	rtl::CStdListT <CImportIntModType> m_ImportIntModTypeList;
	rtl::CStdListT <CReactorClassType> m_ReactorClassTypeList;
	rtl::CStdListT <CFunctionClosureClassType> m_FunctionClosureClassTypeList;
	rtl::CStdListT <CPropertyClosureClassType> m_PropertyClosureClassTypeList;
	rtl::CStdListT <CDataClosureClassType> m_DataClosureClassTypeList;
	rtl::CStdListT <CMulticastClassType> m_MulticastClassTypeList;
	rtl::CStdListT <CMcSnapshotClassType> m_McSnapshotClassTypeList;

	rtl::CStdListT <CTypedef> m_TypedefList;
	rtl::CStdListT <CLazyStdType> m_LazyStdTypeList;
	rtl::CStdListT <CFunctionArg> m_FunctionArgList;

	rtl::CStdListT <TSimplePropertyTypeTuple> m_SimplePropertyTypeTupleList;
	rtl::CStdListT <TFunctionArgTuple> m_FunctionArgTupleList;
	rtl::CStdListT <TDataPtrTypeTuple> m_DataPtrTypeTupleList;
	rtl::CStdListT <TClassPtrTypeTuple> m_ClassPtrTypeTupleList;
	rtl::CStdListT <TFunctionPtrTypeTuple> m_FunctionPtrTypeTupleList;
	rtl::CStdListT <TPropertyPtrTypeTuple> m_PropertyPtrTypeTupleList;
	rtl::CStdListT <TDualPtrTypeTuple> m_DualPtrTypeTupleList;

	rtl::CStringHashTableMapT <CType*> m_TypeMap;
	rtl::CArrayT <TGcShadowStackFrameTypePair> m_GcShadowStackFrameTypeArray;

	size_t m_UnnamedEnumTypeCounter;
	size_t m_UnnamedStructTypeCounter;
	size_t m_UnnamedUnionTypeCounter;
	size_t m_UnnamedClassTypeCounter;

public:
	CTypeMgr ();

	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	void
	Clear ();

	bool
	ResolveImportTypes ();

	void
	UpdateTypeSignature (
		CType* pType,
		const rtl::CString& Signature
		);

	rtl::CConstListT <CArrayType>
	GetArrayTypeList ()
	{
		return m_ArrayTypeList;
	}

	rtl::CConstListT <CBitFieldType>
	GetBitFieldTypeList ()
	{
		return m_BitFieldTypeList;
	}

	rtl::CConstListT <CEnumType>
	GetEnumTypeList ()
	{
		return m_EnumTypeList;
	}

	rtl::CConstListT <CStructType>
	GetStructTypeList ()
	{
		return m_StructTypeList;
	}

	rtl::CConstListT <CUnionType>
	GetUnionTypeList ()
	{
		return m_UnionTypeList;
	}

	rtl::CConstListT <CClassType>
	GetClassTypeList ()
	{
		return m_ClassTypeList;
	}

	rtl::CConstListT <CFunctionType>
	GetFunctionTypeList ()
	{
		return m_FunctionTypeList;
	}

	rtl::CConstListT <CPropertyType>
	GetPropertyTypeList ()
	{
		return m_PropertyTypeList;
	}

	rtl::CConstListT <CDataPtrType>
	GetDataPtrTypeList ()
	{
		return m_DataPtrTypeList;
	}

	rtl::CConstListT <CClassPtrType>
	GetClassPtrTypeList ()
	{
		return m_ClassPtrTypeList;
	}

	rtl::CConstListT <CFunctionPtrType>
	GetFunctionPtrTypeList ()
	{
		return m_FunctionPtrTypeList;
	}

	rtl::CConstListT <CPropertyPtrType>
	GetPropertyPtrTypeList ()
	{
		return m_PropertyPtrTypeList;
	}

	rtl::CConstListT <CNamedImportType>
	GetNamedImportTypeList ()
	{
		return m_NamedImportTypeList;
	}

	rtl::CConstListT <CImportPtrType>
	GetImportPtrTypeList ()
	{
		return m_ImportPtrTypeList;
	}

	rtl::CConstListT <CImportIntModType>
	GetImportIntModTypeList ()
	{
		return m_ImportIntModTypeList;
	}

	rtl::CConstListT <CReactorClassType>
	GetReactorClassTypeList ()
	{
		return m_ReactorClassTypeList;
	}

	rtl::CConstListT <CFunctionClosureClassType>
	GetFunctionClosureClassTypeList ()
	{
		return m_FunctionClosureClassTypeList;
	}

	rtl::CConstListT <CPropertyClosureClassType>
	GetPropertyClosureClassTypeList ()
	{
		return m_PropertyClosureClassTypeList;
	}

	rtl::CConstListT <CDataClosureClassType>
	GetDataClosureClassTypeList ()
	{
		return m_DataClosureClassTypeList;
	}

	rtl::CConstListT <CMulticastClassType>
	GetMulticastClassTypeList ()
	{
		return m_MulticastClassTypeList;
	}

	rtl::CConstListT <CMcSnapshotClassType>
	GetMcSnapshotClassTypeList ()
	{
		return m_McSnapshotClassTypeList;
	}

	rtl::CConstListT <CTypedef>
	GetTypedefList ()
	{
		return m_TypedefList;
	}

	CType*
	GetPrimitiveType (EType TypeKind)
	{
		ASSERT (TypeKind < EType__PrimitiveTypeCount);
		return &m_PrimitiveTypeArray [TypeKind];
	}

	bool
	IsStdTypeUsed (EStdType StdType)
	{
		ASSERT (StdType < EStdType__Count);
		return m_StdTypeArray [StdType] != NULL;
	}

	CType*
	GetStdType (EStdType StdType);

	CLazyStdType*
	GetLazyStdType (EStdType StdType);

	CType*
	GetInt32Type (int32_t Integer)
	{
		return GetPrimitiveType (GetInt32TypeKind (Integer));
	}

	CType*
	GetUInt32Type (uint32_t Integer)
	{
		return GetPrimitiveType (GetInt32TypeKind_u (Integer));
	}

	CType*
	GetInt64Type (int64_t Integer)
	{
		return GetPrimitiveType (GetInt64TypeKind (Integer));
	}

	CType*
	GetInt64Type_u (uint64_t Integer)
	{
		return GetPrimitiveType (GetInt64TypeKind_u (Integer));
	}

	CBitFieldType*
	GetBitFieldType (
		CType* pBaseType,
		size_t BitOffset,
		size_t BitCount
		);

	CArrayType*
	CreateAutoSizeArrayType (CType* pElementType);

	CArrayType*
	CreateArrayType (
		CType* pElementType,
		rtl::CBoxListT <CToken>* pElementCountInitializer
		);

	CArrayType*
	GetArrayType (
		CType* pElementType,
		size_t ElementCount
		);

	CArrayType*
	GetArrayType (
		EType ElementTypeKind,
		size_t ElementCount
		)
	{
		return GetArrayType (GetPrimitiveType (ElementTypeKind), ElementCount);
	}

	CTypedef*
	CreateTypedef (
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		CType* pType
		);

	CEnumType*
	CreateEnumType (
		EEnumType EnumTypeKind,
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		CType* pBaseType = NULL,
		uint_t Flags = 0
		);

	CEnumType*
	CreateUnnamedEnumType (
		EEnumType EnumTypeKind,
		CType* pBaseType = NULL,
		uint_t Flags = 0
		)
	{
		return CreateEnumType (EnumTypeKind, rtl::CString (), rtl::CString (), pBaseType, Flags);
	}

	CStructType*
	CreateStructType (
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		size_t PackFactor = 8
		);

	CStructType*
	CreateUnnamedStructType (size_t PackFactor = 8)
	{
		return CreateStructType (rtl::CString (), rtl::CString (), PackFactor);
	}

	CStructType*
	GetStructType (
		CType** ppMemberTypeArray,
		size_t MemberCount,
		size_t PackFactor = 8
		);

	CUnionType*
	CreateUnionType (
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		size_t PackFactor = 8
		);

	CUnionType*
	CreateUnnamedUnionType (size_t PackFactor = 8)
	{
		return CreateUnionType (rtl::CString (), rtl::CString (), PackFactor);
	}

	CClassType*
	CreateClassType (
		EClassType ClassTypeKind,
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		size_t PackFactor = 8,
		uint_t Flags = 0
		);

	CClassType*
	CreateClassType (
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		size_t PackFactor = 8,
		uint_t Flags = 0
		)
	{
		return CreateClassType (EClassType_Normal, Name, QualifiedName, PackFactor, Flags);
	}

	CClassType*
	CreateUnnamedClassType (
		EClassType ClassTypeKind,
		size_t PackFactor = 8,
		uint_t Flags = 0
		)
	{
		return CreateClassType (ClassTypeKind,	rtl::CString (), rtl::CString (), PackFactor, Flags);
	}

	CClassType*
	CreateUnnamedClassType (
		size_t PackFactor = 8,
		uint_t Flags = 0
		)
	{
		return CreateClassType (EClassType_Normal, rtl::CString (), rtl::CString (), PackFactor, Flags);
	}

	CClassType*
	GetBoxClassType (CType* pBaseType);

	CFunctionArg*
	CreateFunctionArg (
		const rtl::CString& Name,
		CType* pType,
		uint_t PtrTypeFlags = 0,
		rtl::CBoxListT <CToken>* pInitializer = NULL
		);

	CFunctionArg*
	GetSimpleFunctionArg (
		EStorage StorageKind,
		CType* pType,
		uint_t PtrTypeFlags = 0
		);

	CFunctionArg*
	GetSimpleFunctionArg (
		CType* pType,
		uint_t PtrTypeFlags = 0
		)
	{
		return GetSimpleFunctionArg (EStorage_Stack, pType, PtrTypeFlags);
	}

	CCallConv*
	GetCallConv (ECallConv CallConvKind)
	{
		ASSERT (CallConvKind < ECallConv__Count);
		return m_CallConvTable [CallConvKind];
	}

	CFunctionType*
	GetFunctionType (
		CCallConv* pCallConv,
		CType* pReturnType,
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		);

	CFunctionType*
	GetFunctionType (
		CType* pReturnType,
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		)
	{
		return GetFunctionType (m_CallConvTable [ECallConv_Default], pReturnType, ArgArray, Flags);
	}

	CFunctionType*
	GetFunctionType (
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		)
	{
		return GetFunctionType (
			m_CallConvTable [ECallConv_Default],
			&m_PrimitiveTypeArray [EType_Void],
			ArgArray,
			Flags
			);
	}

	CFunctionType*
	GetFunctionType (
		CCallConv* pCallConv,
		CType* pReturnType,
		CType* const* ppArgType,
		size_t ArgCount,
		uint_t Flags = 0
		);

	CFunctionType*
	GetFunctionType (
		CType* pReturnType,
		CType* const* ppArgType,
		size_t ArgCount,
		uint_t Flags = 0
		)
	{
		return GetFunctionType (m_CallConvTable [ECallConv_Default], pReturnType, ppArgType, ArgCount, Flags);
	}

	CFunctionType*
	GetFunctionType (
		CType* const* ppArgType,
		size_t ArgCount,
		uint_t Flags = 0
		)
	{
		return GetFunctionType (
			m_CallConvTable [ECallConv_Default],
			&m_PrimitiveTypeArray [EType_Void],
			ppArgType,
			ArgCount,
			Flags
			);
	}

	CFunctionType*
	GetFunctionType ()
	{
		return (CFunctionType*) GetStdType (EStdType_SimpleFunction);
	}

	CFunctionType*
	CreateUserFunctionType (
		CCallConv* pCallConv,
		CType* pReturnType,
		rtl::CBoxListT <CToken>* pThrowCondition,
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		);

	CFunctionType*
	CreateUserFunctionType (
		CCallConv* pCallConv,
		CType* pReturnType,
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		)
	{
		return CreateUserFunctionType (pCallConv, pReturnType, NULL, ArgArray, Flags);
	}

	CFunctionType*
	CreateUserFunctionType (
		CType* pReturnType,
		rtl::CBoxListT <CToken>* pThrowCondition,
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		)
	{
		return CreateUserFunctionType (m_CallConvTable [ECallConv_Default], pReturnType, pThrowCondition, ArgArray, Flags);
	}

	CFunctionType*
	CreateUserFunctionType (
		CType* pReturnType,
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		)
	{
		return CreateUserFunctionType (m_CallConvTable [ECallConv_Default], pReturnType, NULL, ArgArray, Flags);
	}

	CFunctionType*
	CreateUserFunctionType (
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		)
	{
		return CreateUserFunctionType (
			m_CallConvTable [ECallConv_Default],
			&m_PrimitiveTypeArray [EType_Void],
			NULL,
			ArgArray,
			Flags
			);
	}

	CFunctionType*
	GetMemberMethodType (
		CNamedType* pParentType,
		CFunctionType* pFunctionType,
		uint_t ThisArgPtrTypeFlags = 0
		);

	CFunctionType*
	GetStdObjectMemberMethodType (CFunctionType* pFunctionType);

	CPropertyType*
	GetPropertyType (
		CFunctionType* pGetterType,
		const CFunctionTypeOverload& SetterType,
		uint_t Flags = 0
		);

	CPropertyType*
	GetSimplePropertyType (
		CCallConv* pCallConv,
		CType* pReturnType,
		uint_t Flags = 0
		);

	CPropertyType*
	GetSimplePropertyType (
		CType* pReturnType,
		uint_t Flags = 0
		)
	{
		return GetSimplePropertyType (
			m_CallConvTable [ECallConv_Default],
			pReturnType,
			Flags
			);
	}

	CPropertyType*
	GetIndexedPropertyType (
		CCallConv* pCallConv,
		CType* pReturnType,
		CType* const* ppIndexArgType,
		size_t IndexArgCount,
		uint_t Flags = 0
		);

	CPropertyType*
	GetIndexedPropertyType (
		CType* pReturnType,
		CType* const* ppIndexArgType,
		size_t IndexArgCount,
		uint_t Flags = 0
		)
	{
		return GetIndexedPropertyType (NULL, pReturnType, ppIndexArgType, IndexArgCount, Flags);
	}

	CPropertyType*
	GetIndexedPropertyType (
		CCallConv* pCallConv,
		CType* pReturnType,
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		);

	CPropertyType*
	GetIndexedPropertyType (
		CType* pReturnType,
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		)
	{
		return GetIndexedPropertyType (NULL, pReturnType, ArgArray, Flags);
	}

	CPropertyType*
	CreateIndexedPropertyType (
		CCallConv* pCallConv,
		CType* pReturnType,
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		);

	CPropertyType*
	CreateIndexedPropertyType (
		CType* pReturnType,
		const rtl::CArrayT <CFunctionArg*>& ArgArray,
		uint_t Flags = 0
		)
	{
		return CreateIndexedPropertyType (NULL, pReturnType, ArgArray, Flags);
	}

	CPropertyType*
	GetMemberPropertyType (
		CNamedType* pParentType,
		CPropertyType* pPropertyType
		);

	CPropertyType*
	GetStdObjectMemberPropertyType (CPropertyType* pPropertyType);

	CPropertyType*
	GetShortPropertyType (CPropertyType* pPropertyType);

	CClassType*
	GetMulticastType (
		CFunctionType* pFunctionType,
		EFunctionPtrType PtrTypeKind = EFunctionPtrType_Normal
		)
	{
		return GetMulticastType (GetFunctionPtrType (pFunctionType, PtrTypeKind));
	}

	CClassType*
	GetMulticastType (CFunctionPtrType* pFunctionPtrType);

	CClassType*
	GetReactorInterfaceType (CFunctionType* pStartMethodType);

	CReactorClassType*
	CreateReactorType (
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		CClassType* pIfaceType,
		CClassType* pParentType
		);

	CFunctionClosureClassType*
	GetFunctionClosureClassType (
		CFunctionType* pTargetType,
		CFunctionType* pThunkType,
		CType* const* ppArgTypeArray,
		const size_t* pClosureMap,
		size_t ArgCount,
		uint64_t WeakMask
		);

	CPropertyClosureClassType*
	GetPropertyClosureClassType (
		CPropertyType* pTargetType,
		CPropertyType* pThunkType,
		CType* const* ppArgTypeArray,
		const size_t* pClosureMap,
		size_t ArgCount,
		uint64_t WeakMask
		);

	CDataClosureClassType*
	GetDataClosureClassType (
		CType* pTargetType,
		CPropertyType* pThunkType
		);

	CDataPtrType*
	GetDataPtrType (
		CNamespace* pAnchorNamespace,
		CType* pDataType,
		EType TypeKind,
		EDataPtrType PtrTypeKind = EDataPtrType_Normal,
		uint_t Flags = 0
		);

	CDataPtrType*
	GetDataPtrType (
		CType* pDataType,
		EType TypeKind,
		EDataPtrType PtrTypeKind = EDataPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetDataPtrType (NULL, pDataType, TypeKind, PtrTypeKind, Flags);

	}

	CDataPtrType*
	GetDataPtrType (
		CType* pDataType,
		EDataPtrType PtrTypeKind = EDataPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetDataPtrType (pDataType, EType_DataPtr, PtrTypeKind, Flags);
	}

	CStructType*
	GetDataPtrStructType (CType* pDataType);

	CClassPtrType*
	GetClassPtrType (
		CNamespace* pAnchorNamespace,
		CClassType* pClassType,
		EType TypeKind,
		EClassPtrType PtrTypeKind = EClassPtrType_Normal,
		uint_t Flags = 0
		);

	CClassPtrType*
	GetClassPtrType (
		CClassType* pClassType,
		EType TypeKind,
		EClassPtrType PtrTypeKind = EClassPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetClassPtrType (NULL, pClassType, TypeKind, PtrTypeKind, Flags);
	}

	CClassPtrType*
	GetClassPtrType (
		CClassType* pClassType,
		EClassPtrType PtrTypeKind = EClassPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetClassPtrType (pClassType, EType_ClassPtr, PtrTypeKind, Flags);
	}

	CFunctionPtrType*
	GetFunctionPtrType (
		CFunctionType* pFunctionType,
		EType TypeKind,
		EFunctionPtrType PtrTypeKind = EFunctionPtrType_Normal,
		uint_t Flags = 0
		);

	CFunctionPtrType*
	GetFunctionPtrType (
		CFunctionType* pFunctionType,
		EFunctionPtrType PtrTypeKind = EFunctionPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetFunctionPtrType (pFunctionType, EType_FunctionPtr, PtrTypeKind, Flags);
	}

	CStructType*
	GetFunctionPtrStructType (CFunctionType* pFunctionType);

	CPropertyPtrType*
	GetPropertyPtrType (
		CNamespace* pAnchorNamespace,
		CPropertyType* pPropertyType,
		EType TypeKind,
		EPropertyPtrType PtrTypeKind = EPropertyPtrType_Normal,
		uint_t Flags = 0
		);

	CPropertyPtrType*
	GetPropertyPtrType (
		CPropertyType* pPropertyType,
		EType TypeKind,
		EPropertyPtrType PtrTypeKind = EPropertyPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetPropertyPtrType (NULL, pPropertyType, TypeKind, PtrTypeKind, Flags);
	}

	CPropertyPtrType*
	GetPropertyPtrType (
		CPropertyType* pPropertyType,
		EPropertyPtrType PtrTypeKind = EPropertyPtrType_Normal,
		uint_t Flags = 0
		)
	{
		return GetPropertyPtrType (pPropertyType, EType_PropertyPtr, PtrTypeKind, Flags);
	}

	CStructType*
	GetPropertyVTableStructType (CPropertyType* pPropertyType);

	CStructType*
	GetPropertyPtrStructType (CPropertyType* pPropertyType);

	CNamedImportType*
	GetNamedImportType (
		const CQualifiedName& Name,
		CNamespace* pAnchorNamespace
		);

	CImportPtrType*
	GetImportPtrType (
		CNamedImportType* pImportType,
		uint_t TypeModifiers = 0,
		uint_t Flags = 0
		);

	CImportIntModType*
	GetImportIntModType (
		CNamedImportType* pImportType,
		uint_t TypeModifiers = 0,
		uint_t Flags = 0
		);

	CType*
	GetCheckedPtrType (CType* pType);

	CStructType*
	GetGcShadowStackFrameMapType (size_t RootCount);

	CStructType*
	GetGcShadowStackFrameType (size_t RootCount);

protected:
	TDualPtrTypeTuple*
	GetDualPtrTypeTuple (
		CNamespace* pAnchorNamespace,
		CType* pType
		);

	TSimplePropertyTypeTuple*
	GetSimplePropertyTypeTuple (CType* pType);

	TFunctionArgTuple*
	GetFunctionArgTuple (CType* pType);

	TDataPtrTypeTuple*
	GetDataPtrTypeTuple (CType* pType);

	TDataPtrTypeTuple*
	GetConstDDataPtrTypeTuple (
		CNamespace* pAnchorNamespace,
		CType* pType
		);

	TClassPtrTypeTuple*
	GetClassPtrTypeTuple (CClassType* pClassType);

	TClassPtrTypeTuple*
	GetConstDClassPtrTypeTuple (
		CNamespace* pAnchorNamespace,
		CClassType* pClassType
		);

	TClassPtrTypeTuple*
	GetEventClassPtrTypeTuple (CMulticastClassType* pClassType);

	TClassPtrTypeTuple*
	GetEventDClassPtrTypeTuple (
		CNamespace* pAnchorNamespace,
		CMulticastClassType* pClassType
		);

	TFunctionPtrTypeTuple*
	GetFunctionPtrTypeTuple (CFunctionType* pFunctionType);

	TPropertyPtrTypeTuple*
	GetPropertyPtrTypeTuple (CPropertyType* pPropertyType);

	TPropertyPtrTypeTuple*
	GetConstDPropertyPtrTypeTuple (
		CNamespace* pAnchorNamespace,
		CPropertyType* pPropertyType
		);

	void
	SetupAllPrimitiveTypes ();

	void
	SetupCallConvTable ();

	void
	SetupPrimitiveType (
		EType TypeKind,
		size_t Size,
		const char* pSignature
		);

	CStructType*
	CreateObjHdrType ();

	CClassType*
	CreateObjectType ();

	CStructType*
	CreateReactorBindSiteType ();

	CClassType*
	CreateSchedulerType ();

	CStructType*
	CreateFmtLiteralType ();

	CStructType*
	CreateGuidType ();

	CStructType*
	CreateErrorType ();

	CStructType*
	CreatePairType (
		const rtl::CString& Name,
		const rtl::CString& QualifiedName,
		CType* pType1,
		CType* pType2
		);

	bool
	CreateMulticastCallMethod (CClassType* pMulticastType);
};

//.............................................................................

} // namespace jnc {
