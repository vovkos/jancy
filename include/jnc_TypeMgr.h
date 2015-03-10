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

class Module;

//.............................................................................

struct DualPtrTypeTuple: rtl::ListLink
{
	union
	{
		DataPtrTypeTuple* m_constDDataPtrTypeTuple;
		ClassPtrTypeTuple* m_constDClassPtrTypeTuple;
		PropertyPtrTypeTuple* m_constDPropertyPtrTypeTuple;
	};

	ClassPtrTypeTuple* m_eventDClassPtrTypeTuple;
};

//.............................................................................

class TypeMgr
{
	friend class Module;

protected:
	struct GcShadowStackFrameTypePair
	{
		StructType* m_gcShadowStackFrameType;
		StructType* m_gcShadowStackFrameMapType;
	};

protected:
	Module* m_module;

	Type m_primitiveTypeArray [TypeKind__PrimitiveTypeCount];
	Type* m_stdTypeArray [StdType__Count];
	Typedef m_stdTypedefArray [StdTypedef__Count];
	LazyStdType* m_lazyStdTypeArray [StdType__Count];
	CallConv* m_callConvArray [CallConvKind__Count];

	JnccallCallConv_msc32 m_jnccallCallConv_msc32;
	JnccallCallConv_msc64 m_jnccallCallConv_msc64;
	JnccallCallConv_gcc32 m_jnccallCallConv_gcc32;
	JnccallCallConv_gcc64 m_jnccallCallConv_gcc64;
	CdeclCallConv_msc32 m_cdeclCallConv_msc32;
	CdeclCallConv_msc64 m_cdeclCallConv_msc64;
	CdeclCallConv_gcc32 m_cdeclCallConv_gcc32;
	CdeclCallConv_gcc64 m_cdeclCallConv_gcc64;
	StdcallCallConv_msc32 m_stdcallCallConv_msc32;
	StdcallCallConv_gcc32 m_stdcallCallConv_gcc32;
	ThiscallCallConv_msc32 m_thiscallCallConv_msc32;

	rtl::StdList <ArrayType> m_arrayTypeList;
	rtl::StdList <BitFieldType> m_bitFieldTypeList;
	rtl::StdList <EnumType> m_enumTypeList;
	rtl::StdList <StructType> m_structTypeList;
	rtl::StdList <UnionType> m_unionTypeList;
	rtl::StdList <ClassType> m_classTypeList;
	rtl::StdList <FunctionType> m_functionTypeList;
	rtl::StdList <PropertyType> m_propertyTypeList;
	rtl::StdList <DataPtrType> m_dataPtrTypeList;
	rtl::StdList <ClassPtrType> m_classPtrTypeList;
	rtl::StdList <FunctionPtrType> m_functionPtrTypeList;
	rtl::StdList <PropertyPtrType> m_propertyPtrTypeList;
	rtl::StdList <NamedImportType> m_namedImportTypeList;
	rtl::StdList <ImportPtrType> m_importPtrTypeList;
	rtl::StdList <ImportIntModType> m_importIntModTypeList;
	rtl::StdList <ReactorClassType> m_reactorClassTypeList;
	rtl::StdList <FunctionClosureClassType> m_functionClosureClassTypeList;
	rtl::StdList <PropertyClosureClassType> m_propertyClosureClassTypeList;
	rtl::StdList <DataClosureClassType> m_dataClosureClassTypeList;
	rtl::StdList <MulticastClassType> m_multicastClassTypeList;
	rtl::StdList <McSnapshotClassType> m_mcSnapshotClassTypeList;

	rtl::StdList <Typedef> m_typedefList;
	rtl::StdList <LazyStdType> m_lazyStdTypeList;
	rtl::StdList <FunctionArg> m_functionArgList;
	rtl::StdList <StructField> m_structFieldList;

