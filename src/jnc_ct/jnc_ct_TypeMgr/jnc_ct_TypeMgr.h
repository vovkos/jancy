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

#include "jnc_ClassType.h"
#include "jnc_PropertyType.h"
#include "jnc_ct_JnccallCallConv.h"
#include "jnc_ct_CallConv_msc32.h"
#include "jnc_ct_CallConv_gcc32.h"
#include "jnc_ct_CdeclCallConv_gcc64.h"
#include "jnc_ct_CdeclCallConv_msc64.h"
#include "jnc_ct_Typedef.h"
#include "jnc_ct_StructType.h"
#include "jnc_ct_ImportType.h"

namespace jnc {
namespace ct {

class Module;
class Declarator;

class ArrayType;
class EnumType;
class StructType;
class UnionType;
class ClassType;
class FunctionType;
class PropertyType;
class DataPtrType;
class ClassPtrType;
class FunctionPtrType;
class PropertyPtrType;
class ReactorClassType;
class DynamicLibClassType;
class ClosureClassType;
class FunctionClosureClassType;
class PropertyClosureClassType;
class DataClosureClassType;
class MulticastClassType;
class McSnapshotClassType;
class TemplateArgType;
class TemplatePtrType;
class TemplateIntModType;
class TemplateDeclType;

struct DataPtrTypeTuple;
struct ClassPtrTypeTuple;
struct FunctionPtrTypeTuple;
struct PropertyPtrTypeTuple;

//..............................................................................

struct DualPtrTypeTuple: sl::ListLink {
	union {
		DataPtrTypeTuple* m_readOnlyDataPtrTypeTuple;
		ClassPtrTypeTuple* m_readOnlyClassPtrTypeTuple;
		PropertyPtrTypeTuple* m_readOnlyPropertyPtrTypeTuple;
	};

	ClassPtrTypeTuple* m_dualEventClassPtrTypeTuple;
};

//..............................................................................

class TypeMgr {
	friend class Module;

protected:
	Module* m_module;

	Type m_primitiveTypeArray[TypeKind__PrimitiveTypeCount];
	Type* m_stdTypeArray[StdType__Count];
	Typedef m_stdTypedefArray[StdTypedef__Count];
	CallConv* m_callConvArray[CallConvKind__Count];

	JnccallCallConv_msc32 m_jnccallCallConv_msc32;
	JnccallCallConv_msc64 m_jnccallCallConv_msc64;
	JnccallCallConv_gcc32 m_jnccallCallConv_gcc32;
	JnccallCallConv_gcc64 m_jnccallCallConv_gcc64;
	JnccallCallConv_arm32 m_jnccallCallConv_arm32;
	JnccallCallConv_arm64 m_jnccallCallConv_arm64;
	CdeclCallConv_msc32 m_cdeclCallConv_msc32;
	CdeclCallConv_msc64 m_cdeclCallConv_msc64;
	CdeclCallConv_gcc32 m_cdeclCallConv_gcc32;
	CdeclCallConv_gcc64 m_cdeclCallConv_gcc64;
	CdeclCallConv_arm32 m_cdeclCallConv_arm32;
	CdeclCallConv_arm64 m_cdeclCallConv_arm64;
	StdcallCallConv_msc32 m_stdcallCallConv_msc32;
	StdcallCallConv_gcc32 m_stdcallCallConv_gcc32;
	ThiscallCallConv_msc32 m_thiscallCallConv_msc32;

	sl::List<Type> m_typeList;
	sl::List<Typedef> m_typedefList;
	sl::List<FunctionArg> m_functionArgList;
	sl::List<Field> m_fieldList;

	sl::List<SimplePropertyTypeTuple> m_simplePropertyTypeTupleList;
	sl::List<FunctionArgTuple> m_functionArgTupleList;
	sl::List<DataPtrTypeTuple> m_dataPtrTypeTupleList;
	sl::List<ClassPtrTypeTuple> m_classPtrTypeTupleList;
	sl::List<FunctionPtrTypeTuple> m_functionPtrTypeTupleList;
	sl::List<PropertyPtrTypeTuple> m_propertyPtrTypeTupleList;
	sl::List<DualTypeTuple> m_dualTypeTupleList;

	sl::Array<MulticastClassType*> m_multicastClassTypeArray;
	sl::SimpleHashTable<DerivableType*, bool> m_externalReturnTypeSet;

	sl::StringHashTable<Type*> m_typeMap;
	size_t m_unnamedTypeId;

public:
	TypeMgr();

	Module*
	getModule() {
		return m_module;
	}

	void
	clear();

