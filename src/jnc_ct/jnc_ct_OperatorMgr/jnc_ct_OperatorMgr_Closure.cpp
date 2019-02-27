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
#include "jnc_ct_Closure.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
OperatorMgr::createClosureObject(
	const Value& opValue, // thin function or property ptr with closure
	Type* thunkType, // function or property type
	bool isWeak,
	Value* resultValue
	)
{
	ASSERT(thunkType->getTypeKind() == TypeKind_Function || thunkType->getTypeKind() == TypeKind_Property);

	bool result;

	// choose reference function type

	FunctionType* srcFunctionType;
	if (opValue.getType()->getTypeKindFlags() & TypeKindFlag_FunctionPtr)
	{
		ASSERT(((FunctionPtrType*)opValue.getType())->getPtrTypeKind() == FunctionPtrTypeKind_Thin);
		srcFunctionType = ((FunctionPtrType*)opValue.getType())->getTargetType();
	}
	else
	{
		ASSERT(opValue.getType()->getTypeKindFlags() & TypeKindFlag_PropertyPtr);
		ASSERT(((PropertyPtrType*)opValue.getType())->getPtrTypeKind() == PropertyPtrTypeKind_Thin);
		srcFunctionType = ((PropertyPtrType*)opValue.getType())->getTargetType()->getGetterType();
	}

	char buffer1[256];
	char buffer2[256];

	sl::Array<Type*> closureArgTypeArray(ref::BufKind_Stack, buffer1, sizeof(buffer1));
	sl::Array<size_t> closureMap(ref::BufKind_Stack, buffer2, sizeof(buffer2));
	size_t closureArgCount = 0;
	size_t closureThisArgIdx = -1;

	// build closure arg type array & closure map

	Closure* closure = opValue.getClosure();
	if (closure)
	{
		closureArgCount = closure->getArgValueList()->getCount();
		closureThisArgIdx = closure->getThisArgIdx();

		sl::Array<FunctionArg*> srcArgArray = srcFunctionType->getArgArray();
		size_t srcArgCount = srcArgArray.getCount();

		if (closureArgCount > srcArgCount)
		{
			err::setFormatStringError("closure is too big for '%s'", srcFunctionType->getTypeString ().sz ());
			return false;
		}

		closureArgTypeArray.setCount(closureArgCount);
		closureMap.setCount(closureArgCount);

		sl::BoxIterator<Value> closureArgValue = closure->getArgValueList()->getHead();

		size_t j = 0;

		for (size_t i = 0; i < closureArgCount; closureArgValue++, i++)
		{
			if (closureArgValue->isEmpty())
				continue;

			closureArgTypeArray[j] = srcArgArray[i]->getType();
			closureMap[j] = i;
			j++;
		}

		closureArgCount = j; // account for possible skipped args
	}

	// find or create closure class type

	ClosureClassType* closureType;

	if (thunkType->getTypeKind() == TypeKind_Function)
	{
		closureType = m_module->m_typeMgr.getFunctionClosureClassType(
			((FunctionPtrType*)opValue.getType())->getTargetType(),
			(FunctionType*)thunkType,
			closureArgTypeArray,
			closureMap,
			closureArgCount,
			closureThisArgIdx
			);
	}
	else
	{
		ASSERT(thunkType->getTypeKind() == TypeKind_Property);

		closureType = m_module->m_typeMgr.getPropertyClosureClassType(
			((PropertyPtrType*)opValue.getType())->getTargetType(),
			(PropertyType*)thunkType,
			closureArgTypeArray,
			closureMap,
			closureArgCount,
			closureThisArgIdx
			);
	}

	// create instance

	Value closureValue;
	result = m_module->m_operatorMgr.newOperator(closureType, NULL, &closureValue);
	if (!result)
		return false;

	sl::Array<StructField*> fieldArray = closureType->getMemberFieldArray();
	size_t fieldIdx = 0;

	// save function/property pointer

	Value pfnValue = opValue;
	pfnValue.clearClosure();

	Value fieldValue;
	result =
		getClassField(closureValue, fieldArray[fieldIdx], NULL, &fieldValue) &&
		binaryOperator(BinOpKind_Assign, fieldValue, pfnValue);

	if (!result)
		return false;

	fieldIdx++;

	// save closure arguments (if any)

	if (closure)
	{
		sl::BoxIterator<Value> closureArgValue = closure->getArgValueList()->getHead();
		for (; closureArgValue; closureArgValue++)
		{
			if (closureArgValue->isEmpty())
				continue;

			Value fieldValue;
			result =
				getClassField(closureValue, fieldArray[fieldIdx], NULL, &fieldValue) &&
				binaryOperator(BinOpKind_Assign, fieldValue, *closureArgValue);

			if (!result)
				return false;

			fieldIdx++;
		}
	}

	*resultValue = closureValue;
	return true;
}

bool
OperatorMgr::createDataClosureObject(
	const Value& opValue, // data ptr
	PropertyType* thunkType, // function or property type
	Value* resultValue
	)
{
	ASSERT(opValue.getType()->getTypeKind() == TypeKind_DataPtr);

	bool result;

	// find or create closure class type

	DataClosureClassType* closureType = m_module->m_typeMgr.getDataClosureClassType(
		((DataPtrType*)opValue.getType())->getTargetType(),
		thunkType
		);

	// create instance

	Value closureValue;
	result = m_module->m_operatorMgr.newOperator(closureType, NULL, &closureValue);
	if (!result)
		return false;

	sl::Array<StructField*> fieldArray = closureType->getMemberFieldArray();
	ASSERT(!fieldArray.isEmpty());

	// save data pointer

	Value fieldValue;
	result =
		getClassField(closureValue, fieldArray[0], NULL, &fieldValue) &&
		binaryOperator(BinOpKind_Assign, fieldValue, opValue);

	if (!result)
		return false;

	*resultValue = closureValue;
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
