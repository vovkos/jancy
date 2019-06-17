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
#include "jnc_ct_MulticastClassType.h"
#include "jnc_ct_McSnapshotClassType.h"
#include "jnc_ct_Parser.llk.h"
#include "jnc_Variant.h"

// it's very common for classes and structs to reference themselves
// in pointer fields, retvals, arguments etc
// adding self to the namespace avoids creating unnecessary import types

#define _JNC_NAMED_TYPE_ADD_SELF

namespace jnc {
namespace ct {

//..............................................................................

TypeMgr::TypeMgr()
{
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	setupAllPrimitiveTypes();
	setupStdTypedefArray();
	setupCallConvArray();

	memset(m_stdTypeArray, 0, sizeof(m_stdTypeArray));
	memset(m_lazyStdTypeArray, 0, sizeof(m_lazyStdTypeArray));

	m_unnamedTypeCounter = 0;
	m_parseStdTypeLevel = 0;
}

void
TypeMgr::clear()
{
	m_typeList.clear();
	m_typedefList.clear();
	m_lazyStdTypeList.clear();
	m_functionArgList.clear();
	m_structFieldList.clear();

	m_simplePropertyTypeTupleList.clear();
	m_functionArgTupleList.clear();
	m_dataPtrTypeTupleList.clear();
	m_classPtrTypeTupleList.clear();
	m_functionPtrTypeTupleList.clear();
	m_propertyPtrTypeTupleList.clear();
	m_dualTypeTupleList.clear();

	m_typeMap.clear();

	m_multicastClassTypeArray.clear();
	m_unresolvedNamedImportTypeArray.clear();
	m_unresolvedImportPtrTypeArray.clear();
	m_unresolvedImportIntModTypeArray.clear();

	setupAllPrimitiveTypes();

	memset(m_stdTypeArray, 0, sizeof(m_stdTypeArray));
	memset(m_lazyStdTypeArray, 0, sizeof(m_lazyStdTypeArray));

	m_unnamedTypeCounter = 0;
	m_parseStdTypeLevel = 0;
}

Type*
TypeMgr::getStdType(StdType stdType)
{
	ASSERT((size_t)stdType < StdType__Count);
	if (m_stdTypeArray[stdType])
		return m_stdTypeArray[stdType];

	Type* type;
	switch (stdType)
	{
	case StdType_BytePtr:
		type = getPrimitiveType(TypeKind_Int8_u)->getDataPtrType_c();
		break;

	case StdType_CharConstPtr:
		type = getPrimitiveType(TypeKind_Char)->getDataPtrType_c(TypeKind_DataPtr, PtrTypeFlag_Const);
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
		type = getFunctionType(getPrimitiveType(TypeKind_Void), NULL, 0, 0);
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

	case StdType_RegexMatch:
		type = (Type*)m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc)->findItemByName("RegexMatch");
		if (!type)
			type = parseStdType(stdType);
		break;

	case StdType_RegexState:
		type = (Type*)m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc)->findItemByName("RegexState");
		if (!type)
			type = parseStdType(stdType);
		break;

	case StdType_RegexDfa:
		type = (Type*)m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc)->findItemByName("RegexDfa");
		if (!type)
			type = parseStdType(stdType);
		break;

	case StdType_Promise:
		type = (Type*)m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc)->findItemByName("Promise");
		if (type)
			break;

		type = parseStdType(stdType);
		getStdType(StdType_Promisifier); // make sure Promisifier is also added
		break;

	case StdType_PromisePtr:
		type = ((ClassType*)getStdType(StdType_Promise))->getClassPtrType(ClassPtrTypeKind_Normal);
		break;

	case StdType_Promisifier:
		type = (Type*)m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc)->findItemByName("Promisifier");
		if (!type)
			type = parseStdType(stdType);
		break;

	case StdType_DynamicLib:
		type = (Type*)m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc)->findItemByName("DynamicLib");
		if (!type)
			type = parseStdType(stdType);
		break;

	case StdType_Scheduler:
		type = (Type*)m_module->m_namespaceMgr.getStdNamespace(StdNamespace_Jnc)->findItemByName("Scheduler");
		if (!type)
			type = parseStdType(stdType);
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

	case StdType_FmtLiteral:
	case StdType_Int64Int64:
	case StdType_Fp64Fp64:
	case StdType_Int64Fp64:
	case StdType_Fp64Int64:
	case StdType_DynamicLayout:
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

LazyStdType*
TypeMgr::getLazyStdType(StdType stdType)
{
	ASSERT((size_t)stdType < StdType__Count);

	if (m_lazyStdTypeArray[stdType])
		return m_lazyStdTypeArray[stdType];

	LazyStdType* type = AXL_MEM_NEW(LazyStdType);
	type->m_module = m_module;
	type->m_stdType = stdType;
	m_lazyStdTypeList.insertTail(type);
	m_lazyStdTypeArray[stdType] = type;
	return type;
}

void
pushImportSrcPosError(NamedImportType* importType)
{
	lex::pushSrcPosError(
		importType->getParentUnit()->getFilePath(),
		*importType->getPos()
		);
}

bool
TypeMgr::resolveImportTypes()
{
	char buffer[256];
	sl::Array<NamedImportType*> superImportTypeArray(ref::BufKind_Stack, buffer, sizeof(buffer));

	// new items can be added in the process of resolving

	while (
		!m_unresolvedNamedImportTypeArray.isEmpty() ||
		!m_unresolvedImportPtrTypeArray.isEmpty() ||
		!m_unresolvedImportIntModTypeArray.isEmpty()
		)
	{
		superImportTypeArray.clear();

		sl::Array<NamedImportType*> unresolvedNamedImportTypeArray = m_unresolvedNamedImportTypeArray;
		sl::Array<ImportPtrType*> unresolvedImportPtrTypeArray = m_unresolvedImportPtrTypeArray;
		sl::Array<ImportIntModType*> unresolvedImportIntModTypeArray = m_unresolvedImportIntModTypeArray;

		m_unresolvedNamedImportTypeArray.clear();
		m_unresolvedImportIntModTypeArray.clear();
		m_unresolvedImportPtrTypeArray.clear();

		size_t count = unresolvedNamedImportTypeArray.getCount();
		for (size_t i = 0; i < count; i++)
		{
			NamedImportType* importType = unresolvedNamedImportTypeArray[i];
			Namespace* anchorNamespace = importType->m_anchorNamespace;
			if (!importType->m_anchorName.isEmpty())
			{
				ModuleItem* item = anchorNamespace->findItemTraverse(importType->m_anchorName);
				anchorNamespace = item ? item->getNamespace() : NULL;
			}

			ModuleItem* item = anchorNamespace ? anchorNamespace->findItemTraverse(importType->m_name) : NULL;
			if (!item)
			{
				err::setFormatStringError("unresolved import '%s'", importType->getTypeString().sz());
				pushImportSrcPosError(importType);
				return false;
			}

			ModuleItemKind itemKind = item->getItemKind();
			switch (itemKind)
			{
			case ModuleItemKind_Type:
				importType->m_actualType = (Type*)item;
				break;

			case ModuleItemKind_Typedef:
				importType->m_actualType = (m_module->getCompileFlags() & ModuleCompileFlag_KeepTypedefShadow) ?
					((Typedef*)item)->getShadowType() :
					((Typedef*)item)->getType();

				if (importType->m_actualType->getTypeKind() == TypeKind_NamedImport)
					superImportTypeArray.append(importType);
				break;

			default:
				err::setFormatStringError("'%s' is not a type", importType->getTypeString().sz());
				pushImportSrcPosError(importType);
				return false;
			}

			importType->applyFixups();
		}

		// eliminate super-imports and detect import loops

		count = superImportTypeArray.getCount();
		for (size_t i = 0; i < count; i++)
		{
			NamedImportType* superImportType = superImportTypeArray[i];
			superImportType->m_flags |= ImportTypeFlag_ImportLoop;

			Type* type = superImportType->m_actualType;
			while (type->m_typeKind == TypeKind_NamedImport)
			{
				ImportType* importType = (ImportType*)type;
				if (importType->m_flags & ImportTypeFlag_ImportLoop)
				{
					err::setFormatStringError("'%s': import loop detected", importType->getTypeString().sz());
					pushImportSrcPosError(superImportType);
					return false;
				}

				importType->m_flags |= ImportTypeFlag_ImportLoop;
				type = importType->m_actualType;
			}

			Type* externType = type;
			while (type->m_typeKind == TypeKind_NamedImport)
			{
				ImportType* importType = (ImportType*)type;
				importType->m_actualType = externType;
				importType->m_flags &= ~ImportTypeFlag_ImportLoop;
				importType->applyFixups();

				type = importType->m_actualType;
			}
		}

		count = unresolvedImportIntModTypeArray.getCount();
		for (size_t i = 0; i < count; i++)
		{
			ImportIntModType* importType = unresolvedImportIntModTypeArray[i];

			DeclTypeCalc typeCalc;

			Type* type = typeCalc.calcIntModType(
				importType->m_importType->m_actualType,
				importType->m_typeModifiers
				);

			if (!type)
				return false;

			importType->m_actualType = type;
			importType->applyFixups();
		}

		count = unresolvedImportPtrTypeArray.getCount();
		for (size_t i = 0; i < count; i++)
		{
			ImportPtrType* importType = unresolvedImportPtrTypeArray[i];

			DeclTypeCalc typeCalc;

			Type* type = typeCalc.calcPtrType(
				importType->m_targetType->m_actualType,
				importType->m_typeModifiers
				);

			if (!type)
				return false;

			if (importType->getFlags() & PtrTypeFlag_Safe)
				type = getCheckedPtrType(type);

			importType->m_actualType = type;
			importType->applyFixups();
		}
	}

	return true;
}

