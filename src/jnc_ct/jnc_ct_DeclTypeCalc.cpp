#include "pch.h"
#include "jnc_ct_DeclTypeCalc.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

Type*
DeclTypeCalc::calcType (
	Type* baseType,
	TypeModifiers* typeModifiers,
	const sl::ConstList <DeclPointerPrefix>& pointerPrefixList,
	const sl::ConstList <DeclSuffix>& suffixList,
	Value* elementCountValue,
	uint_t* flags
	)
{
	bool result;

	Type* type = baseType;
	m_module = type->getModule ();

	sl::Iterator <DeclSuffix> firstSuffix = suffixList.getHead ();
	sl::Iterator <DeclSuffix> suffixEnd;

	// strip non-const array suffix if any

	if (elementCountValue &&
		firstSuffix &&
		firstSuffix->getSuffixKind () == DeclSuffixKind_Array)
	{
		DeclArraySuffix* arraySuffix = (DeclArraySuffix*) *firstSuffix;
		sl::BoxList <Token>* elementCountInitializer = arraySuffix->getElementCountInitializer ();

		if (!elementCountInitializer->isEmpty ())
		{
			result = m_module->m_operatorMgr.parseExpression (
				NULL,
				*elementCountInitializer,
				elementCountValue
				);

			if (!result)
				return NULL;

			suffixEnd = arraySuffix;
		}
		else if (arraySuffix->getElementCount () != -1)
		{
			elementCountValue->setConstSizeT (arraySuffix->getElementCount (), m_module);
			suffixEnd = arraySuffix;
		}
	}

	m_suffix = suffixList.getTail ();

	// pointer prefixes

	sl::Iterator <DeclPointerPrefix> pointerPrefix = pointerPrefixList.getHead ();
	for (; pointerPrefix; pointerPrefix++)
	{
		TypeKind typeKind = type->getTypeKind ();

		m_typeModifiers = pointerPrefix->getTypeModifiers ();
		if (m_typeModifiers & TypeModifier_Array)
		{
			ArrayType* arrayType = getArrayType (type);
			if (!arrayType)
				return NULL;

			type = getDataPtrType (arrayType);
		}
		else if (m_typeModifiers & (TypeModifier_Function | TypeModifier_Automaton))
		{
			FunctionType* functionType = getFunctionType (type);
			if (!functionType)
				return NULL;

			type = getFunctionPtrType (functionType);
		}
		else if (m_typeModifiers & TypeModifier_Property)
		{
			PropertyType* propertyType = getPropertyType (type);
			if (!propertyType)
				return NULL;

			type = getPropertyPtrType (propertyType);
		}
		else if (m_typeModifiers & (TypeModifier_Multicast | TypeModifier_Event))
		{
			ClassType* classType = getMulticastType (type);
			if (!classType)
				return NULL;

			type = getClassPtrType (classType);
		}
		else if (m_typeModifiers & TypeModifier_Reactor)
		{
			ClassType* classType = getReactorType (type);
			if (!classType)
				return NULL;

			type = getClassPtrType (classType);
		}
		else switch (typeKind)
		{
		case TypeKind_Class:
			type = getClassPtrType ((ClassType*) type);
			break;

		case TypeKind_Function:
			type = getFunctionPtrType ((FunctionType*) type);
			break;

		case TypeKind_Property:
			type = getPropertyPtrType ((PropertyType*) type);
			break;

		case TypeKind_NamedImport:
			type = getImportPtrType ((NamedImportType*) type);
			break;

		default:
			type = getDataPtrType (type);
		}

		if (!type || !checkUnusedModifiers ())
			return NULL;
	}

	takeOver (typeModifiers);

	if (m_typeModifiers & TypeModifierMaskKind_Integer)
	{
		type = getIntegerType (type);
		if (!type)
			return NULL;
	}	
	else if (type->getStdType () == StdType_AbstractData)
	{
		err::setStringError ("can only use 'anydata' in pointer declaration");
		return NULL;
	}

	if (m_typeModifiers & TypeModifier_Property)
	{
		type = getPropertyType (type);
		if (!type)
			return NULL;

		if (flags)
			*flags = getPropertyFlags ();
	}
	else if (m_typeModifiers & (TypeModifier_Multicast | TypeModifier_Event))
	{
		type = getMulticastType (type);
		if (!type)
			return NULL;
	}
	else if (m_typeModifiers & TypeModifier_Bindable)
	{
		type = getBindableDataType (type);
		if (!type)
			return NULL;

		if (flags)
			*flags = PropertyFlag_AutoGet | PropertyFlag_AutoSet;
	}
	else if (m_typeModifiers & TypeModifier_Reactor)
	{
		type = getReactorType (type);
		if (!type)
			return NULL;
	}

	while (m_suffix != suffixEnd)
	{
		DeclSuffix* suffix = (DeclSuffix*) *m_suffix;
		DeclSuffixKind suffixKind = suffix->getSuffixKind ();

		switch (suffixKind)
		{
		case DeclSuffixKind_Array:
			type = getArrayType (type);
			if (!type)
				return NULL;

			break;

		case DeclSuffixKind_Function:
			if (m_typeModifiers & TypeModifier_Reactor)
				type = getReactorType (type);
			else
				type = getFunctionType (type);

			if (!type)
				return NULL;

			if (!checkUnusedModifiers ())
				return NULL;

			break;

		default:
			ASSERT (false);
		}
	}

	if (!(type->getTypeKindFlags () & TypeKindFlag_Code) && flags != NULL)
	{
		result = getPtrTypeFlags (type, flags);
		if (!result)
			return NULL;
	}

	if (!checkUnusedModifiers ())
		return NULL;

	return type;
}