	rtl::StdList <SimplePropertyTypeTuple> m_simplePropertyTypeTupleList;
	rtl::StdList <FunctionArgTuple> m_functionArgTupleList;
	rtl::StdList <DataPtrTypeTuple> m_dataPtrTypeTupleList;
	rtl::StdList <ClassPtrTypeTuple> m_classPtrTypeTupleList;
	rtl::StdList <FunctionPtrTypeTuple> m_functionPtrTypeTupleList;
	rtl::StdList <PropertyPtrTypeTuple> m_propertyPtrTypeTupleList;
	rtl::StdList <DualPtrTypeTuple> m_dualPtrTypeTupleList;

	rtl::StringHashTableMap <Type*> m_typeMap;

	rtl::Array <GcShadowStackFrameTypePair> m_gcShadowStackFrameTypeArray;
	rtl::Array <NamedImportType*> m_unresolvedNamedImportTypeArray;
	rtl::Array <ImportPtrType*> m_unresolvedImportPtrTypeArray;
	rtl::Array <ImportIntModType*> m_unresolvedImportIntModTypeArray;

	size_t m_unnamedEnumTypeCounter;
	size_t m_unnamedStructTypeCounter;
	size_t m_unnamedUnionTypeCounter;
	size_t m_unnamedClassTypeCounter;

	size_t m_parseStdTypeLevel;

public:
	TypeMgr ();

	Module*
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	bool
	resolveImportTypes ();

	void
	updateTypeSignature (
		Type* type,
		const rtl::String& signature
		);

	rtl::ConstList <ArrayType>
	getArrayTypeList ()
	{
		return m_arrayTypeList;
	}

	rtl::ConstList <BitFieldType>
	getBitFieldTypeList ()
	{
		return m_bitFieldTypeList;
	}

	rtl::ConstList <EnumType>
	getEnumTypeList ()
	{
		return m_enumTypeList;
	}

	rtl::ConstList <StructType>
	getStructTypeList ()
	{
		return m_structTypeList;
	}

	rtl::ConstList <UnionType>
	getUnionTypeList ()
	{
		return m_unionTypeList;
	}

	rtl::ConstList <ClassType>
	getClassTypeList ()
	{
		return m_classTypeList;
	}

	rtl::ConstList <FunctionType>
	getFunctionTypeList ()
	{
		return m_functionTypeList;
	}

	rtl::ConstList <PropertyType>
	getPropertyTypeList ()
	{
		return m_propertyTypeList;
	}

	rtl::ConstList <DataPtrType>
	getDataPtrTypeList ()
	{
		return m_dataPtrTypeList;
	}

	rtl::ConstList <ClassPtrType>
	getClassPtrTypeList ()
	{
		return m_classPtrTypeList;
	}

	rtl::ConstList <FunctionPtrType>
	getFunctionPtrTypeList ()
	{
		return m_functionPtrTypeList;
	}

	rtl::ConstList <PropertyPtrType>
	getPropertyPtrTypeList ()
	{
		return m_propertyPtrTypeList;
	}

	rtl::ConstList <NamedImportType>
	getNamedImportTypeList ()
	{
		return m_namedImportTypeList;
	}

	rtl::ConstList <ImportPtrType>
	getImportPtrTypeList ()
	{
		return m_importPtrTypeList;
	}

	rtl::ConstList <ImportIntModType>
	getImportIntModTypeList ()
	{
		return m_importIntModTypeList;
	}

	rtl::ConstList <ReactorClassType>
	getReactorClassTypeList ()
	{
		return m_reactorClassTypeList;
	}

	rtl::ConstList <FunctionClosureClassType>
	getFunctionClosureClassTypeList ()
	{
		return m_functionClosureClassTypeList;
	}

	rtl::ConstList <PropertyClosureClassType>
	getPropertyClosureClassTypeList ()
	{
		return m_propertyClosureClassTypeList;
	}

	rtl::ConstList <DataClosureClassType>
	getDataClosureClassTypeList ()
	{
		return m_dataClosureClassTypeList;
	}

	rtl::ConstList <MulticastClassType>
	getMulticastClassTypeList ()
	{
		return m_multicastClassTypeList;
	}

