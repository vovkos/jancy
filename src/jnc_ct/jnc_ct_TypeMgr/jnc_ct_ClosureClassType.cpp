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
#include "jnc_ct_ClosureClassType.h"
#include "jnc_ct_Module.h"
#include "jnc_rt_Runtime.h"
#include "jnc_Runtime.h"

namespace jnc {
namespace ct {

//..............................................................................

ClosureClassType::ClosureClassType()
{
	m_namespaceStatus = NamespaceStatus_Ready;
	m_flags |= ClassTypeFlag_Closure;
	m_thisArgFieldIdx = -1;
}

sl::String
ClosureClassType::createSignature(
	Type* targetType, // function or property
	Type* thunkType, // function or property
	Type* const* argTypeArray,
	const size_t* closureMap,
	size_t argCount,
	size_t thisArgIdx
	)
{
	sl::String signature = "CF";

	signature.appendFormat(
		"%s-%s(",
		targetType->getSignature().sz(),
		thunkType->getSignature().sz()
		);

	for (size_t i = 0; i < argCount; i++)
		signature.appendFormat("%d:%s", closureMap [i], argTypeArray [i]->getSignature ().sz());

	signature.appendFormat("::%d)", thisArgIdx);
	return signature;
}

void
ClosureClassType::buildArgValueList(
	const Value& closureValue,
	const Value* thunkArgValueArray,
	size_t thunkArgCount,
	sl::BoxList<Value>* argValueList
	)
{
	size_t fieldIdx = 1; // skip function / property ptr

	size_t iClosure = 0;
	size_t iThunk = 1; // skip 'this' arg

	// part 1 -- arguments come both from closure and from thunk

	size_t fieldCount = m_fieldArray.getCount();
	for (size_t i = 0; fieldIdx < fieldCount; i++)
	{
		Value argValue;

		if (i == m_closureMap[iClosure])
		{
			m_module->m_operatorMgr.getClassField(closureValue, m_fieldArray[fieldIdx], NULL, &argValue);
			fieldIdx++;
			iClosure++;
		}
		else
		{
			argValue = thunkArgValueArray[iThunk];
			iThunk++;
		}

		argValueList->insertTail(argValue);
	}

	// part 2 -- arguments come from thunk only

	for (; iThunk < thunkArgCount; iThunk++)
		argValueList->insertTail(thunkArgValueArray[iThunk]);
}

IfaceHdr*
ClosureClassType::strengthen(IfaceHdr* p)
{
	if (m_thisArgFieldIdx == -1)
		return p;

	Field* field = getFieldByIndex(m_thisArgFieldIdx);
	ASSERT(field && field->getType()->getTypeKind() == TypeKind_ClassPtr);

	void* p2 = (char*)p + field->getOffset();

	return jnc::strengthenClassPtr(*(IfaceHdr**) p2) ? p : NULL;
}

//..............................................................................

FunctionClosureClassType::FunctionClosureClassType()
{
	m_classTypeKind = ClassTypeKind_FunctionClosure;
	m_thunkFunction = NULL;
}

bool
FunctionClosureClassType::compileThunkFunction(Function* function)
{
	ASSERT(function == m_thunkFunction);

	size_t argCount = function->getType()->getArgArray().getCount();

	char buffer[256];
	sl::Array<Value> argValueArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	argValueArray.setCount(argCount);

	m_module->m_functionMgr.internalPrologue(function, argValueArray, argCount);

	Value thisValue = m_module->m_functionMgr.getThisValue();
	ASSERT(thisValue);

	Value pfnValue;
	m_module->m_operatorMgr.getClassField(thisValue, m_fieldArray[0], NULL, &pfnValue);

	sl::BoxList<Value> argValueList;
	buildArgValueList(thisValue, argValueArray, argCount, &argValueList);

	Value returnValue;
	bool result = m_module->m_operatorMgr.callOperator(pfnValue, &argValueList, &returnValue);
	if (!result)
		return false;

	if (function->getType()->getReturnType()->getTypeKind() != TypeKind_Void)
	{
		result = m_module->m_controlFlowMgr.ret(returnValue);
		if (!result)
			return false;
	}

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

//..............................................................................

Function*
PropertyClosureClassType::ThunkProperty::createAccessor(
	FunctionKind functionKind,
	FunctionType* type
	)
{
	return m_module->m_functionMgr.createFunction<Accessor>(functionKind, type);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

PropertyClosureClassType::PropertyClosureClassType()
{
	m_classTypeKind = ClassTypeKind_PropertyClosure;
	m_thunkProperty = NULL;
}

bool
PropertyClosureClassType::compileAccessor(Function* accessor)
{
	ASSERT(accessor->getProperty() == m_thunkProperty);

	bool result;

	size_t argCount = accessor->getType()->getArgArray().getCount();

	char buffer[256];
	sl::Array<Value> argValueArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	argValueArray.setCount(argCount);

	m_module->m_functionMgr.internalPrologue(accessor, argValueArray, argCount);

	Value thisValue = m_module->m_functionMgr.getThisValue();
	ASSERT(thisValue);

	Value propertyPtrValue;
	result = m_module->m_operatorMgr.getClassField(thisValue, m_fieldArray[0], NULL, &propertyPtrValue);
	ASSERT(result);

	Value pfnValue;

	FunctionKind accessorKind = accessor->getFunctionKind();
	switch (accessorKind)
	{
	case FunctionKind_Binder:
		result = m_module->m_operatorMgr.getPropertyBinder(propertyPtrValue, &pfnValue);
		break;

	case FunctionKind_Getter:
		result = m_module->m_operatorMgr.getPropertyGetter(propertyPtrValue, &pfnValue);
		break;

	case FunctionKind_Setter:
		result = m_module->m_operatorMgr.getPropertySetter(propertyPtrValue, argValueArray[argCount - 1], &pfnValue);
		break;

	default:
		err::setFormatStringError("invalid property accessor '%s' in property closure", getFunctionKindString(accessorKind));
		return false;
	}

	if (!result)
		return false;

	sl::BoxList<Value> argValueList;
	buildArgValueList(thisValue, argValueArray, argCount, &argValueList);

	Value returnValue;
	result = m_module->m_operatorMgr.callOperator(pfnValue, &argValueList, &returnValue);
	if (!result)
		return false;

	if (accessor->getType()->getReturnType()->getTypeKind() != TypeKind_Void)
	{
		result = m_module->m_controlFlowMgr.ret(returnValue);
		if (!result)
			return false;
	}

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

//..............................................................................

Function*
DataClosureClassType::ThunkProperty::createAccessor(
	FunctionKind functionKind,
	FunctionType* type
	)
{
	switch (functionKind)
	{
	case FunctionKind_Getter:
		return m_module->m_functionMgr.createFunction<Getter>(functionKind, type);

	case FunctionKind_Setter:
		return m_module->m_functionMgr.createFunction<Setter>(functionKind, type);

	default:
		return Property::createAccessor(functionKind, type);
	}
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataClosureClassType::DataClosureClassType()
{
	m_classTypeKind = ClassTypeKind_DataClosure;
	m_thunkProperty = NULL;
}

sl::String
DataClosureClassType::createSignature(
	Type* targetType,
	PropertyType* thunkType
	)
{
	sl::String signature = "CD";

	signature.appendFormat(
		"%s-%s",
		targetType->getTypeString().sz(),
		thunkType->getTypeString().sz()
		);

	return signature;
}

bool
DataClosureClassType::compileGetter(Function* function)
{
	ASSERT(function == m_thunkProperty->getGetter());

	m_module->m_functionMgr.internalPrologue(function);

	Value thisValue = m_module->m_functionMgr.getThisValue();
	ASSERT(thisValue);

	Value ptrValue;

	bool result =
		m_module->m_operatorMgr.getClassField(thisValue, m_fieldArray[0], NULL, &ptrValue) &&
		m_module->m_operatorMgr.unaryOperator(UnOpKind_Indir, &ptrValue) &&
		m_module->m_controlFlowMgr.ret(ptrValue);

	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

bool
DataClosureClassType::compileSetter(Function* function)
{
	Value argValue;
	m_module->m_functionMgr.internalPrologue(function, &argValue, 1);

	Value thisValue = m_module->m_functionMgr.getThisValue();
	ASSERT(thisValue);

	Value ptrValue;

	bool result =
		m_module->m_operatorMgr.getClassField(thisValue, m_fieldArray[0], NULL, &ptrValue) &&
		m_module->m_operatorMgr.unaryOperator(UnOpKind_Indir, &ptrValue) &&
		m_module->m_operatorMgr.storeDataRef(ptrValue, argValue);

	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