Type*
DeclTypeCalc::calcPtrType (
	Type* type,
	uint_t typeModifiers
	)
{
	m_module = type->getModule ();
	m_typeModifiers = typeModifiers;

	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_Class:
		type = getClassPtrType ((ClassType*) type);
		break;

	case TypeKind_Function:
		type = getFunctionPtrType ((FunctionType*) type);
		break;

	case TypeKind_Property:
		type = getPropertyPtrType ((PropertyType*) type);
		break;

	default:
		type = getDataPtrType (type);
	}

	if (!checkUnusedModifiers ())
		return NULL;

	return type;
}

Type*
DeclTypeCalc::calcIntModType (
	Type* type,
	uint_t typeModifiers
	)
{
	m_module = type->getModule ();
	m_typeModifiers = typeModifiers;

	type = getIntegerType (type);

	if (!checkUnusedModifiers ())
		return NULL;

	return type;
}

FunctionType*
DeclTypeCalc::calcPropertyGetterType (Declarator* declarator)
{
	uint_t typeModifiers = declarator->getTypeModifiers ();
	ASSERT (typeModifiers & TypeModifier_Property);

	DeclFunctionSuffix* functionSuffix = NULL;
	if (!(typeModifiers & TypeModifier_Indexed))
		functionSuffix = declarator->addFunctionSuffix ();

	declarator->m_typeModifiers &= ~(
		TypeModifier_Const |
		TypeModifier_Property |
		TypeModifier_Bindable |
		TypeModifier_AutoGet |
		TypeModifier_Indexed
		);

	Type* type = calcType (
		declarator->getBaseType (),
		declarator,
		declarator->getPointerPrefixList (),
		declarator->getSuffixList (),
		NULL,
		NULL
		);

	declarator->m_typeModifiers = typeModifiers;

	if (functionSuffix)
		declarator->deleteSuffix (functionSuffix);

	if (!type)
		return NULL;

	ASSERT (type->getTypeKind () == TypeKind_Function);
	return (FunctionType*) type;
}