	const sl::List<Type>&
	getTypeList() {
		return m_typeList;
	}

	const sl::List<Typedef>&
	getTypedefList() {
		return m_typedefList;
	}

	const sl::Array<MulticastClassType*>&
	getMulticastClassTypeArray() {
		return m_multicastClassTypeArray;
	}

	Type*
	getPrimitiveType(TypeKind typeKind) {
		ASSERT(typeKind < TypeKind__PrimitiveTypeCount);
		return &m_primitiveTypeArray[typeKind];
	}

	bool
	isStdTypeUsed(StdType stdType) {
		ASSERT(stdType < StdType__Count);
		return m_stdTypeArray[stdType] != NULL;
	}

	Type*
	getStdType(StdType stdType);

	Typedef*
	getStdTypedef(StdTypedef stdTypedef) {
		ASSERT(stdTypedef < StdTypedef__Count);
		return &m_stdTypedefArray[stdTypedef];
	}

	void
	createStdTypes();

	Type*
	getInt32Type(int32_t integer) {
		return getPrimitiveType(getInt32TypeKind(integer));
	}

	Type*
	getUInt32Type(uint32_t integer) {
		return getPrimitiveType(getInt32TypeKind_u(integer));
	}

	Type*
	getInt64Type(int64_t integer) {
		return getPrimitiveType(getInt64TypeKind(integer));
	}

	Type*
	getInt64Type_u(uint64_t integer) {
		return getPrimitiveType(getInt64TypeKind_u(integer));
	}

	ArrayType*
	createAutoSizeArrayType(Type* elementType);

	ArrayType*
	createArrayType(
		Type* elementType,
		sl::List<Token>* elementCountInitializer
	);

	ArrayType*
	getArrayType(
		Type* elementType,
		size_t elementCount
	);

	Typedef*
	createTypedef(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		Type* type
	);

	TypedefShadowType*
	createTypedefShadowType(Typedef* tdef);

	EnumType*
	createEnumType(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		Type* baseType = NULL,
		uint_t flags = 0
	);

	EnumType*
	createUnnamedEnumType(
		Type* baseType = NULL,
		uint_t flags = 0
	) {
		return createEnumType(
			sl::StringRef(),
			sl::formatString("enum.%d", createUnnamedTypeId()),
			baseType,
			flags
		);
	}

	StructType*
	createStructType(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		size_t fieldAlignment = 8
	);

	StructType*
	createInternalStructType(
		const sl::StringRef& tag,
		size_t fieldAlignment = 8
	);

	StructType*
	createUnnamedStructType(size_t fieldAlignment = 8) {
		return createStructType(
			sl::StringRef(),
			sl::formatString("struct.%d", createUnnamedTypeId()),
			fieldAlignment
		);
	}

	StructType*
	createUnnamedInternalStructType(
		const sl::StringRef& tag,
		size_t fieldAlignment = 8
	) {
		return createInternalStructType(
			sl::formatString("struct.%s.%d", tag.sz(), createUnnamedTypeId()),
			fieldAlignment
		);
	}

	UnionType*
	createUnionType(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		size_t fieldAlignment = 8
	);

	UnionType*
	createUnnamedUnionType(size_t fieldAlignment = 8) {
		return createUnionType(
			sl::StringRef(),
			sl::formatString("union.%d", createUnnamedTypeId()),
			fieldAlignment
		);
	}

	template <typename T>
	T*
	createClassType(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		size_t fieldAlignment = 8,
		uint_t flags = 0
	);

	ClassType*
	createClassType(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		size_t fieldAlignment = 8,
		uint_t flags = 0
	) {
		return createClassType<ClassType>(name, qualifiedName, fieldAlignment, flags);
	}

	template <typename T>
	T*
	createUnnamedClassType(
		size_t fieldAlignment = 8,
		uint_t flags = 0
	) {
		return createClassType<T>(
			sl::StringRef(),
			sl::formatString("class.%d", createUnnamedTypeId()),
			fieldAlignment,
			flags
		);
	}

	ClassType*
	createUnnamedClassType(
		size_t fieldAlignment = 8,
		uint_t flags = 0
	) {
		return createUnnamedClassType<ClassType>(fieldAlignment, flags);
	}

	template <typename T>
	T*
	createInternalClassType(
		const sl::StringRef& tag,
		size_t fieldAlignment = 8,
		uint_t flags = 0
	);

	ClassType*
	createInternalClassType(
		const sl::StringRef& tag,
		size_t fieldAlignment = 8,
		uint_t flags = 0
	) {
		return createInternalClassType<ClassType>(tag, fieldAlignment, flags);
	}

