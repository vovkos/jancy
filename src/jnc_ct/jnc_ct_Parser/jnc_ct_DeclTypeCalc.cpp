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
#include "jnc_ct_DeclTypeCalc.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"

namespace jnc {
namespace ct {

//..............................................................................

Type*
DeclTypeCalc::calcType(
	Type* baseType,
	uint_t typeModifiers,
	const sl::List<DeclPointerPrefix>& pointerPrefixList,
	const sl::List<DeclSuffix>& suffixList,
	Value* elementCountValue,
	uint_t* flags
) {
	bool result;
	Type* type = baseType;
	m_module = type->getModule();

	sl::ConstIterator<DeclSuffix> firstSuffix = suffixList.getHead();
	sl::ConstIterator<DeclSuffix> suffixEnd;

	// strip non-const array suffix if any

	if (elementCountValue &&
		firstSuffix &&
		firstSuffix->getSuffixKind() == DeclSuffixKind_Array
	) {
		DeclArraySuffix* arraySuffix = (DeclArraySuffix*)*firstSuffix;
		sl::List<Token>* elementCountInitializer = arraySuffix->getElementCountInitializer();

		if (!elementCountInitializer->isEmpty()) {
			result = m_module->m_operatorMgr.parseExpression(elementCountInitializer, elementCountValue);
			if (!result)
				return NULL;

			suffixEnd = arraySuffix;
		} else if (arraySuffix->getElementCount() != -1) {
			elementCountValue->setConstSizeT(arraySuffix->getElementCount(), m_module);
			suffixEnd = arraySuffix;
		}
	}

	m_suffix = suffixList.getTail();

	// pointer prefixes

	sl::ConstIterator<DeclPointerPrefix> pointerPrefix = pointerPrefixList.getHead();
	for (; pointerPrefix; pointerPrefix++) {
		TypeKind typeKind = type->getTypeKind();

		m_typeModifiers = pointerPrefix->getTypeModifiers();
		if (m_typeModifiers & TypeModifier_Array) {
			ArrayType* arrayType = getArrayType(type);
			if (!arrayType)
				return NULL;

			type = getDataPtrType(arrayType);
		} else if (m_typeModifiers & (TypeModifier_Function | TypeModifier_Async)) {
			FunctionType* functionType = getFunctionType(type);
			if (!functionType)
				return NULL;

			type = getFunctionPtrType(functionType);
		} else if (m_typeModifiers & TypeModifier_Property) {
			PropertyType* propertyType = getPropertyType(type);
			if (!propertyType)
				return NULL;

			type = getPropertyPtrType(propertyType);
		} else if (m_typeModifiers & (TypeModifier_Multicast | TypeModifier_Event)) {
			ClassType* classType = getMulticastType(type);
			if (!classType)
				return NULL;

			type = getClassPtrType(classType);
		} else if (m_typeModifiers & TypeModifier_Reactor) {
			type = m_module->m_typeMgr.getStdType(StdType_ReactorBase);
			m_typeModifiers &= ~TypeModifier_Reactor;
		} else switch (typeKind) {
		case TypeKind_Class:
			type = getClassPtrType((ClassType*)type);
			break;

		case TypeKind_Function:
			type = getFunctionPtrType((FunctionType*)type);
			break;

		case TypeKind_Property:
			type = getPropertyPtrType((PropertyType*)type);
			break;

		case TypeKind_ImportTypeName:
			type = m_module->m_typeMgr.getModType<ImportPtrType>(
				(ImportTypeName*)type,
				m_typeModifiers & TypeModifierMaskKind_ImportPtr,
				(m_typeModifiers & TypeModifierMaskKind_Dual) ? TypeFlag_Dual : 0
			);

			m_typeModifiers &= ~TypeModifierMaskKind_ImportPtr;
			break;

		case TypeKind_TemplateArg:
			type = m_module->m_typeMgr.getModType<TemplatePtrType>(
				(TemplateArgType*)type,
				m_typeModifiers & TypeModifierMaskKind_TemplatePtr,
				(m_typeModifiers & TypeModifierMaskKind_Dual) ? TypeFlag_Dual : 0
			);

			m_typeModifiers &= ~TypeModifierMaskKind_TemplatePtr;
			break;

		default:
			type = getDataPtrType(type);
		}

		if (!type || !checkUnusedModifiers())
			return NULL;
	}

	m_typeModifiers = typeModifiers;

	if (m_typeModifiers & TypeModifierMaskKind_Integer) {
		type = getIntegerType(type);
		if (!type)
			return NULL;
	} else if (type->getStdType() == StdType_AbstractData) {
		err::setError("can only use 'anydata' in pointer declaration");
		return NULL;
	}

	if (m_typeModifiers & TypeModifier_Property) {
		type = getPropertyType(type);
		if (!type)
			return NULL;

		if (flags)
			*flags = getPropertyFlags();
	} else if (m_typeModifiers & (TypeModifier_Multicast | TypeModifier_Event)) {
		type = getMulticastType(type);
		if (!type)
			return NULL;
	} else if (
		(m_typeModifiers & TypeModifier_Bindable) &&
		type->getTypeKind() != TypeKind_Void) { // bindable aliases
		type = getBindableDataType(type);
		if (!type)
			return NULL;

		if (flags)
			*flags = PropertyFlag_AutoGet | PropertyFlag_AutoSet;
	} else if (m_typeModifiers & TypeModifier_Reactor) {
		type = m_module->m_typeMgr.getStdType(StdType_ReactorBase);
		m_typeModifiers &= ~TypeModifier_Reactor;
	}

	while (m_suffix != suffixEnd) {
		DeclSuffix* suffix = (DeclSuffix*)*m_suffix;
		DeclSuffixKind suffixKind = suffix->getSuffixKind();

		switch (suffixKind) {
		case DeclSuffixKind_Array:
			type = getArrayType(type);
			if (!type)
				return NULL;

			break;

		case DeclSuffixKind_Getter:
			if (type->getTypeKind() == TypeKind_Function) {
				m_suffix--;
				ASSERT(!m_suffix);
				break;
			}

			suffix->m_suffixKind = DeclSuffixKind_Function;
			// fall through

		case DeclSuffixKind_Function:
			type = getFunctionType(type);
			if (!type)
				return NULL;

			break;

		default:
			ASSERT(false);
		}
	}

	if (flags != NULL) {
		TypeKind typeKind = type->getTypeKind();
		if (typeKind == TypeKind_Function)
			*flags = getThisArgTypeFlags();
		else if (typeKind != TypeKind_Property)
			*flags = getDataPtrTypeFlags();
	}

	if (!checkUnusedModifiers())
		return NULL;

	return type;
}

Type*
DeclTypeCalc::calcPtrType(
	Type* type,
	uint_t typeModifiers
) {
	m_module = type->getModule();
	m_typeModifiers = typeModifiers;

	TypeKind typeKind = type->getTypeKind();
	switch (typeKind) {
	case TypeKind_Class:
		type = getClassPtrType((ClassType*)type);
		break;

	case TypeKind_Function:
		type = getFunctionPtrType((FunctionType*)type);
		break;

	case TypeKind_Property:
		type = getPropertyPtrType((PropertyType*)type);
		break;

	default:
		type = getDataPtrType(type);
	}

	if (!checkUnusedModifiers())
		return NULL;

	return type;
}

Type*
DeclTypeCalc::calcIntModType(
	Type* type,
	uint_t typeModifiers
) {
	m_module = type->getModule();
	m_typeModifiers = typeModifiers;

	type = getIntegerType(type);

	if (!checkUnusedModifiers())
		return NULL;

	return type;
}

uint_t
DeclTypeCalc::getDataPtrTypeFlags() {
	uint_t flags = 0;

	if (m_typeModifiers & TypeModifier_Const)
		flags |= PtrTypeFlag_Const;
	else if (m_typeModifiers & TypeModifier_MaybeConst)
		flags |= PtrTypeFlag_MaybeConst;
	else if (m_typeModifiers & TypeModifier_ConstIf)
		flags |= PtrTypeFlag_ConstIf;
	else if (m_typeModifiers & TypeModifier_ReadOnly)
		flags |= PtrTypeFlag_ReadOnly;

	if (m_typeModifiers & TypeModifier_BigEndian)
		flags |= PtrTypeFlag_BigEndian;

	if (m_typeModifiers & TypeModifier_Volatile)
		flags |= PtrTypeFlag_Volatile;

	if (m_typeModifiers & TypeModifier_Event) // convert 'event' to 'dualevent'
		flags |= PtrTypeFlag_DualEvent;

	if (m_typeModifiers & TypeModifier_Bindable)
		flags |= PtrTypeFlag_Bindable;

	if (m_typeModifiers & TypeModifier_AutoGet)
		flags |= PtrTypeFlag_AutoGet;

	m_typeModifiers &= ~(
		TypeModifier_Const |
		TypeModifier_MaybeConst |
		TypeModifier_ConstIf |
		TypeModifier_ReadOnly |
		TypeModifier_BigEndian |
		TypeModifier_Volatile |
		TypeModifier_Event |
		TypeModifier_Bindable |
		TypeModifier_AutoGet
	);

	return flags;
}

uint_t
DeclTypeCalc::getThisArgTypeFlags() {
	uint_t flags = 0;

	if (m_typeModifiers & TypeModifier_Const)
		flags |= PtrTypeFlag_Const;

	if (m_typeModifiers & TypeModifier_MaybeConst)
		flags |= PtrTypeFlag_MaybeConst;

	if (m_typeModifiers & TypeModifier_Thin)
		flags |= PtrTypeFlag_ThinThis;

	m_typeModifiers &= ~(TypeModifier_Const | TypeModifier_MaybeConst | TypeModifier_Thin);
	return flags;
}

inline
uint_t
DeclTypeCalc::getPropertyFlags() {
	if (!(m_typeModifiers & TypeModifier_AutoGet))
		return 0;

	m_typeModifiers &= ~TypeModifier_AutoGet;
	return PropertyFlag_AutoGet;
}

Type*
DeclTypeCalc::getIntegerType(Type* type) {
	ASSERT(m_typeModifiers & TypeModifierMaskKind_Integer);

	uint_t typeModifiers = m_typeModifiers & TypeModifierMaskKind_Integer;
	m_typeModifiers &= ~TypeModifierMaskKind_Integer;

	TypeKind typeKind = type->getTypeKind();
	if (typeKind == TypeKind_TypedefShadow)
		type = ((TypedefShadowType*)type)->getActualType();

	switch (typeKind) {
	case TypeKind_ImportTypeName:
		return m_module->m_typeMgr.getModType<ImportIntModType>((ImportTypeName*)type, typeModifiers);

	case TypeKind_TemplateArg:
		return m_module->m_typeMgr.getModType<TemplateIntModType>((TemplateArgType*)type, typeModifiers);
	}

	if (!(type->getTypeKindFlags() & TypeKindFlag_Integer)) {
		err::setFormatStringError("'%s' modifier cannot be applied to '%s'",
			getTypeModifierString(typeModifiers).sz(),
			type->getTypeString().sz()
		);
		return NULL;
	}

	if (typeModifiers & TypeModifier_Unsigned) {
		TypeKind modTypeKind = getUnsignedIntegerTypeKind(typeKind);
		if (modTypeKind != typeKind)
			type = m_module->m_typeMgr.getPrimitiveType(modTypeKind);
	}

	return type;
}

ArrayType*
DeclTypeCalc::getArrayType(Type* elementType) {
	if (!m_suffix || m_suffix->getSuffixKind() != DeclSuffixKind_Array) {
		err::setError("missing array suffix");
		return NULL;
	}

	DeclArraySuffix* suffix = (DeclArraySuffix*)*m_suffix--;

	TypeKind typeKind = elementType->getTypeKind();
	switch (typeKind) {
	case TypeKind_Void:
	case TypeKind_Class:
	case TypeKind_Function:
	case TypeKind_Property:
		err::setFormatStringError("cannot create array of '%s'", elementType->getTypeString().sz() );
		return NULL;

	default:
		if (isAutoSizeArrayType(elementType)) {
			err::setFormatStringError("cannot create array of auto-size-array '%s'", elementType->getTypeString().sz() );
			return NULL;
		}

		if (m_typeModifiers & TypeModifierMaskKind_Integer) {
			elementType = getIntegerType(elementType);
			if (!elementType)
				return NULL;
		} else if (elementType->getStdType() == StdType_AbstractData) {
			err::setError("can only use 'anydata' in pointer declaration");
			return NULL;
		}
	}

	m_typeModifiers &= ~TypeModifier_Array;

	sl::List<Token>* elementCountInitializer = suffix->getElementCountInitializer();
	if (!elementCountInitializer->isEmpty()) {
		UserArrayType* arrayType = m_module->m_typeMgr.createArrayType<UserArrayType>(elementType, ModuleItemFlag_User);
		arrayType->captureContext(m_module);
		sl::takeOver(&arrayType->m_initializer, elementCountInitializer);
		return arrayType;
	}

	size_t elementCount = suffix->getElementCount();
	return elementCount != -1 ?
		m_module->m_typeMgr.getArrayType(elementType, elementCount) :
		m_module->m_typeMgr.createArrayType<ArrayType>(elementType, ArrayTypeFlag_AutoSize);
}

Type*
DeclTypeCalc::prepareReturnType(Type* type) {
	while (m_suffix && m_suffix->getSuffixKind() == DeclSuffixKind_Array) {
		type = getArrayType(type);
		if (!type)
			return NULL;
	}

	TypeKind typeKind = type->getTypeKind();
	switch (typeKind) {
	case TypeKind_Class:
	case TypeKind_Function:
	case TypeKind_Property:
		err::setFormatStringError(
			"function cannot return '%s'",
			type->getTypeString().sz()
		);
		return NULL;

	default:
		if (isAutoSizeArrayType(type)) {
			err::setFormatStringError("function cannot return auto-size-array '%s'", type->getTypeString().sz() );
			return NULL;
		}

		if (m_typeModifiers & TypeModifierMaskKind_Integer)
			return getIntegerType(type);
		else if (type->getStdType() == StdType_AbstractData) {
			err::setError("can only use 'anydata' in pointer declaration");
			return NULL;
		}
	}

	return type;
}

bool
DeclTypeCalc::instantiateFunctionArgArray(
	sl::Array<FunctionArg*>* dstArgArray,
	const sl::Array<FunctionArg*>& srcArgArray
) {
	size_t argCount = srcArgArray.getCount();
	dstArgArray->setCount(argCount);
	sl::Array<FunctionArg*>::Rwi rwi = dstArgArray->rwi();
	for (size_t i = 0; i < argCount; i++) {
		FunctionArg* srcArg = srcArgArray[i];
		Type* srcArgType = srcArg->getType();
		ASSERT(srcArgType->getTypeKind() == TypeKind_TemplateDecl);
		Type* dstArgType = ((TemplateDeclType*)srcArgType)->instantiate(m_templateArgArray);
		if (!dstArgType)
			return false;

		rwi[i] = m_module->m_typeMgr.cloneFunctionArgOverrideType(srcArg, dstArgType);
	}

	return true;
}

FunctionType*
DeclTypeCalc::getFunctionType(Type* returnType) {
	returnType = prepareReturnType(returnType);
	if (!returnType)
		return NULL;

	if (!m_suffix || m_suffix->getSuffixKind() != DeclSuffixKind_Function) {
		err::setError("missing function suffix");
		return NULL;
	}

	DeclFunctionSuffix* suffix = (DeclFunctionSuffix*)*m_suffix--;

	CallConvKind callConvKind = getCallConvKindFromModifiers(m_typeModifiers);
	CallConv* callConv = m_module->m_typeMgr.getCallConv(callConvKind);

	uint_t typeFlags = suffix->getFunctionTypeFlags();
	if (m_typeModifiers & TypeModifier_ErrorCode)
		typeFlags |= FunctionTypeFlag_ErrorCode;

	if (m_typeModifiers & TypeModifier_Unsafe)
		typeFlags |= FunctionTypeFlag_Unsafe;

	if (typeFlags & FunctionTypeFlag_VarArg) {
		uint_t callConvFlags = callConv->getFlags();

		if (callConvFlags & CallConvFlag_NoVarArg) {
			err::setFormatStringError("vararg cannot be used with '%s'", callConv->getCallConvDisplayString());
			return NULL;
		}

		if (!(callConvFlags & CallConvFlag_UnsafeVarArg)) {
			err::setError("only 'cdecl' vararg is currently supported");
			return NULL;
		}
	}

	if (m_typeModifiers & TypeModifier_Async)
		typeFlags |= FunctionTypeFlag_Async;

	m_typeModifiers &= ~TypeModifierMaskKind_Function;

	if (m_templateArgArray.isEmpty())
		return m_module->m_typeMgr.createUserFunctionType(
			callConv,
			returnType,
			suffix->getArgArray(),
			typeFlags
		);

	char buffer[256];
	sl::Array<FunctionArg*> argArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	return instantiateFunctionArgArray(&argArray, suffix->getArgArray()) ?
		m_module->m_typeMgr.createUserFunctionType(
			callConv,
			returnType,
			argArray,
			typeFlags
		) :
		NULL;
}

PropertyType*
DeclTypeCalc::getPropertyType(Type* returnType) {
	returnType = prepareReturnType(returnType);
	if (!returnType)
		return NULL;

	if (returnType->getTypeKind() == TypeKind_Void) {
		err::setError("property cannot return 'void'");
		return NULL;
	}

	CallConvKind callConvKind = getCallConvKindFromModifiers(m_typeModifiers);
	CallConv* callConv = m_module->m_typeMgr.getCallConv(callConvKind);

	uint_t typeFlags = 0;
	if (m_typeModifiers & TypeModifier_Const)
		typeFlags |= PropertyTypeFlag_Const;

	if (m_typeModifiers & TypeModifier_Bindable)
		typeFlags |= PropertyTypeFlag_Bindable;

	bool isIndexed = (m_typeModifiers & TypeModifier_Indexed) != 0;
	m_typeModifiers &= ~TypeModifierMaskKind_Property;

	if (!isIndexed)
		return m_module->m_typeMgr.getSimplePropertyType(callConv, returnType, typeFlags);

	// indexed property

	if (!m_suffix || m_suffix->getSuffixKind() != DeclSuffixKind_Function) {
		err::setError("missing indexed property suffix");
		return NULL;
	}

	if (!m_templateArgArray.isEmpty()) {
		err::setError("property cannot be templated");
		return NULL;
	}

	DeclFunctionSuffix* suffix = (DeclFunctionSuffix*)*m_suffix--;
	return m_module->m_typeMgr.createIndexedPropertyType(
		callConv,
		returnType,
		suffix->getArgArray(),
		typeFlags
	);

	char buffer[256];
	sl::Array<FunctionArg*> argArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	return instantiateFunctionArgArray(&argArray, suffix->getArgArray()) ?
		m_module->m_typeMgr.createIndexedPropertyType(
			callConv,
			returnType,
			suffix->getArgArray(),
			typeFlags
		) :
		NULL;
}

PropertyType*
DeclTypeCalc::getBindableDataType(Type* dataType) {
	dataType = prepareReturnType(dataType);
	if (!dataType)
		return NULL;

	if (dataType->getTypeKind() == TypeKind_Void) {
		err::setError("bindable data cannot be 'void'");
		return NULL;
	}

	if (m_typeModifiers & TypeModifier_Indexed) {
		err::setError("bindable data cannot be 'indexed'");
		return NULL;
	}

	CallConvKind callConvKind = getCallConvKindFromModifiers(m_typeModifiers);
	CallConv* callConv = m_module->m_typeMgr.getCallConv(callConvKind);

	m_typeModifiers &= ~TypeModifierMaskKind_Property;
	return m_module->m_typeMgr.getSimplePropertyType(callConv, dataType, PropertyTypeFlag_Bindable);
}

ClassType*
DeclTypeCalc::getMulticastType(Type* leftType) {
	FunctionPtrType* ptrType;

	TypeKind typeKind = leftType->getTypeKind();
	if (typeKind == TypeKind_FunctionPtr) {
		ptrType = (FunctionPtrType*)leftType;
	} else if (typeKind == TypeKind_Function) {
		ptrType = getFunctionPtrType((FunctionType*)leftType);
		if (!ptrType)
			return NULL;
	} else {
		FunctionType* functionType = getFunctionType(leftType);
		if (!functionType)
			return NULL;

		ptrType = getFunctionPtrType(functionType);
		if (!ptrType)
			return NULL;
	}

	m_typeModifiers &= ~TypeModifier_Multicast;
	return m_module->m_typeMgr.getMulticastType(ptrType);
}

DataPtrType*
DeclTypeCalc::getDataPtrType(Type* dataType) {
	if (m_typeModifiers & TypeModifierMaskKind_Integer) {
		dataType = getIntegerType(dataType);
		if (!dataType)
			return NULL;
	}

	DataPtrTypeKind ptrTypeKind = (m_typeModifiers & TypeModifier_Thin) ?
		ptrTypeKind = DataPtrTypeKind_Thin :
		DataPtrTypeKind_Normal;

	uint_t typeFlags = getPtrTypeFlagsFromModifiers(m_typeModifiers & TypeModifierMaskKind_DataPtr);
	m_typeModifiers &= ~TypeModifierMaskKind_DataPtr;

	return dataType->getDataPtrType(
		TypeKind_DataPtr,
		ptrTypeKind,
		typeFlags
	);
}

ClassPtrType*
DeclTypeCalc::getClassPtrType(ClassType* classType) {
	ClassPtrTypeKind ptrTypeKind = (m_typeModifiers & TypeModifier_Weak) ?
		ClassPtrTypeKind_Weak :
		ClassPtrTypeKind_Normal;

	uint_t typeFlags = getPtrTypeFlagsFromModifiers(m_typeModifiers & TypeModifierMaskKind_ClassPtr);
	m_typeModifiers &= ~TypeModifierMaskKind_ClassPtr;

	return classType->getClassPtrType(
		TypeKind_ClassPtr,
		ptrTypeKind,
		typeFlags
	);
}

FunctionPtrType*
DeclTypeCalc::getFunctionPtrType(FunctionType* functionType) {
	FunctionPtrTypeKind ptrTypeKind =
		(m_typeModifiers & TypeModifier_Weak) ? FunctionPtrTypeKind_Weak :
		(m_typeModifiers & TypeModifier_Thin) ? FunctionPtrTypeKind_Thin : FunctionPtrTypeKind_Normal;

	uint_t typeFlags = getPtrTypeFlagsFromModifiers(m_typeModifiers & TypeModifierMaskKind_FunctionPtr);
	m_typeModifiers &= ~TypeModifierMaskKind_FunctionPtr;

	return functionType->getFunctionPtrType(ptrTypeKind, typeFlags);
}

PropertyPtrType*
DeclTypeCalc::getPropertyPtrType(PropertyType* propertyType) {
	PropertyPtrTypeKind ptrTypeKind =
		(m_typeModifiers & TypeModifier_Weak) ? PropertyPtrTypeKind_Weak :
		(m_typeModifiers & TypeModifier_Thin) ? PropertyPtrTypeKind_Thin : PropertyPtrTypeKind_Normal;

	uint_t typeFlags = getPtrTypeFlagsFromModifiers(m_typeModifiers & TypeModifierMaskKind_PropertyPtr);
	m_typeModifiers &= ~TypeModifierMaskKind_PropertyPtr;

	return propertyType->getPropertyPtrType(ptrTypeKind, typeFlags);
}

//..............................................................................

} // namespace ct
} // namespace jnc