bool
DeclTypeCalc::checkUnusedModifiers ()
{
	if (m_typeModifiers)
	{
		err::setFormatStringError ("unused modifier '%s'", getTypeModifierString (m_typeModifiers).cc ());
		return false;
	}

	return true;
}

bool
DeclTypeCalc::getPtrTypeFlags (
	Type* type,
	uint_t* flags_o
	)
{
	uint_t flags = 0;

	if (m_typeModifiers & TypeModifier_Const)
		flags |= PtrTypeFlag_Const;

	if (m_typeModifiers & TypeModifier_ReadOnly)
		flags |= PtrTypeFlag_ReadOnly;

	if (m_typeModifiers & TypeModifier_Volatile)
	{
		if (type->getTypeKindFlags () & TypeKindFlag_Code)
		{
			err::setFormatStringError ("'volatile' cannot be applied to '%s'", type->getTypeString ().cc ());
			return false;
		}

		flags |= PtrTypeFlag_Volatile;
	}

	if (m_typeModifiers & TypeModifier_Event) // convert 'event' to 'dualevent'
	{
		ASSERT (isClassType (type, ClassTypeKind_Multicast));
		flags |= PtrTypeFlag_DualEvent;
	}

	if (m_typeModifiers & TypeModifier_Bindable)
	{
		if (!isClassType (type, ClassTypeKind_Multicast))
		{
			err::setFormatStringError ("'bindable' cannot be applied to '%s'", type->getTypeString ().cc ());
			return false;
		}

		flags |= PtrTypeFlag_Bindable;
	}

	if (m_typeModifiers & TypeModifier_AutoGet)
		flags |= PtrTypeFlag_AutoGet;

	m_typeModifiers &= ~TypeModifierMaskKind_DeclPtr;
	*flags_o = flags;
	return true;
}

uint_t
DeclTypeCalc::getPropertyFlags ()
{
	uint_t flags = 0;

	if (m_typeModifiers & TypeModifier_AutoGet)
		flags |= PropertyFlag_AutoGet;

	m_typeModifiers &= ~TypeModifier_AutoGet;
	return flags;
}

Type*
DeclTypeCalc::getIntegerType (Type* type)
{
	ASSERT (m_typeModifiers & TypeModifierMaskKind_Integer);

	if (type->getTypeKind () == TypeKind_TypedefShadow)
		type = ((TypedefShadowType*) type)->getTypedef ()->getType ();

	if (type->getTypeKind () == TypeKind_NamedImport)
		return getImportIntModType ((NamedImportType*) type);

	if (!(type->getTypeKindFlags () & TypeKindFlag_Integer))
	{
		err::setFormatStringError ("'%s' modifier cannot be applied to '%s'",
			getTypeModifierString (m_typeModifiers & TypeModifierMaskKind_Integer).cc (),
			type->getTypeString ().cc ()
			);
		return NULL;
	}

	if (m_typeModifiers & TypeModifier_Unsigned)
	{
		TypeKind modTypeKind = getUnsignedIntegerTypeKind (type->getTypeKind ());
		type = m_module->m_typeMgr.getPrimitiveType (modTypeKind);
	}

	if (m_typeModifiers & TypeModifier_BigEndian)
	{
		TypeKind modTypeKind = getBigEndianIntegerTypeKind (type->getTypeKind ());
		type = m_module->m_typeMgr.getPrimitiveType (modTypeKind);
	}

	m_typeModifiers &= ~TypeModifierMaskKind_Integer;
	return type;
}

