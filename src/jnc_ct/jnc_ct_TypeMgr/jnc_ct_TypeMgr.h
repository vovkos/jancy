//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_ct_Type.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_BitFieldType.h"
#include "jnc_ct_EnumType.h"
#include "jnc_ct_StructType.h"
#include "jnc_ct_UnionType.h"
#include "jnc_ct_ClassType.h"
#include "jnc_ct_FunctionType.h"
#include "jnc_ct_PropertyType.h"
#include "jnc_ct_DataPtrType.h"
#include "jnc_ct_ClassPtrType.h"
#include "jnc_ct_FunctionPtrType.h"
#include "jnc_ct_PropertyPtrType.h"
#include "jnc_ct_ImportType.h"
#include "jnc_ct_ReactorClassType.h"
#include "jnc_ct_ClosureClassType.h"
#include "jnc_ct_McSnapshotClassType.h"
#include "jnc_ct_MulticastClassType.h"
#include "jnc_ct_JnccallCallConv.h"
#include "jnc_ct_CallConv_msc32.h"
#include "jnc_ct_CallConv_gcc32.h"
#include "jnc_ct_CdeclCallConv_gcc64.h"
#include "jnc_ct_CdeclCallConv_msc64.h"

namespace jnc {
namespace ct {

class Module;

//..............................................................................

struct DualPtrTypeTuple: sl::ListLink
{
	union
	{
		DataPtrTypeTuple* m_readOnlyDataPtrTypeTuple;
		ClassPtrTypeTuple* m_readOnlyClassPtrTypeTuple;
		PropertyPtrTypeTuple* m_readOnlyPropertyPtrTypeTuple;
	};

	ClassPtrTypeTuple* m_dualEventClassPtrTypeTuple;
};

//..............................................................................

class TypeMgr
{
	friend class Module;

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

	sl::StdList <ArrayType> m_arrayTypeList;
	sl::StdList <BitFieldType> m_bitFieldTypeList;
	sl::StdList <EnumType> m_enumTypeList;
	sl::StdList <StructType> m_structTypeList;
	sl::StdList <UnionType> m_unionTypeList;
	sl::StdList <ClassType> m_classTypeList;
	sl::StdList <FunctionType> m_functionTypeList;
	sl::StdList <PropertyType> m_propertyTypeList;
	sl::StdList <DataPtrType> m_dataPtrTypeList;
	sl::StdList <ClassPtrType> m_classPtrTypeList;
	sl::StdList <FunctionPtrType> m_functionPtrTypeList;
	sl::StdList <PropertyPtrType> m_propertyPtrTypeList;
	sl::StdList <NamedImportType> m_namedImportTypeList;
	sl::StdList <ImportPtrType> m_importPtrTypeList;
	sl::StdList <ImportIntModType> m_importIntModTypeList;
	sl::StdList <ReactorClassType> m_reactorClassTypeList;
	sl::StdList <FunctionClosureClassType> m_functionClosureClassTypeList;
	sl::StdList <PropertyClosureClassType> m_propertyClosureClassTypeList;
	sl::StdList <DataClosureClassType> m_dataClosureClassTypeList;
	sl::StdList <MulticastClassType> m_multicastClassTypeList;
	sl::StdList <McSnapshotClassType> m_mcSnapshotClassTypeList;
	sl::StdList <TypedefShadowType> m_typedefShadowTypeList;

	sl::StdList <Typedef> m_typedefList;
	sl::StdList <LazyStdType> m_lazyStdTypeList;
	sl::StdList <FunctionArg> m_functionArgList;
	sl::StdList <StructField> m_structFieldList;

	sl::StdList <SimplePropertyTypeTuple> m_simplePropertyTypeTupleList;
	sl::StdList <FunctionArgTuple> m_functionArgTupleList;
	sl::StdList <DataPtrTypeTuple> m_dataPtrTypeTupleList;
	sl::StdList <ClassPtrTypeTuple> m_classPtrTypeTupleList;
	sl::StdList <FunctionPtrTypeTuple> m_functionPtrTypeTupleList;
	sl::StdList <PropertyPtrTypeTuple> m_propertyPtrTypeTupleList;
	sl::StdList <DualTypeTuple> m_dualTypeTupleList;

	sl::StringHashTable <Type*> m_typeMap;

	sl::Array <NamedImportType*> m_unresolvedNamedImportTypeArray;
	sl::Array <ImportPtrType*> m_unresolvedImportPtrTypeArray;
	sl::Array <ImportIntModType*> m_unresolvedImportIntModTypeArray;

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
		const sl::StringRef& signature
		);

	sl::ConstList <ArrayType>
	getArrayTypeList ()
	{
		return m_arrayTypeList;
	}

	sl::ConstList <BitFieldType>
	getBitFieldTypeList ()
	{
		return m_bitFieldTypeList;
	}

