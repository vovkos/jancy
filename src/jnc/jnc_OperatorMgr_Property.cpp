#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
OperatorMgr::getPropertyThinPtr (
	Property* prop,
	Closure* closure,
	PropertyPtrType* ptrType,
	Value* resultValue
	)
{
	ASSERT (prop->getType ()->cmp (ptrType->getTargetType ()) == 0);

	bool result = getPropertyVTable (prop, closure, resultValue);
	if (!result)
		return false;

	resultValue->overrideType (ptrType);
	return true;
}

bool
OperatorMgr::getPropertyVTable (
	Property* prop,
	Closure* closure,
	Value* resultValue
	)
{
	if (prop->isVirtual ())
		return getVirtualProperty (prop, closure, resultValue);

	*resultValue = prop->getVTablePtrValue ();
	return true;
}

bool
OperatorMgr::getPropertyVTable (
	const Value& opValue,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr);
	
	PropertyPtrType* ptrType = (PropertyPtrType*) opValue.getType ();
	PropertyPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

	Type* vtableType = ptrType->getTargetType ()->getVTableStructType ()->getDataPtrType_c ();

	switch (ptrTypeKind)
	{
	case PropertyPtrTypeKind_Normal:
		break;

	case PropertyPtrTypeKind_Weak:
		err::setFormatStringError ("cannot invoke weak '%s'", ptrType->getTypeString ().cc ());
		return false;
	
	case PropertyPtrTypeKind_Thin:
		if (opValue.getValueKind () == ValueKind_Property)
			return getPropertyVTable (opValue.getProperty (), opValue.getClosure (), resultValue);

		*resultValue = opValue;
		return true;

	default:
		ASSERT (false);
	}

	Type* closureType = m_module->m_typeMgr.getStdType (StdType_ObjectPtr);

	Value closureValue;
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 0, vtableType, resultValue);
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 1, closureType, &closureValue);

	resultValue->setClosure (opValue.getClosure ());
	resultValue->insertToClosureHead (closureValue);
	return true;
}

Type*
OperatorMgr::getPropertyGetterType (const Value& rawOpValue)
{
	PropertyType* propertyType;

	Value opValue;
	prepareOperandType (rawOpValue, &opValue, OpFlag_KeepPropertyRef);

	if (opValue.getValueKind () == ValueKind_Property)
	{
		propertyType = opValue.getProperty ()->getType ();
	}
	else
	{
		ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr);

		PropertyPtrType* ptrType = (PropertyPtrType*) opValue.getType ();
		propertyType = ptrType->hasClosure () ? 
			ptrType->getTargetType ()->getStdObjectMemberPropertyType () :
			ptrType->getTargetType ();
	}

	return getFunctionType (opValue, propertyType->getGetterType ());
}

bool
OperatorMgr::getPropertyGetterType (
	const Value& opValue,
	Value* resultValue
	)
{
	Type* resultType = getPropertyGetterType (opValue);
	if (!resultType)
		return false;

	resultValue->setType (resultType);
	return true;
}

bool
OperatorMgr::getPropertyGetter (
	const Value& rawOpValue,
	Value* resultValue
	)
{
	bool result;

	Value opValue;
	result = prepareOperand (rawOpValue, &opValue, OpFlag_KeepPropertyRef);
	if (!result)
		return false;

	if (opValue.getValueKind () == ValueKind_Property)
	{
		*resultValue = opValue.getProperty ()->getGetter ();
		resultValue->setClosure (opValue.getClosure ());
		return true;
	}	

	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr);	

	PropertyPtrType* ptrType = (PropertyPtrType*) opValue.getType ();
	PropertyType* propertyType = ptrType->hasClosure () ? 
		ptrType->getTargetType ()->getStdObjectMemberPropertyType () :
		ptrType->getTargetType ();

	Value VTableValue;
	result = getPropertyVTable (opValue, &VTableValue);
	if (!result)
		return false;

	size_t index = (propertyType->getFlags () & PropertyTypeFlag_Bindable) ? 1 : 0;

	Value pfnValue;
	m_module->m_llvmIrBuilder.createGep2 (VTableValue, index, NULL, &pfnValue);
	m_module->m_llvmIrBuilder.createLoad (
		pfnValue, 
		propertyType->getGetterType ()->getFunctionPtrType (FunctionPtrTypeKind_Thin, ptrType->getFlags ()), 
		resultValue
		);

	resultValue->setClosure (VTableValue.getClosure ());
	return true;
}