ArrayType*
DeclTypeCalc::getArrayType (Type* elementType)
{
	if (!m_suffix || m_suffix->getSuffixKind () != DeclSuffixKind_Array)
	{
		err::setFormatStringError ("missing array suffix");
		return NULL;
	}

	DeclArraySuffix* suffix = (DeclArraySuffix*) *m_suffix--;

	TypeKind typeKind = elementType->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_Void:
	case TypeKind_Class:
	case TypeKind_Function:
	case TypeKind_Property:
		err::setFormatStringError ("cannot create array of '%s'", elementType->getTypeString ().cc () );
		return NULL;

	default:
		if (isAutoSizeArrayType (elementType))
		{
			err::setFormatStringError ("cannot create array of auto-size-array '%s'", elementType->getTypeString ().cc () );
			return NULL;
		}

		if (m_typeModifiers & TypeModifierMaskKind_Integer)
		{
			elementType = getIntegerType (elementType);
			if (!elementType)
				return NULL;
		}
		else if (elementType->getStdType () == StdType_AbstractData)
		{
			err::setStringError ("can only use 'anydata' in pointer declaration");
			return NULL;
		}
	}

	m_typeModifiers &= ~TypeModifier_Array;

	sl::BoxList <Token>* elementCountInitializer = suffix->getElementCountInitializer ();
	if (!elementCountInitializer->isEmpty ())
		return m_module->m_typeMgr.createArrayType (elementType, elementCountInitializer);

	size_t elementCount = suffix->getElementCount ();
	return elementCount == -1 ?
		m_module->m_typeMgr.createAutoSizeArrayType (elementType) :
		m_module->m_typeMgr.getArrayType (elementType, elementCount);
}

Type*
DeclTypeCalc::prepareReturnType (Type* type)
{
	while (m_suffix && m_suffix->getSuffixKind () == DeclSuffixKind_Array)
	{
		type = getArrayType (type);
		if (!type)
			return NULL;
	}

	TypeKind typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_Class:
	case TypeKind_Function:
	case TypeKind_Property:
		err::setFormatStringError (
			"function cannot return '%s'",
			type->getTypeString ().cc ()
			);
		return NULL;

	default:
		if (isAutoSizeArrayType (type))
		{
			err::setFormatStringError ("function cannot return auto-size-array '%s'", type->getTypeString ().cc () );
			return NULL;
		}

		if (m_typeModifiers & TypeModifierMaskKind_Integer)
		{
			return getIntegerType (type);
		}
		else if (type->getStdType () == StdType_AbstractData)
		{
			err::setStringError ("can only use 'anydata' in pointer declaration");
			return NULL;
		}
	}

	return type;
}

FunctionType*
DeclTypeCalc::getFunctionType (Type* returnType)
{
	returnType = prepareReturnType (returnType);
	if (!returnType)
		return NULL;

	if (!m_suffix || m_suffix->getSuffixKind () != DeclSuffixKind_Function)
	{
		err::setFormatStringError ("missing function suffix");
		return NULL;
	}

	DeclFunctionSuffix* suffix = (DeclFunctionSuffix*) *m_suffix--;

	CallConvKind callConvKind = getCallConvKindFromModifiers (m_typeModifiers);
	CallConv* callConv = m_module->m_typeMgr.getCallConv (callConvKind);

	uint_t typeFlags = suffix->getFunctionTypeFlags ();

	if (m_typeModifiers & TypeModifier_ErrorCode)
	{
		if (returnType->getTypeKind () != TypeKind_NamedImport &&
			!(returnType->getTypeKindFlags () & TypeKindFlag_ErrorCode))
		{
			err::setFormatStringError ("'%s' cannot be used as error code", returnType->getTypeString ().cc ());
			return NULL;
		}

		typeFlags |= FunctionTypeFlag_ErrorCode;
	}

	if (m_typeModifiers & TypeModifier_Unsafe)
		typeFlags |= FunctionTypeFlag_Unsafe;

	if (m_typeModifiers & TypeModifier_Automaton) // automatons takes 2 implicit arguments
	{
		typeFlags |= FunctionTypeFlag_Automaton;
		FunctionArg* stateArg = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int)->getSimpleFunctionArg ();
		suffix->m_argArray.append (stateArg);
	}

	if (typeFlags & FunctionTypeFlag_VarArg)
	{
		uint_t callConvFlags = callConv->getFlags ();

		if (callConvFlags & CallConvFlag_NoVarArg)
		{
			err::setFormatStringError ("vararg cannot be used with '%s'", callConv->getCallConvDisplayString ());
			return NULL;
		}

		if (!(callConvFlags & CallConvFlag_UnsafeVarArg))
		{
			err::setFormatStringError ("only 'cdecl' vararg is currently supported");
			return NULL;
		}
	}

	m_typeModifiers &= ~TypeModifierMaskKind_Function;

	return m_module->m_typeMgr.createUserFunctionType (
		callConv,
		returnType,
		suffix->getArgArray (),
		typeFlags
		);
}