	sl::ConstList <EnumType>
	getEnumTypeList ()
	{
		return m_enumTypeList;
	}

	sl::ConstList <StructType>
	getStructTypeList ()
	{
		return m_structTypeList;
	}

	sl::ConstList <UnionType>
	getUnionTypeList ()
	{
		return m_unionTypeList;
	}

	sl::ConstList <ClassType>
	getClassTypeList ()
	{
		return m_classTypeList;
	}

	sl::ConstList <FunctionType>
	getFunctionTypeList ()
	{
		return m_functionTypeList;
	}

	sl::ConstList <PropertyType>
	getPropertyTypeList ()
	{
		return m_propertyTypeList;
	}

	sl::ConstList <DataPtrType>
	getDataPtrTypeList ()
	{
		return m_dataPtrTypeList;
	}

	sl::ConstList <ClassPtrType>
	getClassPtrTypeList ()
	{
		return m_classPtrTypeList;
	}

	sl::ConstList <FunctionPtrType>
	getFunctionPtrTypeList ()
	{
		return m_functionPtrTypeList;
	}

	sl::ConstList <PropertyPtrType>
	getPropertyPtrTypeList ()
	{
		return m_propertyPtrTypeList;
	}

	sl::ConstList <NamedImportType>
	getNamedImportTypeList ()
	{
		return m_namedImportTypeList;
	}

	sl::ConstList <ImportPtrType>
	getImportPtrTypeList ()
	{
		return m_importPtrTypeList;
	}

	sl::ConstList <ImportIntModType>
	getImportIntModTypeList ()
	{
		return m_importIntModTypeList;
	}

	sl::ConstList <ReactorClassType>
	getReactorClassTypeList ()
	{
		return m_reactorClassTypeList;
	}

	sl::ConstList <FunctionClosureClassType>
	getFunctionClosureClassTypeList ()
	{
		return m_functionClosureClassTypeList;
	}

	sl::ConstList <PropertyClosureClassType>
	getPropertyClosureClassTypeList ()
	{
		return m_propertyClosureClassTypeList;
	}

	sl::ConstList <DataClosureClassType>
	getDataClosureClassTypeList ()
	{
		return m_dataClosureClassTypeList;
	}

	sl::ConstList <MulticastClassType>
	getMulticastClassTypeList ()
	{
		return m_multicastClassTypeList;
	}

	sl::ConstList <McSnapshotClassType>
	getMcSnapshotClassTypeList ()
	{
		return m_mcSnapshotClassTypeList;
	}

	sl::ConstList <Typedef>
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
		sl::BoxList <Token>* elementCountInitializer
		);

	ArrayType*
	getArrayType (
		Type* elementType,
		size_t elementCount
		);

	Typedef*
	createTypedef (
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		Type* type
		);

	TypedefShadowType*
	createTypedefShadowType (Typedef* tdef);

	EnumType*
	createEnumType (
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		Type* baseType = NULL,
		uint_t flags = 0
		);

	EnumType*
	createUnnamedEnumType (
		Type* baseType = NULL,
		uint_t flags = 0
		)
	{
		return createEnumType (sl::String (), sl::String (), baseType, flags);
	}

	StructType*
	createStructType (
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		size_t fieldAlignment = 8
		);

	StructType*
	createUnnamedStructType (size_t fieldAlignment = 8)
	{
		return createStructType (sl::String (), sl::String (), fieldAlignment);
	}

	StructType*
	getStructType (
		Type** memberTypeArray,
		size_t memberCount,
		size_t fieldAlignment = 8
		);

	UnionType*
	createUnionType (
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		size_t fieldAlignment = 8
		);

	UnionType*
	createUnnamedUnionType (size_t fieldAlignment = 8)
	{
		return createUnionType (sl::String (), sl::String (), fieldAlignment);
	}

	ClassType*
	createClassType (
		ClassTypeKind classTypeKind,
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		size_t fieldAlignment = 8,
		uint_t flags = 0
		);

	ClassType*
	createClassType (
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
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
		return createClassType (classTypeKind,	sl::String (), sl::String (), fieldAlignment, flags);
	}

	ClassType*
	createUnnamedClassType (
		size_t fieldAlignment = 8,
		uint_t flags = 0
		)
	{
		return createClassType (ClassTypeKind_Normal, sl::String (), sl::String (), fieldAlignment, flags);
	}

	FunctionArg*
	createFunctionArg (
		const sl::StringRef& name,
		Type* type,
		uint_t ptrTypeFlags = 0,
		sl::BoxList <Token>* initializer = NULL
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
		const sl::StringRef& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList <Token>* constructor = NULL,
		sl::BoxList <Token>* initializer = NULL
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
		const sl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		);