Type*
OperatorMgr::getPropertySetterType (
	const Value& rawOpValue,
	const Value& argValue
	)
{
	Value opValue;
	prepareOperandType (rawOpValue, &opValue, OpFlag_KeepPropertyRef);

	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr);	
	
	PropertyPtrType* ptrType = (PropertyPtrType*) opValue.getType ();
	PropertyType* propertyType;

	if (opValue.getValueKind () == ValueKind_Property)
	{
		propertyType = opValue.getProperty ()->getType ();
	}
	else
	{
		ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr);

		propertyType = ptrType->hasClosure () ? 
			ptrType->getTargetType ()->getStdObjectMemberPropertyType () :
			ptrType->getTargetType ();
	}

	if (propertyType->isReadOnly ())
	{
		err::setFormatStringError ("read-only '%s' has no 'set'", propertyType->getTypeString ().cc ());
		return NULL;
	}
	else if (ptrType->isConstPtrType ())
	{
		err::setFormatStringError ("'set' is inaccessible via 'const' property pointer");
		return NULL;
	}

	FunctionTypeOverload* setterTypeOverload = propertyType->getSetterType ();
	size_t i = setterTypeOverload->chooseSetterOverload (argValue);
	if (i == -1)
	{
		err::setFormatStringError ("cannot choose one of '%d' setter overloads", setterTypeOverload->getOverloadCount ());
		return NULL;
	}

	FunctionType* setterType = setterTypeOverload->getOverload (i);
	return getFunctionType (opValue, setterType);
}

bool
OperatorMgr::getPropertySetterType (
	const Value& opValue,
	const Value& argValue,
	Value* resultValue
	)
{
	Type* resultType = getPropertySetterType (opValue, argValue);
	if (!resultType)
		return false;

	resultValue->setType (resultType);
	return true;
}

bool
OperatorMgr::getPropertySetter (
	const Value& rawOpValue,
	const Value& argValue,
	Value* resultValue
	)
{
	bool result;

	Value opValue;
	result = prepareOperand (rawOpValue, &opValue, OpFlag_KeepPropertyRef);
	if (!result)
		return false;

	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr);	

	PropertyPtrType* ptrType = (PropertyPtrType*) opValue.getType ();
	PropertyType* propertyType = ptrType->hasClosure () ? 
		ptrType->getTargetType ()->getStdObjectMemberPropertyType () :
		ptrType->getTargetType ();

	if (propertyType->isReadOnly ())
	{
		err::setFormatStringError ("read-only '%s' has no setter", propertyType->getTypeString ().cc ());
		return false;
	}
	else if (ptrType->isConstPtrType ())
	{
		err::setFormatStringError ("'set' is inaccessible via 'const' property pointer");
		return NULL;
	}

	if (opValue.getValueKind () == ValueKind_Property)
	{
		*resultValue = opValue.getProperty ()->getSetter ();
		resultValue->setClosure (opValue.getClosure ());
		return true;
	}

	FunctionTypeOverload* setterTypeOverload = propertyType->getSetterType ();
	size_t i = 0;

	if (setterTypeOverload->isOverloaded ())
	{
		if (!argValue)
		{
			err::setFormatStringError ("no argument value to help choose one of '%d' setter overloads", setterTypeOverload->getOverloadCount ());
			return false;
		}
		
		i = setterTypeOverload->chooseSetterOverload (argValue);
		if (i == -1)
		{
			err::setFormatStringError ("cannot choose one of '%d' setter overloads", setterTypeOverload->getOverloadCount ());
			return false;
		}
	}

	FunctionType* setterType = setterTypeOverload->getOverload (i);

	Value VTableValue;
	result = getPropertyVTable (opValue, &VTableValue);
	if (!result)
		return false;

	size_t index = (propertyType->getFlags () & PropertyTypeFlag_Bindable) ? 2 : 1;
	index += i;

	Value pfnValue;
	m_module->m_llvmIrBuilder.createGep2 (VTableValue, index, NULL, &pfnValue);
	m_module->m_llvmIrBuilder.createLoad (
		pfnValue, 
		setterType->getFunctionPtrType (FunctionPtrTypeKind_Thin, ptrType->getFlags ()), 
		resultValue
		);

	resultValue->setClosure (VTableValue.getClosure ());
	return true;
}

