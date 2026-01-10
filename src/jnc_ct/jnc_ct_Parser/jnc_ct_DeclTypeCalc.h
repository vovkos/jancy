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
class ImportTypeName;
class ImportIntModType;
class ImportPtrType;

//..............................................................................

class DeclTypeCalc: protected TypeModifiers {
protected:
	Module* m_module;
	sl::ConstIterator<DeclSuffix> m_suffix;
	sl::ArrayRef<Type*> m_templateArgArray;

public:
	DeclTypeCalc() {
		m_module = NULL;
	}

	void
	setTemplateArgArray(const sl::ArrayRef<Type*>& argArray) {
		m_templateArgArray = argArray;
	}

	Type*
	calcType(
		const Declarator& declarator,
		Value* elementCountValue,
		uint_t* flags
	) {
		return calcType(
			declarator.getBaseType(),
			declarator.getTypeModifiers(),
			declarator.getPointerPrefixList(),
			declarator.getSuffixList(),
			elementCountValue,
			flags
		);
	}

	Type*
	calcType(
		Type* baseType,
		uint_t typeModifiers,
		const sl::List<DeclPointerPrefix>& pointerPrefixList,
		const sl::List<DeclSuffix>& suffixList,
		Value* elementCountValue,
		uint_t* flags
	);

	Type*
	calcPtrType(
		Type* type,
		uint_t typeModifiers
	);

	Type*
	calcIntModType(
		Type* type,
		uint_t typeModifiers
	);

protected:
	bool
	checkUnusedModifiers();

	uint_t
	getDataPtrTypeFlags();

	uint_t
	getThisArgTypeFlags();

	uint_t
	getPropertyFlags();

	Type*
	getIntegerType(Type* type);

	ArrayType*
	getArrayType(Type* elementType);

	FunctionType*
	getFunctionType(Type* returnType);

	PropertyType*
	getPropertyType(Type* returnType);

	PropertyType*
	getBindableDataType(Type* dataType);

	ClassType*
	getMulticastType(Type* leftType);

	DataPtrType*
	getDataPtrType(Type* dataType);

	ClassPtrType*
	getClassPtrType(ClassType* classType);

	FunctionPtrType*
	getFunctionPtrType(FunctionType* functionType);

	PropertyPtrType*
	getPropertyPtrType(PropertyType* propertyType);

	Type*
	prepareReturnType(Type* type);

	bool
	instantiateFunctionArgArray(
		sl::Array<FunctionArg*>* dstArgArray,
		const sl::Array<FunctionArg*>& srcArgArray
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
DeclTypeCalc::checkUnusedModifiers() {
	if (m_typeModifiers) {
		err::setFormatStringError("unused modifier '%s'", getTypeModifierString(m_typeModifiers).sz());
		return false;
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
