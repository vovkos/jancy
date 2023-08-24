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
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
OperatorMgr::getPropertyThinPtr(
	Property* prop,
	Closure* closure,
	PropertyPtrType* ptrType,
	Value* resultValue
) {
	ASSERT(prop->getType()->cmp(ptrType->getTargetType()) == 0);

	bool result = getPropertyVtable(prop, closure, resultValue);
	if (!result)
		return false;

	resultValue->overrideType(ptrType);
	return true;
}

bool
OperatorMgr::getPropertyVtable(
	Property* prop,
	Closure* closure,
	Value* resultValue
) {
	if (prop->isVirtual())
		return getVirtualProperty(prop, closure, resultValue);

	*resultValue = prop->getVtableVariable();
	return true;
}

bool
OperatorMgr::getPropertyVtable(
	const Value& opValue,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_PropertyPtr);

	PropertyPtrType* ptrType = (PropertyPtrType*)opValue.getType();
	PropertyPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind();

	switch (ptrTypeKind) {
	case PropertyPtrTypeKind_Normal:
		break;

	case PropertyPtrTypeKind_Weak:
		err::setFormatStringError("cannot invoke weak '%s'", ptrType->getTypeString().sz());
		return false;

	case PropertyPtrTypeKind_Thin:
		if (opValue.getValueKind() == ValueKind_Property)
			return getPropertyVtable(opValue.getProperty(), opValue.getClosure(), resultValue);

		*resultValue = opValue;
		return true;

	default:
		ASSERT(false);
	}

	PropertyType* propertyType = ptrType->getTargetType();
	PropertyType* stdObjectMemberPropertyType = propertyType->getStdObjectMemberPropertyType();
	Type* vtableType = stdObjectMemberPropertyType->getVtableStructType()->getDataPtrType_c();
	Type* resultType = propertyType->getVtableStructType()->getDataPtrType_c();
	Type* closureType = m_module->m_typeMgr.getStdType(StdType_AbstractClassPtr);

	Value closureValue;

	if (!m_module->hasCodeGen()) {
		resultValue->setType(resultType);
		closureValue.setType(closureType);
	} else {
		Value vtableValue;
		m_module->m_llvmIrBuilder.createExtractValue(opValue, 0, NULL, &vtableValue);
		m_module->m_llvmIrBuilder.createExtractValue(opValue, 1, closureType, &closureValue);
		m_module->m_llvmIrBuilder.createBitCast(vtableValue, vtableType, resultValue);
		resultValue->overrideType(resultType);
	}

	Closure* closure = opValue.getClosure();
	if (closure)
		resultValue->setClosure(closure);
	else
		closure = resultValue->createClosure();

	closure->insertThisArgValue(closureValue);
	return true;
}

bool
OperatorMgr::getPropertyGetter(
	const Value& rawOpValue,
	Value* resultValue
) {
	bool result;

	Value opValue;
	result = prepareOperand(rawOpValue, &opValue, OpFlag_KeepPropertyRef);
	if (!result)
		return false;

	if (opValue.getValueKind() == ValueKind_Property) {
		result = resultValue->trySetFunction(opValue.getProperty()->getGetter());
		if (!result)
			return false;

		resultValue->setClosure(opValue.getClosure());
		return true;
	}

	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_PropertyPtr);

	PropertyPtrType* ptrType = (PropertyPtrType*)opValue.getType();
	PropertyType* propertyType = ptrType->hasClosure() ?
		ptrType->getTargetType()->getStdObjectMemberPropertyType() :
		ptrType->getTargetType();

	Value VtableValue;
	result = getPropertyVtable(opValue, &VtableValue);
	if (!result)
		return false;

	size_t index = (propertyType->getFlags() & PropertyTypeFlag_Bindable) ? 1 : 0;
	FunctionPtrType* getterPtrType = propertyType->getGetterType()->getFunctionPtrType(FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe);

	if (!m_module->hasCodeGen()) {
		resultValue->setType(getterPtrType);
	} else {
		Value pfnValue;
		m_module->m_llvmIrBuilder.createGep2(VtableValue, index, NULL, &pfnValue);
		m_module->m_llvmIrBuilder.createLoad(pfnValue, getterPtrType, resultValue);
	}

	resultValue->setClosure(VtableValue.getClosure());
	return true;
}