	FunctionType*
	getFunctionType (
		Type* returnType,
		const sl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		)
	{
		return getFunctionType (m_callConvArray [CallConvKind_Default], returnType, argArray, flags);
	}

	FunctionType*
	getFunctionType (
		const sl::Array <FunctionArg*>& argArray,
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
		const sl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		);

	FunctionType*
	createUserFunctionType (
		Type* returnType,
		const sl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		)
	{
		return createUserFunctionType (m_callConvArray [CallConvKind_Default], returnType, argArray, flags);
	}

	FunctionType*
	createUserFunctionType (
		const sl::Array <FunctionArg*>& argArray,
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
		const sl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		);

	PropertyType*
	getIndexedPropertyType (
		Type* returnType,
		const sl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		)
	{
		return getIndexedPropertyType (NULL, returnType, argArray, flags);
	}

	PropertyType*
	createIndexedPropertyType (
		CallConv* callConv,
		Type* returnType,
		const sl::Array <FunctionArg*>& argArray,
		uint_t flags = 0
		);

	PropertyType*
	createIndexedPropertyType (
		Type* returnType,
		const sl::Array <FunctionArg*>& argArray,
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
	getReactorIfaceType (FunctionType* startMethodType);

	ReactorClassType*
	createReactorType (
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
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
		size_t thisArgIdx
		);

	PropertyClosureClassType*
	getPropertyClosureClassType (
		PropertyType* targetType,
		PropertyType* thunkType,
		Type* const* argTypeArray,
		const size_t* closureMap,
		size_t argCount,
		size_t thisArgIdx
		);

	DataClosureClassType*
	getDataClosureClassType (
		Type* targetType,
		PropertyType* thunkType
		);

	DataPtrType*
	getDataPtrType (
		Type* targetType,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		);

	DataPtrType*
	getDataPtrType (
		Type* targetType,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getDataPtrType (targetType, TypeKind_DataPtr, ptrTypeKind, flags);
	}

	ClassPtrType*
	getClassPtrType (
		ClassType* targetType,
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
		);

	ClassPtrType*
	getClassPtrType (
		ClassType* targetType,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getClassPtrType (targetType, TypeKind_ClassPtr, ptrTypeKind, flags);
	}

	FunctionPtrType*
	getFunctionPtrType (
		FunctionType* targetType,
		TypeKind typeKind,
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal,
		uint_t flags = 0
		);

	FunctionPtrType*
	getFunctionPtrType (
		FunctionType* targetType,
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getFunctionPtrType (targetType, TypeKind_FunctionPtr, ptrTypeKind, flags);
	}

	PropertyPtrType*
	getPropertyPtrType (
		PropertyType* targetType,
		TypeKind typeKind,
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
		);

	PropertyPtrType*
	getPropertyPtrType (
		PropertyType* targetType,
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
		)
	{
		return getPropertyPtrType (targetType, TypeKind_PropertyPtr, ptrTypeKind, flags);
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

	Type*
	foldDualType (
		Type* type,
		bool isAlien,
		bool isContainerConst
		);

protected:
	DualTypeTuple*
	getDualTypeTuple (Type* type);

	SimplePropertyTypeTuple*
	getSimplePropertyTypeTuple (Type* type);

	FunctionArgTuple*
	getFunctionArgTuple (Type* type);

	DataPtrTypeTuple*
	getDataPtrTypeTuple (Type* type);

	ClassPtrTypeTuple*
	getClassPtrTypeTuple (ClassType* classType);

	ClassPtrTypeTuple*
	getEventClassPtrTypeTuple (MulticastClassType* classType);

	FunctionPtrTypeTuple*
	getFunctionPtrTypeTuple (FunctionType* functionType);

	PropertyPtrTypeTuple*
	getPropertyPtrTypeTuple (PropertyType* propertyType);

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
		const sl::StringRef& signature
		);

	void
	setupStdTypedef (
		StdTypedef stdTypedef,
		TypeKind typeKind,
		const sl::StringRef& name
		);

	NamedType*
	parseStdType (StdType stdType);

	NamedType*
	parseStdType (
		StdNamespace stdNamespace,
		const sl::StringRef& source
		);

	ClassType*
	createAbstractClassType ();

	StructType*
	createAbstractDataType ();

	StructType*
	createSimpleIfaceHdrType ();

	StructType*
	createBoxType ();

	StructType*
	createDataBoxType ();

	StructType*
	createDynamicArrayBoxType ();

	StructType*
	createStaticDataBoxType ();

	StructType*
	createDataPtrValidatorType ();

	StructType*
	createDataPtrStructType ();

	StructType*
	createFunctionPtrStructType ();

	StructType*
	createVariantStructType ();

	StructType*
	createGcShadowStackFrameType ();

	StructType*
	createSjljFrameType ();
};

//..............................................................................

} // namespace ct
} // namespace jnc