	rtl::ConstList <McSnapshotClassType>
	getMcSnapshotClassTypeList ()
	{
		return m_mcSnapshotClassTypeList;
	}

	rtl::ConstList <Typedef>
	getTypedefList ()
	{
		return m_typedefList;
	}

	Type*
	getPrimitiveType (TypeKind typeKind)
	{
		ASSERT (typeKind < TypeKind__PrimitiveTypeCount);
		return &m_primitiveTypeArray [typeKind];
	}

	bool
	isStdTypeUsed (StdType stdType)
	{
		ASSERT (stdType < StdType__Count);
		return m_stdTypeArray [stdType] != NULL;
	}

	Type*
	getStdType (StdType stdType);

	LazyStdType*
	getLazyStdType (StdType stdType);

	Typedef*
	getStdTypedef (StdTypedef stdTypedef)
	{
		ASSERT (stdTypedef < StdTypedef__Count);
		return &m_stdTypedefArray [stdTypedef];
	}

	Type*
	getInt32Type (int32_t integer)
	{
		return getPrimitiveType (getInt32TypeKind (integer));
	}

	Type*
	getUInt32Type (uint32_t integer)
	{
		return getPrimitiveType (getInt32TypeKind_u (integer));
	}

	Type*
	getInt64Type (int64_t integer)
	{
		return getPrimitiveType (getInt64TypeKind (integer));
	}

	Type*
	getInt64Type_u (uint64_t integer)
	{
		return getPrimitiveType (getInt64TypeKind_u (integer));
	}

	BitFieldType*
	getBitFieldType (
		Type* baseType,
		size_t bitOffset,
		size_t bitCount
		);

	ArrayType*
	createAutoSizeArrayType (Type* elementType);

	ArrayType*
	createArrayType (
		Type* elementType,
		rtl::BoxList <Token>* elementCountInitializer
		);

	ArrayType*
	getArrayType (
		Type* elementType,
		size_t elementCount
		);

	ArrayType*
	getArrayType (
		TypeKind elementTypeKind,
		size_t elementCount
		)
	{
		return getArrayType (getPrimitiveType (elementTypeKind), elementCount);
	}

	Typedef*
	createTypedef (
		const rtl::String& name,
		const rtl::String& qualifiedName,
		Type* type
		);

	EnumType*
	createEnumType (
		const rtl::String& name,
		const rtl::String& qualifiedName,
		Type* baseType = NULL,
		uint_t flags = 0
		);

	EnumType*
	createUnnamedEnumType (
		Type* baseType = NULL,
		uint_t flags = 0
		)
	{
		return createEnumType (rtl::String (), rtl::String (), baseType, flags);
	}

	StructType*
	createStructType (
		const rtl::String& name,
		const rtl::String& qualifiedName,
		size_t fieldAlignment = 8
		);

	StructType*
	createUnnamedStructType (size_t fieldAlignment = 8)
	{
		return createStructType (rtl::String (), rtl::String (), fieldAlignment);
	}

	StructType*
	getStructType (
		Type** memberTypeArray,
		size_t memberCount,
		size_t fieldAlignment = 8
		);

	UnionType*
	createUnionType (
		const rtl::String& name,
		const rtl::String& qualifiedName,
		size_t fieldAlignment = 8
		);

	UnionType*
	createUnnamedUnionType (size_t fieldAlignment = 8)
	{
		return createUnionType (rtl::String (), rtl::String (), fieldAlignment);
	}

	ClassType*
	createClassType (
		ClassTypeKind classTypeKind,
		const rtl::String& name,
		const rtl::String& qualifiedName,
		size_t fieldAlignment = 8,
		uint_t flags = 0
		);

	ClassType*
	createClassType (
		const rtl::String& name,
		const rtl::String& qualifiedName,
		size_t fieldAlignment = 8,
		uint_t flags = 0
		)
	{
		return createClassType (ClassTypeKind_Normal, name, qualifiedName, fieldAlignment, flags);
	}