	template <typename T>
	T*
	createUnnamedInternalClassType(
		const sl::StringRef& tag,
		size_t fieldAlignment = 8,
		uint_t flags = 0
	) {
		return createInternalClassType<T>(
			sl::formatString("class.%s.%d", tag.sz(), createUnnamedTypeId()),
			fieldAlignment,
			flags
		);
	}

	ClassType*
	createUnnamedInternalClassType(
		const sl::StringRef& tag,
		size_t fieldAlignment = 8,
		uint_t flags = 0
	) {
		return createUnnamedInternalClassType<ClassType>(tag, fieldAlignment, flags);
	}

	void
	addExternalReturnType(DerivableType* type) {
		m_externalReturnTypeSet.visit(type);
	}

	bool
	requireExternalReturnTypes();

	FunctionArg*
	createFunctionArg(
		const sl::StringRef& name,
		Type* type,
		uint_t ptrTypeFlags = 0,
		sl::List<Token>* initializer = NULL
	);

	FunctionArg*
	getSimpleFunctionArg(
		StorageKind storageKind,
		Type* type,
		uint_t ptrTypeFlags = 0
	);

	FunctionArg*
	getSimpleFunctionArg(
		Type* type,
		uint_t ptrTypeFlags = 0
	) {
		return getSimpleFunctionArg(StorageKind_Stack, type, ptrTypeFlags);
	}

	Field*
	createField(
		const sl::StringRef& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::List<Token>* constructor = NULL,
		sl::List<Token>* initializer = NULL
	);

	CallConv*
	getCallConv(CallConvKind callConvKind) {
		ASSERT(callConvKind < CallConvKind__Count);
		return m_callConvArray[callConvKind];
	}

	FunctionType*
	getFunctionType(
		CallConv* callConv,
		Type* returnType,
		const sl::Array<FunctionArg*>& argArray,
		uint_t flags = 0
	);

	FunctionType*
	getFunctionType(
		Type* returnType,
		const sl::Array<FunctionArg*>& argArray,
		uint_t flags = 0
	) {
		return getFunctionType(m_callConvArray[CallConvKind_Default], returnType, argArray, flags);
	}

	FunctionType*
	getFunctionType(
		const sl::Array<FunctionArg*>& argArray,
		uint_t flags = 0
	) {
		return getFunctionType(
			m_callConvArray[CallConvKind_Default],
			&m_primitiveTypeArray[TypeKind_Void],
			argArray,
			flags
		);
	}

	FunctionType*
	getFunctionType(
		CallConv* callConv,
		Type* returnType,
		Type* const* argType,
		size_t argCount,
		uint_t flags = 0
	);

	FunctionType*
	getFunctionType(
		Type* returnType,
		Type* const* argType,
		size_t argCount,
		uint_t flags = 0
	) {
		return getFunctionType(m_callConvArray[CallConvKind_Default], returnType, argType, argCount, flags);
	}

	FunctionType*
	getFunctionType(
		Type* const* argType,
		size_t argCount,
		uint_t flags = 0
	) {
		return getFunctionType(
			m_callConvArray[CallConvKind_Default],
			&m_primitiveTypeArray[TypeKind_Void],
			argType,
			argCount,
			flags
		);
	}

	FunctionType*
	createUserFunctionType(
		CallConv* callConv,
		Type* returnType,
		const sl::Array<FunctionArg*>& argArray,
		uint_t flags = 0
	);

	FunctionType*
	createUserFunctionType(
		Type* returnType,
		const sl::Array<FunctionArg*>& argArray,
		uint_t flags = 0
	) {
		return createUserFunctionType(m_callConvArray[CallConvKind_Default], returnType, argArray, flags);
	}

	FunctionType*
	createUserFunctionType(
		const sl::Array<FunctionArg*>& argArray,
		uint_t flags = 0
	) {
		return createUserFunctionType(
			m_callConvArray[CallConvKind_Default],
			&m_primitiveTypeArray[TypeKind_Void],
			argArray,
			flags
		);
	}

	FunctionType*
	getMemberMethodType(
		DerivableType* parentType,
		FunctionType* functionType,
		uint_t thisArgPtrTypeFlags = 0
	);

	FunctionType*
	getStdObjectMemberMethodType(FunctionType* functionType);

	PropertyType*
	getPropertyType(
		FunctionType* getterType,
		const FunctionTypeOverload& setterType,
		uint_t flags = 0
	);

	PropertyType*
	getSimplePropertyType(
		CallConv* callConv,
		Type* returnType,
		uint_t flags = 0
	);