Type*
OperatorMgr::getPropertyBinderType (const Value& rawOpValue)
{
	PropertyType* propertyType;

	Value opValue;
	prepareOperandType (rawOpValue, &opValue, OpFlag_KeepPropertyRef);

	if (opValue.getValueKind () == ValueKind_Property)
	{
		propertyType = opValue.getProperty ()->getType ();
	}
	else
	{
		ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr);	

		PropertyPtrType* ptrType = (PropertyPtrType*) opValue.getType ();
		propertyType = ptrType->hasClosure () ? 
			ptrType->getTargetType ()->getStdObjectMemberPropertyType () :
			ptrType->getTargetType ();
	}

	if (!(propertyType->getFlags () & PropertyTypeFlag_Bindable))
	{
		err::setFormatStringError ("'%s' has no 'onchanged' binder", propertyType->getTypeString ().cc ());
		return NULL;
	}

	return getFunctionType (opValue, propertyType->getBinderType ());
}

bool
OperatorMgr::getPropertyBinderType (
	const Value& opValue,
	Value* resultValue
	)
{
	Type* type = getPropertyBinderType (opValue);
	if (!type)
		return false;

	resultValue->setType (type);
	return true;		
}

bool
OperatorMgr::getPropertyBinder (
	const Value& rawOpValue,
	Value* resultValue
	)
{
	bool result;

	Value opValue;
	result = prepareOperand (rawOpValue, &opValue, OpFlag_KeepPropertyRef);
	if (!result)
		return false;

	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr);	

	PropertyPtrType* ptrType = (PropertyPtrType*) opValue.getType ();
	PropertyType* propertyType = ptrType->getTargetType ();

	if (!(propertyType->getFlags () & PropertyTypeFlag_Bindable))
	{
		err::setFormatStringError ("'%s' has no 'onchanged' binder", propertyType->getTypeString ().cc ());
		return false;
	}

	if (opValue.getValueKind () == ValueKind_Property)
	{
		*resultValue = opValue.getProperty ()->getBinder ();
		resultValue->setClosure (opValue.getClosure ());
		return true;
	}

	if (ptrType->hasClosure ())
		propertyType = propertyType->getStdObjectMemberPropertyType ();

	Value VTableValue;
	result = getPropertyVTable (opValue, &VTableValue);
	if (!result)
		return false;

	Value pfnValue;
	m_module->m_llvmIrBuilder.createGep2 (VTableValue, 0, NULL, &pfnValue);
	m_module->m_llvmIrBuilder.createLoad (
		pfnValue, 
		propertyType->getBinderType ()->getFunctionPtrType (FunctionPtrTypeKind_Thin, ptrType->getFlags ()), 
		resultValue
		);

	resultValue->setClosure (VTableValue.getClosure ());
	return true;
}