void
TypeMgr::updateTypeSignature(
	Type* type,
	const sl::StringRef& signature
	)
{
	if (type->m_signature == signature)
		return;

	if (!type->m_typeMapIt)
	{
		type->m_signature = signature;
		return;
	}

	m_typeMap.erase(type->m_typeMapIt);
	type->m_signature = signature;
	type->m_typeMapIt = m_typeMap.visit(signature);
	type->m_typeMapIt->m_value = type;
}

BitFieldType*
TypeMgr::getBitFieldType(
	Type* baseType,
	size_t bitOffset,
	size_t bitCount
	)
{
	sl::String signature = BitFieldType::createSignature(baseType, bitOffset, bitCount);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		BitFieldType* type = (BitFieldType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	BitFieldType* type = AXL_MEM_NEW(BitFieldType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_baseType = baseType;
	type->m_bitOffset = bitOffset;
	type->m_bitCount = bitCount;
	m_typeList.insertTail(type);
	it->m_value = type;

	if (baseType->getTypeKindFlags() & TypeKindFlag_Import)
	{
		((ImportType*)baseType)->addFixup(&type->m_baseType);
		m_module->markForLayout(type, true);
	}
	else
	{
		bool result = type->ensureLayout();
		if (!result)
			return NULL;
	}

	return type;
}

ArrayType*
TypeMgr::createAutoSizeArrayType(Type* elementType)
{
	ArrayType* type = AXL_MEM_NEW(ArrayType);
	type->m_flags |= ArrayTypeFlag_AutoSize;
	type->m_module = m_module;
	type->m_elementType = elementType;
	m_typeList.insertTail(type);

	if (elementType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)elementType)->addFixup(&type->m_elementType);

	if (!m_module->m_namespaceMgr.getCurrentScope())
		m_module->markForLayout(type, true); // can't calclayout yet

	return type;
}

ArrayType*
TypeMgr::createArrayType(
	Type* elementType,
	sl::BoxList<Token>* elementCountInitializer
	)
{
	ArrayType* type = AXL_MEM_NEW(ArrayType);
	type->m_module = m_module;
	type->m_elementType = elementType;
	sl::takeOver(&type->m_elementCountInitializer, elementCountInitializer);
	type->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
	type->m_parentNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	m_typeList.insertTail(type);

	if (elementType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)elementType)->addFixup(&type->m_elementType);

	if (!m_module->m_namespaceMgr.getCurrentScope())
	{
		m_module->markForLayout(type, true);
	}
	else
	{
		bool result = type->ensureLayout();
		if (!result)
			return NULL;
	}

	return type;
}

ArrayType*
TypeMgr::getArrayType(
	Type* elementType,
	size_t elementCount
	)
{
	sl::String signature = ArrayType::createSignature(elementType, elementCount);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		ArrayType* type = (ArrayType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	ArrayType* type = AXL_MEM_NEW(ArrayType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_elementType = elementType;
	type->m_elementCount = elementCount;
	m_typeList.insertTail(type);

	if (elementType->getTypeKindFlags() & TypeKindFlag_Import)
	{
		((ImportType*)elementType)->addFixup(&type->m_elementType);
		m_module->markForLayout(type, true);
	}
	else
	{
		bool result = type->ensureLayout();
		if (!result)
			return NULL;
	}

	it->m_value = type;
	return type;
}

Typedef*
TypeMgr::createTypedef(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	Type* type
	)
{
	Typedef* tdef = AXL_MEM_NEW(Typedef);
	tdef->m_module = m_module;
	tdef->m_name = name;
	tdef->m_qualifiedName = qualifiedName;
	tdef->m_type = type;
	m_typedefList.insertTail(tdef);

	if (type->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)type)->addFixup(&tdef->m_type);

	return tdef;
}

TypedefShadowType*
TypeMgr::createTypedefShadowType(Typedef* tdef)
{
	TypedefShadowType* type = AXL_MEM_NEW(TypedefShadowType);
	type->m_module = m_module;
	type->m_signature.format("T%s", tdef->getQualifiedName().sz());
	type->m_parentUnit = tdef->m_parentUnit;
	type->m_parentNamespace = tdef->m_parentNamespace;
	type->m_pos = tdef->m_pos;
	type->m_storageKind = tdef->m_storageKind;
	type->m_accessKind = tdef->m_accessKind;
	type->m_name = tdef->m_name;
	type->m_qualifiedName = tdef->m_qualifiedName;
	type->m_attributeBlock = tdef->m_attributeBlock;
	type->m_typedef = tdef;
	m_typeList.insertTail(type);

	return type;
}

EnumType*
TypeMgr::createEnumType(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	Type* baseType,
	uint_t flags
	)
{
	const char* signaturePrefix = (flags & EnumTypeFlag_BitFlag) ?
		(flags & EnumTypeFlag_Exposed) ? "EZ" : "EF" :
		(flags & EnumTypeFlag_Exposed) ? "EC" : "EE";

	EnumType* type = AXL_MEM_NEW(EnumType);
	type->m_signature.format("%s%s", signaturePrefix, qualifiedName.sz());
	type->m_name = name;
	type->m_qualifiedName = qualifiedName;
	type->m_flags |= TypeFlag_Named;

#ifdef _JNC_NAMED_TYPE_ADD_SELF
	type->addItem(type);
#endif

	if (!baseType)
		baseType = getPrimitiveType(TypeKind_Int);

	type->m_module = m_module;
	type->m_baseType = baseType;
	type->m_flags |= flags;
	m_typeList.insertTail(type);

	if (baseType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)baseType)->addFixup(&type->m_baseType);

	m_module->markForLayout(type, true);
	return type;
}

StructType*
TypeMgr::createStructType(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	size_t fieldAlignment,
	uint_t flags
	)
{
	StructType* type = AXL_MEM_NEW(StructType);
	type->m_signature.format("S%s", qualifiedName.sz());
	type->m_name = name;
	type->m_qualifiedName = qualifiedName;
	type->m_flags |= TypeFlag_Named;

#ifdef _JNC_NAMED_TYPE_ADD_SELF
	type->addItem(type);
#endif

	type->m_module = m_module;
	type->m_fieldAlignment = fieldAlignment;
	type->m_flags |= flags;
	m_typeList.insertTail(type);
	m_module->markForLayout(type, true);
	return type;
}

UnionType*
TypeMgr::createUnionType(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	size_t fieldAlignment,
	uint_t flags
	)
{
	UnionType* type = AXL_MEM_NEW(UnionType);
	type->m_signature.format("U%s", qualifiedName.sz());
	type->m_name = name;
	type->m_qualifiedName = qualifiedName;
	type->m_flags |= TypeFlag_Named;

#ifdef _JNC_NAMED_TYPE_ADD_SELF
	type->addItem(type);
#endif

	m_module->markForLayout(type, true); // before child struct

	type->m_module = m_module;
	type->m_flags |= flags;

	if (!(flags & TypeFlag_Dynamic))
	{
		StructType* unionStructType = createUnnamedStructType(type->createQualifiedName("Struct"), fieldAlignment);
		unionStructType->m_parentNamespace = type;
		unionStructType->m_structTypeKind = StructTypeKind_UnionStruct;
		type->m_structType = unionStructType;
	}

	m_typeList.insertTail(type);
	return type;
}