bool
OperatorMgr::getPropertySetter(
	const Value& rawOpValue,
	const Value& argValue,
	Value* resultValue
) {
	bool result;

	Value opValue;
	result = prepareOperand(rawOpValue, &opValue, OpFlag_KeepPropertyRef);
	if (!result)
		return false;

	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_PropertyPtr);

	PropertyPtrType* ptrType = (PropertyPtrType*)opValue.getType();
	PropertyType* propertyType = ptrType->hasClosure() ?
		ptrType->getTargetType()->getStdObjectMemberPropertyType() :
		ptrType->getTargetType();

	if (propertyType->isConst()) {
		err::setFormatStringError("const '%s' has no setter", propertyType->getTypeString().sz());
		return false;
	} else if (ptrType->getFlags() & PtrTypeFlag_Const) {
		err::setFormatStringError("'set' is inaccessible via 'const' property pointer");
		return false;
	}

	if (opValue.getValueKind() == ValueKind_Property) {
		*resultValue = opValue.getProperty()->getSetter();
		resultValue->setClosure(opValue.getClosure());
		return true;
	}

	FunctionTypeOverload* setterTypeOverload = propertyType->getSetterType();
	size_t i = 0;

	if (setterTypeOverload->isOverloaded()) {
		if (!argValue) {
			err::setFormatStringError("no argument value to help choose one of '%d' setter overloads", setterTypeOverload->getOverloadCount());
			return false;
		}

		i = setterTypeOverload->chooseSetterOverload(argValue);
		if (i == -1) {
			err::setFormatStringError("cannot choose one of '%d' setter overloads", setterTypeOverload->getOverloadCount ());
			return false;
		}
	}

	FunctionType* setterType = setterTypeOverload->getOverload(i);
	FunctionPtrType* setterPtrType = setterType->getFunctionPtrType(FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe);

	Value vtableValue;
	result = getPropertyVtable(opValue, &vtableValue);
	if (!result)
		return false;

	if (!m_module->hasCodeGen()) {
		resultValue->setType(setterPtrType);
	} else {
		size_t index = (propertyType->getFlags() & PropertyTypeFlag_Bindable) ? 2 : 1;
		index += i;

		Value pfnValue;
		m_module->m_llvmIrBuilder.createGep2(vtableValue, index, NULL, &pfnValue);
		m_module->m_llvmIrBuilder.createLoad(pfnValue, setterPtrType, resultValue);
	}

	resultValue->setClosure(vtableValue.getClosure());
	return true;
}

bool
OperatorMgr::getPropertyBinder(
	const Value& rawOpValue,
	Value* resultValue
) {
	bool result;

	Value opValue;
	result = prepareOperand(rawOpValue, &opValue, OpFlag_KeepPropertyRef);
	if (!result)
		return false;

	ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_PropertyPtr);

	PropertyPtrType* ptrType = (PropertyPtrType*)opValue.getType();
	PropertyType* propertyType = ptrType->getTargetType();

	if (!(propertyType->getFlags() & PropertyTypeFlag_Bindable)) {
		err::setFormatStringError("'%s' has no 'onchanged' binder", propertyType->getTypeString().sz());
		return false;
	}

	if (opValue.getValueKind() == ValueKind_Property) {
		*resultValue = opValue.getProperty()->getBinder();
		resultValue->setClosure(opValue.getClosure());
		return true;
	}

	if (ptrType->hasClosure())
		propertyType = propertyType->getStdObjectMemberPropertyType();

	Value VtableValue;
	result = getPropertyVtable(opValue, &VtableValue);
	if (!result)
		return false;

	FunctionPtrType* binderPtrType = propertyType->getBinderType()->getFunctionPtrType(FunctionPtrTypeKind_Thin, PtrTypeFlag_Safe);

	if (!m_module->hasCodeGen()) {
		resultValue->setType(binderPtrType);
	} else {
		Value pfnValue;
		m_module->m_llvmIrBuilder.createGep2(VtableValue, 0, NULL, &pfnValue);
		m_module->m_llvmIrBuilder.createLoad(
			pfnValue,
			binderPtrType,
			resultValue
		);
	}

	resultValue->setClosure(VtableValue.getClosure());
	return true;
}

