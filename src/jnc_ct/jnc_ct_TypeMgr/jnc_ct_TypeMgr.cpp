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

#include "pch.h"
#include "jnc_ct_TypeMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_DeclTypeCalc.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_EnumType.h"
#include "jnc_ct_UnionType.h"
#include "jnc_ct_ClassType.h"
#include "jnc_ct_FunctionType.h"
#include "jnc_ct_PropertyType.h"
#include "jnc_ct_DataPtrType.h"
#include "jnc_ct_ClassPtrType.h"
#include "jnc_ct_FunctionPtrType.h"
#include "jnc_ct_PropertyPtrType.h"
#include "jnc_ct_ReactorClassType.h"
#include "jnc_ct_ClosureClassType.h"
#include "jnc_ct_MulticastClassType.h"
#include "jnc_ct_McSnapshotClassType.h"
#include "jnc_ct_TemplateType.h"
#include "jnc_ct_Parser.llk.h"
#include "jnc_Variant.h"
#include "jnc_String.h"

namespace jnc {
namespace ct {

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
NamedType*
TypeMgr::parseStdType(StdType stdType) {
	return parseStdType(stdType, m_module->m_unitMgr.getCoreLibUnit());
}

//..............................................................................

TypeMgr::TypeMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	setupAllPrimitiveTypes();
	setupStdTypedefArray();
	setupCallConvArray();

	memset(m_stdTypeArray, 0, sizeof(m_stdTypeArray));
}

void
TypeMgr::clear() {
	m_typeList.clear();
	m_typedefList.clear();
	m_functionArgList.clear();
	m_fieldList.clear();
	m_simplePropertyTypeTupleList.clear();
	m_functionArgTupleList.clear();
	m_dataPtrTypeTupleList.clear();
	m_classPtrTypeTupleList.clear();
	m_functionPtrTypeTupleList.clear();
	m_propertyPtrTypeTupleList.clear();
	m_dualTypeTupleList.clear();
	m_typeMap.clear();
	m_importTypeNameAnchorArray.clear();
	m_multicastClassTypeArray.clear();
	m_externalReturnTypeSet.clear();

	setupAllPrimitiveTypes();

	memset(m_stdTypeArray, 0, sizeof(m_stdTypeArray));
}

void
TypeMgr::createStdTypes() {
	// these std types may be required at runtime without being referenced at compile time

	getStdType(StdType_AbstractClassPtr);
	getStdType(StdType_DetachedDataBox);
	getStdType(StdType_DataPtrValidator);
}


Type*
TypeMgr::getStdType(StdType stdType) {
	ASSERT((size_t)stdType < StdType__Count);
	if (m_stdTypeArray[stdType])
		return m_stdTypeArray[stdType];

	// all required std types should be ready at ModuleCompileState_Compiled
	// (parsed types starting with StdType_GcTriggers may not be in the table yet)

	ASSERT(
		m_module->getCompileState() < ModuleCompileState_Compiled ||
		(size_t)stdType >= StdType_GcTriggers);

	const char* name;
	Type* type;
	switch (stdType) {
	case StdType_ByteThinPtr:
		type = getPrimitiveType(TypeKind_Byte)->getDataPtrType_c();
		break;

	case StdType_CharConstThinPtr:
		type = getPrimitiveType(TypeKind_Char)->getDataPtrType_c(
			TypeKind_DataPtr,
			PtrTypeFlag_Const
		);
		break;

	case StdType_CharConstPtr:
		type = getPrimitiveType(TypeKind_Char)->getDataPtrType(
			TypeKind_DataPtr,
			DataPtrTypeKind_Normal,
			PtrTypeFlag_Const
		);
		break;

	case StdType_IfaceHdr:
		type = createIfaceHdrType();
		break;

	case StdType_IfaceHdrPtr:
		type = getStdType(StdType_IfaceHdr)->getDataPtrType_c();
		break;

	case StdType_Box:
		type = createBoxType();
		break;

	case StdType_BoxPtr:
		type = getStdType(StdType_Box)->getDataPtrType_c();
		break;

	case StdType_DataBox:
		type = createDataBoxType();
		break;

	case StdType_DataBoxPtr:
		type = getStdType(StdType_DataBox)->getDataPtrType_c();
		break;

	case StdType_DetachedDataBox:
		type = createDetachedDataBoxType();
		break;

	case StdType_DetachedDataBoxPtr:
		type = getStdType(StdType_DetachedDataBox)->getDataPtrType_c();
		break;

	case StdType_DataPtrValidator:
		type = createDataPtrValidatorType();
		break;

	case StdType_DataPtrValidatorPtr:
		type = getStdType(StdType_DataPtrValidator)->getDataPtrType_c();
		break;

	case StdType_DataPtrStruct:
		type = createDataPtrStructType();
		break;

	case StdType_FunctionPtrStruct:
		type = createFunctionPtrStructType();
		break;

	case StdType_VariantStruct:
		type = createVariantStructType();
		break;

	case StdType_StringStruct:
		type = createStringStructType();
		break;

	case StdType_GcShadowStackFrame:
		type = createGcShadowStackFrameType();
		break;

	case StdType_SjljFrame:
		type = createSjljFrameType();
		break;

	case StdType_AbstractClass:
		type = createAbstractClassType();
		break;

	case StdType_AbstractClassPtr:
		type = ((ClassType*)getStdType(StdType_AbstractClass))->getClassPtrType();
		break;

	case StdType_AbstractData:
		type = createAbstractDataType();
		break;

	case StdType_AbstractDataPtr:
		type = getStdType(StdType_AbstractData)->getDataPtrType();
		break;

	case StdType_SimpleFunction:
		type = getFunctionType(getPrimitiveType(TypeKind_Void), NULL, 0);
		break;

	case StdType_SimpleMulticast:
		type = getMulticastType((FunctionType*)getStdType(StdType_SimpleFunction));
		break;

	case StdType_SimpleEventPtr:
		type = ((ClassType*)getStdType(StdType_SimpleMulticast))->getClassPtrType(ClassPtrTypeKind_Normal);
		break;

	case StdType_Binder:
		type = getFunctionType(getStdType(StdType_SimpleEventPtr), NULL, 0);
		break;

	case StdType_PromisePtr:
		type = ((ClassType*)getStdType(StdType_Promise))->getClassPtrType(ClassPtrTypeKind_Normal);
		break;

	case StdType_SchedulerPtr:
		type = ((ClassType*)getStdType(StdType_Scheduler))->getClassPtrType(ClassPtrTypeKind_Normal);
		break;

	case StdType_ReactorBase:
		type = createReactorBaseType();
		break;

	case StdType_ReactorClosure:
		type = createReactorClosureType();
		break;

	case StdType_GcTriggers:
	case StdType_GcStats:
	case StdType_RegexCapture:
	case StdType_RegexMatch:
	case StdType_RegexState:
	case StdType_RegexFlags:
	case StdType_RegexExecFlags:
	case StdType_Regex:
	case StdType_Promise:
	case StdType_Promisifier:
	case StdType_DynamicLayout:
	case StdType_DynamicSection:
	case StdType_DynamicLib:
	case StdType_Scheduler:
	case StdType_ModuleItem:
	case StdType_ModuleItemDecl:
	case StdType_ModuleItemInitializer:
	case StdType_Attribute:
	case StdType_AttributeBlock:
	case StdType_Namespace:
	case StdType_GlobalNamespace:
	case StdType_Type:
	case StdType_DataPtrType:
	case StdType_NamedType:
	case StdType_MemberBlock:
	case StdType_BaseTypeSlot:
	case StdType_DerivableType:
	case StdType_ArrayType:
	case StdType_FunctionArg:
	case StdType_FunctionType:
	case StdType_FunctionPtrType:
	case StdType_PropertyType:
	case StdType_PropertyPtrType:
	case StdType_EnumConst:
	case StdType_EnumType:
	case StdType_ClassType:
	case StdType_ClassPtrType:
	case StdType_Field:
	case StdType_StructType:
	case StdType_UnionType:
	case StdType_Alias:
	case StdType_Variable:
	case StdType_Const:
	case StdType_Function:
	case StdType_FunctionOverload:
	case StdType_Property:
	case StdType_Typedef:
	case StdType_Module:
	case StdType_Unit:
		name = getStdTypeName(stdType);
		ASSERT(name);

		type = (Type*)m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc)->findDirectChildItem(name).m_item;
		ASSERT(type && type->getItemKind() == ModuleItemKind_Type);
		break;

	case StdType_FmtLiteral:
	case StdType_Int64Int64:
	case StdType_Fp64Fp64:
	case StdType_Int64Fp64:
	case StdType_Fp64Int64:
		type = parseStdType(stdType);
		break;

	default:
		ASSERT(false);
		return NULL;
	}

	type->m_stdType = stdType;
	m_stdTypeArray[stdType] = type;
	return type;
}

