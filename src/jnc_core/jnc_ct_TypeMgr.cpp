#include "pch.h"
#include "jnc_ct_TypeMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_DeclTypeCalc.h"
#include "jnc_rt_VariantUtils.h"
#include "jnc_ct_Parser.llk.h"

// it's very common for classes and structs to reference themselves
// in pointer fields, retvals, arguments etc
// adding self to the namespace avoids creating unnecessary import types

#define _JNC_NAMED_TYPE_ADD_SELF

namespace jnc {
namespace ct {

//.............................................................................

TypeMgr::TypeMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	setupAllPrimitiveTypes ();
	setupStdTypedefArray ();
	setupCallConvArray ();

	memset (m_stdTypeArray, 0, sizeof (m_stdTypeArray));
	memset (m_lazyStdTypeArray, 0, sizeof (m_lazyStdTypeArray));

	m_unnamedEnumTypeCounter = 0;
	m_unnamedStructTypeCounter = 0;
	m_unnamedUnionTypeCounter = 0;
	m_unnamedClassTypeCounter = 0;

	m_parseStdTypeLevel = 0;
}

void
TypeMgr::clear ()
{
	m_arrayTypeList.clear ();
	m_bitFieldTypeList.clear ();
	m_enumTypeList.clear ();
	m_structTypeList.clear ();
	m_unionTypeList.clear ();
	m_classTypeList.clear ();
	m_functionTypeList.clear ();
	m_propertyTypeList.clear ();
	m_dataPtrTypeList.clear ();
	m_classPtrTypeList.clear ();
	m_functionPtrTypeList.clear ();
	m_propertyPtrTypeList.clear ();
	m_namedImportTypeList.clear ();
	m_importPtrTypeList.clear ();
	m_importIntModTypeList.clear ();
	m_reactorClassTypeList.clear ();
	m_functionClosureClassTypeList.clear ();
	m_propertyClosureClassTypeList.clear ();
	m_dataClosureClassTypeList.clear ();
	m_multicastClassTypeList.clear ();
	m_mcSnapshotClassTypeList.clear ();

	m_simplePropertyTypeTupleList.clear ();
	m_functionArgTupleList.clear ();
	m_dataPtrTypeTupleList.clear ();
	m_classPtrTypeTupleList.clear ();
	m_functionPtrTypeTupleList.clear ();
	m_propertyPtrTypeTupleList.clear ();
	m_dualPtrTypeTupleList.clear ();

	m_typedefList.clear ();
	m_lazyStdTypeList.clear ();
	m_functionArgList.clear ();
	m_structFieldList.clear ();

	m_typeMap.clear ();

	m_unresolvedNamedImportTypeArray.clear ();
	m_unresolvedImportPtrTypeArray.clear ();
	m_unresolvedImportIntModTypeArray.clear ();

	setupAllPrimitiveTypes ();

	memset (m_stdTypeArray, 0, sizeof (m_stdTypeArray));
	memset (m_lazyStdTypeArray, 0, sizeof (m_lazyStdTypeArray));

	m_unnamedEnumTypeCounter = 0;
	m_unnamedStructTypeCounter = 0;
	m_unnamedUnionTypeCounter = 0;
	m_unnamedClassTypeCounter = 0;

	m_parseStdTypeLevel = 0;
}

Type*
TypeMgr::getStdType (StdType stdType)
{
	ASSERT ((size_t) stdType < StdType__Count);
	if (m_stdTypeArray [stdType])
		return m_stdTypeArray [stdType];

	Type* type;
	Type* argTypeArray [8];

	switch (stdType)
	{
	case StdType_BytePtr:
		type = getPrimitiveType (TypeKind_Int8_u)->getDataPtrType_c ();
		break;

	case StdType_ByteConstPtr:
		type = getPrimitiveType (TypeKind_Int8_u)->getDataPtrType_c (TypeKind_DataPtr, PtrTypeFlag_Const);
		break;

	case StdType_SimpleIfaceHdr:
		type = createSimpleIfaceHdrType ();
		break;

	case StdType_SimpleIfaceHdrPtr:
		type = getStdType (StdType_SimpleIfaceHdr)->getDataPtrType_c ();
		break;

	case StdType_Box:
		type = createBoxType ();
		break;

	case StdType_BoxPtr:
		type = getStdType (StdType_Box)->getDataPtrType_c ();
		break;

	case StdType_DataBox:
		type = createDataBoxType ();
		break;

	case StdType_DataBoxPtr:
		type = getStdType (StdType_DataBox)->getDataPtrType_c ();
		break;

	case StdType_DynamicArrayBox:
		type = createDynamicArrayBoxType ();
		break;

	case StdType_DynamicArrayBoxPtr:
		type = getStdType (StdType_DynamicArrayBox)->getDataPtrType_c ();
		break;

	case StdType_StaticDataBox:
		type = createStaticDataBoxType ();
		break;

	case StdType_StaticDataBoxPtr:
		type = getStdType (StdType_StaticDataBox)->getDataPtrType_c ();
		break;

	case StdType_DataPtrValidator:
		type = createDataPtrValidatorType ();
		break;

	case StdType_DataPtrValidatorPtr:
		type = getStdType (StdType_DataPtrValidator)->getDataPtrType_c ();
		break;

	case StdType_DataPtrStruct:
		type = createDataPtrStructType ();
		break;

	case StdType_FunctionPtrStruct:
		type = createFunctionPtrStructType ();
		break;

	case StdType_VariantStruct:
		type = createVariantStructType ();
		break;

	case StdType_GcShadowStackFrame:
		type = createGcShadowStackFrameType ();
		break;

	case StdType_SjljFrame:
		type = createSjljFrameType ();
		break;

	case StdType_AbstractClass:
		type = createAbstractClassType ();
		break;

	case StdType_AbstractClassPtr:
		type = ((ClassType*) getStdType (StdType_AbstractClass))->getClassPtrType ();
		break;

	case StdType_AbstractData:
		type = createAbstractDataType ();
		break;

	case StdType_AbstractDataPtr:
		type = getStdType (StdType_AbstractData)->getDataPtrType ();
		break;

	case StdType_SimpleFunction:
		type = getFunctionType (getPrimitiveType (TypeKind_Void), NULL, 0, 0);
		break;

	case StdType_SimpleMulticast:
		type = getMulticastType ((FunctionType*) getStdType (StdType_SimpleFunction));
		break;

	case StdType_SimpleEventPtr:
		type = ((ClassType*) getStdType (StdType_SimpleMulticast))->getClassPtrType (ClassPtrTypeKind_Normal);
		break;

	case StdType_Binder:
		type = getFunctionType (getStdType (StdType_SimpleEventPtr), NULL, 0);
		break;

	case StdType_AutomatonFunc:
		argTypeArray [0] = ((ClassType*) getStdType (StdType_Recognizer))->getClassPtrType ();
		argTypeArray [1] = getPrimitiveType (TypeKind_Int);

		type = getFunctionType (
			getStdType (StdType_AutomatonResult), 
			argTypeArray, 
			2,
			FunctionTypeFlag_Automaton
			);
		break;

	case StdType_AutomatonResult:
		type = (Type*) m_module->m_namespaceMgr.getStdNamespace (StdNamespace_Jnc)->findItemByName ("AutomatonResult");
		if (!type)
			type = parseStdType (stdType);
		break;

	case StdType_Recognizer:		
		type = (Type*) m_module->m_namespaceMgr.getStdNamespace (StdNamespace_Jnc)->findItemByName ("Recognizer");
		if (!type)
			type = parseStdType (stdType);
		break;

	case StdType_DynamicLib:
		type = (Type*) m_module->m_namespaceMgr.getStdNamespace (StdNamespace_Jnc)->findItemByName ("DynamicLib");
		if (!type)
			type = parseStdType (stdType);
		break;

	case StdType_Scheduler:
		type = (Type*) m_module->m_namespaceMgr.getStdNamespace (StdNamespace_Jnc)->findItemByName ("Scheduler");
		if (!type)
			type = parseStdType (stdType);
		break;

	case StdType_FmtLiteral:
	case StdType_ReactorBindSite:
	case StdType_Int64Int64:
	case StdType_Fp64Fp64:
	case StdType_Int64Fp64:
	case StdType_Fp64Int64:
		type = parseStdType (stdType);
		break;

	default:
		ASSERT (false);
		return NULL;
	}

	type->m_stdType = stdType;
	m_stdTypeArray [stdType] = type;
	return type;
}

LazyStdType*
TypeMgr::getLazyStdType (StdType stdType)
{
	ASSERT ((size_t) stdType < StdType__Count);

	if (m_lazyStdTypeArray [stdType])
		return m_lazyStdTypeArray [stdType];

	LazyStdType* type = AXL_MEM_NEW (LazyStdType);
	type->m_module = m_module;
	type->m_stdType = stdType;
	m_lazyStdTypeList.insertTail (type);
	m_lazyStdTypeArray [stdType] = type;
	return type;
}

void
pushImportSrcPosError (NamedImportType* importType)
{
	lex::pushSrcPosError (
		importType->getParentUnit ()->getFilePath (),
		*importType->getPos ()
		);
}

bool
TypeMgr::resolveImportTypes ()
{
	char buffer [256];
	sl::Array <NamedImportType*> superImportTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));

	// new items can be added in the process of resolving

	while (
		!m_unresolvedNamedImportTypeArray.isEmpty () ||
		!m_unresolvedImportPtrTypeArray.isEmpty () ||
		!m_unresolvedImportIntModTypeArray.isEmpty ()
		)
	{
		superImportTypeArray.clear ();

		sl::Array <NamedImportType*> unresolvedNamedImportTypeArray = m_unresolvedNamedImportTypeArray;
		sl::Array <ImportPtrType*> unresolvedImportPtrTypeArray = m_unresolvedImportPtrTypeArray;
		sl::Array <ImportIntModType*> unresolvedImportIntModTypeArray = m_unresolvedImportIntModTypeArray;

		m_unresolvedNamedImportTypeArray.clear ();
		m_unresolvedImportIntModTypeArray.clear ();
		m_unresolvedImportPtrTypeArray.clear ();

		size_t count = unresolvedNamedImportTypeArray.getCount ();
		for (size_t i = 0; i < count; i++)
		{
			NamedImportType* importType = unresolvedNamedImportTypeArray [i];
			ModuleItem* item = importType->m_anchorNamespace->findItemTraverse (importType->m_name);
			if (!item)
			{
				err::setFormatStringError ("unresolved import '%s'", importType->getTypeString ().cc ());
				pushImportSrcPosError (importType);
				return false;
			}

			ModuleItemKind itemKind = item->getItemKind ();
			switch (itemKind)
			{
			case ModuleItemKind_Type:
				importType->m_actualType = (Type*) item;
				break;

			case ModuleItemKind_Typedef:
				importType->m_actualType = ((Typedef*) item)->getType ();
				if (importType->m_actualType->getTypeKind () == TypeKind_NamedImport)
					superImportTypeArray.append (importType);
				break;

			default:
				err::setFormatStringError ("'%s' is not a type", importType->getTypeString ().cc ());
				pushImportSrcPosError (importType);
				return false;
			}
		}

		// eliminate super-imports and detect import loops

		count = superImportTypeArray.getCount ();
		for (size_t i = 0; i < count; i++)
		{
			NamedImportType* superImportType = superImportTypeArray [i];
			superImportType->m_flags |= ImportTypeFlag_ImportLoop;

			Type* type = superImportType->m_actualType;
			while (type->m_typeKind == TypeKind_NamedImport)
			{
				ImportType* importType = (ImportType*) type;
				if (importType->m_flags & ImportTypeFlag_ImportLoop)
				{
					err::setFormatStringError ("'%s': import loop detected", importType->getTypeString ().cc ());
					pushImportSrcPosError (superImportType);
					return false;
				}

				importType->m_flags |= ImportTypeFlag_ImportLoop;
				type = importType->m_actualType;
			}

			Type* externType = type;
			while (type->m_typeKind == TypeKind_NamedImport)
			{
				ImportType* importType = (ImportType*) type;
				importType->m_actualType = externType;
				importType->m_flags &= ~ImportTypeFlag_ImportLoop;
				type = importType->m_actualType;
			}
		}

		count = unresolvedImportIntModTypeArray.getCount ();
		for (size_t i = 0; i < count; i++)
		{
			ImportIntModType* importType = unresolvedImportIntModTypeArray [i];

			DeclTypeCalc typeCalc;

			Type* type = typeCalc.calcIntModType (
				importType->m_importType->m_actualType,
				importType->m_typeModifiers
				);

			if (!type)
				return false;

			importType->m_actualType = type;
		}

		count = unresolvedImportPtrTypeArray.getCount ();
		for (size_t i = 0; i < count; i++)
		{
			ImportPtrType* importType = unresolvedImportPtrTypeArray [i];

			DeclTypeCalc typeCalc;

			Type* type = typeCalc.calcPtrType (
				importType->m_targetType->m_actualType,
				importType->m_typeModifiers
				);

			if (!type)
				return false;

			if (importType->getFlags () & PtrTypeFlag_Safe)
				type = getCheckedPtrType (type);

			importType->m_actualType = type;
		}
	}

	return true;
}