	PropertyType*
	getSimplePropertyType(
		Type* returnType,
		uint_t flags = 0
	) {
		return getSimplePropertyType(m_callConvArray[CallConvKind_Default], returnType, flags);
	}

	PropertyType*
	createIndexedPropertyType(
		CallConv* callConv,
		Type* returnType,
		const sl::Array<FunctionArg*>& argArray,
		uint_t flags = 0
	);

	PropertyType*
	createIndexedPropertyType(
		Type* returnType,
		const sl::Array<FunctionArg*>& argArray,
		uint_t flags = 0
	) {
		return createIndexedPropertyType(NULL, returnType, argArray, flags);
	}

	PropertyType*
	getMemberPropertyType(
		DerivableType* parentType,
		PropertyType* propertyType
	);

	PropertyType*
	getStdObjectMemberPropertyType(PropertyType* propertyType);

	PropertyType*
	getShortPropertyType(PropertyType* propertyType);

	ClassType*
	getMulticastType(
		FunctionType* functionType,
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal
	) {
		return getMulticastType(getFunctionPtrType(functionType, ptrTypeKind));
	}

	ClassType*
	getMulticastType(FunctionPtrType* functionPtrType);

	ClassType*
	createReactorBaseType();

	ReactorClassType*
	createReactorType(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		ClassType* parentType
	);

	FunctionClosureClassType*
	createReactorClosureType();

	FunctionClosureClassType*
	getFunctionClosureClassType(
		FunctionType* targetType,
		FunctionType* thunkType,
		Type* const* argTypeArray,
		const size_t* closureMap,
		size_t argCount,
		size_t thisArgIdx
	);

	PropertyClosureClassType*
	getPropertyClosureClassType(
		PropertyType* targetType,
		PropertyType* thunkType,
		Type* const* argTypeArray,
		const size_t* closureMap,
		size_t argCount,
		size_t thisArgIdx
	);

	DataClosureClassType*
	getDataClosureClassType(
		Type* targetType,
		PropertyType* thunkType
	);

	DataPtrType*
	getDataPtrType(
		Type* targetType,
		uint_t bitOffset,
		uint_t bitCount,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
	);

	DataPtrType*
	getDataPtrType(
		Type* targetType,
		TypeKind typeKind,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
	);

	DataPtrType*
	getDataPtrType(
		Type* targetType,
		DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal,
		uint_t flags = 0
	) {
		return getDataPtrType(targetType, 0, 0, TypeKind_DataPtr, ptrTypeKind, flags);
	}

	ClassPtrType*
	getClassPtrType(
		ClassType* targetType,
		TypeKind typeKind,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
	);

	ClassPtrType*
	getClassPtrType(
		ClassType* targetType,
		ClassPtrTypeKind ptrTypeKind = ClassPtrTypeKind_Normal,
		uint_t flags = 0
	) {
		return getClassPtrType(targetType, TypeKind_ClassPtr, ptrTypeKind, flags);
	}

	FunctionPtrType*
	getFunctionPtrType(
		FunctionType* targetType,
		TypeKind typeKind,
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal,
		uint_t flags = 0
	);

	FunctionPtrType*
	getFunctionPtrType(
		FunctionType* targetType,
		FunctionPtrTypeKind ptrTypeKind = FunctionPtrTypeKind_Normal,
		uint_t flags = 0
	) {
		return getFunctionPtrType(targetType, TypeKind_FunctionPtr, ptrTypeKind, flags);
	}

	PropertyPtrType*
	getPropertyPtrType(
		PropertyType* targetType,
		TypeKind typeKind,
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
	);

	PropertyPtrType*
	getPropertyPtrType(
		PropertyType* targetType,
		PropertyPtrTypeKind ptrTypeKind = PropertyPtrTypeKind_Normal,
		uint_t flags = 0
	) {
		return getPropertyPtrType(targetType, TypeKind_PropertyPtr, ptrTypeKind, flags);
	}

	StructType*
	getPropertyVtableStructType(PropertyType* propertyType);

	NamedImportType*
	getNamedImportType(
		QualifiedName* name, // destructive
		Namespace* anchorNamespace,
		QualifiedName* anchorName = NULL // destructive
	);

	template <
		typename T,
		typename B
	>
	T*
	getModType(
		B* baseType,
		uint_t typeModifiers
	);

	ImportPtrType*
	getImportPtrType(
		NamedImportType* baseType,
		uint_t typeModifiers
	) {
		return getModType<ImportPtrType>(baseType, typeModifiers);
	}

	ImportIntModType*
	getImportIntModType(
		NamedImportType* baseType,
		uint_t typeModifiers
	) {
		return getModType<ImportIntModType>(baseType, typeModifiers);
	}