PropertyType*
DeclTypeCalc::getPropertyType (Type* returnType)
{
	uint_t typeFlags = 0;

	returnType = prepareReturnType (returnType);
	if (!returnType)
		return NULL;

	if (returnType->getTypeKind () == TypeKind_Void)
	{
		err::setFormatStringError ("property cannot return 'void'");
		return NULL;
	}

	CallConvKind callConvKind = getCallConvKindFromModifiers (m_typeModifiers);
	CallConv* callConv = m_module->m_typeMgr.getCallConv (callConvKind);

	if (m_typeModifiers & TypeModifier_Const)
		typeFlags |= PropertyTypeFlag_Const;

	if (m_typeModifiers & TypeModifier_Bindable)
		typeFlags |= PropertyTypeFlag_Bindable;

	bool isIndexed = (m_typeModifiers & TypeModifier_Indexed) != 0;
	m_typeModifiers &= ~TypeModifierMaskKind_Property;

	if (!isIndexed)
		return m_module->m_typeMgr.getSimplePropertyType (
			callConv,
			returnType,
			typeFlags
			);

	// indexed property

	if (!m_suffix || m_suffix->getSuffixKind () != DeclSuffixKind_Function)
	{
		err::setFormatStringError ("missing indexed property suffix");
		return NULL;
	}

	DeclFunctionSuffix* suffix = (DeclFunctionSuffix*) *m_suffix--;
	return m_module->m_typeMgr.createIndexedPropertyType (
		callConv,
		returnType,
		suffix->getArgArray (),
		typeFlags
		);
}

PropertyType*
DeclTypeCalc::getBindableDataType (Type* dataType)
{
	dataType = prepareReturnType (dataType);
	if (!dataType)
		return NULL;

	if (dataType->getTypeKind () == TypeKind_Void)
	{
		err::setFormatStringError ("bindable data cannot be 'void'");
		return NULL;
	}

	if (m_typeModifiers & TypeModifier_Indexed)
	{
		err::setFormatStringError ("bindable data cannot be 'indexed'");
		return NULL;
	}

	CallConvKind callConvKind = getCallConvKindFromModifiers (m_typeModifiers);
	CallConv* callConv = m_module->m_typeMgr.getCallConv (callConvKind);

	m_typeModifiers &= ~TypeModifierMaskKind_Property;
	return m_module->m_typeMgr.getSimplePropertyType (callConv, dataType, PropertyTypeFlag_Bindable);
}

ClassType*
DeclTypeCalc::getReactorType (Type* returnType)
{
	FunctionType* startMethodType = getFunctionType (returnType);
	if (!startMethodType)
		return NULL;

	m_typeModifiers &= ~TypeModifier_Reactor;
	return m_module->m_typeMgr.getReactorIfaceType (startMethodType);
}

ClassType*
DeclTypeCalc::getMulticastType (Type* leftType)
{
	FunctionPtrType* ptrType;

	TypeKind typeKind = leftType->getTypeKind ();
	if (typeKind == TypeKind_FunctionPtr)
	{
		ptrType = (FunctionPtrType*) leftType;
	}
	else if (typeKind == TypeKind_Function)
	{
		ptrType = getFunctionPtrType ((FunctionType*) leftType);
		if (!ptrType)
			return NULL;
	}
	else
	{
		FunctionType* functionType = getFunctionType (leftType);
		if (!functionType)
			return NULL;

		ptrType = getFunctionPtrType (functionType);
		if (!ptrType)
			return NULL;
	}

	m_typeModifiers &= ~TypeModifier_Multicast;
	return m_module->m_typeMgr.getMulticastType (ptrType);
}