bool
OperatorMgr::getProperty(
	const Value& opValue,
	Value* resultValue
) {
	ASSERT(opValue.getType()->getTypeKind() == TypeKind_PropertyRef);

	if (opValue.getValueKind() == ValueKind_Property) {
		Property* prop = opValue.getProperty();
		if (prop->getFlags() & PropertyFlag_AutoGet)
			return getPropertyAutoGetValue(opValue, resultValue);
	}

	Value getterValue;
	return
		getPropertyGetter(opValue, &getterValue) &&
		callOperator(getterValue, NULL, resultValue);
}

bool
OperatorMgr::setProperty(
	const Value& opValue,
	const Value& srcValue
) {
	ASSERT(opValue.getType()->getTypeKind() == TypeKind_PropertyRef);

	Value setterValue;
	return
		getPropertySetter(opValue, srcValue, &setterValue) &&
		callOperator(setterValue, srcValue);
}

Type*
OperatorMgr::getPropertyAutoGetValueType(const Value& opValue) {
	if (opValue.getValueKind() != ValueKind_Property ||
		!(opValue.getProperty()->getFlags() & PropertyFlag_AutoGet)) {
		err::setFormatStringError("'%s' has no autoget field", opValue.getType ()->getTypeString().sz());
		return NULL;
	}

	Type* type;

	ModuleItem* autoGetValue = opValue.getProperty()->getAutoGetValue();
	type = autoGetValue->getItemKind() == ModuleItemKind_Field ?
		((Field*)autoGetValue)->getType() :
		((Variable*)autoGetValue)->getType();

	return type->getDataPtrType(TypeKind_DataRef, DataPtrTypeKind_Lean);
}

bool
OperatorMgr::getPropertyAutoGetValueType(
	const Value& opValue,
	Value* resultValue
) {
	Type* type = getPropertyAutoGetValueType(opValue);
	if (!type)
		return false;

	resultValue->setType(type);
	return true;
}

bool
OperatorMgr::getPropertyAutoGetValue(
	const Value& opValue,
	Value* resultValue
) {
	if (opValue.getValueKind() != ValueKind_Property ||
		!(opValue.getProperty()->getFlags() & PropertyFlag_AutoGet)) {
		err::setFormatStringError("'%s' has no autoget field", opValue.getType ()->getTypeString().sz());
		return false;
	}

	return getPropertyField(opValue, opValue.getProperty()->getAutoGetValue(), resultValue);
}

Type*
OperatorMgr::getPropertyOnChangedType(const Value& rawOpValue) {
	Value opValue;
	bool result = prepareOperandType(rawOpValue, &opValue, OpFlag_KeepPropertyRef);
	if (!result)
		return NULL;

	if (!(opValue.getType()->getTypeKindFlags() & TypeKindFlag_PropertyPtr) ||
		!(((PropertyPtrType*)opValue.getType())->getTargetType()->getFlags() & PropertyTypeFlag_Bindable)) {
		err::setFormatStringError("'%s' has no bindable event", opValue.getType ()->getTypeString().sz());
		return NULL;
	}

	return m_module->m_typeMgr.getStdType(StdType_SimpleEventPtr);
}

bool
OperatorMgr::getPropertyOnChangedType(
	const Value& opValue,
	Value* resultValue
) {
	Type* type = getPropertyOnChangedType(opValue);
	if (!type)
		return false;

	resultValue->setType(type);
	return true;
}

bool
OperatorMgr::getPropertyOnChanged(
	const Value& rawOpValue,
	Value* resultValue
) {
	Value opValue;
	bool result = prepareOperand(rawOpValue, &opValue, OpFlag_KeepPropertyRef);
	if (!result)
		return false;

	if (!(opValue.getType()->getTypeKindFlags() & TypeKindFlag_PropertyPtr) ||
		!(((PropertyPtrType*)opValue.getType())->getTargetType()->getFlags() & PropertyTypeFlag_Bindable)) {
		err::setFormatStringError("'%s' has no bindable event", opValue.getType ()->getTypeString().sz());
		return false;
	}

	if (opValue.getValueKind() == ValueKind_Property)
		return getPropertyField(opValue, opValue.getProperty()->getOnChanged(), resultValue);

	Value binderValue;
	return
		getPropertyBinder(opValue, &binderValue) &&
		callOperator(binderValue, NULL, resultValue);
}

//..............................................................................

} // namespace ct
} // namespace jnc