	ClassType*
	createUnnamedClassType (
		ClassTypeKind classTypeKind,
		size_t fieldAlignment = 8,
		uint_t flags = 0
		)
	{
		return createClassType (classTypeKind,	rtl::String (), rtl::String (), fieldAlignment, flags);
	}

	ClassType*
	createUnnamedClassType (
		size_t fieldAlignment = 8,
		uint_t flags = 0
		)
	{
		return createClassType (ClassTypeKind_Normal, rtl::String (), rtl::String (), fieldAlignment, flags);
	}

	ClassType*
	getBoxClassType (Type* baseType);

	FunctionArg*
	createFunctionArg (
		const rtl::String& name,
		Type* type,
		uint_t ptrTypeFlags = 0,
		rtl::BoxList <Token>* initializer = NULL
		);

	FunctionArg*
	getSimpleFunctionArg (
		StorageKind storageKind,
		Type* type,
		uint_t ptrTypeFlags = 0
		);

	FunctionArg*
	getSimpleFunctionArg (
		Type* type,
		uint_t ptrTypeFlags = 0
		)
	{
		return getSimpleFunctionArg (StorageKind_Stack, type, ptrTypeFlags);
	}

	StructField*
	createStructField (
		const rtl::String& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		rtl::BoxList <Token>* constructor = NULL,
		rtl::BoxList <Token>* initializer = NULL
		);

	CallConv*
	getCallConv (CallConvKind callConvKind)
	{
		ASSERT (callConvKind < CallConvKind__Count);
		return m_callConvArray [callConvKind];
	}

	FunctionType*
	getFunctionType (
		CallConv* callConv,
		Type* returnType,
		const rtl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		);

	FunctionType*
	getFunctionType (
		Type* returnType,
		const rtl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		)
	{
		return getFunctionType (m_callConvArray [CallConvKind_Default], returnType, argArray, flags);
	}

	FunctionType*
	getFunctionType (
		const rtl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		)
	{
		return getFunctionType (
			m_callConvArray [CallConvKind_Default],
			&m_primitiveTypeArray [TypeKind_Void],
			argArray,
			flags
			);
	}

	FunctionType*
	getFunctionType (
		CallConv* callConv,
		Type* returnType,
		Type* const* argType,
		size_t argCount,
		uint_t flags = 0
		);

	FunctionType*
	getFunctionType (
		Type* returnType,
		Type* const* argType,
		size_t argCount,
		uint_t flags = 0
		)
	{
		return getFunctionType (m_callConvArray [CallConvKind_Default], returnType, argType, argCount, flags);
	}

	FunctionType*
	getFunctionType (
		Type* const* argType,
		size_t argCount,
		uint_t flags = 0
		)
	{
		return getFunctionType (
			m_callConvArray [CallConvKind_Default],
			&m_primitiveTypeArray [TypeKind_Void],
			argType,
			argCount,
			flags
			);
	}

	FunctionType*
	getFunctionType ()
	{
		return (FunctionType*) getStdType (StdType_SimpleFunction);
	}

	FunctionType*
	createUserFunctionType (
		CallConv* callConv,
		Type* returnType,
		const rtl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		);

	FunctionType*
	createUserFunctionType (
		Type* returnType,
		const rtl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		)
	{
		return createUserFunctionType (m_callConvArray [CallConvKind_Default], returnType, argArray, flags);
	}

	FunctionType*
	createUserFunctionType (
		const rtl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		)
	{
		return createUserFunctionType (
			m_callConvArray [CallConvKind_Default],
			&m_primitiveTypeArray [TypeKind_Void],
			argArray,
			flags
			);
	}

	FunctionType*
	getMemberMethodType (
		DerivableType* parentType,
		FunctionType* functionType,
		uint_t thisArgPtrTypeFlags = 0
		);

	FunctionType*
	getStdObjectMemberMethodType (FunctionType* functionType);

	FunctionType*
	getOperatorNewType (FunctionType* functionType);