DataPtrType*
DeclTypeCalc::getDataPtrType (Type* dataType)
{
	if (m_typeModifiers & TypeModifierMaskKind_Integer)
	{
		dataType = getIntegerType (dataType);
		if (!dataType)
			return NULL;
	}

	DataPtrTypeKind ptrTypeKind = DataPtrTypeKind_Normal;

	if (m_typeModifiers & TypeModifier_Thin)
		ptrTypeKind = DataPtrTypeKind_Thin;
	
	uint_t typeFlags = getPtrTypeFlagsFromModifiers (m_typeModifiers);

	m_typeModifiers &= ~TypeModifierMaskKind_DataPtr;
	return dataType->getDataPtrType (
		m_module->m_namespaceMgr.getCurrentNamespace (),
		TypeKind_DataPtr,
		ptrTypeKind,
		typeFlags
		);
}

ClassPtrType*
DeclTypeCalc::getClassPtrType (ClassType* classType)
{
	ClassPtrTypeKind ptrTypeKind = (m_typeModifiers & TypeModifier_Weak) ? ClassPtrTypeKind_Weak : ClassPtrTypeKind_Normal;
	uint_t typeFlags = getPtrTypeFlagsFromModifiers (m_typeModifiers);

	m_typeModifiers &= ~TypeModifierMaskKind_ClassPtr;
	return classType->getClassPtrType (
		m_module->m_namespaceMgr.getCurrentNamespace (),
		TypeKind_ClassPtr,
		ptrTypeKind,
		typeFlags
		);
}

FunctionPtrType*
DeclTypeCalc::getFunctionPtrType (FunctionType* functionType)
{
	FunctionPtrTypeKind ptrTypeKind =
		(m_typeModifiers & TypeModifier_Weak) ? FunctionPtrTypeKind_Weak :
		(m_typeModifiers & TypeModifier_Thin) ? FunctionPtrTypeKind_Thin : FunctionPtrTypeKind_Normal;

	uint_t typeFlags = getPtrTypeFlagsFromModifiers (m_typeModifiers);

	m_typeModifiers &= ~TypeModifierMaskKind_FunctionPtr;
	return functionType->getFunctionPtrType (ptrTypeKind);
}

PropertyPtrType*
DeclTypeCalc::getPropertyPtrType (PropertyType* propertyType)
{
	PropertyPtrTypeKind ptrTypeKind =
		(m_typeModifiers & TypeModifier_Weak) ? PropertyPtrTypeKind_Weak :
		(m_typeModifiers & TypeModifier_Thin) ? PropertyPtrTypeKind_Thin : PropertyPtrTypeKind_Normal;

	uint_t typeFlags = getPtrTypeFlagsFromModifiers (m_typeModifiers);

	m_typeModifiers &= ~TypeModifierMaskKind_PropertyPtr;
	return propertyType->getPropertyPtrType (ptrTypeKind);
}

ImportPtrType*
DeclTypeCalc::getImportPtrType (NamedImportType* importType)
{
	uint_t typeModifiers = m_typeModifiers & TypeModifierMaskKind_ImportPtr;
	m_typeModifiers &= ~TypeModifierMaskKind_ImportPtr;
	return m_module->m_typeMgr.getImportPtrType (importType, typeModifiers);
}

ImportIntModType*
DeclTypeCalc::getImportIntModType (NamedImportType* importType)
{
	uint_t typeModifiers = m_typeModifiers & TypeModifierMaskKind_Integer;
	m_typeModifiers &= ~TypeModifierMaskKind_Integer;
	return m_module->m_typeMgr.getImportIntModType (importType, typeModifiers);
}

//.............................................................................

} // namespace ct
} // namespace jnc