ArrayType*
TypeMgr::getArrayType(
	Type* elementType,
	size_t elementCount
) {
	sl::String signature = ArrayType::createSignature(elementType, elementCount);
	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
		return (ArrayType*)it->m_value;

	ArrayType* type = createArrayType<ArrayType>(
		elementType,
		elementType->getFlags() & (TypeFlag_SignatureReady | TypeFlag_SignatureFinal)
	);

	type->m_elementCount = elementCount;
	type->m_signature = signature;
	it->m_value = type;
	return type;
}

Typedef*
TypeMgr::createTypedef(
	const sl::StringRef& name,
	Type* type
) {
	Typedef* tdef = new Typedef;
	tdef->m_module = m_module;
	tdef->m_name = name;
	tdef->m_type = type;
	m_typedefList.insertTail(tdef);

	if (type->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)type)->addFixup(&tdef->m_type);

	return tdef;
}

TypedefShadowType*
TypeMgr::createTypedefShadowType(Typedef* tdef) {
	TypedefShadowType* type = new TypedefShadowType;
	type->m_module = m_module;
	type->m_parentUnit = tdef->m_parentUnit;
	type->m_parentNamespace = tdef->m_parentNamespace;
	type->m_pos = tdef->m_pos;
	type->m_storageKind = tdef->m_storageKind;
	type->m_accessKind = tdef->m_accessKind;
	type->m_name = tdef->m_name;
	type->m_attributeBlock = tdef->m_attributeBlock;
	type->m_typedef = tdef;
	m_typeList.insertTail(type);

	return type;
}

EnumType*
TypeMgr::createEnumType(
	const sl::StringRef& name,
	Type* baseType,
	uint_t flags
) {
	ASSERT(!(flags & ~EnumTypeFlag__All));

	EnumType* type = new EnumType;
	type->m_name = name;

	if (!baseType)
		baseType = getPrimitiveType(TypeKind_Int);

	type->m_module = m_module;
	type->m_baseType = baseType;
	type->m_flags |= flags;

	if (baseType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)baseType)->addFixup(&type->m_baseType);

	m_typeList.insertTail(type);
	return type;
}

StructType*
TypeMgr::createStructType(
	const sl::StringRef& name,
	size_t fieldAlignment
) {
	StructType* type = new StructType;
	type->m_module = m_module;
	type->m_name = name;
	type->m_fieldAlignment = fieldAlignment;
	m_typeList.insertTail(type);
	return type;
}

UnionType*
TypeMgr::createUnionType(
	const sl::StringRef& name,
	size_t fieldAlignment
) {
	UnionType* type = new UnionType;
	type->m_module = m_module;
	type->m_name = name;

	StructType* unionStructType = createInternalStructType("!Struct", fieldAlignment);
	unionStructType->m_parentNamespace = type;
	unionStructType->m_structTypeKind = StructTypeKind_UnionStruct;
	type->m_structType = unionStructType;

	m_typeList.insertTail(type);
	return type;
}

void
TypeMgr::addClassType(
	ClassType* type,
	const sl::StringRef& name,
	size_t fieldAlignment,
	uint_t flags
) {
	ASSERT(!(flags & ~ClassTypeFlag__All));

	type->m_module = m_module;
	type->m_name = name;
	type->m_flags |= flags;

	StructType* ifaceStructType = createInternalStructType("!Iface", fieldAlignment);
	ifaceStructType->m_parentNamespace = type;
	ifaceStructType->m_structTypeKind = StructTypeKind_IfaceStruct;
	ifaceStructType->m_storageKind = StorageKind_Member;

	StructType* classStructType = createInternalStructType("!Class", fieldAlignment);
	classStructType->m_parentNamespace = type;
	classStructType->m_structTypeKind = StructTypeKind_ClassStruct;
	classStructType->createField("!m_box", getStdType(StdType_Box));
	classStructType->createField("!m_iface", ifaceStructType);

	type->m_ifaceStructType = ifaceStructType;
	type->m_classStructType = classStructType;
	m_typeList.insertTail(type);

	if (type->m_classTypeKind == ClassTypeKind_Multicast)
		m_multicastClassTypeArray.append((MulticastClassType*)type);
}

bool
TypeMgr::requireExternalReturnTypes() {
	bool finalResult = true;

	sl::HashTableIterator<DerivableType*, bool> it = m_externalReturnTypeSet.getHead();
	for (; it; it++) {
		DerivableType* type = it->getKey();
		bool result = type->requireExternalReturn();
		if (!result)
			finalResult = false;
	}

	m_externalReturnTypeSet.clear();
	return finalResult;
}

FunctionArg*
TypeMgr::createFunctionArg(
	const sl::StringRef& name,
	Type* type,
	uint_t ptrTypeFlags,
	sl::List<Token>* initializer
) {
	FunctionArg* functionArg = new FunctionArg;
	functionArg->m_module = m_module;
	functionArg->m_name = name;
	functionArg->m_type = type;
	functionArg->m_ptrTypeFlags = ptrTypeFlags;

	if (initializer)
		sl::takeOver(&functionArg->m_initializer, initializer);

	m_functionArgList.insertTail(functionArg);

	if (type->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)type)->addFixup(&functionArg->m_type);

	return functionArg;
}

Field*
TypeMgr::createField(
	const sl::StringRef& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	sl::List<Token>* constructor,
	sl::List<Token>* initializer
) {
	Field* field = new Field;
	field->m_module = m_module;
	field->m_name = name;
	field->m_type = type;
	field->m_ptrTypeFlags = ptrTypeFlags;
	field->m_bitCount = bitCount;

	if (constructor)
		sl::takeOver(&field->m_constructor, constructor);

	if (initializer)
		sl::takeOver(&field->m_initializer, initializer);

	m_fieldList.insertTail(field);

	if (type->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)type)->addFixup(&field->m_type);

	return field;
}

FunctionArg*
TypeMgr::getSimpleFunctionArg(
	StorageKind storageKind,
	Type* type,
	uint_t ptrTypeFlags
) {
	FunctionArgTuple* tuple = getFunctionArgTuple(type);

	// this x const x volatile

	size_t i1 = storageKind == StorageKind_This;
	size_t i2 = (ptrTypeFlags & PtrTypeFlag_Const) != 0;
	size_t i3 = (ptrTypeFlags & PtrTypeFlag_Volatile) != 0;

	if (tuple->m_argArray[i1][i2][i3])
		return tuple->m_argArray[i1][i2][i3];

	FunctionArg* arg = createFunctionArg(sl::String(), type, ptrTypeFlags);
	arg->m_storageKind = storageKind;
	tuple->m_argArray[i1][i2][i3] = arg;
	return arg;
}

FunctionType*
TypeMgr::getFunctionType(
	CallConv* callConv,
	Type* returnType,
	Type* const* argTypeArray,
	size_t argCount,
	uint_t flags
) {
	ASSERT(callConv && returnType);
	ASSERT(!(flags & ~FunctionTypeFlag__All));

	sl::String signature;
	sl::StringRef argSignature;
	flags |= FunctionType::createSignature(
		&signature,
		&argSignature,
		callConv->getCallConvKind(),
		returnType,
		argTypeArray,
		argCount,
		flags
	);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
		return (FunctionType*)it->m_value;

	sl::Array<FunctionArg*> argArray;
	argArray.setCount(argCount);
	sl::Array<FunctionArg*>::Rwi rwi = argArray;
	for (size_t i = 0; i < argCount; i++)
		rwi[i] = getSimpleFunctionArg(argTypeArray[i]);

	FunctionType* type = new FunctionType;
	type->m_module = m_module;
	type->m_callConv = callConv;
	type->m_returnType = returnType;
	type->m_argArray = std::move(argArray);
	type->m_signature = signature;
	type->m_argSignature = argSignature;
	type->m_flags = flags;
	m_typeList.insertTail(type);

	if (returnType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)returnType)->addFixup(&type->m_returnType);

	it->m_value = type;
	return type;
}

FunctionType*
TypeMgr::getFunctionType(
	CallConv* callConv,
	Type* returnType,
	const sl::Array<FunctionArg*>& argArray,
	uint_t flags
) {
	ASSERT(callConv && returnType);
	ASSERT(!(flags & ~FunctionTypeFlag__All));

	sl::String signature;
	sl::StringRef argSignature;
	flags |= FunctionType::createSignature(
		&signature,
		&argSignature,
		callConv->getCallConvKind(),
		returnType,
		argArray,
		flags
	);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
		return (FunctionType*)it->m_value;

	FunctionType* type = new FunctionType;
	type->m_module = m_module;
	type->m_callConv = callConv;
	type->m_returnType = returnType;
	type->m_argArray = argArray;
	type->m_signature = signature;
	type->m_argSignature = argSignature;
	type->m_flags = flags;
	m_typeList.insertTail(type);

	if (returnType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)returnType)->addFixup(&type->m_returnType);

	it->m_value = type;
	return type;
}