ClassType*
TypeMgr::createClassType(
	ClassTypeKind classTypeKind,
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	size_t fieldAlignment,
	uint_t flags
	)
{
	ClassType* type;

	switch (classTypeKind)
	{
	case ClassTypeKind_Reactor:
		type = AXL_MEM_NEW(ReactorClassType);
		break;

	case ClassTypeKind_FunctionClosure:
		type = AXL_MEM_NEW(FunctionClosureClassType);
		break;

	case ClassTypeKind_PropertyClosure:
		type = AXL_MEM_NEW(PropertyClosureClassType);
		break;

	case ClassTypeKind_DataClosure:
		type = AXL_MEM_NEW(DataClosureClassType);
		break;

	case ClassTypeKind_Multicast:
		type = AXL_MEM_NEW(MulticastClassType);
		m_multicastClassTypeArray.append((MulticastClassType*)type);
		break;

	case ClassTypeKind_McSnapshot:
		type = AXL_MEM_NEW(McSnapshotClassType);
		break;

	default:
		type = AXL_MEM_NEW(ClassType);
	}

	m_typeList.insertTail(type);

	type->m_signature.format("CC%s", qualifiedName.sz());
	type->m_name = name;
	type->m_qualifiedName = qualifiedName;
	type->m_flags |= TypeFlag_Named;

#ifdef _JNC_NAMED_TYPE_ADD_SELF
	type->addItem(type);
#endif

	m_module->markForLayout(type, true); // before child structs

	StructType* ifaceStructType = createUnnamedStructType(type->createQualifiedName("Iface"), fieldAlignment);
	ifaceStructType->m_structTypeKind = StructTypeKind_IfaceStruct;
	ifaceStructType->m_parentNamespace = type;
	ifaceStructType->m_storageKind = StorageKind_Member;

	StructType* classStructType = createUnnamedStructType(type->createQualifiedName("Class"), fieldAlignment);
	classStructType->m_structTypeKind = StructTypeKind_ClassStruct;
	classStructType->m_parentNamespace = type;
	classStructType->createField("!m_box", getStdType (StdType_Box));
	classStructType->createField("!m_iface", ifaceStructType);

	type->m_module = m_module;
	type->m_flags |= flags;
	type->m_classTypeKind = classTypeKind;
	type->m_ifaceStructType = ifaceStructType;
	type->m_classStructType = classStructType;
	return type;
}

FunctionArg*
TypeMgr::createFunctionArg(
	const sl::StringRef& name,
	Type* type,
	uint_t ptrTypeFlags,
	sl::BoxList<Token>* initializer
	)
{
	FunctionArg* functionArg = AXL_MEM_NEW(FunctionArg);
	functionArg->m_module = m_module;
	functionArg->m_name = name;
	functionArg->m_qualifiedName = name;
	functionArg->m_type = type;
	functionArg->m_ptrTypeFlags = ptrTypeFlags;

	if (initializer)
		sl::takeOver(&functionArg->m_initializer, initializer);

	m_functionArgList.insertTail(functionArg);

	if (type->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)type)->addFixup(&functionArg->m_type);

	return functionArg;
}

StructField*
TypeMgr::createStructField(
	const sl::StringRef& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	sl::BoxList<Token>* constructor,
	sl::BoxList<Token>* initializer
	)
{
	StructField* field = AXL_MEM_NEW(StructField);
	field->m_module = m_module;
	field->m_name = name;
	field->m_type = type;
	field->m_ptrTypeFlags = ptrTypeFlags;
	field->m_bitFieldBaseType = bitCount ? type : NULL;
	field->m_bitCount = bitCount;

	if (constructor)
		sl::takeOver(&field->m_constructor, constructor);

	if (initializer)
		sl::takeOver(&field->m_initializer, initializer);

	m_structFieldList.insertTail(field);

	if (type->getTypeKindFlags() & TypeKindFlag_Import)
	{
		((ImportType*)type)->addFixup(&field->m_type);
		if (bitCount)
			((ImportType*)type)->addFixup(&field->m_bitFieldBaseType);
	}

	return field;
}

FunctionArg*
TypeMgr::getSimpleFunctionArg(
	StorageKind storageKind,
	Type* type,
	uint_t ptrTypeFlags
	)
{
	FunctionArgTuple* tuple = getFunctionArgTuple(type);

	// this x const x volatile

	size_t i1 = storageKind == StorageKind_This;
	size_t i2 = (ptrTypeFlags & PtrTypeFlag_Const) != 0;
	size_t i3 = (ptrTypeFlags & PtrTypeFlag_Volatile) != 0;

	if (tuple->m_argArray[i1][i2][i3])
		return tuple->m_argArray[i1][i2][i3];

	FunctionArg* arg = createFunctionArg(sl::String(), type, ptrTypeFlags);
	if (!arg)
		return NULL;

	arg->m_storageKind = storageKind;

	tuple->m_argArray[i1][i2][i3] = arg;
	return arg;
}