	TemplateArgType*
	getTemplateArgType(
		const sl::StringRef& name,
		size_t index
	);

	TemplatePtrType*
	getTemplatePtrType(
		TemplateArgType* baseType,
		uint_t typeModifiers
	) {
		return getModType<TemplatePtrType>(baseType, typeModifiers);
	}

	TemplateIntModType*
	getTemplateIntModType(
		TemplateArgType* baseType,
		uint_t typeModifiers
	) {
		return getModType<TemplateIntModType>(baseType, typeModifiers);
	}

	TemplateDeclType*
	createTemplateDeclType(Declarator* declarator);

	Type*
	foldDualType(
		Type* type,
		bool isAlien,
		bool isContainerConst
	);

protected:
	size_t
	createUnnamedTypeId() {
		return ++m_unnamedTypeId;
	}

	void
	addClassType(
		ClassType* type,
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		size_t fieldAlignment,
		uint_t flags
	);

	DualTypeTuple*
	getDualTypeTuple(Type* type);

	SimplePropertyTypeTuple*
	getSimplePropertyTypeTuple(Type* type);

	FunctionArgTuple*
	getFunctionArgTuple(Type* type);

	DataPtrTypeTuple*
	getDataPtrTypeTuple(
		Type* type,
		uint_t flags
	);

	ClassPtrTypeTuple*
	getClassPtrTypeTuple(ClassType* classType);

	ClassPtrTypeTuple*
	getEventClassPtrTypeTuple(MulticastClassType* classType);

	FunctionPtrTypeTuple*
	getFunctionPtrTypeTuple(FunctionType* functionType);

	PropertyPtrTypeTuple*
	getPropertyPtrTypeTuple(PropertyType* propertyType);

	void
	setupAllPrimitiveTypes();

	void
	setupStdTypedefArray();

	void
	setupCallConvArray();

	void
	setupPrimitiveType(
		TypeKind typeKind,
		const sl::StringRef& signature,
		size_t size,
		size_t alignment,
		uint_t flags
	);

	void
	setupStdTypedef(
		StdTypedef stdTypedef,
		TypeKind typeKind,
		const sl::StringRef& name
	);

	NamedType*
	parseStdType(StdType stdType);

	NamedType*
	parseStdType(
		StdType stdType,
		Unit* unit
	);

	NamedType*
	parseStdType(
		const sl::StringRef& source,
		StdNamespace stdNamespace,
		Unit* unit
	);

	ClassType*
	createAbstractClassType();

	StructType*
	createAbstractDataType();

	StructType*
	createIfaceHdrType();

	StructType*
	createBoxType();

	StructType*
	createDataBoxType();

	StructType*
	createDetachedDataBoxType();

	StructType*
	createDataPtrValidatorType();

	StructType*
	createDataPtrStructType();

	StructType*
	createFunctionPtrStructType();

	StructType*
	createVariantStructType();

	StructType*
	createStringStructType();

	StructType*
	createGcShadowStackFrameType();

	StructType*
	createSjljFrameType();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
StructType*
TypeMgr::createInternalStructType(
	const sl::StringRef& tag,
	size_t fieldAlignment
) {
	StructType* type = createStructType(sl::StringRef(), tag, fieldAlignment);
	type->m_namespaceStatus = NamespaceStatus_Ready;
	return type;
}

template <typename T>
T*
TypeMgr::createClassType(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	size_t fieldAlignment,
	uint_t flags
) {
	T* type = new T;
	addClassType(type, name, qualifiedName, fieldAlignment, flags);
	return type;
}

template <typename T>
T*
TypeMgr::createInternalClassType(
	const sl::StringRef& tag,
	size_t fieldAlignment,
	uint_t flags
) {
	T* type = createClassType<T>(sl::StringRef(), tag, fieldAlignment, flags);
	type->m_namespaceStatus = NamespaceStatus_Ready;
	return type;
}

template <
	typename T,
	typename B
>
T*
TypeMgr::getModType(
	B* baseType,
	uint_t typeModifiers
) {
	sl::String signature = T::createSignature(baseType, typeModifiers);
	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value) {
		ASSERT(it->m_value->m_signature == signature);
		return (T*)it->m_value;
	}

	T* type = new T;
	type->m_module = m_module;
	type->m_baseType = baseType;
	type->m_typeModifiers = typeModifiers;
	type->m_signature = signature;
	type->m_flags |= TypeFlag_SignatureReady;

	m_typeList.insertTail(type);
	it->m_value = type;
	return type;
}

//..............................................................................

} // namespace ct
} // namespace jnc