FunctionType*
TypeMgr::createUserFunctionType(
	CallConv* callConv,
	Type* returnType,
	const sl::Array<FunctionArg*>& argArray,
	uint_t flags
) {
	ASSERT(callConv && returnType);
	ASSERT(!(flags & ~FunctionTypeFlag__All));

	FunctionType* type = new FunctionType;
	type->m_module = m_module;
	type->m_callConv = callConv;

	if (flags & FunctionTypeFlag_Async) {
		type->m_asyncReturnType = returnType;
		if (returnType->getTypeKindFlags() & TypeKindFlag_Import)
			((ImportType*)returnType)->addFixup(&type->m_asyncReturnType);

		uint_t compileFlags = m_module->getCompileFlags();

		// when compiling stdlib docs, we don't add standard types

		returnType = (compileFlags & ModuleCompileFlag_StdLibDoc) ?
			getStdType(StdType_AbstractClassPtr) :
			getStdType(StdType_PromisePtr);

		if (flags & FunctionTypeFlag_ErrorCode) {
			flags &= ~FunctionTypeFlag_ErrorCode;
			flags |= FunctionTypeFlag_AsyncErrorCode;
		}
	}

	type->m_returnType = returnType;
	type->m_flags = flags | ModuleItemFlag_User;
	type->m_argArray = argArray;
	m_typeList.insertTail(type);

	if (returnType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)returnType)->addFixup(&type->m_returnType);

	return type;
}

FunctionType*
TypeMgr::getMemberMethodType(
	DerivableType* parentType,
	FunctionType* functionType,
	uint_t thisArgPtrTypeFlags
) {
	if (!isClassType(parentType, ClassTypeKind_Abstract)) // std object members are miscellaneous closures
		thisArgPtrTypeFlags |= PtrTypeFlag_Safe;

	Type* returnType = (functionType->m_flags & FunctionTypeFlag_Async) ?
		functionType->m_asyncReturnType :
		functionType->m_returnType;

	Type* thisArgType = parentType->getThisArgType(thisArgPtrTypeFlags);
	FunctionArg* thisArg = getSimpleFunctionArg(StorageKind_This, thisArgType);

	sl::Array<FunctionArg*> argArray = functionType->m_argArray;
	argArray.insert(0, thisArg);

	FunctionType* memberMethodType = (functionType->m_flags & ModuleItemFlag_User) ?
		createUserFunctionType(
			functionType->m_callConv,
			returnType,
			argArray,
			functionType->m_flags & FunctionTypeFlag__All
		) :
		getFunctionType(
			functionType->m_callConv,
			returnType,
			argArray,
			functionType->m_flags & FunctionTypeFlag__All
		);

	memberMethodType->m_shortType = functionType;
	return memberMethodType;
}

FunctionType*
TypeMgr::getStdObjectMemberMethodType(FunctionType* functionType) {
	if (functionType->m_stdObjectMemberMethodType)
		return functionType->m_stdObjectMemberMethodType;

	ClassType* classType = (ClassType*)getStdType(StdType_AbstractClass);
	functionType->m_stdObjectMemberMethodType = classType->getMemberMethodType(functionType);
	return functionType->m_stdObjectMemberMethodType;
}

PropertyType*
TypeMgr::getPropertyType(
	FunctionType* getterType,
	const FunctionTypeOverload& setterType,
	uint_t flags
) {
	ASSERT(!(flags & ~PropertyTypeFlag__All));

	sl::String signature;
	flags |= PropertyType::createSignature(&signature, getterType, setterType, flags);
	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
		return (PropertyType*)it->m_value;

	if (setterType.isEmpty())
		flags |= PropertyTypeFlag_Const;

	PropertyType* type = new PropertyType;
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_getterType = getterType;
	type->m_setterType = setterType;
	type->m_flags = flags;

	if (flags & PropertyTypeFlag_Bindable) {
		FunctionType* binderType = (FunctionType*)getStdType(StdType_Binder);
		if (getterType->isMemberMethodType())
			binderType = binderType->getMemberMethodType(getterType->getThisTargetType(), PtrTypeFlag_Const);

		type->m_binderType = binderType;
	}

	m_typeList.insertTail(type);
	it->m_value = type;
	return type;
}

PropertyType*
TypeMgr::getSimplePropertyType(
	CallConv* callConv,
	Type* returnType,
	uint_t flags
) {
	SimplePropertyTypeTuple* tuple = getSimplePropertyTypeTuple(returnType);

	uint_t callConvFlags = callConv->getFlags();

	size_t i1 =
		(callConvFlags & CallConvFlag_Stdcall) ? 2 :
		(callConvFlags & CallConvFlag_Cdecl) ? 1 : 0;

	size_t i2 = (flags & PropertyTypeFlag_Const) != 0;
	size_t i3 = (flags & PropertyTypeFlag_Bindable) != 0;

	if (tuple->m_propertyTypeArray[i1][i2][i3])
		return tuple->m_propertyTypeArray[i1][i2][i3];

	PropertyType* propertyType;

	FunctionType* getterType = getFunctionType(callConv, returnType, NULL, 0);
	if (flags & PropertyTypeFlag_Const) {
		propertyType = getPropertyType(getterType, NULL, flags);
	} else {
		Type* voidType = &m_primitiveTypeArray[TypeKind_Void];
		FunctionType* setterType = getFunctionType(callConv, voidType, &returnType, 1);
		propertyType = getPropertyType(getterType, setterType, flags);
	}

	tuple->m_propertyTypeArray[i1][i2][i3] = propertyType;
	return propertyType;
}

PropertyType*
TypeMgr::createIndexedPropertyType(
	CallConv* callConv,
	Type* returnType,
	const sl::Array<FunctionArg*>& argArray,
	uint_t flags
) {
	FunctionType* getterType = createUserFunctionType(callConv, returnType, argArray);

	if (flags & PropertyTypeFlag_Const)
		return getPropertyType(getterType, NULL, flags);

	sl::Array<FunctionArg*> setterArgArray = argArray;
	setterArgArray.append(returnType->getSimpleFunctionArg());

	Type* voidType = &m_primitiveTypeArray[TypeKind_Void];
	FunctionType* setterType = createUserFunctionType(callConv, voidType, setterArgArray);
	return getPropertyType(getterType, setterType, flags);
}

PropertyType*
TypeMgr::getMemberPropertyType(
	DerivableType* parentType,
	PropertyType* propertyType
) {
	FunctionType* getterType = getMemberMethodType(
		parentType,
		propertyType->m_getterType,
		PtrTypeFlag_Const
	);

	size_t setterTypeOverloadCount = propertyType->m_setterType.getOverloadCount();

	char buffer[256];
	sl::Array<FunctionType*> setterTypeOverloadArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	setterTypeOverloadArray.setCount(setterTypeOverloadCount);
	sl::Array<FunctionType*>::Rwi rwi = setterTypeOverloadArray;

	for (size_t i = 0; i < setterTypeOverloadCount; i++) {
		FunctionType* overloadType = propertyType->m_setterType.getOverload(i);
		rwi[i] = getMemberMethodType(parentType, overloadType);
	}

	PropertyType* memberPropertyType = getPropertyType(
		getterType,
		FunctionTypeOverload(setterTypeOverloadArray, setterTypeOverloadCount),
		propertyType->m_flags & PropertyTypeFlag__All
	);

	memberPropertyType->m_shortType = propertyType;
	return memberPropertyType;
}

PropertyType*
TypeMgr::getStdObjectMemberPropertyType(PropertyType* propertyType) {
	if (propertyType->m_stdObjectMemberPropertyType)
		return propertyType->m_stdObjectMemberPropertyType;

	ClassType* classType = (ClassType*)getStdType(StdType_AbstractClass);
	propertyType->m_stdObjectMemberPropertyType = classType->getMemberPropertyType(propertyType);
	return propertyType->m_stdObjectMemberPropertyType;
}

PropertyType*
TypeMgr::getShortPropertyType(PropertyType* propertyType) {
	if (propertyType->m_shortType)
		return propertyType->m_shortType;

	if (!propertyType->isMemberPropertyType()) {
		propertyType->m_shortType = propertyType;
		return propertyType;
	}

	FunctionType* getterType = propertyType->m_getterType->getShortType();
	FunctionTypeOverload setterType;

	size_t setterCount = propertyType->m_setterType.getOverloadCount();
	for (size_t i = 0; i < setterCount; i++) {
		FunctionType* type = propertyType->m_setterType.getOverload(i)->getShortType();
		setterType.addOverload(type);
	}

	propertyType->m_shortType = getPropertyType(
		getterType,
		setterType,
		propertyType->m_flags & PropertyTypeFlag__All
	);

	return propertyType->m_shortType;
}