bool
OperatorMgr::getProperty (
	const Value& opValue,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_PropertyRef);

	if (opValue.getValueKind () == ValueKind_Property)
	{
		Property* prop = opValue.getProperty ();
		if (prop->getFlags () & PropertyFlag_AutoGet)
			return getPropertyAutoGetValue (opValue, resultValue);
	}

	Value getterValue;
	return 
		getPropertyGetter (opValue, &getterValue) &&
		callOperator (getterValue, NULL, resultValue);
}

bool
OperatorMgr::setProperty (
	const Value& opValue,
	const Value& srcValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_PropertyRef);

	Value setterValue;
	return 
		getPropertySetter (opValue, srcValue, &setterValue) &&
		callOperator (setterValue, srcValue);
}

Type*
OperatorMgr::getPropertyAutoGetValueType (const Value& opValue)
{
	if (opValue.getValueKind () != ValueKind_Property || 
		!(opValue.getProperty ()->getFlags () & PropertyFlag_AutoGet))
	{
		err::setFormatStringError ("'%s' has no autoget field", opValue.getType ()->getTypeString ().cc ());
		return NULL;
	}

	Type* type;

	ModuleItem* autoGetValue = opValue.getProperty ()->getAutoGetValue ();
	type = autoGetValue->getItemKind () == ModuleItemKind_StructField ? 
		((StructField*) autoGetValue)->getType() :
		((Variable*) autoGetValue)->getType();

	return type->getDataPtrType (TypeKind_DataRef, DataPtrTypeKind_Lean);
}

bool
OperatorMgr::getPropertyAutoGetValueType (
	const Value& opValue,
	Value* resultValue
	)
{
	Type* type = getPropertyAutoGetValueType (opValue);
	if (!type)
		return false;

	resultValue->setType (type);
	return true;
}

bool
OperatorMgr::getPropertyAutoGetValue (
	const Value& opValue,
	Value* resultValue
	)
{
	if (opValue.getValueKind () != ValueKind_Property || 
		!(opValue.getProperty ()->getFlags () & PropertyFlag_AutoGet))
	{
		err::setFormatStringError ("'%s' has no autoget field", opValue.getType ()->getTypeString ().cc ());
		return false;
	}

	return getPropertyField (opValue, opValue.getProperty ()->getAutoGetValue (), resultValue);
}

Type*
OperatorMgr::getPropertyOnChangedType (const Value& rawOpValue)
{
	Value opValue;
	prepareOperandType (rawOpValue, &opValue, OpFlag_KeepPropertyRef);

	if (!(opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr) || 
		!(((PropertyPtrType*) opValue.getType ())->getTargetType ()->getFlags () & PropertyTypeFlag_Bindable))
	{
		err::setFormatStringError ("'%s' has no bindable event", opValue.getType ()->getTypeString ().cc ());
		return NULL;
	}

	return m_module->m_typeMgr.getStdType (StdType_SimpleEventPtr);
}

bool
OperatorMgr::getPropertyOnChangedType (
	const Value& opValue,
	Value* resultValue
	)
{
	Type* type = getPropertyOnChangedType (opValue);
	if (!type)
		return false;

	resultValue->setType (type);
	return true;
}

bool
OperatorMgr::getPropertyOnChanged (
	const Value& rawOpValue,
	Value* resultValue
	)
{
	Value opValue;
	bool result = prepareOperand (rawOpValue, &opValue, OpFlag_KeepPropertyRef);
	if (!result)
		return false;

	if (!(opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr) || 
		!(((PropertyPtrType*) opValue.getType ())->getTargetType ()->getFlags () & PropertyTypeFlag_Bindable))
	{
		err::setFormatStringError ("'%s' has no bindable event", opValue.getType ()->getTypeString ().cc ());
		return NULL;
	}

	if (opValue.getValueKind () == ValueKind_Property)
		return getPropertyField (opValue, opValue.getProperty ()->getOnChanged (), resultValue);

	Value binderValue;
	return 
		getPropertyBinder (opValue, &binderValue) &&
		callOperator (binderValue, NULL, resultValue);
}

//.............................................................................

} // namespace jnc {