FunctionType*
TypeMgr::getFunctionType(
	CallConv* callConv,
	Type* returnType,
	const sl::Array<FunctionArg*>& argArray,
	uint_t flags
	)
{
	ASSERT(callConv && returnType);

	sl::String signature = FunctionType::createSignature(
		callConv,
		returnType,
		argArray,
		argArray.getCount(),
		flags
		);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		FunctionType* type = (FunctionType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	FunctionType* type = AXL_MEM_NEW(FunctionType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_callConv = callConv;
	type->m_returnType = returnType;
	type->m_flags = flags;
	type->m_argArray = argArray;
	m_typeList.insertTail(type);

	if (returnType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)returnType)->addFixup(&type->m_returnType);

	if (m_parseStdTypeLevel || !m_module->m_namespaceMgr.getCurrentScope())
	{
		m_module->markForLayout(type, true);
	}
	else
	{
		bool result = type->ensureLayout();
		if (!result)
			return NULL;
	}

	it->m_value = type;
	return type;
}

FunctionType*
TypeMgr::getFunctionType(
	CallConv* callConv,
	Type* returnType,
	Type* const* argTypeArray,
	size_t argCount,
	uint_t flags
	)
{
	ASSERT(callConv && returnType);

	sl::Array<FunctionArg*> argArray;
	argArray.setCount(argCount);
	for (size_t i = 0; i < argCount; i++)
	{
		FunctionArg* arg = getSimpleFunctionArg(argTypeArray[i]);
		if (!arg)
			return NULL;

		argArray[i] = arg;
	}

	sl::String signature = FunctionType::createSignature(
		callConv,
		returnType,
		argTypeArray,
		argCount,
		flags
		);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		FunctionType* type = (FunctionType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	FunctionType* type = AXL_MEM_NEW(FunctionType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_callConv = callConv;
	type->m_returnType = returnType;
	type->m_flags = flags;
	type->m_argArray = argArray;
	m_typeList.insertTail(type);

	if (returnType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)returnType)->addFixup(&type->m_returnType);

	if (m_parseStdTypeLevel || !m_module->m_namespaceMgr.getCurrentScope())
	{
		m_module->markForLayout(type, true);
	}
	else
	{
		bool result = type->ensureLayout();
		if (!result)
			return NULL;
	}

	it->m_value = type;
	return type;
}

FunctionType*
TypeMgr::createUserFunctionType(
	CallConv* callConv,
	Type* returnType,
	const sl::Array<FunctionArg*>& argArray,
	uint_t flags
	)
{
	ASSERT(callConv && returnType);

	sl::String signature = FunctionType::createSignature(
		callConv,
		returnType,
		argArray,
		argArray.getCount(),
		flags
		);

	FunctionType* type = AXL_MEM_NEW(FunctionType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_callConv = callConv;

	if (flags & FunctionTypeFlag_Async)
	{
		type->m_asyncReturnType = returnType;
		returnType = m_module->m_typeMgr.getStdType(StdType_PromisePtr);

		if (flags & FunctionTypeFlag_ErrorCode)
		{
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

	if (m_parseStdTypeLevel || !m_module->m_namespaceMgr.getCurrentScope())
	{
		m_module->markForLayout(type, true);
	}
	else
	{
		bool result = type->ensureLayout();
		if (!result)
			return NULL;
	}

	return type;
}

FunctionType*
TypeMgr::getMemberMethodType(
	DerivableType* parentType,
	FunctionType* functionType,
	uint_t thisArgPtrTypeFlags
	)
{
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
			functionType->m_flags
			) :
		getFunctionType(
			functionType->m_callConv,
			returnType,
			argArray,
			functionType->m_flags
			);

	memberMethodType->m_shortType = functionType;
	return memberMethodType;
}

FunctionType*
TypeMgr::getStdObjectMemberMethodType(FunctionType* functionType)
{
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
	)
{
	sl::String signature = PropertyType::createSignature(getterType, setterType, flags);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		PropertyType* type = (PropertyType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	if (setterType.isEmpty())
		flags |= PropertyTypeFlag_Const;

	PropertyType* type = AXL_MEM_NEW(PropertyType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_getterType = getterType;
	type->m_setterType = setterType;
	type->m_flags = flags;

	if (flags & PropertyTypeFlag_Bindable)
	{
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
	)
{
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

	FunctionType* getterType = getFunctionType(callConv, returnType, NULL, 0, 0);
	if (flags & PropertyTypeFlag_Const)
	{
		propertyType = getPropertyType(getterType, NULL, flags);
	}
	else
	{
		Type* voidType = &m_primitiveTypeArray[TypeKind_Void];
		FunctionType* setterType = getFunctionType(callConv, voidType, &returnType, 1, 0);
		propertyType = getPropertyType(getterType, setterType, flags);
	}

	tuple->m_propertyTypeArray[i1][i2][i3] = propertyType;
	return propertyType;
}

PropertyType*
TypeMgr::getIndexedPropertyType(
	CallConv* callConv,
	Type* returnType,
	Type* const* indexArgTypeArray,
	size_t indexArgCount,
	uint_t flags
	)
{
	FunctionType* getterType = getFunctionType(callConv, returnType, indexArgTypeArray, indexArgCount, 0);

	if (flags & PropertyTypeFlag_Const)
		return getPropertyType(getterType, NULL, flags);

	char buffer[256];
	sl::Array<Type*> argTypeArray(ref::BufKind_Stack, buffer, sizeof(buffer));
	argTypeArray.copy(indexArgTypeArray, indexArgCount);
	argTypeArray.append(returnType);

	Type* voidType = &m_primitiveTypeArray[TypeKind_Void];
	FunctionType* setterType = getFunctionType(callConv, voidType, argTypeArray, indexArgCount + 1, 0);
	return getPropertyType(getterType, setterType, flags);
}

PropertyType*
TypeMgr::getIndexedPropertyType(
	CallConv* callConv,
	Type* returnType,
	const sl::Array<FunctionArg*>& argArray,
	uint_t flags
	)
{
	FunctionType* getterType = getFunctionType(callConv, returnType, argArray, 0);

	if (flags & PropertyTypeFlag_Const)
		return getPropertyType(getterType, NULL, flags);

	sl::Array<FunctionArg*> setterArgArray = argArray;
	setterArgArray.append(returnType->getSimpleFunctionArg());

	Type* voidType = &m_primitiveTypeArray[TypeKind_Void];
	FunctionType* setterType = getFunctionType(callConv, voidType, setterArgArray, 0);
	return getPropertyType(getterType, setterType, flags);
}

PropertyType*
TypeMgr::createIndexedPropertyType(
	CallConv* callConv,
	Type* returnType,
	const sl::Array<FunctionArg*>& argArray,
	uint_t flags
	)
{
	FunctionType* getterType = createUserFunctionType(callConv, returnType, argArray, 0);

	if (flags & PropertyTypeFlag_Const)
		return getPropertyType(getterType, NULL, flags);

	sl::Array<FunctionArg*> setterArgArray = argArray;
	setterArgArray.append(returnType->getSimpleFunctionArg());

	Type* voidType = &m_primitiveTypeArray[TypeKind_Void];
	FunctionType* setterType = createUserFunctionType(callConv, voidType, setterArgArray, 0);
	return getPropertyType(getterType, setterType, flags);
}

PropertyType*
TypeMgr::getMemberPropertyType(
	DerivableType* parentType,
	PropertyType* propertyType
	)
{
	FunctionType* getterType = getMemberMethodType(
		parentType,
		propertyType->m_getterType,
		PtrTypeFlag_Const
		);

	size_t setterTypeOverloadCount = propertyType->m_setterType.getOverloadCount();

	char buffer[256];
	sl::Array<FunctionType*> setterTypeOverloadArray(ref::BufKind_Stack, buffer, sizeof(buffer));
	setterTypeOverloadArray.setCount(setterTypeOverloadCount);

	for (size_t i = 0; i < setterTypeOverloadCount; i++)
	{
		FunctionType* overloadType = propertyType->m_setterType.getOverload(i);
		setterTypeOverloadArray[i] = getMemberMethodType(parentType, overloadType);
	}

	PropertyType* memberPropertyType = getPropertyType(
		getterType,
		FunctionTypeOverload(setterTypeOverloadArray, setterTypeOverloadCount),
		propertyType->m_flags
		);

	memberPropertyType->m_shortType = propertyType;
	return memberPropertyType;
}

PropertyType*
TypeMgr::getStdObjectMemberPropertyType(PropertyType* propertyType)
{
	if (propertyType->m_stdObjectMemberPropertyType)
		return propertyType->m_stdObjectMemberPropertyType;

	ClassType* classType = (ClassType*)getStdType(StdType_AbstractClass);
	propertyType->m_stdObjectMemberPropertyType = classType->getMemberPropertyType(propertyType);
	return propertyType->m_stdObjectMemberPropertyType;
}

PropertyType*
TypeMgr::getShortPropertyType(PropertyType* propertyType)
{
	if (propertyType->m_shortType)
		return propertyType->m_shortType;

	if (!propertyType->isMemberPropertyType())
	{
		propertyType->m_shortType = propertyType;
		return propertyType;
	}

	FunctionType* getterType = propertyType->m_getterType->getShortType();
	FunctionTypeOverload setterType;

	size_t setterCount = propertyType->m_setterType.getOverloadCount();
	for (size_t i = 0; i < setterCount; i++)
	{
		FunctionType* type = propertyType->m_setterType.getOverload(i)->getShortType();
		setterType.addOverload(type);
	}

	propertyType->m_shortType = getPropertyType(getterType, setterType, propertyType->m_flags);
	return propertyType->m_shortType;
}

ClassType*
TypeMgr::getMulticastType(FunctionPtrType* functionPtrType)
{
	bool result;

	if (functionPtrType->m_multicastType)
		return functionPtrType->m_multicastType;

	Type* returnType = functionPtrType->getTargetType()->getReturnType();
	if (returnType->getTypeKind() != TypeKind_Void)
	{
		err::setFormatStringError("multicast cannot only return 'void', not '%s'", returnType->getTypeString().sz());
		return NULL;
	}

	MulticastClassType* type = (MulticastClassType*)createUnnamedClassType(ClassTypeKind_Multicast, "Multicast");
	type->m_targetType = functionPtrType;
	type->m_flags |= (functionPtrType->m_flags & TypeFlag_GcRoot);

	// fields

	type->m_fieldArray[MulticastFieldKind_Lock] = type->createField("!m_lock", getPrimitiveType (TypeKind_IntPtr), 0, PtrTypeFlag_Volatile);
	type->m_fieldArray[MulticastFieldKind_PtrArray] = type->createField("!m_arrayPtr", functionPtrType->getDataPtrType ());
	type->m_fieldArray[MulticastFieldKind_Count] = type->createField("!m_count", getPrimitiveType (TypeKind_SizeT));
	type->m_fieldArray[MulticastFieldKind_MaxCount] = type->createField("!m_maxCount", getPrimitiveType (TypeKind_SizeT));
	type->m_fieldArray[MulticastFieldKind_HandleTable] = type->createField("!m_handleTable", getPrimitiveType (TypeKind_IntPtr));

	Type* argType;
	Function* method;
	FunctionType* methodType;

	bool isThin = functionPtrType->getPtrTypeKind() == FunctionPtrTypeKind_Thin;

	// destrutor

	methodType = getFunctionType();
	method = type->createUnnamedMethod(StorageKind_Member, FunctionKind_Destructor, methodType);
	method->m_flags |= ModuleItemFlag_User; // no need to generate default destructor
	type->m_destructor = method;

	// methods

	methodType = getFunctionType();
	method = type->createMethod(StorageKind_Member, "clear", methodType);
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray[MulticastMethodKind_Clear] = method;

	returnType = getPrimitiveType(TypeKind_IntPtr);
	argType = functionPtrType;
	methodType = getFunctionType(returnType, &argType, 1);

	method = type->createMethod(StorageKind_Member, "setup", methodType);
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray[MulticastMethodKind_Setup] = method;

	method = type->createMethod(StorageKind_Member, "add", methodType);
	type->m_methodArray[MulticastMethodKind_Add] = method;

	returnType = functionPtrType;
	argType = getPrimitiveType(TypeKind_IntPtr);
	methodType = getFunctionType(returnType, &argType, 1);
	method = type->createMethod(StorageKind_Member, "remove", methodType);
	type->m_methodArray[MulticastMethodKind_Remove] = method;

	returnType = functionPtrType->getNormalPtrType();
	methodType = getFunctionType(returnType, NULL, 0);
	method = type->createMethod(StorageKind_Member, "getSnapshot", methodType);
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray[MulticastMethodKind_GetSnapshot] = method;

	methodType = functionPtrType->getTargetType();
	method = type->createMethod(StorageKind_Member, "call", methodType);
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray[MulticastMethodKind_Call] = method;

	// overloaded operators

	type->m_binaryOperatorTable.setCountZeroConstruct(BinOpKind__Count);
	type->m_binaryOperatorTable[BinOpKind_RefAssign] = type->m_methodArray[MulticastMethodKind_Setup];
	type->m_binaryOperatorTable[BinOpKind_AddAssign] = type->m_methodArray[MulticastMethodKind_Add];
	type->m_binaryOperatorTable[BinOpKind_SubAssign] = type->m_methodArray[MulticastMethodKind_Remove];
	type->m_callOperator = type->m_methodArray[MulticastMethodKind_Call];

	// snapshot closure (snapshot is shared between weak and normal multicasts)

	McSnapshotClassType* snapshotType = (McSnapshotClassType*)createUnnamedClassType(ClassTypeKind_McSnapshot, "McSnapshot");
	snapshotType->m_targetType = functionPtrType->getUnWeakPtrType();
	snapshotType->m_flags |= (functionPtrType->m_flags & TypeFlag_GcRoot);

	// fields

	snapshotType->m_fieldArray[McSnapshotFieldKind_PtrArray] = snapshotType->createField("!m_arrayPtr", functionPtrType->getDataPtrType ());
	snapshotType->m_fieldArray[McSnapshotFieldKind_Count] = snapshotType->createField("!m_count", getPrimitiveType (TypeKind_SizeT));

	// call method

	methodType = functionPtrType->getTargetType();
	snapshotType->m_methodArray[McSnapshotMethodKind_Call] = snapshotType->createMethod(StorageKind_Member, "call", methodType);

	type->m_snapshotType = snapshotType;

	if (!m_module->m_namespaceMgr.getCurrentScope())
	{
		m_module->markForLayout(type);
		m_module->markForLayout(type->m_snapshotType);
	}
	else
	{
		result =
			type->ensureLayout  () &&
			type->m_snapshotType->ensureLayout();

		if (!result)
			return NULL;
	}

	m_module->markForCompile(type);
	m_module->markForCompile(type->m_snapshotType);

	functionPtrType->m_multicastType = type;
	return type;
}

ClassType*
TypeMgr::createReactorBaseType()
{
	Type* voidType = getPrimitiveType(TypeKind_Void);
	Type* eventPtrType = getStdType(StdType_SimpleEventPtr);
	Type* abstractPtrType = getStdType(StdType_AbstractClassPtr); // onevent statement can have different event types

	FunctionType* simpleFunctionType = (FunctionType*)getStdType(StdType_SimpleFunction);
	FunctionType* addOnChangedBindingType = getFunctionType(voidType, &eventPtrType, 1);
	FunctionType* addOnEventBindingType = getFunctionType(voidType, &abstractPtrType, 1);

	ClassType* type = createClassType("ReactorBase", "jnc.ReactorBase", 8, ClassTypeFlag_Opaque);
	type->createField("m_activationCountLimit", getPrimitiveType (TypeKind_SizeT));

	type->createDefaultMethod(FunctionKind_Constructor, StorageKind_Member, ModuleItemFlag_User); // no need to auto-generate ctor
	type->createDefaultMethod(FunctionKind_Destructor, StorageKind_Member, ModuleItemFlag_User); // no need to auto-generate dtor
	type->createMethod(StorageKind_Member, "start", simpleFunctionType);
	type->createMethod(StorageKind_Member, "stop", simpleFunctionType);
	type->createMethod(StorageKind_Member, "restart", simpleFunctionType);
	type->createMethod(StorageKind_Member, "!addOnChangedBinding", addOnChangedBindingType);
	type->createMethod(StorageKind_Member, "!addOnEventBinding", addOnEventBindingType);
	type->createMethod(StorageKind_Member, "!resetOnChangedBindings", simpleFunctionType);

	return type;
}

ReactorClassType*
TypeMgr::createReactorType(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	ClassType* parentType
	)
{
	ReactorClassType* type = (ReactorClassType*)createClassType(ClassTypeKind_Reactor, name, qualifiedName);

	type->addBaseType(getStdType(StdType_ReactorBase));
	type->m_parentType = parentType;

	Type* voidType = getPrimitiveType(TypeKind_Void);
	Type* sizeType = getPrimitiveType(TypeKind_SizeT);
	FunctionType* reactionType;

	if (!parentType)
	{
		reactionType = getFunctionType(voidType, (Type**) &sizeType, 1);
	}
	else
	{
		Type* argTypeArray[] = { parentType->getClassPtrType(), sizeType };
		reactionType = getFunctionType(voidType, argTypeArray, 2);
	}

	type->m_reaction = type->createMethod(StorageKind_Member, "!reaction", reactionType);

	m_module->markForCompile(type);
	getStdType(StdType_ReactorClosure); // ensure closure type is created
	return type;
}

FunctionClosureClassType*
TypeMgr::createReactorClosureType()
{
	FunctionClosureClassType* type = (FunctionClosureClassType*)createClassType(ClassTypeKind_FunctionClosure, "ReactorClosure", "jnc.ReactorClosure");
	type->m_thisArgFieldIdx = 0;
	type->createField("m_self", type->getClassPtrType ());
	type->createField("m_event", getStdType (StdType_BytePtr));
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
	)
{
	sl::String signature = ClosureClassType::createSignature(
		targetType,
		thunkType,
		argTypeArray,
		closureMap,
		argCount,
		thisArgIdx
		);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		FunctionClosureClassType* type = (FunctionClosureClassType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	FunctionClosureClassType* type = (FunctionClosureClassType*)createUnnamedClassType(ClassTypeKind_FunctionClosure, "FunctionClosure");
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_closureMap.copy(closureMap, argCount);
	type->m_thisArgFieldIdx = thisArgIdx + 1;

	type->createField("m_target", targetType->getFunctionPtrType (FunctionPtrTypeKind_Thin));

	sl::String argFieldName;

	for (size_t i = 0; i < argCount; i++)
	{
		argFieldName.format("m_arg%d", i);
		type->createField(argFieldName, argTypeArray[i]);
	}

	Function* thunkFunction = m_module->m_functionMgr.createFunction(
		FunctionKind_Internal,
		sl::String(),
		"jnc.thunkFunction",
		thunkType
		);

	type->addMethod(thunkFunction);
	type->m_thunkFunction = thunkFunction;

	type->ensureLayout();
	m_module->markForCompile(type);

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
	)
{
	sl::String signature = ClosureClassType::createSignature(
		targetType,
		thunkType,
		argTypeArray,
		closureMap,
		argCount,
		thisArgIdx
		);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		PropertyClosureClassType* type = (PropertyClosureClassType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	PropertyClosureClassType* type = (PropertyClosureClassType*)createUnnamedClassType(ClassTypeKind_PropertyClosure, "PropertyClosure");
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_closureMap.copy(closureMap, argCount);
	type->m_thisArgFieldIdx = thisArgIdx + 1;

	type->createField("m_target", targetType->getPropertyPtrType (PropertyPtrTypeKind_Thin));

	sl::String argFieldName;

	for (size_t i = 0; i < argCount; i++)
	{
		argFieldName.format("m_arg%d", i);
		type->createField(argFieldName, argTypeArray[i]);
	}

	Property* thunkProperty = m_module->m_functionMgr.createProperty(
		PropertyKind_Internal,
		sl::String(),
		type->createQualifiedName("m_thunkProperty")
		);

	type->addProperty(thunkProperty);
	type->m_thunkProperty = thunkProperty;

	thunkProperty->create(thunkType);

	type->ensureLayout();
	m_module->markForCompile(type);

	it->m_value = type;

	return type;
}

DataClosureClassType*
TypeMgr::getDataClosureClassType(
	Type* targetType,
	PropertyType* thunkType
	)
{
	sl::String signature = DataClosureClassType::createSignature(targetType, thunkType);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		DataClosureClassType* type = (DataClosureClassType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	DataClosureClassType* type = (DataClosureClassType*)createUnnamedClassType(ClassTypeKind_DataClosure, "DataClosure");
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->createField("!m_target", targetType->getDataPtrType ());

	Property* thunkProperty = m_module->m_functionMgr.createProperty(
		PropertyKind_Internal,
		sl::String(),
		type->createQualifiedName("m_thunkProperty")
		);

	type->addProperty(thunkProperty);
	type->m_thunkProperty = thunkProperty;

	thunkProperty->create(thunkType);

	type->ensureLayout();
	m_module->markForCompile(type);

	it->m_value = type;

	return type;
}

DataPtrType*
TypeMgr::getDataPtrType(
	Type* targetType,
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	ASSERT((size_t)ptrTypeKind < DataPtrTypeKind__Count);
	ASSERT(targetType->getTypeKind() != TypeKind_NamedImport); // for imports, getImportPtrType() should be called
	ASSERT(typeKind != TypeKind_DataRef || targetType->m_typeKind != TypeKind_DataRef); // double reference

	if (ptrTypeKind == DataPtrTypeKind_Normal)
		flags |= TypeFlag_GcRoot | TypeFlag_StructRet;

	if (isDualType(targetType))
		flags |= PtrTypeFlag_DualTarget;

	DataPtrTypeTuple* tuple = getDataPtrTypeTuple(targetType);

	// ref x ptrkind x const x volatile x checked/markup

	size_t i1 = typeKind == TypeKind_DataRef;
	size_t i2 = ptrTypeKind;
	size_t i3 = (flags & PtrTypeFlag_Const) ? 0 : (flags & PtrTypeFlag_ReadOnly) ? 1 : 2;
	size_t i4 = (flags & PtrTypeFlag_Volatile) ? 0 : 1;
	size_t i5 = (flags & PtrTypeFlag_Safe) ? 1 : 0;

	if (tuple->m_ptrTypeArray[i1][i2][i3][i4][i5])
		return tuple->m_ptrTypeArray[i1][i2][i3][i4][i5];

	size_t size = ptrTypeKind == DataPtrTypeKind_Normal ? sizeof(DataPtr) : sizeof(void*);

	DataPtrType* type = AXL_MEM_NEW(DataPtrType);
	type->m_module = m_module;
	type->m_signature = DataPtrType::createSignature(targetType, typeKind, ptrTypeKind, flags);
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_size = size;
	type->m_alignment = sizeof(void*);
	type->m_targetType = targetType;
	type->m_flags = flags;

	if (targetType->getTypeKindFlags() & TypeKindFlag_Import)
		((ImportType*)targetType)->addFixup(&type->m_targetType);

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
	)
{
	ASSERT((size_t)ptrTypeKind < ClassPtrTypeKind__Count);

	if (typeKind == TypeKind_ClassPtr)
		flags |= TypeFlag_GcRoot;

	ClassPtrTypeTuple* tuple;

	if (flags & (PtrTypeFlag_Event | PtrTypeFlag_DualEvent))
	{
		ASSERT(targetType->getClassTypeKind() == ClassTypeKind_Multicast);
		tuple = getEventClassPtrTypeTuple((MulticastClassType*)targetType);
	}
	else
	{
		tuple = getClassPtrTypeTuple(targetType);
	}

	// ref x ptrkind x const x volatile x checked

	size_t i1 = typeKind == TypeKind_ClassRef;
	size_t i2 = ptrTypeKind;
	size_t i3 = (flags & PtrTypeFlag_Const) ? 0 : (flags & PtrTypeFlag_ReadOnly) ? 1 : 2;
	size_t i4 = (flags & PtrTypeFlag_Volatile) ? 0 : 1;
	size_t i5 = (flags & PtrTypeFlag_Safe) ? 0 : 1;

	if (tuple->m_ptrTypeArray[i1][i2][i3][i4][i5])
		return tuple->m_ptrTypeArray[i1][i2][i3][i4][i5];

	ClassPtrType* type = AXL_MEM_NEW(ClassPtrType);
	type->m_module = m_module;
	type->m_signature = ClassPtrType::createSignature(targetType, typeKind, ptrTypeKind, flags);
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
	)
{
	ASSERT(typeKind == TypeKind_FunctionPtr || typeKind == TypeKind_FunctionRef);
	ASSERT((size_t)ptrTypeKind < FunctionPtrTypeKind__Count);

	if (typeKind == TypeKind_FunctionPtr && ptrTypeKind != FunctionPtrTypeKind_Thin)
		flags |= TypeFlag_GcRoot | TypeFlag_StructRet;

	if (functionType->m_flags & FunctionTypeFlag_Unsafe)
		flags &= ~PtrTypeFlag_Safe;

	FunctionPtrTypeTuple* tuple = getFunctionPtrTypeTuple(functionType);

	// ref x kind x checked

	size_t i1 = typeKind == TypeKind_FunctionRef;
	size_t i2 = ptrTypeKind;
	size_t i3 = (flags & PtrTypeFlag_Safe) ? 0 : 1;

	if (tuple->m_ptrTypeArray[i1][i2][i3])
		return tuple->m_ptrTypeArray[i1][i2][i3];

	size_t size = ptrTypeKind == FunctionPtrTypeKind_Thin ? sizeof(void*) : sizeof(FunctionPtr);

	FunctionPtrType* type = AXL_MEM_NEW(FunctionPtrType);
	type->m_module = m_module;
	type->m_signature = FunctionPtrType::createSignature(functionType, typeKind, ptrTypeKind, flags);
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_size = size;
	type->m_alignment = sizeof(void*);
	type->m_targetType = functionType;
	type->m_flags = flags;

	m_typeList.insertTail(type);
	tuple->m_ptrTypeArray[i1][i2][i3] = type;

	if (m_parseStdTypeLevel || !m_module->m_namespaceMgr.getCurrentScope())
	{
		m_module->markForLayout(type, true);
	}
	else
	{
		bool result = type->ensureLayout();
		if (!result)
			return NULL;
	}

	return type;
}

PropertyPtrType*
TypeMgr::getPropertyPtrType(
	PropertyType* propertyType,
	TypeKind typeKind,
	PropertyPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	ASSERT(typeKind == TypeKind_PropertyPtr || typeKind == TypeKind_PropertyRef);
	ASSERT((size_t)ptrTypeKind < PropertyPtrTypeKind__Count);

	if (typeKind == TypeKind_PropertyPtr && ptrTypeKind != PropertyPtrTypeKind_Thin)
		flags |= TypeFlag_GcRoot | TypeFlag_StructRet;

	PropertyPtrTypeTuple* tuple = getPropertyPtrTypeTuple(propertyType);

	// ref x kind x checked

	size_t i1 = typeKind == TypeKind_PropertyRef;
	size_t i2 = ptrTypeKind;
	size_t i3 = (flags & PtrTypeFlag_Safe) ? 0 : 1;

	if (tuple->m_ptrTypeArray[i1][i2][i3])
		return tuple->m_ptrTypeArray[i1][i2][i3];

	size_t size = ptrTypeKind == PropertyPtrTypeKind_Thin ? sizeof(void*) : sizeof(PropertyPtr);

	PropertyPtrType* type = AXL_MEM_NEW(PropertyPtrType);
	type->m_module = m_module;
	type->m_signature = PropertyPtrType::createSignature(propertyType, typeKind, ptrTypeKind, flags);
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_size = size;
	type->m_alignment = sizeof(void*);
	type->m_targetType = propertyType;
	type->m_flags = flags;

	m_typeList.insertTail(type);
	tuple->m_ptrTypeArray[i1][i2][i3] = type;
	return type;
}

StructType*
TypeMgr::getPropertyVtableStructType(PropertyType* propertyType)
{
	if (propertyType->m_vtableStructType)
		return propertyType->m_vtableStructType;

	StructType* type = createUnnamedStructType("PropertyVtable");

	if (propertyType->getFlags() & PropertyTypeFlag_Bindable)
		type->createField("!m_binder", propertyType->m_binderType->getFunctionPtrType (FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe));

	type->createField("!m_getter", propertyType->m_getterType->getFunctionPtrType (FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe));

	sl::String setterFieldName;

	size_t setterTypeOverloadCount = propertyType->m_setterType.getOverloadCount();
	for (size_t i = 0; i < setterTypeOverloadCount; i++)
	{
		setterFieldName.format("!m_setter%d", i);

		FunctionType* setterType = propertyType->m_setterType.getOverload(i);
		type->createField(setterFieldName, setterType->getFunctionPtrType(FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe));
	}

	type->ensureLayout();

	propertyType->m_vtableStructType = type;
	return type;
}

NamedImportType*
TypeMgr::getNamedImportType(
	const QualifiedName& name,
	Namespace* anchorNamespace,
	const QualifiedName& anchorName
	)
{
	ASSERT(anchorNamespace->getNamespaceKind() != NamespaceKind_Scope);

	sl::String signature = NamedImportType::createSignature(name, anchorNamespace, anchorName);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		NamedImportType* type = (NamedImportType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	NamedImportType* type = AXL_MEM_NEW(NamedImportType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_name = name;
	type->m_anchorNamespace = anchorNamespace;
	type->m_anchorName = anchorName;

	if (anchorName.isEmpty())
	{
		type->m_qualifiedName = anchorNamespace->createQualifiedName(name);
	}
	else
	{
		type->m_qualifiedName = anchorNamespace->createQualifiedName(anchorName);
		type->m_qualifiedName += '.';
		type->m_qualifiedName += name.getFullName();
	}

	m_typeList.insertTail(type);
	m_unresolvedNamedImportTypeArray.append(type);
	it->m_value = type;

	return type;
}

ImportPtrType*
TypeMgr::getImportPtrType(
	NamedImportType* namedImportType,
	uint_t typeModifiers,
	uint_t flags
	)
{
	sl::String signature = ImportPtrType::createSignature(
		namedImportType,
		typeModifiers,
		flags
		);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		ImportPtrType* type = (ImportPtrType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	ImportPtrType* type = AXL_MEM_NEW(ImportPtrType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_targetType = namedImportType;
	type->m_typeModifiers = typeModifiers;
	type->m_flags = flags;

	m_typeList.insertTail(type);
	m_unresolvedImportPtrTypeArray.append(type);
	it->m_value = type;

	return type;
}

ImportIntModType*
TypeMgr::getImportIntModType(
	NamedImportType* namedImportType,
	uint_t typeModifiers,
	uint_t flags
	)
{
	sl::String signature = ImportIntModType::createSignature(
		namedImportType,
		typeModifiers,
		flags
		);

	sl::StringHashTableIterator<Type*> it = m_typeMap.visit(signature);
	if (it->m_value)
	{
		ImportIntModType* type = (ImportIntModType*)it->m_value;
		ASSERT(type->m_signature == signature);
		return type;
	}

	ImportIntModType* type = AXL_MEM_NEW(ImportIntModType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_importType = namedImportType;
	type->m_typeModifiers = typeModifiers;
	type->m_flags = flags;

	m_typeList.insertTail(type);
	m_unresolvedImportIntModTypeArray.append(type);
	it->m_value = type;

	return type;
}

Type*
TypeMgr::getCheckedPtrType(Type* type)
{
	TypeKind typeKind = type->getTypeKind();
	switch (typeKind)
	{
	case TypeKind_DataPtr:
		return ((DataPtrType*)type)->getCheckedPtrType();

	case TypeKind_ClassPtr:
		return ((ClassPtrType*)type)->getCheckedPtrType();

	case TypeKind_FunctionPtr:
		return ((FunctionPtrType*)type)->getCheckedPtrType();

	case TypeKind_PropertyPtr:
		return ((PropertyPtrType*)type)->getCheckedPtrType();

	case TypeKind_ImportPtr:
		return ((ImportPtrType*)type)->getCheckedPtrType();

	default:
		ASSERT(false);
		return type;
	}
}

Type*
TypeMgr::foldDualType(
	Type* type,
	bool isAlien,
	bool isContainerConst
	)
{
	ASSERT(isDualType(type));

	DualTypeTuple* tuple = getDualTypeTuple(type);
	Type* foldedType = tuple->m_typeArray[isAlien][isContainerConst];
	if (!foldedType)
	{
		foldedType = type->calcFoldedDualType(isAlien, isContainerConst);
		tuple->m_typeArray[isAlien][isContainerConst] = foldedType;
	}

	return foldedType;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DualTypeTuple*
TypeMgr::getDualTypeTuple(Type* type)
{
	if (type->m_dualTypeTuple)
		return type->m_dualTypeTuple;

	DualTypeTuple* tuple = AXL_MEM_ZERO_NEW(DualTypeTuple);
	type->m_dualTypeTuple = tuple;
	m_dualTypeTupleList.insertTail(tuple);
	return tuple;
}

SimplePropertyTypeTuple*
TypeMgr::getSimplePropertyTypeTuple(Type* type)
{
	if (type->m_simplePropertyTypeTuple)
		return type->m_simplePropertyTypeTuple;

	SimplePropertyTypeTuple* tuple = AXL_MEM_ZERO_NEW(SimplePropertyTypeTuple);
	type->m_simplePropertyTypeTuple = tuple;
	m_simplePropertyTypeTupleList.insertTail(tuple);
	return tuple;
}

FunctionArgTuple*
TypeMgr::getFunctionArgTuple(Type* type)
{
	if (type->m_functionArgTuple)
		return type->m_functionArgTuple;

	FunctionArgTuple* tuple = AXL_MEM_ZERO_NEW(FunctionArgTuple);
	type->m_functionArgTuple = tuple;
	m_functionArgTupleList.insertTail(tuple);
	return tuple;
}

DataPtrTypeTuple*
TypeMgr::getDataPtrTypeTuple(Type* type)
{
	if (type->m_dataPtrTypeTuple)
		return type->m_dataPtrTypeTuple;

	DataPtrTypeTuple* tuple = AXL_MEM_ZERO_NEW(DataPtrTypeTuple);
	type->m_dataPtrTypeTuple = tuple;
	m_dataPtrTypeTupleList.insertTail(tuple);
	return tuple;
}

ClassPtrTypeTuple*
TypeMgr::getClassPtrTypeTuple(ClassType* classType)
{
	if (classType->m_classPtrTypeTuple)
		return classType->m_classPtrTypeTuple;

	ClassPtrTypeTuple* tuple = AXL_MEM_ZERO_NEW(ClassPtrTypeTuple);
	classType->m_classPtrTypeTuple = tuple;
	m_classPtrTypeTupleList.insertTail(tuple);
	return tuple;
}

ClassPtrTypeTuple*
TypeMgr::getEventClassPtrTypeTuple(MulticastClassType* classType)
{
	if (classType->m_eventClassPtrTypeTuple)
		return classType->m_eventClassPtrTypeTuple;

	ClassPtrTypeTuple* tuple = AXL_MEM_ZERO_NEW(ClassPtrTypeTuple);
	classType->m_eventClassPtrTypeTuple = tuple;
	m_classPtrTypeTupleList.insertTail(tuple);
	return tuple;
}

FunctionPtrTypeTuple*
TypeMgr::getFunctionPtrTypeTuple(FunctionType* functionType)
{
	if (functionType->m_functionPtrTypeTuple)
		return functionType->m_functionPtrTypeTuple;

	FunctionPtrTypeTuple* tuple = AXL_MEM_ZERO_NEW(FunctionPtrTypeTuple);
	functionType->m_functionPtrTypeTuple = tuple;
	m_functionPtrTypeTupleList.insertTail(tuple);
	return tuple;
}

PropertyPtrTypeTuple*
TypeMgr::getPropertyPtrTypeTuple(PropertyType* propertyType)
{
	if (propertyType->m_propertyPtrTypeTuple)
		return propertyType->m_propertyPtrTypeTuple;

	PropertyPtrTypeTuple* tuple = AXL_MEM_ZERO_NEW(PropertyPtrTypeTuple);
	propertyType->m_propertyPtrTypeTuple = tuple;
	m_propertyPtrTypeTupleList.insertTail(tuple);
	return tuple;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
TypeMgr::setupAllPrimitiveTypes()
{
	setupPrimitiveType(TypeKind_Void,      0, "v");
	setupPrimitiveType(TypeKind_Variant,   sizeof(Variant), "z");
	setupPrimitiveType(TypeKind_Bool,      1, "b");
	setupPrimitiveType(TypeKind_Int8,      1, "is1");
	setupPrimitiveType(TypeKind_Int8_u,    1, "iu1");
	setupPrimitiveType(TypeKind_Int16,     2, "is2");
	setupPrimitiveType(TypeKind_Int16_u,   2, "iu2");
	setupPrimitiveType(TypeKind_Int32,     4, "is4");
	setupPrimitiveType(TypeKind_Int32_u,   4, "iu4");
	setupPrimitiveType(TypeKind_Int64,     8, "is8");
	setupPrimitiveType(TypeKind_Int64_u,   8, "iu8");
	setupPrimitiveType(TypeKind_Int16_be,  2, "ibs2");
	setupPrimitiveType(TypeKind_Int16_beu, 2, "ibu2");
	setupPrimitiveType(TypeKind_Int32_be,  4, "ibs4");
	setupPrimitiveType(TypeKind_Int32_beu, 4, "ibu4");
	setupPrimitiveType(TypeKind_Int64_be,  8, "ibs8");
	setupPrimitiveType(TypeKind_Int64_beu, 8, "ibu8");
	setupPrimitiveType(TypeKind_Float,     4, "f4");
	setupPrimitiveType(TypeKind_Double,    8, "f8");

	// variant requires special treatment

	Type* type = &m_primitiveTypeArray[TypeKind_Variant];
	type->m_flags = ModuleItemFlag_LayoutReady | TypeFlag_StructRet | TypeFlag_GcRoot;
	type->m_alignment = 8;
}

void
TypeMgr::setupStdTypedefArray()
{
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
TypeMgr::setupCallConvArray()
{
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
	size_t size,
	const sl::StringRef& signature
	)
{
	ASSERT(typeKind < TypeKind__PrimitiveTypeCount);

	Type* type = &m_primitiveTypeArray[typeKind];
	type->m_module = m_module;
	type->m_typeKind = typeKind;
	type->m_flags = TypeFlag_Pod | ModuleItemFlag_LayoutReady;
	type->m_size = size;
	type->m_alignment = size;
	type->m_signature = signature;
	type->m_llvmType = NULL;
#if (LLVM_VERSION < 0x030900)
	type->m_llvmDiType = llvm::DIType_vn();
#else
	type->m_llvmDiType = NULL;
#endif
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
	)
{
	ASSERT(stdTypedef < StdTypedef__Count);
	ASSERT(typeKind < TypeKind__PrimitiveTypeCount);

	Typedef* tdef = &m_stdTypedefArray[stdTypedef];
	tdef->m_module = m_module;
	tdef->m_name = name;
	tdef->m_qualifiedName = name;
	tdef->m_type = &m_primitiveTypeArray[typeKind];
}

NamedType*
TypeMgr::parseStdType(StdType stdType)
{
	const StdItemSource* source = getStdTypeSource(stdType);
	ASSERT(source->m_source);

	return parseStdType(
		source->m_stdNamespace,
		sl::StringRef(source->m_source, source->m_length)
		);
}

NamedType*
TypeMgr::parseStdType(
	StdNamespace stdNamespace,
	const sl::StringRef& source
	)
{
	bool result;

	m_parseStdTypeLevel++;

	Lexer lexer;
	lexer.create("jnc_StdTypes.jnc", source);

	if (stdNamespace)
		m_module->m_namespaceMgr.openStdNamespace(stdNamespace);

	Unit* prevUnit = m_module->m_unitMgr.setCurrentUnit(m_module->m_unitMgr.getCoreLibUnit());
	ASSERT(prevUnit);

	Parser parser(m_module);
	parser.create(SymbolKind_named_type_specifier_save_type);

	for (;;)
	{
		const Token* token = lexer.getToken();

		result = parser.parseToken(token);
		if (!result)
		{
			TRACE("parse std type error: %s\n", err::getLastErrorDescription().sz());
			ASSERT(false);
		}

		if (token->m_token == TokenKind_Eof) // EOF token must be parsed
			break;

		lexer.nextToken();
	}

	m_module->m_unitMgr.setCurrentUnit(prevUnit);

	if (stdNamespace)
		m_module->m_namespaceMgr.closeNamespace();

	NamedType* type = parser.m_namedType;
	ASSERT(type);

	ModuleCompileState state = m_module->getCompileState();
	if (state >= ModuleCompileState_LayoutCalculated && m_parseStdTypeLevel == 1)
	{
		result = m_module->postParseStdItem();
		if (!result)
			printf("error: %s\n", err::getLastErrorDescription().sz());

		ASSERT(result);

		// if this assert fires, rewrite std type

		ASSERT(
			m_unresolvedNamedImportTypeArray.isEmpty() &&
			m_unresolvedImportPtrTypeArray.isEmpty() &&
			m_unresolvedImportIntModTypeArray.isEmpty()
			);
	}

	m_parseStdTypeLevel--;
	return type;
}

ClassType*
TypeMgr::createAbstractClassType()
{
	static sl::String typeString = "class";

	ClassType* type = createClassType(ClassTypeKind_Abstract, sl::String(), "jnc.AbstractClass");
	TypeStringTuple* tuple = type->getTypeStringTuple();
	tuple->m_typeStringPrefix = typeString;
	tuple->m_doxyLinkedTextPrefix = typeString;
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createAbstractDataType()
{
	static sl::String typeString = "anydata";

	StructType* type = createStructType(sl::String(), "jnc.AbstractData");
	TypeStringTuple* tuple = type->getTypeStringTuple();
	tuple->m_typeStringPrefix = typeString;
	tuple->m_doxyLinkedTextPrefix = typeString;
	type->ensureLayout();

	type->m_flags |= TypeFlag_GcRoot;
	type->m_flags &= ~TypeFlag_Pod;
	return type;
}

StructType*
TypeMgr::createIfaceHdrType()
{
	StructType* type = createStructType(sl::String(), "jnc.IfaceHdr");
	type->createField("!m_vtable", getStdType (StdType_BytePtr));
	type->createField("!m_box", getStdType (StdType_BoxPtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createBoxType()
{
	StructType* type = createStructType(sl::String(), "jnc.Box");
	type->createField("!m_type", getStdType (StdType_BytePtr));
	type->createField("!m_flags", getPrimitiveType (TypeKind_IntPtr_u));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createDataBoxType()
{
	StructType* type = createStructType(sl::String(), "jnc.DataBox");
	type->createField("!m_type", getStdType (StdType_BytePtr));
	type->createField("!m_flags", getPrimitiveType (TypeKind_IntPtr_u));
	type->createField("!m_validator", getStdType (StdType_DataPtrValidator));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createDetachedDataBoxType()
{
	StructType* type = createStructType(sl::String(), "jnc.DetachedDataBox");
	type->createField("!m_type", getStdType (StdType_BytePtr));
	type->createField("!m_flags", getPrimitiveType (TypeKind_IntPtr_u));
	type->createField("!m_validator", getStdType (StdType_DataPtrValidator));
	type->createField("!m_p", getStdType (StdType_BytePtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createDataPtrValidatorType()
{
	StructType* type = createStructType(sl::String(), "jnc.DataPtrValidator");
	type->createField("!m_validatorBox", getStdType (StdType_BoxPtr));
	type->createField("!m_targetBox", getStdType (StdType_BoxPtr));
	type->createField("!m_rangeBegin", getStdType (StdType_BytePtr));
	type->createField("!m_rangeEnd", getStdType (StdType_BytePtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createDataPtrStructType()
{
	StructType* type = createStructType(sl::String(), "jnc.DataPtr");
	type->createField("!m_p", getStdType (StdType_BytePtr));
	type->createField("!m_validator", getStdType (StdType_DataPtrValidatorPtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createFunctionPtrStructType()
{
	StructType* type = createStructType(sl::String(), "jnc.FunctionPtr");
	type->createField("!m_p", getStdType (StdType_BytePtr));
	type->createField("!m_closure", getStdType (StdType_AbstractClassPtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createVariantStructType()
{
	StructType* type = createStructType(sl::String(), "jnc.Variant");
	type->createField("!m_data1", getPrimitiveType (TypeKind_IntPtr));
	type->createField("!m_data2", getPrimitiveType (TypeKind_IntPtr));
#if (JNC_PTR_SIZE == 4)
	type->createField("!m_padding", getPrimitiveType (TypeKind_Int32));
#endif
	type->createField("!m_type", getStdType (StdType_BytePtr));
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createGcShadowStackFrameType()
{
	StructType* type = createStructType(sl::String(), "jnc.GcShadowStackFrame");
	type->createField("!m_prev", type->getDataPtrType_c ());
	type->createField("!m_map", getStdType (StdType_BytePtr));
	type->createField("!m_gcRootArray", getStdType (StdType_BytePtr)->getDataPtrType_c ());
	type->ensureLayout();
	return type;
}

StructType*
TypeMgr::createSjljFrameType()
{
	ArrayType* arrayType = getArrayType(getPrimitiveType(TypeKind_Char), sizeof(jmp_buf));
	StructType* type = createStructType(sl::String(), "jnc.SjljFrame");
	type->createField("!m_jmpBuf", arrayType);
	type->ensureLayout();
	type->m_alignment = 16; // override alignment
	return type;
}

//..............................................................................

} // namespace ct
} // namespace jnc