ClassType*
TypeMgr::getMulticastType(FunctionPtrType* functionPtrType) {
	if (functionPtrType->m_multicastType)
		return functionPtrType->m_multicastType;

	Type* returnType = functionPtrType->getTargetType()->getReturnType();
	if (returnType->getTypeKind() != TypeKind_Void) {
		err::setFormatStringError("multicast can only return 'void', not '%s'", returnType->getTypeString().sz());
		return NULL;
	}

	sl::String typeName = m_module->createUniqueName("Multicast");
	MulticastClassType* type = createInternalClassType<MulticastClassType>(typeName);
	type->m_targetType = functionPtrType;
	type->m_flags |= (functionPtrType->m_flags & TypeFlag_GcRoot);

	// fields

	type->m_fieldArray[MulticastFieldKind_Lock] = type->createField("!m_lock", getPrimitiveType(TypeKind_IntPtr), 0, PtrTypeFlag_Volatile);
	type->m_fieldArray[MulticastFieldKind_PtrArray] = type->createField("!m_arrayPtr", functionPtrType->getDataPtrType());
	type->m_fieldArray[MulticastFieldKind_Count] = type->createField("!m_count", getPrimitiveType(TypeKind_SizeT));
	type->m_fieldArray[MulticastFieldKind_MaxCount] = type->createField("!m_maxCount", getPrimitiveType(TypeKind_SizeT));
	type->m_fieldArray[MulticastFieldKind_HandleTable] = type->createField("!m_handleTable", getPrimitiveType(TypeKind_IntPtr));

	Type* argType;
	Function* method;
	FunctionType* methodType;

	bool isThin = functionPtrType->getPtrTypeKind() == FunctionPtrTypeKind_Thin;

	// destructor

	methodType = (FunctionType*)getStdType(StdType_SimpleFunction);
	method = type->createUnnamedMethod(FunctionKind_Destructor, methodType);
	method->m_flags |= ModuleItemFlag_User; // no need to generate default destructor
	type->m_destructor = method;

	// methods

	methodType = (FunctionType*)getStdType(StdType_SimpleFunction);
	method = type->createMethod("clear", methodType);
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray[MulticastMethodKind_Clear] = method;

	returnType = getPrimitiveType(TypeKind_IntPtr);
	argType = functionPtrType;
	methodType = getFunctionType(returnType, &argType, 1);

	method = type->createMethod("setup", methodType);
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray[MulticastMethodKind_Setup] = method;

	method = type->createMethod("add", methodType);
	type->m_methodArray[MulticastMethodKind_Add] = method;

	returnType = functionPtrType;
	argType = getPrimitiveType(TypeKind_IntPtr);
	methodType = getFunctionType(returnType, &argType, 1);
	method = type->createMethod("remove", methodType);
	type->m_methodArray[MulticastMethodKind_Remove] = method;

	returnType = functionPtrType->getNormalPtrType();
	methodType = getFunctionType(returnType, NULL, 0);
	method = type->createMethod("getSnapshot", methodType);
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray[MulticastMethodKind_GetSnapshot] = method;

	methodType = functionPtrType->getTargetType();
	method = type->createMethod<MulticastClassType::CallMethod>("call", methodType);
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray[MulticastMethodKind_Call] = method;

	// overloaded operators

	type->m_binaryOperatorArray.setCountZeroConstruct(BinOpKind_SubAssign + 1);
	sl::Array<OverloadableFunction>::Rwi rwi = type->m_binaryOperatorArray;
	rwi[BinOpKind_RefAssign] = type->m_methodArray[MulticastMethodKind_Setup];
	rwi[BinOpKind_AddAssign] = type->m_methodArray[MulticastMethodKind_Add];
	rwi[BinOpKind_SubAssign] = type->m_methodArray[MulticastMethodKind_Remove];
	type->m_callOperator = type->m_methodArray[MulticastMethodKind_Call];

	// snapshot closure (snapshot is shared between weak and normal multicasts)

	McSnapshotClassType* snapshotType = createInternalClassType<McSnapshotClassType>("!McSnapshot");
	snapshotType->m_parentNamespace = type;
	snapshotType->m_targetType = functionPtrType->getUnWeakPtrType();
	snapshotType->m_flags |= (functionPtrType->m_flags & TypeFlag_GcRoot);

	// fields

	snapshotType->m_fieldArray[McSnapshotFieldKind_PtrArray] = snapshotType->createField("!m_arrayPtr", functionPtrType->getDataPtrType());
	snapshotType->m_fieldArray[McSnapshotFieldKind_Count] = snapshotType->createField("!m_count", getPrimitiveType(TypeKind_SizeT));

	// call method

	methodType = functionPtrType->getTargetType();
	snapshotType->m_methodArray[McSnapshotMethodKind_Call] = snapshotType->createMethod<McSnapshotClassType::CallMethod>("call", methodType);

	type->m_snapshotType = snapshotType;

	functionPtrType->m_multicastType = type;
	return type;
}

ClassType*
TypeMgr::createReactorBaseType() {
	Type* voidType = getPrimitiveType(TypeKind_Void);
	Type* sizeType = getPrimitiveType(TypeKind_SizeT);

	// onevent statement can have different event types
	Type* addOnEventBindingTypeArgTypeArray[] = { sizeType, getStdType(StdType_AbstractClassPtr) };
	Type* addOnChangedBindingArgTypeArray[] = { sizeType, getStdType(StdType_SimpleEventPtr) };
	Type* enterReactiveStmtArgTypeArray[] = { sizeType, sizeType };

	FunctionType* simpleFunctionType = (FunctionType*)getStdType(StdType_SimpleFunction);
	FunctionType* addOnChangedBindingType = getFunctionType(voidType, addOnChangedBindingArgTypeArray, 2);
	FunctionType* addOnEventBindingType = getFunctionType(voidType, addOnEventBindingTypeArgTypeArray, 2);
	FunctionType* enterReactiveStmtType = getFunctionType(voidType, enterReactiveStmtArgTypeArray, 2);

	ClassType* type = createInternalClassType("jnc.ReactorBase", 8, ClassTypeFlag_Opaque);
	type->createField("!m_userData", getStdType(StdType_AbstractClassPtr));

	Function* constructor = m_module->m_functionMgr.createFunction(simpleFunctionType);
	constructor->m_functionKind = FunctionKind_Constructor;
	type->addMethod(constructor);

	Function* destructor = m_module->m_functionMgr.createFunction(simpleFunctionType);
	destructor->m_functionKind = FunctionKind_Destructor;
	type->addMethod(destructor);

	type->createMethod("start", simpleFunctionType);
	type->createMethod("stop", simpleFunctionType);
	type->createMethod("restart", simpleFunctionType);
	type->createMethod("!addOnChangedBinding", addOnChangedBindingType);
	type->createMethod("!addOnEventBinding", addOnEventBindingType);
	type->createMethod("!enterReactiveStmt", enterReactiveStmtType);
	return type;
}

ReactorClassType*
TypeMgr::createReactorType(
	const sl::StringRef& name,
	ClassType* parentType
) {
	ReactorClassType* type = createClassType<ReactorClassType>(name);
	type->addBaseType(getStdType(StdType_ReactorBase));
	type->m_parentType = parentType;
	return type;
}

FunctionClosureClassType*
TypeMgr::createReactorClosureType() {
	FunctionClosureClassType* type = createInternalClassType<FunctionClosureClassType>("jnc.ReactorClosure");
	type->m_thisArgFieldIdx = 0;
	type->createField("m_reactor", ((ClassType*)getStdType(StdType_ReactorBase))->getClassPtrType());
	type->createField("m_binding", getStdType(StdType_ByteThinPtr));
	type->ensureLayout();
	return type;
}

FunctionClosureClassType*
TypeMgr::getFunctionClosureClassType(
	FunctionType* targetType,
	FunctionType* thunkType,
	Type* const* argTypeArray,
	const size_t* closureMap,
	size_t argCount,
	size_t thisArgIdx
) {
	ASSERT(m_module->getCompileState() >= ModuleCompileState_Parsed); // signatures are final

	sl::String signature = ClosureClassType::createSignature(
		targetType,
		thunkType,
		argTypeArray,
		closureMap,
		argCount,
		thisArgIdx
	);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value) {
		ASSERT(it->m_value->m_signature == signature);
		return (FunctionClosureClassType*)it->m_value;
	}

	sl::String typeName = m_module->createUniqueName("FunctionClosure");
	FunctionClosureClassType* type = createInternalClassType<FunctionClosureClassType>(typeName);
	type->m_signature = signature;
	type->m_flags |= TypeFlag_SignatureFinal;
	type->m_closureMap.copy(closureMap, argCount);
	type->m_thisArgFieldIdx = thisArgIdx + 1;
	type->createField("m_target", targetType->getFunctionPtrType(FunctionPtrTypeKind_Thin));

	sl::String argFieldName;
	for (size_t i = 0; i < argCount; i++) {
		argFieldName.format("m_arg%d", i);
		type->createField(argFieldName, argTypeArray[i]);
	}

	sl::String thunkName = typeName + "!thunkFunction";
	Function* thunkFunction = m_module->m_functionMgr.createInternalFunction<FunctionClosureClassType::ThunkFunction>(thunkName, thunkType);
	type->addMethod(thunkFunction);
	type->m_thunkFunction = thunkFunction;

	it->m_value = type;
	return type;
}