	PropertyType*
	getPropertyType (
		FunctionType* getterType,
		const FunctionTypeOverload& setterType,
		uint_t flags = 0
		);

	PropertyType*
	getSimplePropertyType (
		CallConv* callConv,
		Type* returnType,
		uint_t flags = 0
		);

	PropertyType*
	getSimplePropertyType (
		Type* returnType,
		uint_t flags = 0
		)
	{
		return getSimplePropertyType (
			m_callConvArray [CallConvKind_Default],
			returnType,
			flags
			);
	}

	PropertyType*
	getIndexedPropertyType (
		CallConv* callConv,
		Type* returnType,
		Type* const* indexArgType,
		size_t indexArgCount,
		uint_t flags = 0
		);

	PropertyType*
	getIndexedPropertyType (
		Type* returnType,
		Type* const* indexArgType,
		size_t indexArgCount,
		uint_t flags = 0
		)
	{
		return getIndexedPropertyType (NULL, returnType, indexArgType, indexArgCount, flags);
	}

	PropertyType*
	getIndexedPropertyType (
		CallConv* callConv,
		Type* returnType,
		const rtl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		);

	PropertyType*
	getIndexedPropertyType (
		Type* returnType,
		const rtl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		)
	{
		return getIndexedPropertyType (NULL, returnType, argArray, flags);
	}

	PropertyType*
	createIndexedPropertyType (
		CallConv* callConv,
		Type* returnType,
		const rtl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		);

	PropertyType*
	createIndexedPropertyType (
		Type* returnType,
		const rtl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		)
	{
		return createIndexedPropertyType (NULL, returnType, argArray, flags);
	}

	PropertyType*
	getMemberPropertyType (
		DerivableType* parentType,
		PropertyType* propertyType
		);

	PropertyType*
	getStdObjectMemberPropertyType (PropertyType* propertyType);

	PropertyType*
	getShortPropertyType (PropertyType* propertyType);

	ClassType*
	getMulticastType (
		FunctionType* functionType,
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal
		)
	{
		return getMulticastType (getFunctionPtrType (functionType, ptrTypeKind));
	}

	ClassType*
	getMulticastType (FunctionPtrType* functionPtrType);

	ClassType*
	getReactorInterfaceType (FunctionType* startMethodType);

	ReactorClassType*
	createReactorType (
		const rtl::String& name,
		const rtl::String& qualifiedName,
		ClassType* ifaceType,
		ClassType* parentType
		);

	FunctionClosureClassType*
	getFunctionClosureClassType (
		FunctionType* targetType,
		FunctionType* thunkType,
		Type* const* argTypeArray,
		const size_t* closureMap,
		size_t argCount,
		uint64_t weakMask
		);

	PropertyClosureClassType*
	getPropertyClosureClassType (
		PropertyType* targetType,
		PropertyType* thunkType,
		Type* const* argTypeArray,
		const size_t* closureMap,
		size_t argCount,
		uint64_t weakMask
		);

	DataClosureClassType*
	getDataClosureClassType (
		Type* targetType,
		PropertyType* thunkType
		);

	DataPtrType*
	getDataPtrType (
		Namespace* anchorNamespace,
		Type* dataType,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		);

	DataPtrType*
	getDataPtrType (
		Type* dataType,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getDataPtrType (NULL, dataType, typeKind, ptrTypeKind, flags);

	}

	DataPtrType*
	getDataPtrType (
		Type* dataType,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getDataPtrType (dataType, TypeKind_DataPtr, ptrTypeKind, flags);
	}

	ClassPtrType*
	getClassPtrType (
		Namespace* anchorNamespace,
		ClassType* classType,
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
		);

	ClassPtrType*
	getClassPtrType (
		ClassType* classType,
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getClassPtrType (NULL, classType, typeKind, ptrTypeKind, flags);
	}

	ClassPtrType*
	getClassPtrType (
		ClassType* classType,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getClassPtrType (classType, TypeKind_ClassPtr, ptrTypeKind, flags);
	}

