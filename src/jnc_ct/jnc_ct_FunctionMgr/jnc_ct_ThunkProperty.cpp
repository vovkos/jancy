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
#include "jnc_ct_ThunkProperty.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

ThunkProperty::ThunkProperty()
{
	m_propertyKind = PropertyKind_Thunk;
	m_targetProperty = NULL;
}

bool
ThunkProperty::create(
	Property* targetProperty,
	PropertyType* thunkPropertyType,
	bool hasUnusedClosure
	)
{
	bool result;

	m_targetProperty = targetProperty;
	m_type = hasUnusedClosure ?
		thunkPropertyType->getStdObjectMemberPropertyType() :
		thunkPropertyType;

	m_getter = m_module->m_functionMgr.getDirectThunkFunction(
		targetProperty->getGetter(),
		thunkPropertyType->getGetterType(),
		hasUnusedClosure
		);

	Function* targetSetter = targetProperty->getSetter();
	FunctionTypeOverload* thunkSetterType = thunkPropertyType->getSetterType();

	size_t setterCount = thunkSetterType->getOverloadCount();
	if (setterCount && !targetSetter)
	{
		setCastError(targetProperty, thunkPropertyType);
		return false;
	}

	for (size_t i = 0; i < setterCount; i++)
	{
		FunctionType* thunkFunctionType = thunkSetterType->getOverload(i);

		Function* overload = targetSetter->chooseSetterOverload(thunkFunctionType);
		if (!overload)
			return false;

		Function* thunkFunction = m_module->m_functionMgr.getDirectThunkFunction(
			overload,
			thunkFunctionType,
			hasUnusedClosure
			);

		if (!m_setter)
		{
			m_setter = thunkFunction;
		}
		else
		{
			result = m_setter->addOverload(thunkFunction) != -1;
			if (!result)
				return false;
		}
	}

	return true;
}

//..............................................................................

DataThunkProperty::DataThunkProperty()
{
	m_propertyKind = PropertyKind_DataThunk;
	m_targetVariable = NULL;
}

bool
DataThunkProperty::compile()
{
	bool result = compileGetter();
	if (!result)
		return false;

	if (m_setter)
	{
		size_t count = m_setter->getOverloadCount();
		for (size_t i = 0; i < count; i++)
		{
			Function* overload = m_setter->getOverload(i);
			result = compileSetter(overload);
			if (!result)
				return false;
		}
	}

	return true;
}

bool
DataThunkProperty::compileGetter()
{
	m_module->m_functionMgr.internalPrologue(m_getter);

	bool result = m_module->m_controlFlowMgr.ret(m_targetVariable);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

bool
DataThunkProperty::compileSetter(Function* setter)
{
	Value srcValue;

	size_t argCount = setter->getType()->getArgArray().getCount();
	ASSERT(argCount == 1 || argCount == 2);

	Value argValueArray[2];
	m_module->m_functionMgr.internalPrologue(setter, argValueArray, argCount);

	bool result = m_module->m_operatorMgr.storeDataRef(m_targetVariable, argValueArray[argCount - 1]);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