PropertyClosureClassType*
TypeMgr::getPropertyClosureClassType(
	PropertyType* targetType,
	PropertyType* thunkType,
	Type* const* argTypeArray,
	const size_t* closureMap,
	size_t argCount,
	size_t thisArgIdx
) {
	ASSERT(m_module->getCompileState() >= ModuleCompileState_Parsed); // signatures are final

	sl::String signature = ClosureClassType::createSignature(
		targetType,
		thunkType,
		argTypeArray,
		closureMap,
		argCount,
		thisArgIdx
	);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value) {
		ASSERT(it->m_value->m_signature == signature);
		return (PropertyClosureClassType*)it->m_value;
	}

	sl::String typeName = m_module->createUniqueName("PropertyClosure");
	PropertyClosureClassType* type = createInternalClassType<PropertyClosureClassType>(typeName);
	type->m_signature = signature;
	type->m_flags |= TypeFlag_SignatureFinal;
	type->m_closureMap.copy(closureMap, argCount);
	type->m_thisArgFieldIdx = thisArgIdx + 1;
	type->createField("m_target", targetType->getPropertyPtrType(PropertyPtrTypeKind_Thin));

	sl::String argFieldName;

	for (size_t i = 0; i < argCount; i++) {
		argFieldName.format("m_arg%d", i);
		type->createField(argFieldName, argTypeArray[i]);
	}

	sl::String thunkName = typeName + "!m_thunkProperty";
	Property* thunkProperty = m_module->m_functionMgr.createInternalProperty<PropertyClosureClassType::ThunkProperty>(thunkName);
	type->addProperty(thunkProperty);
	type->m_thunkProperty = thunkProperty;

	bool result = thunkProperty->create(thunkType);
	ASSERT(result);

	it->m_value = type;
	return type;
}

DataClosureClassType*
TypeMgr::getDataClosureClassType(
	Type* targetType,
	PropertyType* thunkType
) {
	ASSERT(m_module->getCompileState() >= ModuleCompileState_Parsed); // signatures are final

	sl::String signature = DataClosureClassType::createSignature(targetType, thunkType);
	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value) {
		ASSERT(it->m_value->m_signature == signature);
		return (DataClosureClassType*)it->m_value;
	}

	sl::String typeName = m_module->createUniqueName("PropertyClosure");
	DataClosureClassType* type = createInternalClassType<DataClosureClassType>(typeName);
	type->m_signature = signature;
	type->m_flags |= TypeFlag_SignatureFinal;
	type->createField("!m_target", targetType->getDataPtrType());

	sl::String thunkName = typeName + "!m_thunkProperty";
	Property* thunkProperty = m_module->m_functionMgr.createInternalProperty<DataClosureClassType::ThunkProperty>(thunkName);
	type->addProperty(thunkProperty);
	type->m_thunkProperty = thunkProperty;

	bool result = thunkProperty->create(thunkType);
	ASSERT(result);

	it->m_value = type;
	return type;
}

DataPtrType*
TypeMgr::getDataPtrType(
	Type* targetType,
	uint_t bitOffset,
	uint_t bitCount,
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	ASSERT(bitCount && (flags & PtrTypeFlag_BitField)); // otherwise, should go to the original getDataPtrType overload

	sl::String signature = DataPtrType::createSignature(targetType, bitOffset, bitCount, typeKind, ptrTypeKind, flags);
	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value) {
		ASSERT(it->m_value->m_signature == signature);
		return (DataPtrType*)it->m_value;
	}

	DataPtrType* type = new DataPtrType;
	type->m_module = m_module;
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_size = ptrTypeKind == DataPtrTypeKind_Normal ? sizeof(DataPtr) : sizeof(void*);
	type->m_targetType = targetType;
	type->m_bitOffset = bitOffset;
	type->m_bitCount = bitCount;
	type->m_signature = signature;
	type->m_flags = flags | targetType->getFlags() & TypeFlag_SignatureMask;

	if (targetType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)targetType)->addFixup(&type->m_targetType);
	else
		type->m_flags |= TypeFlag_LayoutReady;

	m_typeList.insertTail(type);
	it->m_value = type;
	return type;
}

DataPtrType*
TypeMgr::getDataPtrType(
	Type* targetType,
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	ASSERT((size_t)ptrTypeKind < DataPtrTypeKind__Count);
	ASSERT(targetType->getTypeKind() != TypeKind_ImportTypeName); // for imports, getImportPtrType() should be called
	ASSERT(typeKind != TypeKind_DataRef || targetType->m_typeKind != TypeKind_DataRef); // double reference
	ASSERT(!(flags & ~PtrTypeFlag__All));

	if (ptrTypeKind == DataPtrTypeKind_Normal)
		flags |= TypeFlag_GcRoot | TypeFlag_StructRet;

	if (isDualType(targetType))
		flags |= PtrTypeFlag_DualTarget;

	DataPtrTypeTuple* tuple = getDataPtrTypeTuple(targetType, flags);

	// ref x ptrkind x const/readonly/cmut x volatile x safe

	size_t i1 = typeKind == TypeKind_DataRef;
	size_t i2 = ptrTypeKind;
	size_t i3;
	size_t i4 = (flags & PtrTypeFlag_Volatile) ? 1 : 0;
	size_t i5 = (flags & PtrTypeFlag_Safe) ? 1 : 0;

	if (flags & PtrTypeFlag_Const) {
		i3 = 1;
		flags &= ~(PtrTypeFlag_ReadOnly | PtrTypeFlag_CMut);
	} else if (flags & PtrTypeFlag_ReadOnly) {
		i3 = 2;
		flags &= ~(PtrTypeFlag_Const | PtrTypeFlag_CMut);
	} else if (flags & PtrTypeFlag_CMut) {
		i3 = 3;
		flags &= ~(PtrTypeFlag_Const | PtrTypeFlag_ReadOnly);
	} else
		i3 = 0;

	if (tuple->m_ptrTypeArray[i1][i2][i3][i4][i5])
		return tuple->m_ptrTypeArray[i1][i2][i3][i4][i5];

	DataPtrType* type = new DataPtrType;
	type->m_module = m_module;
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_size = ptrTypeKind == DataPtrTypeKind_Normal ? sizeof(DataPtr) : sizeof(void*);
	type->m_targetType = targetType;
	type->m_flags = flags;

	if (targetType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)targetType)->addFixup(&type->m_targetType);
	else
		type->m_flags |= TypeFlag_LayoutReady;

	m_typeList.insertTail(type);
	tuple->m_ptrTypeArray[i1][i2][i3][i4][i5] = type;
	return type;
}

ClassPtrType*
TypeMgr::getClassPtrType(
	ClassType* targetType,
	TypeKind typeKind,
	ClassPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	ASSERT((size_t)ptrTypeKind < ClassPtrTypeKind__Count);
	ASSERT(!(flags & ~PtrTypeFlag__All));

	flags |= TypeFlag_LayoutReady | TypeFlag_GcRoot;

	ClassPtrTypeTuple* tuple;

	if (flags & (PtrTypeFlag_Event | PtrTypeFlag_DualEvent)) {
		ASSERT(targetType->getClassTypeKind() == ClassTypeKind_Multicast);
		tuple = getEventClassPtrTypeTuple((MulticastClassType*)targetType);
	} else {
		tuple = getClassPtrTypeTuple(targetType);
	}

	// ref x ptrkind x const x volatile x checked

	size_t i1 = typeKind == TypeKind_ClassRef;
	size_t i2 = ptrTypeKind;
	size_t i3;
	size_t i4 = (flags & PtrTypeFlag_Volatile) ? 1 : 0;
	size_t i5 = (flags & PtrTypeFlag_Safe) ? 1 : 0;

	if (flags & PtrTypeFlag_Const) {
		i3 = 1;
		flags &= ~(PtrTypeFlag_ReadOnly | PtrTypeFlag_CMut);
	} else if (flags & PtrTypeFlag_ReadOnly) {
		i3 = 2;
		flags &= ~(PtrTypeFlag_Const | PtrTypeFlag_CMut);
	} else if (flags & PtrTypeFlag_CMut) {
		i3 = 3;
		flags &= ~(PtrTypeFlag_Const | PtrTypeFlag_ReadOnly);
	} else
		i3 = 0;

	if (tuple->m_ptrTypeArray[i1][i2][i3][i4][i5])
		return tuple->m_ptrTypeArray[i1][i2][i3][i4][i5];

	ClassPtrType* type = new ClassPtrType;
	type->m_module = m_module;
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_targetType = targetType;
	type->m_flags = flags;

	m_typeList.insertTail(type);
	tuple->m_ptrTypeArray[i1][i2][i3][i4][i5] = type;
	return type;
}

