// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Decl.h"

namespace jnc {
namespace ct {

class ArrayType;
class DataPtrType;
class ClassType;
class ClassPtrType;
class FunctionType;
class FunctionPtrType;
class PropertyType;
class PropertyPtrType;
class ReactorClassType;
class ImportType;
class ImportPtrType;

//..............................................................................

class DeclTypeCalc: protected TypeModifiers
{
protected:
	Module* m_module;
	sl::Iterator <DeclSuffix> m_suffix;

public:
	DeclTypeCalc ()
	{
		m_module = NULL;
	}

	Type*
	calcType (
		Declarator* declarator,
		Value* elementCountValue,
		uint_t* flags
		)
	{
		return calcType (
			declarator->getBaseType (),
			declarator,
			declarator->getPointerPrefixList (),
			declarator->getSuffixList (),
			elementCountValue,
			flags
			);
	}

	Type*
	calcType (
		Type* baseType,
		TypeModifiers* typeModifiers,
		const sl::ConstList <DeclPointerPrefix>& pointerPrefixList,
		const sl::ConstList <DeclSuffix>& suffixList,
		Value* elementCountValue,
		uint_t* flags
		);

	Type*
	calcPtrType (
		Type* type,
		uint_t typeModifiers
		);

	Type*
	calcIntModType (
		Type* type,
		uint_t typeModifiers
		);

	FunctionType*
	calcPropertyGetterType (Declarator* declarator);

protected:
	bool
	checkUnusedModifiers ();

	bool
	getPtrTypeFlags (
		Type* type,
		uint_t* flags
		);

	uint_t
	getPropertyFlags ();

	Type*
	getIntegerType (Type* type);

	ArrayType*
	getArrayType (Type* elementType);

	FunctionType*
	getFunctionType (Type* returnType);

	PropertyType*
	getPropertyType (Type* returnType);

	PropertyType*
	getBindableDataType (Type* dataType);

	ClassType*
	getReactorType (Type* returnType);

	ClassType*
	getMulticastType (Type* leftType);

	DataPtrType*
	getDataPtrType (Type* dataType);

	ClassPtrType*
	getClassPtrType (ClassType* classType);

	FunctionPtrType*
	getFunctionPtrType (FunctionType* functionType);

	PropertyPtrType*
	getPropertyPtrType (PropertyType* propertyType);

	ImportPtrType*
	getImportPtrType (NamedImportType* importType);

	ImportIntModType*
	getImportIntModType (NamedImportType* importType);

	Type*
	prepareReturnType (Type* type);
};

//..............................................................................

} // namespace ct
} // namespace jnc