void
TypeMgr::updateTypeSignature (
	Type* type,
	const sl::String& signature
	)
{
	if (type->m_signature == signature)
		return;

	if (!type->m_typeMapIt)
	{
		type->m_signature = signature;
		return;
	}

	m_typeMap.erase (type->m_typeMapIt);
	type->m_signature = signature;
	type->m_typeMapIt = m_typeMap.visit (signature);
	type->m_typeMapIt->m_value = type;
}

BitFieldType*
TypeMgr::getBitFieldType (
	Type* baseType,
	size_t bitOffset,
	size_t bitCount
	)
{
	sl::String signature = BitFieldType::createSignature (baseType, bitOffset, bitCount);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		BitFieldType* type = (BitFieldType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	BitFieldType* type = AXL_MEM_NEW (BitFieldType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_baseType = baseType;
	type->m_bitOffset = bitOffset;
	type->m_bitCount = bitCount;

	m_bitFieldTypeList.insertTail (type);
	it->m_value = type;

	if (baseType->getTypeKindFlags () & TypeKindFlag_Import)
	{
		type->m_baseType_i = (ImportType*) baseType;
		m_module->markForLayout (type, true);
	}
	else
	{
		bool result = type->ensureLayout ();
		if (!result)
			return NULL;
	}

	return type;
}

ArrayType*
TypeMgr::createAutoSizeArrayType (Type* elementType)
{
	ArrayType* type = AXL_MEM_NEW (ArrayType);
	type->m_flags |= ArrayTypeFlag_AutoSize;
	type->m_module = m_module;
	type->m_elementType = elementType;
	m_arrayTypeList.insertTail (type);

	if (elementType->getTypeKindFlags () & TypeKindFlag_Import)
		type->m_elementType_i = (ImportType*) elementType;

	if (!m_module->m_namespaceMgr.getCurrentScope ())
		m_module->markForLayout (type, true); // can't calclayout yet

	return type;
}

ArrayType*
TypeMgr::createArrayType (
	Type* elementType,
	sl::BoxList <Token>* elementCountInitializer
	)
{
	ArrayType* type = AXL_MEM_NEW (ArrayType);
	type->m_module = m_module;
	type->m_elementType = elementType;
	type->m_elementCountInitializer.takeOver (elementCountInitializer);
	type->m_parentUnit = m_module->m_unitMgr.getCurrentUnit ();
	type->m_parentNamespace = m_module->m_namespaceMgr.getCurrentNamespace ();
	m_arrayTypeList.insertTail (type);

	if (elementType->getTypeKindFlags () & TypeKindFlag_Import)
		type->m_elementType_i = (ImportType*) elementType;

	if (!m_module->m_namespaceMgr.getCurrentScope ())
	{
		m_module->markForLayout (type, true);
	}
	else
	{
		bool result = type->ensureLayout ();
		if (!result)
			return NULL;
	}

	return type;
}

ArrayType*
TypeMgr::getArrayType (
	Type* elementType,
	size_t elementCount
	)
{
	sl::String signature = ArrayType::createSignature (elementType, elementCount);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		ArrayType* type = (ArrayType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	ArrayType* type = AXL_MEM_NEW (ArrayType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_elementType = elementType;
	type->m_elementCount = elementCount;
	m_arrayTypeList.insertTail (type);

	if (elementType->getTypeKindFlags () & TypeKindFlag_Import)
	{
		type->m_elementType_i = (ImportType*) elementType;
		m_module->markForLayout (type, true);
	}
	else
	{
		bool result = type->ensureLayout ();
		if (!result)
			return NULL;
	}

	it->m_value = type;
	return type;
}

Typedef*
TypeMgr::createTypedef (
	const sl::String& name,
	const sl::String& qualifiedName,
	Type* type
	)
{
	Typedef* tdef = AXL_MEM_NEW (Typedef);
	type->m_module = m_module;
	tdef->m_name = name;
	tdef->m_qualifiedName = qualifiedName;
	tdef->m_tag = qualifiedName;
	tdef->m_type = type;
	m_typedefList.insertTail (tdef);

	return tdef;
}

EnumType*
TypeMgr::createEnumType (
	const sl::String& name,
	const sl::String& qualifiedName,
	Type* baseType,
	uint_t flags
	)
{
	const char* signaturePrefix = (flags & EnumTypeFlag_BitFlag) ? 
		(flags & EnumTypeFlag_Exposed) ? "EZ" : "EF" :
		(flags & EnumTypeFlag_Exposed) ? "EC" : "EE";

	EnumType* type = AXL_MEM_NEW (EnumType);

	if (name.isEmpty ())
	{
		m_unnamedEnumTypeCounter++;
		type->m_signature.format ("%s%d", signaturePrefix, m_unnamedEnumTypeCounter);
		type->m_tag.format (".UnnamedEnum%d", m_unnamedEnumTypeCounter);
	}
	else
	{
		type->m_signature.format ("%s%s", signaturePrefix, qualifiedName.cc ());
		type->m_name = name;
		type->m_qualifiedName = qualifiedName;
		type->m_tag = qualifiedName;
		type->m_flags |= TypeFlag_Named;

		type->addItem (type);
	}

	if (!baseType)
		baseType = getPrimitiveType (TypeKind_Int);

	type->m_module = m_module;
	type->m_baseType = baseType;
	type->m_flags |= flags;
	m_enumTypeList.insertTail (type);

	if (baseType->getTypeKindFlags () & TypeKindFlag_Import)
		type->m_baseType_i = (ImportType*) baseType;

	m_module->markForLayout (type, true);
	return type;
}

StructType*
TypeMgr::createStructType (
	const sl::String& name,
	const sl::String& qualifiedName,
	size_t fieldAlignment
	)
{
	StructType* type = AXL_MEM_NEW (StructType);

	if (name.isEmpty ())
	{
		m_unnamedStructTypeCounter++;
		type->m_signature.format ("S%d", m_unnamedStructTypeCounter);
		type->m_tag.format (".UnnamedStruct%d", m_unnamedStructTypeCounter);
	}
	else
	{
		type->m_signature.format ("S%s", qualifiedName.cc ());
		type->m_name = name;
		type->m_qualifiedName = qualifiedName;
		type->m_tag = qualifiedName;
		type->m_flags |= TypeFlag_Named;

#ifdef _JNC_NAMED_TYPE_ADD_SELF
		type->addItem (type);
#endif
	}

	type->m_module = m_module;
	type->m_fieldAlignment = fieldAlignment;
	m_structTypeList.insertTail (type);
	m_module->markForLayout (type, true);
	return type;
}

UnionType*
TypeMgr::createUnionType (
	const sl::String& name,
	const sl::String& qualifiedName,
	size_t fieldAlignment
	)
{
	UnionType* type = AXL_MEM_NEW (UnionType);

	if (name.isEmpty ())
	{
		m_unnamedUnionTypeCounter++;
		type->m_signature.format ("U%d", m_unnamedUnionTypeCounter);
		type->m_tag.format (".UnamedUnion%d", m_unnamedUnionTypeCounter);
	}
	else
	{
		type->m_signature.format ("U%s", qualifiedName.cc ());
		type->m_name = name;
		type->m_qualifiedName = qualifiedName;
		type->m_tag = qualifiedName;
		type->m_flags |= TypeFlag_Named;

#ifdef _JNC_NAMED_TYPE_ADD_SELF
		type->addItem (type);
#endif
	}

	m_module->markForLayout (type, true); // before child struct

	StructType* unionStructType = createUnnamedStructType ();
	unionStructType->m_parentNamespace = type;
	unionStructType->m_structTypeKind = StructTypeKind_UnionStruct;
	unionStructType->m_fieldAlignment = fieldAlignment;
	unionStructType->m_tag.format ("%s.Struct", type->m_tag.cc ());

	type->m_module = m_module;
	type->m_structType = unionStructType;
	m_unionTypeList.insertTail (type);
	return type;
}

ClassType*
TypeMgr::createClassType (
	ClassTypeKind classTypeKind,
	const sl::String& name,
	const sl::String& qualifiedName,
	size_t fieldAlignment,
	uint_t flags
	)
{
	ClassType* type;

	switch (classTypeKind)
	{
	case ClassTypeKind_Reactor:
		type = AXL_MEM_NEW (ReactorClassType);
		m_reactorClassTypeList.insertTail ((ReactorClassType*) type);
		break;

	case ClassTypeKind_FunctionClosure:
		type = AXL_MEM_NEW (FunctionClosureClassType);
		m_functionClosureClassTypeList.insertTail ((FunctionClosureClassType*) type);
		break;

	case ClassTypeKind_PropertyClosure:
		type = AXL_MEM_NEW (PropertyClosureClassType);
		m_propertyClosureClassTypeList.insertTail ((PropertyClosureClassType*) type);
		break;

	case ClassTypeKind_DataClosure:
		type = AXL_MEM_NEW (DataClosureClassType);
		m_dataClosureClassTypeList.insertTail ((DataClosureClassType*) type);
		break;

	case ClassTypeKind_Multicast:
		type = AXL_MEM_NEW (MulticastClassType);
		m_multicastClassTypeList.insertTail ((MulticastClassType*) type);
		break;

	case ClassTypeKind_McSnapshot:
		type = AXL_MEM_NEW (McSnapshotClassType);
		m_mcSnapshotClassTypeList.insertTail ((McSnapshotClassType*) type);
		break;

	default:
		type = AXL_MEM_NEW (ClassType);
		m_classTypeList.insertTail (type);
	}

	if (name.isEmpty ())
	{
		m_unnamedClassTypeCounter++;
		type->m_signature.format ("CC%d", m_unnamedClassTypeCounter);
		type->m_tag.format (".UnnamedClass%d", m_unnamedClassTypeCounter);
	}
	else
	{
		type->m_signature.format ("CC%s", qualifiedName.cc ());
		type->m_name = name;
		type->m_qualifiedName = qualifiedName;
		type->m_tag = qualifiedName;
		type->m_flags |= TypeFlag_Named;

#ifdef _JNC_NAMED_TYPE_ADD_SELF
		type->addItem (type);
#endif
	}

	m_module->markForLayout (type, true); // before child structs

	StructType* vtableStructType = createUnnamedStructType ();
	vtableStructType->m_tag.format ("%s.VTable", type->m_tag.cc ());

	StructType* ifaceHdrStructType = createUnnamedStructType (fieldAlignment);
	ifaceHdrStructType->m_tag.format ("%s.IfaceHdr", type->m_tag.cc ());
	ifaceHdrStructType->createField ("!m_vtable", vtableStructType->getDataPtrType_c ());
	ifaceHdrStructType->createField ("!m_box", getStdType (StdType_BoxPtr));

	StructType* ifaceStructType = createUnnamedStructType (fieldAlignment);
	ifaceStructType->m_structTypeKind = StructTypeKind_IfaceStruct;
	ifaceStructType->m_tag.format ("%s.Iface", type->m_tag.cc ());
	ifaceStructType->m_parentNamespace = type;
	ifaceStructType->m_storageKind = StorageKind_Member;
	ifaceStructType->m_fieldAlignment = fieldAlignment;

	StructType* classStructType = createUnnamedStructType (fieldAlignment);
	classStructType->m_structTypeKind = StructTypeKind_ClassStruct;
	classStructType->m_tag.format ("%s.Class", type->m_tag.cc ());
	classStructType->m_parentNamespace = type;
	classStructType->createField ("!m_box", getStdType (StdType_Box));
	classStructType->createField ("!m_iface", ifaceStructType);

	type->m_module = m_module;
	type->m_flags |= flags;
	type->m_classTypeKind = classTypeKind;
	type->m_vtableStructType = vtableStructType;
	type->m_ifaceHdrStructType = ifaceHdrStructType;
	type->m_ifaceStructType = ifaceStructType;
	type->m_classStructType = classStructType;
	return type;
}

FunctionArg*
TypeMgr::createFunctionArg (
	const sl::String& name,
	Type* type,
	uint_t ptrTypeFlags,
	sl::BoxList <Token>* initializer
	)
{
	FunctionArg* functionArg = AXL_MEM_NEW (FunctionArg);
	functionArg->m_module = m_module;
	functionArg->m_name = name;
	functionArg->m_qualifiedName = name;
	functionArg->m_tag = name;
	functionArg->m_type = type;
	functionArg->m_ptrTypeFlags = ptrTypeFlags;

	if (initializer)
		functionArg->m_initializer.takeOver (initializer);

	m_functionArgList.insertTail (functionArg);

	if (type->getTypeKindFlags () & TypeKindFlag_Import)
		functionArg->m_type_i = (ImportType*) type;

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

	return functionArg;
}

StructField*
TypeMgr::createStructField (
	const sl::String& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	sl::BoxList <Token>* constructor,
	sl::BoxList <Token>* initializer
	)
{
	StructField* field = AXL_MEM_NEW (StructField);
	field->m_module = m_module;
	field->m_name = name;
	field->m_type = type;
	field->m_ptrTypeFlags = ptrTypeFlags;
	field->m_bitFieldBaseType = bitCount ? type : NULL;
	field->m_bitCount = bitCount;

	if (constructor)
		field->m_constructor.takeOver (constructor);

	if (initializer)
		field->m_initializer.takeOver (initializer);

	m_structFieldList.insertTail (field);
	return field;
}

FunctionArg*
TypeMgr::getSimpleFunctionArg (
	StorageKind storageKind,
	Type* type,
	uint_t ptrTypeFlags
	)
{
	FunctionArgTuple* tuple = getFunctionArgTuple (type);

	// this x const x volatile

	size_t i1 = storageKind == StorageKind_This;
	size_t i2 = (ptrTypeFlags & PtrTypeFlag_Const) != 0;
	size_t i3 = (ptrTypeFlags & PtrTypeFlag_Volatile) != 0;

	if (tuple->m_argArray [i1] [i2] [i3])
		return tuple->m_argArray [i1] [i2] [i3];

	FunctionArg* arg = createFunctionArg (sl::String (), type, ptrTypeFlags);
	if (!arg)
		return NULL;

	arg->m_storageKind = storageKind;

	tuple->m_argArray [i1] [i2] [i3] = arg;
	return arg;
}

FunctionType*
TypeMgr::getFunctionType (
	CallConv* callConv,
	Type* returnType,
	const sl::Array <FunctionArg*>& argArray,
	uint_t flags
	)
{
	ASSERT (callConv && returnType);

	sl::String signature = FunctionType::createSignature (
		callConv,
		returnType,
		argArray,
		argArray.getCount (),
		flags
		);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		FunctionType* type = (FunctionType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	FunctionType* type = AXL_MEM_NEW (FunctionType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_callConv = callConv;
	type->m_returnType = returnType;
	type->m_flags = flags;
	type->m_argArray = argArray;

	m_functionTypeList.insertTail (type);

	if (returnType->getTypeKindFlags () & TypeKindFlag_Import)
		type->m_returnType_i = (ImportType*) returnType;

	if (!m_module->m_namespaceMgr.getCurrentScope ())
	{
		m_module->markForLayout (type, true);
	}
	else
	{
		bool result = type->ensureLayout ();
		if (!result)
			return NULL;
	}

	it->m_value = type;
	return type;
}

FunctionType*
TypeMgr::getFunctionType (
	CallConv* callConv,
	Type* returnType,
	Type* const* argTypeArray,
	size_t argCount,
	uint_t flags
	)
{
	ASSERT (callConv && returnType);

	sl::Array <FunctionArg*> argArray;
	argArray.setCount (argCount);
	for (size_t i = 0; i < argCount; i++)
	{
		FunctionArg* arg = getSimpleFunctionArg (argTypeArray [i]);
		if (!arg)
			return NULL;

		argArray [i] = arg;
	}

	sl::String signature = FunctionType::createSignature (
		callConv,
		returnType,
		argTypeArray,
		argCount,
		flags
		);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		FunctionType* type = (FunctionType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	FunctionType* type = AXL_MEM_NEW (FunctionType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_callConv = callConv;
	type->m_returnType = returnType;
	type->m_flags = flags;
	type->m_argArray = argArray;

	m_functionTypeList.insertTail (type);

	if (returnType->getTypeKindFlags () & TypeKindFlag_Import)
		type->m_returnType_i = (ImportType*) returnType;

	if (!m_module->m_namespaceMgr.getCurrentScope ())
	{
		m_module->markForLayout (type, true);
	}
	else
	{
		bool result = type->ensureLayout ();
		if (!result)
			return NULL;
	}

	it->m_value = type;
	return type;
}

FunctionType*
TypeMgr::createUserFunctionType (
	CallConv* callConv,
	Type* returnType,
	const sl::Array <FunctionArg*>& argArray,
	uint_t flags
	)
{
	ASSERT (callConv && returnType);

	sl::String signature = FunctionType::createSignature (
		callConv,
		returnType,
		argArray,
		argArray.getCount (),
		flags
		);

	FunctionType* type = AXL_MEM_NEW (FunctionType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_callConv = callConv;
	type->m_returnType = returnType;
	type->m_flags = flags | ModuleItemFlag_User;
	type->m_argArray = argArray;

	m_functionTypeList.insertTail (type);

	if (returnType->getTypeKindFlags () & TypeKindFlag_Import)
		type->m_returnType_i = (ImportType*) returnType;

	if (m_parseStdTypeLevel || !m_module->m_namespaceMgr.getCurrentScope ())
	{
		m_module->markForLayout (type, true);
	}
	else
	{
		bool result = type->ensureLayout ();
		if (!result)
			return NULL;
	}

	return type;
}

FunctionType*
TypeMgr::getMemberMethodType (
	DerivableType* parentType,
	FunctionType* functionType,
	uint_t thisArgPtrTypeFlags
	)
{
	if (!isClassType (parentType, ClassTypeKind_Abstract)) // std object members are miscellaneous closures
		thisArgPtrTypeFlags |= PtrTypeFlag_Safe;

	Type* thisArgType = parentType->getThisArgType (thisArgPtrTypeFlags);
	FunctionArg* thisArg = getSimpleFunctionArg (StorageKind_This, thisArgType);

	sl::Array <FunctionArg*> argArray = functionType->m_argArray;
	argArray.insert (0, thisArg);

	FunctionType* memberMethodType;

	if (functionType->m_flags & ModuleItemFlag_User)
	{
		memberMethodType = createUserFunctionType (
			functionType->m_callConv,
			functionType->m_returnType,
			argArray,
			functionType->m_flags
			);

		memberMethodType->m_shortType = functionType;
	}
	else
	{
		memberMethodType = getFunctionType (
			functionType->m_callConv,
			functionType->m_returnType,
			argArray,
			functionType->m_flags
			);

		memberMethodType->m_shortType = functionType;
	}

	return memberMethodType;
}

FunctionType*
TypeMgr::getStdObjectMemberMethodType (FunctionType* functionType)
{
	if (functionType->m_stdObjectMemberMethodType)
		return functionType->m_stdObjectMemberMethodType;

	ClassType* classType = (ClassType*) getStdType (StdType_AbstractClass);
	functionType->m_stdObjectMemberMethodType = classType->getMemberMethodType (functionType);
	return functionType->m_stdObjectMemberMethodType;
}

PropertyType*
TypeMgr::getPropertyType (
	FunctionType* getterType,
	const FunctionTypeOverload& setterType,
	uint_t flags
	)
{
	sl::String signature = PropertyType::createSignature (getterType, setterType, flags);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		PropertyType* type = (PropertyType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	if (setterType.isEmpty ())
		flags |= PropertyTypeFlag_Const;

	PropertyType* type = AXL_MEM_NEW (PropertyType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_getterType = getterType;
	type->m_setterType = setterType;
	type->m_flags = flags;

	if (flags & PropertyTypeFlag_Bindable)
	{
		FunctionType* binderType = (FunctionType*) getStdType (StdType_Binder);
		if (getterType->isMemberMethodType ())
			binderType = binderType->getMemberMethodType (getterType->getThisTargetType (), PtrTypeFlag_Const);

		type->m_binderType = binderType;
	}

	m_propertyTypeList.insertTail (type);

	it->m_value = type;
	return type;
}

PropertyType*
TypeMgr::getSimplePropertyType (
	CallConv* callConv,
	Type* returnType,
	uint_t flags
	)
{
	SimplePropertyTypeTuple* tuple = getSimplePropertyTypeTuple (returnType);

	uint_t callConvFlags = callConv->getFlags ();

	size_t i1 =
		(callConvFlags & CallConvFlag_Stdcall) ? 2 :
		(callConvFlags & CallConvFlag_Cdecl) ? 1 : 0;

	size_t i2 = (flags & PropertyTypeFlag_Const) != 0;
	size_t i3 = (flags & PropertyTypeFlag_Bindable) != 0;

	if (tuple->m_propertyTypeArray [i1] [i2] [i3])
		return tuple->m_propertyTypeArray [i1] [i2] [i3];

	PropertyType* propertyType;

	FunctionType* getterType = getFunctionType (callConv, returnType, NULL, 0, 0);
	if (flags & PropertyTypeFlag_Const)
	{
		propertyType = getPropertyType (getterType, NULL, flags);
	}
	else
	{
		Type* voidType = &m_primitiveTypeArray [TypeKind_Void];
		FunctionType* setterType = getFunctionType (callConv, voidType, &returnType, 1, 0);
		propertyType = getPropertyType (getterType, setterType, flags);
	}

	tuple->m_propertyTypeArray [i1] [i2] [i3] = propertyType;
	return propertyType;
}

PropertyType*
TypeMgr::getIndexedPropertyType (
	CallConv* callConv,
	Type* returnType,
	Type* const* indexArgTypeArray,
	size_t indexArgCount,
	uint_t flags
	)
{
	FunctionType* getterType = getFunctionType (callConv, returnType, indexArgTypeArray, indexArgCount, 0);

	if (flags & PropertyTypeFlag_Const)
		return getPropertyType (getterType, NULL, flags);

	char buffer [256];
	sl::Array <Type*> argTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	argTypeArray.copy (indexArgTypeArray, indexArgCount);
	argTypeArray.append (returnType);

	Type* voidType = &m_primitiveTypeArray [TypeKind_Void];
	FunctionType* setterType = getFunctionType (callConv, voidType, argTypeArray, indexArgCount + 1, 0);
	return getPropertyType (getterType, setterType, flags);
}

PropertyType*
TypeMgr::getIndexedPropertyType (
	CallConv* callConv,
	Type* returnType,
	const sl::Array <FunctionArg*>& argArray,
	uint_t flags
	)
{
	FunctionType* getterType = getFunctionType (callConv, returnType, argArray, 0);

	if (flags & PropertyTypeFlag_Const)
		return getPropertyType (getterType, NULL, flags);

	sl::Array <FunctionArg*> setterArgArray = argArray;
	setterArgArray.append (returnType->getSimpleFunctionArg ());

	Type* voidType = &m_primitiveTypeArray [TypeKind_Void];
	FunctionType* setterType = getFunctionType (callConv, voidType, setterArgArray, 0);
	return getPropertyType (getterType, setterType, flags);
}

PropertyType*
TypeMgr::createIndexedPropertyType (
	CallConv* callConv,
	Type* returnType,
	const sl::Array <FunctionArg*>& argArray,
	uint_t flags
	)
{
	FunctionType* getterType = createUserFunctionType (callConv, returnType, argArray, 0);

	if (flags & PropertyTypeFlag_Const)
		return getPropertyType (getterType, NULL, flags);

	sl::Array <FunctionArg*> setterArgArray = argArray;
	setterArgArray.append (returnType->getSimpleFunctionArg ());

	Type* voidType = &m_primitiveTypeArray [TypeKind_Void];
	FunctionType* setterType = createUserFunctionType (callConv, voidType, setterArgArray, 0);
	return getPropertyType (getterType, setterType, flags);
}

PropertyType*
TypeMgr::getMemberPropertyType (
	DerivableType* parentType,
	PropertyType* propertyType
	)
{
	FunctionType* getterType = getMemberMethodType (
		parentType,
		propertyType->m_getterType,
		PtrTypeFlag_Const
		);

	size_t setterTypeOverloadCount = propertyType->m_setterType.getOverloadCount ();

	char buffer [256];
	sl::Array <FunctionType*> setterTypeOverloadArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	setterTypeOverloadArray.setCount (setterTypeOverloadCount);

	for (size_t i = 0; i < setterTypeOverloadCount; i++)
	{
		FunctionType* overloadType = propertyType->m_setterType.getOverload (i);
		setterTypeOverloadArray [i] = getMemberMethodType (parentType, overloadType);
	}

	PropertyType* memberPropertyType = getPropertyType (
		getterType,
		FunctionTypeOverload (setterTypeOverloadArray, setterTypeOverloadCount),
		propertyType->m_flags
		);

	memberPropertyType->m_shortType = propertyType;
	return memberPropertyType;
}

PropertyType*
TypeMgr::getStdObjectMemberPropertyType (PropertyType* propertyType)
{
	if (propertyType->m_stdObjectMemberPropertyType)
		return propertyType->m_stdObjectMemberPropertyType;

	ClassType* classType = (ClassType*) getStdType (StdType_AbstractClass);
	propertyType->m_stdObjectMemberPropertyType = classType->getMemberPropertyType (propertyType);
	return propertyType->m_stdObjectMemberPropertyType;
}

PropertyType*
TypeMgr::getShortPropertyType (PropertyType* propertyType)
{
	if (propertyType->m_shortType)
		return propertyType->m_shortType;

	if (!propertyType->isMemberPropertyType ())
	{
		propertyType->m_shortType = propertyType;
		return propertyType;
	}

	FunctionType* getterType = propertyType->m_getterType->getShortType ();
	FunctionTypeOverload setterType;

	size_t setterCount = propertyType->m_setterType.getOverloadCount ();
	for (size_t i = 0; i < setterCount; i++)
	{
		FunctionType* type = propertyType->m_setterType.getOverload (i)->getShortType ();
		setterType.addOverload (type);
	}

	propertyType->m_shortType = getPropertyType (getterType, setterType, propertyType->m_flags);
	return propertyType->m_shortType;
}

ClassType*
TypeMgr::getMulticastType (FunctionPtrType* functionPtrType)
{
	bool result;

	if (functionPtrType->m_multicastType)
		return functionPtrType->m_multicastType;

	Type* returnType = functionPtrType->getTargetType ()->getReturnType ();
	if (returnType->getTypeKind () != TypeKind_Void)
	{
		err::setFormatStringError ("multicast cannot only return 'void', not '%s'", returnType->getTypeString ().cc ());
		return NULL;
	}

	MulticastClassType* type = (MulticastClassType*) createUnnamedClassType (ClassTypeKind_Multicast);
	type->m_targetType = functionPtrType;
	type->m_flags |= (functionPtrType->m_flags & TypeFlag_GcRoot);

	// fields

	type->m_fieldArray [MulticastFieldKind_Lock] = type->createField ("!m_lock", getPrimitiveType (TypeKind_IntPtr), 0, PtrTypeFlag_Volatile);
	type->m_fieldArray [MulticastFieldKind_PtrArray] = type->createField ("!m_arrayPtr", functionPtrType->getDataPtrType ());
	type->m_fieldArray [MulticastFieldKind_Count] = type->createField ("!m_count", getPrimitiveType (TypeKind_SizeT));
	type->m_fieldArray [MulticastFieldKind_MaxCount] = type->createField ("!m_maxCount", getPrimitiveType (TypeKind_SizeT));
	type->m_fieldArray [MulticastFieldKind_HandleTable] = type->createField ("!m_handleTable", getPrimitiveType (TypeKind_IntPtr));

	Type* argType;
	Function* method;
	FunctionType* methodType;

	bool isThin = functionPtrType->getPtrTypeKind () == FunctionPtrTypeKind_Thin;

	// destrutor

	methodType = getFunctionType ();
	method = type->createUnnamedMethod (StorageKind_Member, FunctionKind_Destructor, methodType);
	method->m_flags |= ModuleItemFlag_User; // no need to generate default destructor
	type->m_destructor = method;

	// methods

	methodType = getFunctionType ();
	method = type->createMethod (StorageKind_Member, "clear", methodType);
	method->m_tag = "jnc.multicastClear";
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray [MulticastMethodKind_Clear] = method;

	returnType = getPrimitiveType (TypeKind_IntPtr);
	argType = functionPtrType;
	methodType = getFunctionType (returnType, &argType, 1);

	method = type->createMethod (StorageKind_Member, "setup", methodType);
	method->m_tag = isThin ? "jnc.multicastSet_t" : "jnc.multicastSet";
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray [MulticastMethodKind_Setup] = method;

	method = type->createMethod (StorageKind_Member, "add", methodType);
	method->m_tag = isThin ? "jnc.multicastAdd_t" : "jnc.multicastAdd";
	type->m_methodArray [MulticastMethodKind_Add] = method;

	returnType = functionPtrType;
	argType = getPrimitiveType (TypeKind_IntPtr);
	methodType = getFunctionType (returnType, &argType, 1);
	method = type->createMethod (StorageKind_Member, "remove", methodType);
	method->m_tag = isThin ? "jnc.multicastRemove_t" : "jnc.multicastRemove";
	type->m_methodArray [MulticastMethodKind_Remove] = method;

	returnType = functionPtrType->getNormalPtrType ();
	methodType = getFunctionType (returnType, NULL, 0);
	method = type->createMethod (StorageKind_Member, "getSnapshot", methodType);
	method->m_tag = "jnc.multicastGetSnapshot";
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray [MulticastMethodKind_GetSnapshot] = method;

	methodType = functionPtrType->getTargetType ();
	method = type->createMethod (StorageKind_Member, "call", methodType);
	method->m_flags |= MulticastMethodFlag_InaccessibleViaEventPtr;
	type->m_methodArray [MulticastMethodKind_Call] = method;

	// overloaded operators

	type->m_binaryOperatorTable.setCount (BinOpKind__Count);
	type->m_binaryOperatorTable [BinOpKind_RefAssign] = type->m_methodArray [MulticastMethodKind_Setup];
	type->m_binaryOperatorTable [BinOpKind_AddAssign] = type->m_methodArray [MulticastMethodKind_Add];
	type->m_binaryOperatorTable [BinOpKind_SubAssign] = type->m_methodArray [MulticastMethodKind_Remove];
	type->m_callOperator = type->m_methodArray [MulticastMethodKind_Call];

	// snapshot closure (snapshot is shared between weak and normal multicasts)

	McSnapshotClassType* snapshotType = (McSnapshotClassType*) createUnnamedClassType (ClassTypeKind_McSnapshot);
	snapshotType->m_targetType = functionPtrType->getUnWeakPtrType ();
	snapshotType->m_flags |= (functionPtrType->m_flags & TypeFlag_GcRoot);

	// fields

	snapshotType->m_fieldArray [McSnapshotFieldKind_PtrArray] = snapshotType->createField ("!m_arrayPtr", functionPtrType->getDataPtrType ());
	snapshotType->m_fieldArray [McSnapshotFieldKind_Count] = snapshotType->createField ("!m_count", getPrimitiveType (TypeKind_SizeT));

	// call method

	methodType = functionPtrType->getTargetType ();
	snapshotType->m_methodArray [McSnapshotMethodKind_Call] = snapshotType->createMethod (StorageKind_Member, "call", methodType);

	type->m_snapshotType = snapshotType;

	if (!m_module->m_namespaceMgr.getCurrentScope ())
	{
		m_module->markForLayout (type);
		m_module->markForLayout (type->m_snapshotType);
	}
	else
	{
		result =
			type->ensureLayout  () &&
			type->m_snapshotType->ensureLayout ();

		if (!result)
			return NULL;
	}

	m_module->markForCompile (type);
	m_module->markForCompile (type->m_snapshotType);

	functionPtrType->m_multicastType = type;
	return type;
}

ClassType*
TypeMgr::getReactorIfaceType (FunctionType* startMethodType)
{
	Type* returnType = startMethodType->getReturnType ();
	if (returnType->getTypeKind () != TypeKind_Void)
	{
		err::setFormatStringError ("reactor must return 'void', not '%s'", returnType->getTypeString ().cc ());
		return NULL;
	}

	if (startMethodType->m_reactorIfaceType)
		return startMethodType->m_reactorIfaceType;

	ClassType* type = createUnnamedClassType (ClassTypeKind_ReactorIface);
	type->m_signature.format ("CA%s", startMethodType->getTypeString ().cc ());
	Function* starter = type->createMethod (StorageKind_Abstract, "start", startMethodType);
	type->createMethod (StorageKind_Abstract, "stop", (FunctionType*) getStdType (StdType_SimpleFunction));
	type->m_callOperator = starter;
	return type;
}

ReactorClassType*
TypeMgr::createReactorType (
	const sl::String& name,
	const sl::String& qualifiedName,
	ClassType* ifaceType,
	ClassType* parentType
	)
{
	ReactorClassType* type = (ReactorClassType*) createClassType (ClassTypeKind_Reactor, name, qualifiedName);

	type->addBaseType (ifaceType);

	// fields

	type->m_fieldArray [ReactorFieldKind_Lock]  = type->createField ("!m_lock", m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr));
	type->m_fieldArray [ReactorFieldKind_State] = type->createField ("!m_state", m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr));

	sl::Array <Function*> virtualMethodArray = ifaceType->getVirtualMethodArray ();
	ASSERT (virtualMethodArray.getCount () == 2);

	FunctionType* startMethodType = virtualMethodArray [0]->getType ()->getShortType ();
	sl::Array <FunctionArg*> argArray = startMethodType->getArgArray ();

	size_t argCount = argArray.getCount ();
	for (size_t i = 0; i < argCount; i++)
	{
		FunctionArg* arg = argArray [i];
		StructField* field = type->createField (arg->getName (), arg->getType ());
		if (!field)
			return NULL;

		if (i == 0)
			type->m_firstArgField = field;
	}

	// constructor & destructor

	if (parentType)
	{
		ClassPtrType* parentPtrType = parentType->getClassPtrType (ClassPtrTypeKind_Normal, PtrTypeFlag_Safe);

		type->m_flags |= TypeFlag_Child;
		type->m_fieldArray [ReactorFieldKind_Parent] = type->createField ("!m_parent", parentPtrType);

		Type* voidType = &m_primitiveTypeArray [TypeKind_Void];
		FunctionType* constructorType = getFunctionType (voidType, (Type**) &parentPtrType, 1);
		Function* constructor = m_module->m_functionMgr.createFunction (FunctionKind_Constructor, constructorType);
		type->addMethod (constructor);
	}
	else
	{
		type->createDefaultMethod (FunctionKind_Constructor);
	}

	type->createDefaultMethod (FunctionKind_Destructor);

	// methods

	type->m_methodArray [ReactorMethodKind_Start] = type->createMethod (StorageKind_Override, "start", startMethodType);
	type->m_methodArray [ReactorMethodKind_Stop]  = type->createMethod (StorageKind_Override, "stop", (FunctionType*) getStdType (StdType_SimpleFunction));
	type->m_callOperator = type->m_methodArray [ReactorMethodKind_Start];
	return type;
}

FunctionClosureClassType*
TypeMgr::getFunctionClosureClassType (
	FunctionType* targetType,
	FunctionType* thunkType,
	Type* const* argTypeArray,
	const size_t* closureMap,
	size_t argCount,
	size_t thisArgIdx
	)
{
	sl::String signature = ClosureClassType::createSignature (
		targetType,
		thunkType,
		argTypeArray,
		closureMap,
		argCount,
		thisArgIdx
		);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		FunctionClosureClassType* type = (FunctionClosureClassType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	FunctionClosureClassType* type = (FunctionClosureClassType*) createUnnamedClassType (ClassTypeKind_FunctionClosure);
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_closureMap.copy (closureMap, argCount);
	type->m_thisArgFieldIdx = thisArgIdx + 1;

	type->createField ("m_target", targetType->getFunctionPtrType (FunctionPtrTypeKind_Thin));

	sl::String argFieldName;

	for (size_t i = 0; i < argCount; i++)
	{
		argFieldName.format ("m_arg%d", i);
		type->createField (argFieldName, argTypeArray [i]);
	}

	Function* thunkFunction = m_module->m_functionMgr.createFunction (FunctionKind_Internal, "thunkFunction", thunkType);
	type->addMethod (thunkFunction);
	type->m_thunkFunction = thunkFunction;

	type->ensureLayout ();
	m_module->markForCompile (type);

	it->m_value = type;

	return type;
}

PropertyClosureClassType*
TypeMgr::getPropertyClosureClassType (
	PropertyType* targetType,
	PropertyType* thunkType,
	Type* const* argTypeArray,
	const size_t* closureMap,
	size_t argCount,
	size_t thisArgIdx
	)
{
	sl::String signature = ClosureClassType::createSignature (
		targetType,
		thunkType,
		argTypeArray,
		closureMap,
		argCount,
		thisArgIdx
		);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		PropertyClosureClassType* type = (PropertyClosureClassType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	PropertyClosureClassType* type = (PropertyClosureClassType*) createUnnamedClassType (ClassTypeKind_PropertyClosure);
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_closureMap.copy (closureMap, argCount);
	type->m_thisArgFieldIdx = thisArgIdx + 1;

	type->createField ("m_target", targetType->getPropertyPtrType (PropertyPtrTypeKind_Thin));

	sl::String argFieldName;

	for (size_t i = 0; i < argCount; i++)
	{
		argFieldName.format ("m_arg%d", i);
		type->createField (argFieldName, argTypeArray [i]);
	}

	Property* thunkProperty = m_module->m_functionMgr.createProperty (PropertyKind_Normal, "m_thunkProperty");
	type->addProperty (thunkProperty);
	type->m_thunkProperty = thunkProperty;

	thunkProperty->create (thunkType);

	type->ensureLayout ();
	m_module->markForCompile (type);

	it->m_value = type;

	return type;
}

DataClosureClassType*
TypeMgr::getDataClosureClassType (
	Type* targetType,
	PropertyType* thunkType
	)
{
	sl::String signature = DataClosureClassType::createSignature (targetType, thunkType);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		DataClosureClassType* type = (DataClosureClassType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	DataClosureClassType* type = (DataClosureClassType*) createUnnamedClassType (ClassTypeKind_DataClosure);
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->createField ("!m_target", targetType->getDataPtrType ());

	Property* thunkProperty = m_module->m_functionMgr.createProperty (PropertyKind_Normal, "m_thunkProperty");
	type->addProperty (thunkProperty);
	type->m_thunkProperty = thunkProperty;

	thunkProperty->create (thunkType);

	type->ensureLayout ();
	m_module->markForCompile (type);

	it->m_value = type;

	return type;
}

DataPtrType*
TypeMgr::getDataPtrType (
	Namespace* anchorNamespace,
	Type* dataType,
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	ASSERT ((size_t) ptrTypeKind < DataPtrTypeKind__Count);
	ASSERT (dataType->getTypeKind () != TypeKind_NamedImport); // for imports, GetImportPtrType () should be called
	ASSERT (typeKind != TypeKind_DataRef || dataType->m_typeKind != TypeKind_DataRef); // dbl reference

	if (typeKind == TypeKind_DataPtr && ptrTypeKind == DataPtrTypeKind_Normal)
		flags |= TypeFlag_GcRoot | TypeFlag_StructRet;

	DataPtrTypeTuple* tuple;

	if (flags & PtrTypeFlag_ReadOnly)
	{
		ASSERT (anchorNamespace != NULL);
		tuple = getReadOnlyDataPtrTypeTuple (anchorNamespace, dataType);
	}
	else
	{
		tuple = getDataPtrTypeTuple (dataType);
	}

	// ref x ptrkind x const x volatile x checked/markup

	size_t i1 = typeKind == TypeKind_DataRef;
	size_t i2 = ptrTypeKind;
	size_t i3 = (flags & PtrTypeFlag_Const) ? 0 : 1;
	size_t i4 = (flags & PtrTypeFlag_Volatile) ? 0 : 1;
	size_t i5 = (flags & PtrTypeFlag_Safe) ? 1 : 0;

	if (tuple->m_ptrTypeArray [i1] [i2] [i3] [i4] [i5])
		return tuple->m_ptrTypeArray [i1] [i2] [i3] [i4] [i5];

	size_t size = ptrTypeKind == DataPtrTypeKind_Normal ? sizeof (rt::DataPtr) : sizeof (void*);

	DataPtrType* type = AXL_MEM_NEW (DataPtrType);
	type->m_module = m_module;
	type->m_signature = DataPtrType::createSignature (dataType, typeKind, ptrTypeKind, flags);
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_size = size;
	type->m_alignment = sizeof (void*);
	type->m_targetType = dataType;
	type->m_anchorNamespace = (flags & PtrTypeFlag_ReadOnly) ? anchorNamespace : NULL;
	type->m_flags = flags;

	m_dataPtrTypeList.insertTail (type);
	tuple->m_ptrTypeArray [i1] [i2] [i3] [i4] [i5] = type;
	return type;
}

ClassPtrType*
TypeMgr::getClassPtrType (
	Namespace* anchorNamespace,
	ClassType* classType,
	TypeKind typeKind,
	ClassPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	ASSERT ((size_t) ptrTypeKind < ClassPtrTypeKind__Count);
	ASSERT (!(flags & (PtrTypeFlag_ReadOnly | PtrTypeFlag_DualEvent)) || anchorNamespace != NULL);

	if (typeKind == TypeKind_ClassPtr)
		flags |= TypeFlag_GcRoot;

	ClassPtrTypeTuple* tuple;

	if (flags & PtrTypeFlag_ReadOnly)
	{
		ASSERT (anchorNamespace != NULL);
		tuple = getReadOnlyClassPtrTypeTuple (anchorNamespace, classType);
	}
	else if (flags & PtrTypeFlag_DualEvent)
	{
		ASSERT (anchorNamespace != NULL && classType->getClassTypeKind () == ClassTypeKind_Multicast);
		tuple = getDualEventClassPtrTypeTuple (anchorNamespace, (MulticastClassType*) classType);
	}
	else if (flags & PtrTypeFlag_Event)
	{
		ASSERT (classType->getClassTypeKind () == ClassTypeKind_Multicast);
		tuple = getEventClassPtrTypeTuple ((MulticastClassType*) classType);
	}
	else
	{
		tuple = getClassPtrTypeTuple (classType);
	}

	// ref x ptrkind x const x volatile x checked

	size_t i1 = typeKind == TypeKind_ClassRef;
	size_t i2 = ptrTypeKind;
	size_t i3 = (flags & PtrTypeFlag_Const) ? 0 : 1;
	size_t i4 = (flags & PtrTypeFlag_Volatile) ? 0 : 1;
	size_t i5 = (flags & PtrTypeFlag_Safe) ? 0 : 1;

	if (tuple->m_ptrTypeArray [i1] [i2] [i3] [i4] [i5])
		return tuple->m_ptrTypeArray [i1] [i2] [i3] [i4] [i5];

	ClassPtrType* type = AXL_MEM_NEW (ClassPtrType);
	type->m_module = m_module;
	type->m_signature = ClassPtrType::createSignature (classType, typeKind, ptrTypeKind, flags);
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_targetType = classType;
	type->m_anchorNamespace = (flags & (PtrTypeFlag_ReadOnly | PtrTypeFlag_DualEvent)) ? anchorNamespace : NULL;
	type->m_flags = flags;

	m_classPtrTypeList.insertTail (type);
	tuple->m_ptrTypeArray [i1] [i2] [i3] [i4] [i5] = type;
	return type;
}

FunctionPtrType*
TypeMgr::getFunctionPtrType (
	FunctionType* functionType,
	TypeKind typeKind,
	FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	ASSERT (typeKind == TypeKind_FunctionPtr || typeKind == TypeKind_FunctionRef);
	ASSERT ((size_t) ptrTypeKind < FunctionPtrTypeKind__Count);

	if (typeKind == TypeKind_FunctionPtr && ptrTypeKind != FunctionPtrTypeKind_Thin)
		flags |= TypeFlag_GcRoot | TypeFlag_StructRet;

	if (functionType->m_flags & FunctionTypeFlag_Unsafe)
		flags &= ~PtrTypeFlag_Safe;

	FunctionPtrTypeTuple* tuple = getFunctionPtrTypeTuple (functionType);

	// ref x kind x checked

	size_t i1 = typeKind == TypeKind_FunctionRef;
	size_t i2 = ptrTypeKind;
	size_t i3 = (flags & PtrTypeFlag_Safe) ? 0 : 1;

	if (tuple->m_ptrTypeArray [i1] [i2] [i3])
		return tuple->m_ptrTypeArray [i1] [i2] [i3];

	size_t size = ptrTypeKind == FunctionPtrTypeKind_Thin ? sizeof (void*) : sizeof (rt::FunctionPtr);

	FunctionPtrType* type = AXL_MEM_NEW (FunctionPtrType);
	type->m_module = m_module;
	type->m_signature = FunctionPtrType::createSignature (functionType, typeKind, ptrTypeKind, flags);
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_size = size;
	type->m_alignment = sizeof (void*);
	type->m_targetType = functionType;
	type->m_flags = flags;

	m_functionPtrTypeList.insertTail (type);
	tuple->m_ptrTypeArray [i1] [i2] [i3] = type;

	if (!m_module->m_namespaceMgr.getCurrentScope ())
	{
		m_module->markForLayout (type, true);
	}
	else
	{
		bool result = type->ensureLayout ();
		if (!result)
			return NULL;
	}

	return type;
}

PropertyPtrType*
TypeMgr::getPropertyPtrType (
	Namespace* anchorNamespace,
	PropertyType* propertyType,
	TypeKind typeKind,
	PropertyPtrTypeKind ptrTypeKind,
	uint_t flags
	)
{
	ASSERT (typeKind == TypeKind_PropertyPtr || typeKind == TypeKind_PropertyRef);
	ASSERT ((size_t) ptrTypeKind < PropertyPtrTypeKind__Count);

	if (typeKind == TypeKind_PropertyPtr && ptrTypeKind != PropertyPtrTypeKind_Thin)
		flags |= TypeFlag_GcRoot | TypeFlag_StructRet;

	PropertyPtrTypeTuple* tuple;

	if (flags & PtrTypeFlag_ReadOnly)
	{
		ASSERT (anchorNamespace != NULL);
		tuple = getConstDPropertyPtrTypeTuple (anchorNamespace, propertyType);
	}
	else
	{
		tuple = getPropertyPtrTypeTuple (propertyType);
	}

	// ref x kind x checked

	size_t i1 = typeKind == TypeKind_PropertyRef;
	size_t i2 = ptrTypeKind;
	size_t i3 = (flags & PtrTypeFlag_Safe) ? 0 : 1;

	if (tuple->m_ptrTypeArray [i1] [i2] [i3])
		return tuple->m_ptrTypeArray [i1] [i2] [i3];

	size_t size = ptrTypeKind == PropertyPtrTypeKind_Thin ? sizeof (void*) : sizeof (rt::PropertyPtr);

	PropertyPtrType* type = AXL_MEM_NEW (PropertyPtrType);
	type->m_module = m_module;
	type->m_signature = PropertyPtrType::createSignature (propertyType, typeKind, ptrTypeKind, flags);
	type->m_typeKind = typeKind;
	type->m_ptrTypeKind = ptrTypeKind;
	type->m_size = size;
	type->m_alignment = sizeof (void*);
	type->m_targetType = propertyType;
	type->m_anchorNamespace = (flags & PtrTypeFlag_ReadOnly) ? anchorNamespace : NULL;
	type->m_flags = flags;

	m_propertyPtrTypeList.insertTail (type);
	tuple->m_ptrTypeArray [i1] [i2] [i3] = type;
	return type;
}

StructType*
TypeMgr::getPropertyVTableStructType (PropertyType* propertyType)
{
	if (propertyType->m_vtableStructType)
		return propertyType->m_vtableStructType;

	StructType* type = createUnnamedStructType ();
	type->m_tag.format ("%s.VTable", propertyType->getTypeString ().cc ());

	if (propertyType->getFlags () & PropertyTypeFlag_Bindable)
		type->createField ("!m_binder", propertyType->m_binderType->getFunctionPtrType (FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe));

	type->createField ("!m_getter", propertyType->m_getterType->getFunctionPtrType (FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe));

	sl::String setterFieldName;

	size_t setterTypeOverloadCount = propertyType->m_setterType.getOverloadCount ();
	for (size_t i = 0; i < setterTypeOverloadCount; i++)
	{
		setterFieldName.format ("!m_setter%d", i);

		FunctionType* setterType = propertyType->m_setterType.getOverload (i);
		type->createField (setterFieldName, setterType->getFunctionPtrType (FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe));
	}

	type->ensureLayout ();

	propertyType->m_vtableStructType = type;
	return type;
}

NamedImportType*
TypeMgr::getNamedImportType (
	const QualifiedName& name,
	Namespace* anchorNamespace
	)
{
	ASSERT (anchorNamespace->getNamespaceKind () != NamespaceKind_Scope);

	sl::String signature = NamedImportType::createSignature (name, anchorNamespace);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		NamedImportType* type = (NamedImportType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	NamedImportType* type = AXL_MEM_NEW (NamedImportType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_name = name;
	type->m_qualifiedName = anchorNamespace->createQualifiedName (name);
	type->m_anchorNamespace = anchorNamespace;
	type->m_module = m_module;

	m_namedImportTypeList.insertTail (type);
	m_unresolvedNamedImportTypeArray.append (type);
	it->m_value = type;

	return type;
}

ImportPtrType*
TypeMgr::getImportPtrType (
	NamedImportType* namedImportType,
	uint_t typeModifiers,
	uint_t flags
	)
{
	sl::String signature = ImportPtrType::createSignature (
		namedImportType,
		typeModifiers,
		flags
		);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		ImportPtrType* type = (ImportPtrType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	ImportPtrType* type = AXL_MEM_NEW (ImportPtrType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_targetType = namedImportType;
	type->m_typeModifiers = typeModifiers;
	type->m_flags = flags;

	m_importPtrTypeList.insertTail (type);
	m_unresolvedImportPtrTypeArray.append (type);
	it->m_value = type;

	return type;
}

ImportIntModType*
TypeMgr::getImportIntModType (
	NamedImportType* namedImportType,
	uint_t typeModifiers,
	uint_t flags
	)
{
	sl::String signature = ImportIntModType::createSignature (
		namedImportType,
		typeModifiers,
		flags
		);

	sl::StringHashTableMapIterator <Type*> it = m_typeMap.visit (signature);
	if (it->m_value)
	{
		ImportIntModType* type = (ImportIntModType*) it->m_value;
		ASSERT (type->m_signature == signature);
		return type;
	}

	ImportIntModType* type = AXL_MEM_NEW (ImportIntModType);
	type->m_module = m_module;
	type->m_signature = signature;
	type->m_typeMapIt = it;
	type->m_importType = namedImportType;
	type->m_typeModifiers = typeModifiers;
	type->m_flags = flags;

	m_importIntModTypeList.insertTail (type);
	m_unresolvedImportIntModTypeArray.append (type);
	it->m_value = type;

	return type;
}

Type*
TypeMgr::getCheckedPtrType (Type* type)
{
	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_DataPtr:
		return ((DataPtrType*) type)->getCheckedPtrType ();

	case TypeKind_ClassPtr:
		return ((ClassPtrType*) type)->getCheckedPtrType ();

	case TypeKind_FunctionPtr:
		return ((FunctionPtrType*) type)->getCheckedPtrType ();

	case TypeKind_PropertyPtr:
		return ((PropertyPtrType*) type)->getCheckedPtrType ();

	case TypeKind_ImportPtr:
		return ((ImportPtrType*) type)->getCheckedPtrType ();

	default:
		ASSERT (false);
		return type;
	}
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DualPtrTypeTuple*
TypeMgr::getDualPtrTypeTuple (
	Namespace* anchorNamespace,
	Type* type
	)
{
	sl::String signature = type->getSignature ();
	sl::StringHashTableMapIterator <DualPtrTypeTuple*> it = anchorNamespace->m_dualPtrTypeTupleMap.visit (signature);
	if (it->m_value)
		return it->m_value;

	DualPtrTypeTuple* dualPtrTypeTuple = AXL_MEM_NEW (DualPtrTypeTuple);
	m_dualPtrTypeTupleList.insertTail (dualPtrTypeTuple);
	it->m_value = dualPtrTypeTuple;
	return dualPtrTypeTuple;
}

SimplePropertyTypeTuple*
TypeMgr::getSimplePropertyTypeTuple (Type* type)
{
	if (type->m_simplePropertyTypeTuple)
		return type->m_simplePropertyTypeTuple;

	SimplePropertyTypeTuple* tuple = AXL_MEM_NEW (SimplePropertyTypeTuple);
	type->m_simplePropertyTypeTuple = tuple;
	m_simplePropertyTypeTupleList.insertTail (tuple);
	return tuple;
}

FunctionArgTuple*
TypeMgr::getFunctionArgTuple (Type* type)
{
	if (type->m_functionArgTuple)
		return type->m_functionArgTuple;

	FunctionArgTuple* tuple = AXL_MEM_NEW (FunctionArgTuple);
	type->m_functionArgTuple = tuple;
	m_functionArgTupleList.insertTail (tuple);
	return tuple;
}

DataPtrTypeTuple*
TypeMgr::getDataPtrTypeTuple (Type* type)
{
	if (type->m_dataPtrTypeTuple)
		return type->m_dataPtrTypeTuple;

	DataPtrTypeTuple* tuple = AXL_MEM_NEW (DataPtrTypeTuple);
	type->m_dataPtrTypeTuple = tuple;
	m_dataPtrTypeTupleList.insertTail (tuple);
	return tuple;
}

DataPtrTypeTuple*
TypeMgr::getReadOnlyDataPtrTypeTuple (
	Namespace* anchorNamespace,
	Type* type
	)
{
	DualPtrTypeTuple* dualPtrTypeTuple = getDualPtrTypeTuple (anchorNamespace, type);
	if (dualPtrTypeTuple->m_constDDataPtrTypeTuple)
		return dualPtrTypeTuple->m_constDDataPtrTypeTuple;

	DataPtrTypeTuple* tuple = AXL_MEM_NEW (DataPtrTypeTuple);
	dualPtrTypeTuple->m_constDDataPtrTypeTuple = tuple;
	m_dataPtrTypeTupleList.insertTail (tuple);
	return tuple;
}

ClassPtrTypeTuple*
TypeMgr::getClassPtrTypeTuple (ClassType* classType)
{
	if (classType->m_classPtrTypeTuple)
		return classType->m_classPtrTypeTuple;

	ClassPtrTypeTuple* tuple = AXL_MEM_NEW (ClassPtrTypeTuple);
	classType->m_classPtrTypeTuple = tuple;
	m_classPtrTypeTupleList.insertTail (tuple);
	return tuple;
}

ClassPtrTypeTuple*
TypeMgr::getReadOnlyClassPtrTypeTuple (
	Namespace* anchorNamespace,
	ClassType* classType
	)
{
	DualPtrTypeTuple* dualPtrTypeTuple = getDualPtrTypeTuple (anchorNamespace, classType);
	if (dualPtrTypeTuple->m_constDClassPtrTypeTuple)
		return dualPtrTypeTuple->m_constDClassPtrTypeTuple;

	ClassPtrTypeTuple* tuple = AXL_MEM_NEW (ClassPtrTypeTuple);
	dualPtrTypeTuple->m_constDClassPtrTypeTuple = tuple;
	m_classPtrTypeTupleList.insertTail (tuple);
	return tuple;
}

ClassPtrTypeTuple*
TypeMgr::getEventClassPtrTypeTuple (MulticastClassType* classType)
{
	if (classType->m_eventClassPtrTypeTuple)
		return classType->m_eventClassPtrTypeTuple;

	ClassPtrTypeTuple* tuple = AXL_MEM_NEW (ClassPtrTypeTuple);
	classType->m_eventClassPtrTypeTuple = tuple;
	m_classPtrTypeTupleList.insertTail (tuple);
	return tuple;
}

ClassPtrTypeTuple*
TypeMgr::getDualEventClassPtrTypeTuple (
	Namespace* anchorNamespace,
	MulticastClassType* classType
	)
{
	DualPtrTypeTuple* dualPtrTypeTuple = getDualPtrTypeTuple (anchorNamespace, classType);
	if (dualPtrTypeTuple->m_eventDClassPtrTypeTuple)
		return dualPtrTypeTuple->m_eventDClassPtrTypeTuple;

	ClassPtrTypeTuple* tuple = AXL_MEM_NEW (ClassPtrTypeTuple);
	dualPtrTypeTuple->m_eventDClassPtrTypeTuple = tuple;
	m_classPtrTypeTupleList.insertTail (tuple);
	return tuple;
}

FunctionPtrTypeTuple*
TypeMgr::getFunctionPtrTypeTuple (FunctionType* functionType)
{
	if (functionType->m_functionPtrTypeTuple)
		return functionType->m_functionPtrTypeTuple;

	FunctionPtrTypeTuple* tuple = AXL_MEM_NEW (FunctionPtrTypeTuple);
	functionType->m_functionPtrTypeTuple = tuple;
	m_functionPtrTypeTupleList.insertTail (tuple);
	return tuple;
}

PropertyPtrTypeTuple*
TypeMgr::getPropertyPtrTypeTuple (PropertyType* propertyType)
{
	if (propertyType->m_propertyPtrTypeTuple)
		return propertyType->m_propertyPtrTypeTuple;

	PropertyPtrTypeTuple* tuple = AXL_MEM_NEW (PropertyPtrTypeTuple);
	propertyType->m_propertyPtrTypeTuple = tuple;
	m_propertyPtrTypeTupleList.insertTail (tuple);
	return tuple;
}

PropertyPtrTypeTuple*
TypeMgr::getConstDPropertyPtrTypeTuple (
	Namespace* anchorNamespace,
	PropertyType* propertyType
	)
{
	DualPtrTypeTuple* dualPtrTypeTuple = getDualPtrTypeTuple (anchorNamespace, propertyType);
	if (dualPtrTypeTuple->m_constDPropertyPtrTypeTuple)
		return dualPtrTypeTuple->m_constDPropertyPtrTypeTuple;

	PropertyPtrTypeTuple* tuple = AXL_MEM_NEW (PropertyPtrTypeTuple);
	dualPtrTypeTuple->m_constDPropertyPtrTypeTuple = tuple;
	m_propertyPtrTypeTupleList.insertTail (tuple);
	return tuple;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
TypeMgr::setupAllPrimitiveTypes ()
{
	setupPrimitiveType (TypeKind_Void,      0, "v");
	setupPrimitiveType (TypeKind_Variant,   sizeof (rt::Variant), "z");
	setupPrimitiveType (TypeKind_Bool,      1, "b");
	setupPrimitiveType (TypeKind_Int8,      1, "is1");
	setupPrimitiveType (TypeKind_Int8_u,    1, "iu1");
	setupPrimitiveType (TypeKind_Int16,     2, "is2");
	setupPrimitiveType (TypeKind_Int16_u,   2, "iu2");
	setupPrimitiveType (TypeKind_Int32,     4, "is4");
	setupPrimitiveType (TypeKind_Int32_u,   4, "iu4");
	setupPrimitiveType (TypeKind_Int64,     8, "is8");
	setupPrimitiveType (TypeKind_Int64_u,   8, "iu8");
	setupPrimitiveType (TypeKind_Int16_be,  2, "ibs2");
	setupPrimitiveType (TypeKind_Int16_beu, 2, "ibu2");
	setupPrimitiveType (TypeKind_Int32_be,  4, "ibs4");
	setupPrimitiveType (TypeKind_Int32_beu, 4, "ibu4");
	setupPrimitiveType (TypeKind_Int64_be,  8, "ibs8");
	setupPrimitiveType (TypeKind_Int64_beu, 8, "ibu8");
	setupPrimitiveType (TypeKind_Float,     4, "f4");
	setupPrimitiveType (TypeKind_Double,    8, "f8");

	// variant requires special treatment

	Type* type = &m_primitiveTypeArray [TypeKind_Variant];
	type->m_flags = ModuleItemFlag_LayoutReady | TypeFlag_StructRet | TypeFlag_GcRoot;
	type->m_alignment = 8;
}

void
TypeMgr::setupStdTypedefArray ()
{
	setupStdTypedef (StdTypedef_uint_t,    TypeKind_Int_u,    "uint_t");
	setupStdTypedef (StdTypedef_uintptr_t, TypeKind_IntPtr_u, "uintptr_t");
	setupStdTypedef (StdTypedef_size_t,    TypeKind_SizeT,    "size_t");
	setupStdTypedef (StdTypedef_uint8_t,   TypeKind_Int8_u,   "uint8_t");
	setupStdTypedef (StdTypedef_uchar_t,   TypeKind_Int8_u,   "uchar_t");
	setupStdTypedef (StdTypedef_byte_t,    TypeKind_Int8_u,   "byte_t");
	setupStdTypedef (StdTypedef_uint16_t,  TypeKind_Int16_u,  "uint16_t");
	setupStdTypedef (StdTypedef_ushort_t,  TypeKind_Int16_u,  "ushort_t");
	setupStdTypedef (StdTypedef_word_t,    TypeKind_Int16_u,  "word_t");
	setupStdTypedef (StdTypedef_uint32_t,  TypeKind_Int32_u,  "uint32_t");
	setupStdTypedef (StdTypedef_dword_t,   TypeKind_Int32_u,  "dword_t");
	setupStdTypedef (StdTypedef_uint64_t,  TypeKind_Int64_u,  "uint64_t");
	setupStdTypedef (StdTypedef_qword_t,   TypeKind_Int64_u,  "qword_t");
}

void
TypeMgr::setupCallConvArray ()
{
	m_callConvArray [CallConvKind_Jnccall_msc32]  = &m_jnccallCallConv_msc32;
	m_callConvArray [CallConvKind_Jnccall_msc64]  = &m_jnccallCallConv_msc64;
	m_callConvArray [CallConvKind_Jnccall_gcc32]  = &m_jnccallCallConv_gcc32;
	m_callConvArray [CallConvKind_Jnccall_gcc64]  = &m_jnccallCallConv_gcc64;
	m_callConvArray [CallConvKind_Cdecl_msc32]    = &m_cdeclCallConv_msc32;
	m_callConvArray [CallConvKind_Cdecl_msc64]    = &m_cdeclCallConv_msc64;
	m_callConvArray [CallConvKind_Cdecl_gcc32]    = &m_cdeclCallConv_gcc32;
	m_callConvArray [CallConvKind_Cdecl_gcc64]    = &m_cdeclCallConv_gcc64;
	m_callConvArray [CallConvKind_Stdcall_msc32]  = &m_stdcallCallConv_msc32;
	m_callConvArray [CallConvKind_Stdcall_gcc32]  = &m_stdcallCallConv_gcc32;
	m_callConvArray [CallConvKind_Thiscall_msc32] = &m_thiscallCallConv_msc32;
}

void
TypeMgr::setupPrimitiveType (
	TypeKind typeKind,
	size_t size,
	const char* signature
	)
{
	ASSERT (typeKind < TypeKind__PrimitiveTypeCount);

	Type* type = &m_primitiveTypeArray [typeKind];
	type->m_module = m_module;
	type->m_typeKind = typeKind;
	type->m_flags = TypeFlag_Pod | ModuleItemFlag_LayoutReady;
	type->m_size = size;
	type->m_alignment = size;
	type->m_signature = signature;
	type->m_llvmType = NULL;
	type->m_llvmDiType = llvm::DIType ();
	type->m_simplePropertyTypeTuple = NULL;
	type->m_functionArgTuple = NULL;
	type->m_dataPtrTypeTuple = NULL;
	type->m_boxClassType = NULL;
}

void
TypeMgr::setupStdTypedef (
	StdTypedef stdTypedef,
	TypeKind typeKind,
	const sl::String& name
	)
{
	ASSERT (stdTypedef < StdTypedef__Count);
	ASSERT (typeKind < TypeKind__PrimitiveTypeCount);

	Typedef* tdef = &m_stdTypedefArray [stdTypedef];
	tdef->m_module = m_module;
	tdef->m_name = name;
	tdef->m_qualifiedName = name;
	tdef->m_tag = name;
	tdef->m_type = &m_primitiveTypeArray [typeKind];
}

NamedType*
TypeMgr::parseStdType (StdType stdType)
{
	const StdItemSource* source = getStdTypeSource (stdType);
	ASSERT (source->m_source);

	return parseStdType (
		source->m_stdNamespace,
		source->m_source,
		source->m_length
		);
}

NamedType*
TypeMgr::parseStdType (
	StdNamespace stdNamespace,
	const char* source,
	size_t length
	)
{
	bool result;

	m_parseStdTypeLevel++;

	Lexer lexer;
	lexer.create ("jnc_StdTypes.jnc", source, length);

	if (stdNamespace)
		m_module->m_namespaceMgr.openStdNamespace (stdNamespace);

	Parser parser (m_module);
	parser.create (SymbolKind_named_type_specifier_save_type);

	for (;;)
	{
		const Token* token = lexer.getToken ();

		result = parser.parseToken (token);
		if (!result)
		{
			dbg::trace ("parse std type error: %s\n", err::getLastErrorDescription ().cc ());
			ASSERT (false);
		}

		if (token->m_token == TokenKind_Eof) // EOF token must be parsed
			break;

		lexer.nextToken ();
	}

	if (stdNamespace)
		m_module->m_namespaceMgr.closeNamespace ();

	NamedType* type = parser.m_namedType;
	ASSERT (type);

	ModuleCompileState state = m_module->getCompileState ();
	if (state > ModuleCompileState_Resolving && m_parseStdTypeLevel == 1)
	{
		result = m_module->postParseStdItem ();
		ASSERT (result);

		// if this assert fires, rewrite std type

		ASSERT (
			m_unresolvedNamedImportTypeArray.isEmpty () &&
			m_unresolvedImportPtrTypeArray.isEmpty () &&
			m_unresolvedImportIntModTypeArray.isEmpty ()
			);
	}

	m_parseStdTypeLevel--;
	return type;
}

ClassType*
TypeMgr::createAbstractClassType ()
{
	static sl::String typeString = "class";

	ClassType* type = createClassType (ClassTypeKind_Abstract, "AbstractClass", "jnc.AbstractClass");
	type->m_typeString = typeString;
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createAbstractDataType ()
{
	static sl::String typeString = "anydata";

	StructType* type = createStructType ("AbstractData", "jnc.AbstractData");
	type->m_typeString = typeString;
	type->createField ("!m_dummy", getStdType (StdType_AbstractClassPtr));
	type->ensureLayout ();

	ASSERT (!(type->m_flags & TypeFlag_Pod));
	return type;
}

StructType*
TypeMgr::createSimpleIfaceHdrType ()
{
	StructType* type = createStructType ("SimpleIfaceHdr", "jnc.SimpleIfaceHdr");
	type->createField ("!m_vtable", getStdType (StdType_BytePtr));
	type->createField ("!m_box", getStdType (StdType_BoxPtr));
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createBoxType ()
{
	StructType* type = createStructType ("Box", "jnc.Box");
	type->createField ("!m_type", getStdType (StdType_BytePtr));
	type->createField ("!m_flags", getPrimitiveType (TypeKind_IntPtr_u));
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createDataBoxType ()
{
	StructType* type = createStructType ("DataBox", "jnc.DataBox");
	type->createField ("!m_type", getStdType (StdType_BytePtr));
	type->createField ("!m_flags", getPrimitiveType (TypeKind_IntPtr_u));
	type->createField ("!m_validator", getStdType (StdType_DataPtrValidator));
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createDynamicArrayBoxType ()
{
	StructType* type = createStructType ("DynamicArrayBox", "jnc.DynamicArrayBox");
	type->createField ("!m_type", getStdType (StdType_BytePtr));
	type->createField ("!m_flags", getPrimitiveType (TypeKind_IntPtr_u));
	type->createField ("!m_count", getPrimitiveType (TypeKind_Int64_u));
	type->createField ("!m_validator", getStdType (StdType_DataPtrValidator));
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createStaticDataBoxType ()
{
	StructType* type = createStructType ("StaticDataBox", "jnc.StaticDataBox");
	type->createField ("!m_type", getStdType (StdType_BytePtr));
	type->createField ("!m_flags", getPrimitiveType (TypeKind_IntPtr_u));
	type->createField ("!m_p", getStdType (StdType_BytePtr));
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createDataPtrValidatorType ()
{
	StructType* type = createStructType ("DataPtrValidator", "jnc.DataPtrValidator");
	type->createField ("!m_validatorBox", getStdType (StdType_BoxPtr));
	type->createField ("!m_targetBox", getStdType (StdType_BoxPtr));
	type->createField ("!m_rangeBegin", getStdType (StdType_BytePtr));
	type->createField ("!m_rangeLength", getPrimitiveType (TypeKind_SizeT));
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createDataPtrStructType ()
{
	StructType* type = createStructType ("DataPtr", "jnc.DataPtr");
	type->createField ("!m_p", getStdType (StdType_BytePtr));
	type->createField ("!m_validator", getStdType (StdType_DataPtrValidatorPtr));
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createFunctionPtrStructType ()
{
	StructType* type = createStructType ("FunctionPtr", "jnc.FunctionPtr");
	type->createField ("!m_p", getStdType (StdType_BytePtr));
	type->createField ("!m_closure", getStdType (StdType_AbstractClassPtr));
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createVariantStructType ()
{
	StructType* type = createStructType ("Variant", "jnc.Variant");
	type->createField ("!m_data1", getPrimitiveType (TypeKind_IntPtr));
	type->createField ("!m_data2", getPrimitiveType (TypeKind_IntPtr));
#if (_AXL_PTR_SIZE == 4)
	type->createField ("!m_padding", getPrimitiveType (TypeKind_Int32));
#endif
	type->createField ("!m_type", getStdType (StdType_BytePtr));
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createGcShadowStackFrameType ()
{
	StructType* type = createStructType ("GcShadowStackFrame", "jnc.GcShadowStackFrame");
	type->createField ("!m_prev", type->getDataPtrType_c ());
	type->createField ("!m_map", getStdType (StdType_BytePtr));
	type->createField ("!m_gcRootArray", getStdType (StdType_BytePtr)->getDataPtrType_c ());
	type->createField ("!m_gcRootTypeArray", getStdType (StdType_BytePtr));
	type->ensureLayout ();
	return type;
}

StructType*
TypeMgr::createSjljFrameType ()
{
	ArrayType* arrayType = getArrayType (getPrimitiveType (TypeKind_Char), sizeof (jmp_buf));
	StructType* type = createStructType ("SjljFrame", "jnc.SjljFrame");
	type->createField ("!m_jmpBuf", arrayType);
	type->ensureLayout ();
	return type;
}

//.............................................................................

} // namespace ct
} // namespace jnc