	FunctionPtrType*
	getFunctionPtrType (
		FunctionType* functionType,
		TypeKind typeKind,
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal,
		uint_t flags = 0
		);

	FunctionPtrType*
	getFunctionPtrType (
		FunctionType* functionType,
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getFunctionPtrType (functionType, TypeKind_FunctionPtr, ptrTypeKind, flags);
	}

	PropertyPtrType*
	getPropertyPtrType (
		Namespace* anchorNamespace,
		PropertyType* propertyType,
		TypeKind typeKind,
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
		);

	PropertyPtrType*
	getPropertyPtrType (
		PropertyType* propertyType,
		TypeKind typeKind,
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getPropertyPtrType (NULL, propertyType, typeKind, ptrTypeKind, flags);
	}

	PropertyPtrType*
	getPropertyPtrType (
		PropertyType* propertyType,
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getPropertyPtrType (propertyType, TypeKind_PropertyPtr, ptrTypeKind, flags);
	}

	StructType*
	getPropertyVTableStructType (PropertyType* propertyType);

	NamedImportType*
	getNamedImportType (
		const QualifiedName& name,
		Namespace* anchorNamespace
		);

	ImportPtrType*
	getImportPtrType (
		NamedImportType* importType,
		uint_t typeModifiers = 0,
		uint_t flags = 0
		);

	ImportIntModType*
	getImportIntModType (
		NamedImportType* importType,
		uint_t typeModifiers = 0,
		uint_t flags = 0
		);

	Type*
	getCheckedPtrType (Type* type);

	StructType*
	getGcShadowStackFrameMapType (size_t rootCount);

	StructType*
	getGcShadowStackFrameType (size_t rootCount);

protected:
	DualPtrTypeTuple*
	getDualPtrTypeTuple (
		Namespace* anchorNamespace,
		Type* type
		);

	SimplePropertyTypeTuple*
	getSimplePropertyTypeTuple (Type* type);

	FunctionArgTuple*
	getFunctionArgTuple (Type* type);

	DataPtrTypeTuple*
	getDataPtrTypeTuple (Type* type);

	DataPtrTypeTuple*
	getReadOnlyDataPtrTypeTuple (
		Namespace* anchorNamespace,
		Type* type
		);

	ClassPtrTypeTuple*
	getClassPtrTypeTuple (ClassType* classType);

	ClassPtrTypeTuple*
	getReadOnlyClassPtrTypeTuple (
		Namespace* anchorNamespace,
		ClassType* classType
		);

	ClassPtrTypeTuple*
	getEventClassPtrTypeTuple (MulticastClassType* classType);

	ClassPtrTypeTuple*
	getDualEventClassPtrTypeTuple (
		Namespace* anchorNamespace,
		MulticastClassType* classType
		);

	FunctionPtrTypeTuple*
	getFunctionPtrTypeTuple (FunctionType* functionType);

	PropertyPtrTypeTuple*
	getPropertyPtrTypeTuple (PropertyType* propertyType);

	PropertyPtrTypeTuple*
	getConstDPropertyPtrTypeTuple (
		Namespace* anchorNamespace,
		PropertyType* propertyType
		);

	void
	setupAllPrimitiveTypes ();

	void
	setupStdTypedefArray ();

	void
	setupCallConvArray ();

	void
	setupPrimitiveType (
		TypeKind typeKind,
		size_t size,
		const char* signature
		);

	void
	setupStdTypedef (
		StdTypedef stdTypedef,
		TypeKind typeKind,
		const rtl::String& name
		);

	NamedType*
	parseStdType (
		StdNamespace stdNamespace,
		const char* source,
		size_t length
		);

	ClassType*
	createObjectType ();

	StructType*
	createSimpleIfaceHdrType ();

	StructType*
	createObjHdrType ();

	StructType*
	createVariableObjHdrType ();

	StructType*
	createDataPtrStructType ();

	StructType*
	createFunctionPtrStructType ();
};

//.............................................................................

} // namespace jnc {