FunctionPtrType*
TypeMgr::getFunctionPtrType(
	FunctionType* functionType,
	TypeKind typeKind,
	FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	ASSERT(typeKind == TypeKind_FunctionPtr || typeKind == TypeKind_FunctionRef);
	ASSERT((size_t)ptrTypeKind < FunctionPtrTypeKind__Count);
	ASSERT(!(flags & ~PtrTypeFlag__All));

	if (ptrTypeKind != FunctionPtrTypeKind_Thin)
		flags |=
			TypeFlag_GcRoot |
			TypeFlag_StructRet |
			(functionType->getFlags() & TypeFlag_LayoutReady);

	if (functionType->m_flags & FunctionTypeFlag_Unsafe)
		flags &= ~PtrTypeFlag_Safe;

	FunctionPtrTypeTuple* tuple = getFunctionPtrTypeTuple(functionType);

	// ref x kind x checked

	size_t i1 = typeKind == TypeKind_FunctionRef;
	size_t i2 = ptrTypeKind;
	size_t i3 = (flags & PtrTypeFlag_Safe) ? 0 : 1;

	if (tuple->m_ptrTypeArray[i1][i2][i3])
		return tuple->m_ptrTypeArray[i1][i2][i3];

	FunctionPtrType* type = new FunctionPtrType;
	type->m_module = m_module;
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_size = ptrTypeKind == FunctionPtrTypeKind_Thin ? sizeof(void*) : sizeof(FunctionPtr);
	type->m_targetType = functionType;
	type->m_flags = flags;

	m_typeList.insertTail(type);
	tuple->m_ptrTypeArray[i1][i2][i3] = type;
	return type;
}

PropertyPtrType*
TypeMgr::getPropertyPtrType(
	PropertyType* propertyType,
	TypeKind typeKind,
	PropertyPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	ASSERT(typeKind == TypeKind_PropertyPtr || typeKind == TypeKind_PropertyRef);
	ASSERT((size_t)ptrTypeKind < PropertyPtrTypeKind__Count);
	ASSERT(!(flags & ~PtrTypeFlag__All));

	flags |= TypeFlag_LayoutReady;

	if (ptrTypeKind != PropertyPtrTypeKind_Thin)
		flags |= TypeFlag_GcRoot | TypeFlag_StructRet;

	PropertyPtrTypeTuple* tuple = getPropertyPtrTypeTuple(propertyType);

	// ref x kind x checked

	size_t i1 = typeKind == TypeKind_PropertyRef;
	size_t i2 = ptrTypeKind;
	size_t i3 = (flags & PtrTypeFlag_Safe) ? 0 : 1;

	if (tuple->m_ptrTypeArray[i1][i2][i3])
		return tuple->m_ptrTypeArray[i1][i2][i3];

	PropertyPtrType* type = new PropertyPtrType;
	type->m_module = m_module;
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_size = ptrTypeKind == PropertyPtrTypeKind_Thin ? sizeof(void*) : sizeof(PropertyPtr);
	type->m_targetType = propertyType;
	type->m_flags = flags;

	m_typeList.insertTail(type);
	tuple->m_ptrTypeArray[i1][i2][i3] = type;
	return type;
}

StructType*
TypeMgr::getPropertyVtableStructType(PropertyType* propertyType) {
	if (propertyType->m_vtableStructType)
		return propertyType->m_vtableStructType;

	sl::String typeName = m_module->createUniqueName("PropertyVtable");
	StructType* type = createInternalStructType(typeName);

	if (propertyType->getFlags() & PropertyTypeFlag_Bindable)
		type->createField("!m_binder", propertyType->m_binderType->getFunctionPtrType(FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe));

	type->createField("!m_getter", propertyType->m_getterType->getFunctionPtrType(FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe));

	sl::String setterFieldName;

	size_t setterTypeOverloadCount = propertyType->m_setterType.getOverloadCount();
	for (size_t i = 0; i < setterTypeOverloadCount; i++) {
		setterFieldName.format("!m_setter%d", i);

		FunctionType* setterType = propertyType->m_setterType.getOverload(i);
		type->createField(setterFieldName, setterType->getFunctionPtrType(FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe));
	}

	type->ensureLayout();

	propertyType->m_vtableStructType = type;
	return type;
}

ImportTypeName*
TypeMgr::getImportTypeName(
	Namespace* parentNamespace,
	QualifiedName* name
) {
	ASSERT(parentNamespace->getNamespaceKind() != NamespaceKind_Scope);

	sl::String signature = ImportTypeName::createSignature(parentNamespace, *name);
	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value) {
		ASSERT(it->m_value->m_signature == signature && !((ImportTypeName*)it->m_value)->isResolved());
		return (ImportTypeName*)it->m_value;
	}

	ImportTypeName* type = new ImportTypeName;
	type->m_module = m_module;
	type->m_parentNamespace = parentNamespace;
	sl::takeOver(&type->m_name, name);
	type->m_signature = signature;
	type->m_flags |= TypeFlag_SignatureReady;

	m_typeList.insertTail(type);
	it->m_value = type;
	return type;
}

ImportTypeName*
TypeMgr::getAnchoredImportTypeName(
	Namespace* parentNamespace,
	ImportTypeNameAnchor* anchor,
	const QualifiedName& name
) {
	ASSERT(parentNamespace->getNamespaceKind() != NamespaceKind_Scope);

	sl::String signature = ImportTypeName::createSignature(parentNamespace, name, anchor);
	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value) {
		ASSERT(it->m_value->m_signature == signature && !((ImportTypeName*)it->m_value)->isResolved());
		return (ImportTypeName*)it->m_value;
	}

	ImportTypeName* type = new ImportTypeName;
	type->m_module = m_module;
	type->m_parentNamespace = parentNamespace;
	type->m_anchor = anchor;
	type->m_name.copy(name);
	type->m_signature = signature;
	type->m_flags |= TypeFlag_SignatureReady;

	m_typeList.insertTail(type);
	it->m_value = type;
	return type;
}

TemplateArgType*
TypeMgr::createTemplateArgType(
	const sl::StringRef& name,
	size_t index,
	Declarator* defaultTypeDeclarator
) {
	TemplateArgType* type = new TemplateArgType;
	type->m_module = m_module;
	type->m_name = name;
	type->m_index = index;

	if (defaultTypeDeclarator)
		type->m_defaultType = createTemplateDeclType(defaultTypeDeclarator);

	m_typeList.insertTail(type);
	return type;
}

TemplateTypeName*
TypeMgr::getTemplateTypeName(
	Namespace* parentNamespace,
	QualifiedName* name // destructive
) {
	ASSERT(parentNamespace->getNamespaceKind() == NamespaceKind_TemplateDeclaration);

	sl::String signature = ImportTypeName::createSignature(parentNamespace, *name);
	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value) {
		ASSERT(it->m_value->m_signature == signature);
		return (TemplateTypeName*)it->m_value;
	}

	TemplateTypeName* type = new TemplateTypeName;
	type->m_module = m_module;
	type->m_parentNamespace = parentNamespace;
	sl::takeOver(&type->m_name, name);
	type->m_signature = signature;
	type->m_flags |= TypeFlag_SignatureReady;

	m_typeList.insertTail(type);
	it->m_value = type;
	return type;
}

TemplateDeclType*
TypeMgr::createTemplateDeclType(Declarator* declarator) {
	TemplateDeclType* type = new TemplateDeclType;
	type->m_module = m_module;
	sl::takeOver(&type->m_declarator, declarator);

	if (type->m_declarator.m_baseType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)type->m_declarator.m_baseType)->addFixup(&type->m_declarator.m_baseType);

	m_typeList.insertTail(type);
	return type;
}

Type*
TypeMgr::foldDualType(
	Type* type,
	bool isAlien,
	bool isContainerConst
) {
	ASSERT(isDualType(type));

	DualTypeTuple* tuple = getDualTypeTuple(type);
	Type* foldedType = tuple->m_typeArray[isAlien][isContainerConst];
	if (!foldedType) {
		foldedType = type->calcFoldedDualType(isAlien, isContainerConst);
		tuple->m_typeArray[isAlien][isContainerConst] = foldedType;
	}

	return foldedType;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DualTypeTuple*
TypeMgr::getDualTypeTuple(Type* type) {
	if (type->m_dualTypeTuple)
		return type->m_dualTypeTuple;

	DualTypeTuple* tuple = new (mem::ZeroInit) DualTypeTuple;
	type->m_dualTypeTuple = tuple;
	m_dualTypeTupleList.insertTail(tuple);
	return tuple;
}

SimplePropertyTypeTuple*
TypeMgr::getSimplePropertyTypeTuple(Type* type) {
	if (type->m_simplePropertyTypeTuple)
		return type->m_simplePropertyTypeTuple;

	SimplePropertyTypeTuple* tuple = new (mem::ZeroInit) SimplePropertyTypeTuple;
	type->m_simplePropertyTypeTuple = tuple;
	m_simplePropertyTypeTupleList.insertTail(tuple);
	return tuple;
}

FunctionArgTuple*
TypeMgr::getFunctionArgTuple(Type* type) {
	if (type->m_functionArgTuple)
		return type->m_functionArgTuple;

	FunctionArgTuple* tuple = new (mem::ZeroInit) FunctionArgTuple;
	type->m_functionArgTuple = tuple;
	m_functionArgTupleList.insertTail(tuple);
	return tuple;
}

DataPtrTypeTuple*
TypeMgr::getDataPtrTypeTuple(
	Type* type,
	uint_t flags
) {
	DataPtrTypeTuple* tuple;

	if (type->m_dataPtrTypeTuple)
		tuple = type->m_dataPtrTypeTuple;
	else {
		tuple = new(mem::ZeroInit) DataPtrTypeTuple;
		type->m_dataPtrTypeTuple = tuple;
		m_dataPtrTypeTupleList.insertTail(tuple);
	}

	if (!(flags & PtrTypeFlag_BigEndian))
		return tuple;

	if (tuple->m_bigEndianTuple)
		return tuple->m_bigEndianTuple;

	DataPtrTypeTuple* tuple2 = new(mem::ZeroInit) DataPtrTypeTuple;
	tuple->m_bigEndianTuple = tuple2;
	m_dataPtrTypeTupleList.insertTail(tuple2);
	return tuple2;
}

ClassPtrTypeTuple*
TypeMgr::getClassPtrTypeTuple(ClassType* classType) {
	if (classType->m_classPtrTypeTuple)
		return classType->m_classPtrTypeTuple;

	ClassPtrTypeTuple* tuple = new (mem::ZeroInit) ClassPtrTypeTuple;
	classType->m_classPtrTypeTuple = tuple;
	m_classPtrTypeTupleList.insertTail(tuple);
	return tuple;
}

ClassPtrTypeTuple*
TypeMgr::getEventClassPtrTypeTuple(MulticastClassType* classType) {
	if (classType->m_eventClassPtrTypeTuple)
		return classType->m_eventClassPtrTypeTuple;

	ClassPtrTypeTuple* tuple = new (mem::ZeroInit) ClassPtrTypeTuple;
	classType->m_eventClassPtrTypeTuple = tuple;
	m_classPtrTypeTupleList.insertTail(tuple);
	return tuple;
}

FunctionPtrTypeTuple*
TypeMgr::getFunctionPtrTypeTuple(FunctionType* functionType) {
	if (functionType->m_functionPtrTypeTuple)
		return functionType->m_functionPtrTypeTuple;

	FunctionPtrTypeTuple* tuple = new (mem::ZeroInit) FunctionPtrTypeTuple;
	functionType->m_functionPtrTypeTuple = tuple;
	m_functionPtrTypeTupleList.insertTail(tuple);
	return tuple;
}

PropertyPtrTypeTuple*
TypeMgr::getPropertyPtrTypeTuple(PropertyType* propertyType) {
	if (propertyType->m_propertyPtrTypeTuple)
		return propertyType->m_propertyPtrTypeTuple;

	PropertyPtrTypeTuple* tuple = new (mem::ZeroInit) PropertyPtrTypeTuple;
	propertyType->m_propertyPtrTypeTuple = tuple;
	m_propertyPtrTypeTupleList.insertTail(tuple);
	return tuple;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
TypeMgr::setupAllPrimitiveTypes() {
	enum {
		PodFlags =
			TypeFlag_Pod |
			TypeFlag_SignatureReady |
			TypeFlag_SignatureFinal |
			TypeFlag_LayoutReady,

		StructFlags =
			TypeFlag_StructRet |
			TypeFlag_GcRoot |
			TypeFlag_SignatureReady |
			TypeFlag_SignatureFinal |
			TypeFlag_LayoutReady,
	};

	setupPrimitiveType(TypeKind_Void,    "v", 0, 0, PodFlags);
	setupPrimitiveType(TypeKind_Bool,    "b", 1, 1, PodFlags);
	setupPrimitiveType(TypeKind_Int8,    "c", 1, 1, PodFlags);
	setupPrimitiveType(TypeKind_Int8_u,  "y", 1, 1, PodFlags);
	setupPrimitiveType(TypeKind_Int16,   "h", 2, 2, PodFlags);
	setupPrimitiveType(TypeKind_Int16_u, "w", 2, 2, PodFlags);
	setupPrimitiveType(TypeKind_Int32,   "i", 4, 4, PodFlags);
	setupPrimitiveType(TypeKind_Int32_u, "d", 4, 4, PodFlags);
	setupPrimitiveType(TypeKind_Int64,   "l", 8, 8, PodFlags);
	setupPrimitiveType(TypeKind_Int64_u, "q", 8, 8, PodFlags);
	setupPrimitiveType(TypeKind_Float,   "f", 4, 4, PodFlags);
	setupPrimitiveType(TypeKind_Double,  "e", 8, 8, PodFlags);

	// variant & string require special treatment

	setupPrimitiveType(
		TypeKind_Variant,
		"z",
		sizeof(Variant),
		sizeof(int64_t),
		StructFlags
	);

	setupPrimitiveType(
		TypeKind_String,
		"s",
		sizeof(String),
		sizeof(void*),
		StructFlags
	);
}

void
TypeMgr::setupStdTypedefArray() {
	setupStdTypedef(StdTypedef_variant_t, TypeKind_Variant,  "variant_t");
	setupStdTypedef(StdTypedef_string_t,  TypeKind_String,   "string_t");
	setupStdTypedef(StdTypedef_uint_t,    TypeKind_Int_u,    "uint_t");
	setupStdTypedef(StdTypedef_intptr_t,  TypeKind_IntPtr,   "intptr_t");
	setupStdTypedef(StdTypedef_uintptr_t, TypeKind_IntPtr_u, "uintptr_t");
	setupStdTypedef(StdTypedef_size_t,    TypeKind_SizeT,    "size_t");
	setupStdTypedef(StdTypedef_int8_t,    TypeKind_Int8,     "int8_t");
	setupStdTypedef(StdTypedef_utf8_t,    TypeKind_Int8,     "utf8_t");
	setupStdTypedef(StdTypedef_uint8_t,   TypeKind_Int8_u,   "uint8_t");
	setupStdTypedef(StdTypedef_uchar_t,   TypeKind_Int8_u,   "uchar_t");
	setupStdTypedef(StdTypedef_byte_t,    TypeKind_Int8_u,   "byte_t");
	setupStdTypedef(StdTypedef_int16_t,   TypeKind_Int16,    "int16_t");
	setupStdTypedef(StdTypedef_utf16_t,   TypeKind_Int16,    "utf16_t");
	setupStdTypedef(StdTypedef_uint16_t,  TypeKind_Int16_u,  "uint16_t");
	setupStdTypedef(StdTypedef_ushort_t,  TypeKind_Int16_u,  "ushort_t");
	setupStdTypedef(StdTypedef_word_t,    TypeKind_Int16_u,  "word_t");
	setupStdTypedef(StdTypedef_int32_t,   TypeKind_Int32,    "int32_t");
	setupStdTypedef(StdTypedef_utf32_t,   TypeKind_Int32,    "utf32_t");
	setupStdTypedef(StdTypedef_uint32_t,  TypeKind_Int32_u,  "uint32_t");
	setupStdTypedef(StdTypedef_dword_t,   TypeKind_Int32_u,  "dword_t");
	setupStdTypedef(StdTypedef_int64_t,   TypeKind_Int64,    "int64_t");
	setupStdTypedef(StdTypedef_uint64_t,  TypeKind_Int64_u,  "uint64_t");
	setupStdTypedef(StdTypedef_ulong_t,   TypeKind_Int64_u,  "ulong_t");
	setupStdTypedef(StdTypedef_qword_t,   TypeKind_Int64_u,  "qword_t");
}

void
TypeMgr::setupCallConvArray() {
	m_callConvArray[CallConvKind_Jnccall_msc32]  = &m_jnccallCallConv_msc32;
	m_callConvArray[CallConvKind_Jnccall_msc64]  = &m_jnccallCallConv_msc64;
	m_callConvArray[CallConvKind_Jnccall_gcc32]  = &m_jnccallCallConv_gcc32;
	m_callConvArray[CallConvKind_Jnccall_gcc64]  = &m_jnccallCallConv_gcc64;
	m_callConvArray[CallConvKind_Jnccall_arm32]  = &m_jnccallCallConv_arm32;
	m_callConvArray[CallConvKind_Jnccall_arm64]  = &m_jnccallCallConv_arm64;
	m_callConvArray[CallConvKind_Cdecl_msc32]    = &m_cdeclCallConv_msc32;
	m_callConvArray[CallConvKind_Cdecl_msc64]    = &m_cdeclCallConv_msc64;
	m_callConvArray[CallConvKind_Cdecl_gcc32]    = &m_cdeclCallConv_gcc32;
	m_callConvArray[CallConvKind_Cdecl_gcc64]    = &m_cdeclCallConv_gcc64;
	m_callConvArray[CallConvKind_Cdecl_arm32]    = &m_cdeclCallConv_arm32;
	m_callConvArray[CallConvKind_Cdecl_arm64]    = &m_cdeclCallConv_arm64;
	m_callConvArray[CallConvKind_Stdcall_msc32]  = &m_stdcallCallConv_msc32;
	m_callConvArray[CallConvKind_Stdcall_gcc32]  = &m_stdcallCallConv_gcc32;
	m_callConvArray[CallConvKind_Thiscall_msc32] = &m_thiscallCallConv_msc32;
}

void
TypeMgr::setupPrimitiveType(
	TypeKind typeKind,
	const sl::StringRef& signature,
	size_t size,
	size_t alignment,
	uint_t flags
) {
	ASSERT(typeKind < TypeKind__PrimitiveTypeCount);

	Type* type = &m_primitiveTypeArray[typeKind];
#if (_JNC_DEBUG)
	*(sl::ListLink*)type = sl::g_nullListLink; // make it look nice in debugger
#endif
	type->m_module = m_module;
	type->m_typeKind = typeKind;
	type->m_signature = signature;
	type->m_size = size;
	type->m_alignment = alignment;
	type->m_flags = flags;
	type->m_llvmType = NULL;
#if (LLVM_VERSION < 0x030900)
	type->m_llvmDiType = llvm::DIType_vn();
#else
	type->m_llvmDiType = NULL;
#endif
	type->m_typeVariable = NULL;
	type->m_simplePropertyTypeTuple = NULL;
	type->m_functionArgTuple = NULL;
	type->m_dataPtrTypeTuple = NULL;
	type->m_dualTypeTuple = NULL;
}

void
TypeMgr::setupStdTypedef(
	StdTypedef stdTypedef,
	TypeKind typeKind,
	const sl::StringRef& name
) {
	ASSERT(stdTypedef < StdTypedef__Count);
	ASSERT(typeKind < TypeKind__PrimitiveTypeCount);

	Typedef* tdef = &m_stdTypedefArray[stdTypedef];
#if (_JNC_DEBUG)
	*(sl::ListLink*)tdef = sl::g_nullListLink; // make it look nice in debugger
#endif
	tdef->m_module = m_module;
	tdef->m_name = name;
	tdef->m_type = &m_primitiveTypeArray[typeKind];
}

NamedType*
TypeMgr::parseStdType(
	StdType stdType,
	Unit* unit
) {
	const StdItemSource* source = getStdTypeSource(stdType);
	ASSERT(source->m_source);

	return parseStdType(
		sl::StringRef(source->m_source, source->m_length),
		source->m_stdNamespace,
		unit
	);
}

NamedType*
TypeMgr::parseStdType(
	const sl::StringRef& source,
	StdNamespace stdNamespace,
	Unit* unit
) {
	bool result;

	sl::StringRef fileName = "jnc_StdTypes.jnc";
	Lexer lexer(LexerFlag_Parse);
	lexer.create(fileName, source);

	if (stdNamespace)
		m_module->m_namespaceMgr.openStdNamespace(stdNamespace);

	Unit* prevUnit = m_module->m_unitMgr.setCurrentUnit(unit);
	ASSERT(prevUnit);

	Parser parser(m_module, NULL, Parser::Mode_Parse);
	parser.create(fileName, SymbolKind_named_type_specifier);
#if (_LLK_RANDOM_ERRORS)
	parser.disableRandomErrors();
#endif

	bool isEof;
	do {
		Token* token = lexer.takeToken();
		isEof = token->m_token == TokenKind_Eof; // EOF token must be parsed
		result = parser.consumeToken(token);
#if (_JNC_DEBUG)
		if (!result) {
			TRACE("std type parse error: %s\n", err::getLastErrorDescription().sz());
			ASSERT(false && "std type parse error");
		}
#endif
	} while (!isEof);

	m_module->m_unitMgr.setCurrentUnit(prevUnit);

	if (stdNamespace)
		m_module->m_namespaceMgr.closeNamespace();

	NamedType* type = (NamedType*)parser.getLastDeclaredItem();

	ASSERT(
		type &&
		type->getItemKind() == ModuleItemKind_Type &&
		(type->getTypeKindFlags() & TypeKindFlag_Named)
	);

	return type;
}

ClassType*
TypeMgr::createAbstractClassType() {
	ClassType* type = createInternalClassType("jnc.AbstractClass");
	type->m_classTypeKind = ClassTypeKind_Abstract;
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createAbstractDataType() {
	StructType* type = createInternalStructType("jnc.AbstractData");
	type->m_structTypeKind = StructTypeKind_AbstractData;
	type->ensureLayout();
	type->m_flags |= TypeFlag_GcRoot;
	type->m_flags &= ~TypeFlag_Pod;
	return type;
}

StructType*
TypeMgr::createIfaceHdrType() {
	StructType* type = createInternalStructType("jnc.IfaceHdr");
	type->createField("!m_vtable", getStdType(StdType_ByteThinPtr));
	type->createField("!m_box", getStdType(StdType_BoxPtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createBoxType() {
	StructType* type = createInternalStructType("jnc.Box");
	type->createField("!m_type", getStdType(StdType_ByteThinPtr));
	type->createField("!m_flags", getPrimitiveType(TypeKind_IntPtr_u));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createDataBoxType() {
	StructType* type = createInternalStructType("jnc.DataBox");
	type->createField("!m_type", getStdType(StdType_ByteThinPtr));
	type->createField("!m_flags", getPrimitiveType(TypeKind_IntPtr_u));
	type->createField("!m_validator", getStdType(StdType_DataPtrValidator));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createDetachedDataBoxType() {
	StructType* type = createInternalStructType("jnc.DetachedDataBox");
	type->createField("!m_type", getStdType(StdType_ByteThinPtr));
	type->createField("!m_flags", getPrimitiveType(TypeKind_IntPtr_u));
	type->createField("!m_validator", getStdType(StdType_DataPtrValidator));
	type->createField("!m_p", getStdType(StdType_ByteThinPtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createDataPtrValidatorType() {
	StructType* type = createInternalStructType("jnc.DataPtrValidator");
	type->createField("!m_validatorBox", getStdType(StdType_BoxPtr));
	type->createField("!m_targetBox", getStdType(StdType_BoxPtr));
	type->createField("!m_rangeBegin", getStdType(StdType_ByteThinPtr));
	type->createField("!m_rangeEnd", getStdType(StdType_ByteThinPtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createDataPtrStructType() {
	StructType* type = createInternalStructType("jnc.DataPtr");
	type->createField("!m_p", getStdType(StdType_ByteThinPtr));
	type->createField("!m_validator", getStdType(StdType_DataPtrValidatorPtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createFunctionPtrStructType() {
	StructType* type = createInternalStructType("jnc.FunctionPtr");
	type->createField("!m_p", getStdType(StdType_ByteThinPtr));
	type->createField("!m_closure", getStdType(StdType_AbstractClassPtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createVariantStructType() {
	ASSERT(sizeof(Variant) == sizeof(void*) * 8);
	ASSERT(!(Variant::DataSize & 7));

	StructType* type = createInternalStructType("jnc.Variant");
	type->createField("!m_data", getPrimitiveType(TypeKind_Int64)->getArrayType(Variant::DataSize / sizeof(int64_t)));
	type->createField("!_m_padding", getPrimitiveType(TypeKind_SizeT));
	type->createField("!m_type", getStdType(StdType_ByteThinPtr));
	type->ensureLayout();
	ASSERT(type->getSize() == sizeof(Variant));
	ASSERT(type->getAlignment() == 8);
	return type;
}

StructType*
TypeMgr::createStringStructType() {
	// m_p && m_length are accessible directly (e.g.: string_t s; file.write(s.m_p, s.m_length);

	StructType* type = createInternalStructType("jnc.string_t");
	type->createField("m_p", getStdType(StdType_CharConstPtr), 0, PtrTypeFlag_ReadOnly);
	type->createField("!m_ptr_sz", getStdType(StdType_CharConstPtr));
	type->createField("m_length", getPrimitiveType(TypeKind_SizeT), 0, PtrTypeFlag_ReadOnly);
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createGcShadowStackFrameType() {
	StructType* type = createInternalStructType("jnc.GcShadowStackFrame");
	type->createField("!m_prev", type->getDataPtrType_c());
	type->createField("!m_map", getStdType(StdType_ByteThinPtr));
	type->createField("!m_gcRootArray", getStdType(StdType_ByteThinPtr)->getDataPtrType_c());
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createSjljFrameType() {
	ArrayType* arrayType = getArrayType(getPrimitiveType(TypeKind_Char), sizeof(jmp_buf));
	StructType* type = createInternalStructType("jnc.SjljFrame");
	type->createField("!m_jmpBuf", arrayType);

#if (_JNC_OS_POSIX)
	type->createField("!m_signal", getPrimitiveType(TypeKind_Int));
	type->createField("!m_code", getPrimitiveType(TypeKind_Int));
	type->createField("!m_codeAddress", getPrimitiveType(TypeKind_IntPtr_u));
	type->createField("!m_faultAddress", getPrimitiveType(TypeKind_IntPtr_u));
#	if (JNC_PTR_SIZE == 8)
	type->createField("!_m_padding", getPrimitiveType(TypeKind_Int64));
#	endif
#endif

	type->ensureLayout();
	type->m_alignment = 16; // override alignment
	return type;
}

//..............................................................................

} // namespace ct
} // namespace jnc
