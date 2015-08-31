#include "pch.h"
#include "jnc_Closure.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

Value
Closure::getThisArgValue ()
{
	ASSERT (m_thisArgIdx != -1);

	if (m_thisArgValue)
		return *m_thisArgValue;

	rtl::BoxIterator <Value> it = m_argValueList.getHead ();
	for (size_t i = 0; i < m_thisArgIdx; i++)
		it++;

	m_thisArgValue = it.p ();
	return *m_thisArgValue;
}

void 
Closure::setThisArgIdx (size_t thisArgIdx)
{
	ASSERT (m_thisArgIdx == -1 && !m_thisArgValue); // only once	
	ASSERT (thisArgIdx < m_argValueList.getCount ());

	m_thisArgIdx = thisArgIdx;
	m_thisArgValue = NULL;
}

void
Closure::insertThisArgValue (const Value& value)
{
	ASSERT (m_thisArgIdx == -1 && !m_thisArgValue); // only once

	rtl::BoxIterator <Value> it = m_argValueList.insertHead (value);
	m_thisArgIdx = 0;
	m_thisArgValue = it.p ();
}

size_t
Closure::append (const rtl::ConstBoxList <Value>& argValueList)
{
	ASSERT (!argValueList.isEmpty ());

	rtl::BoxIterator <Value> internalArg = m_argValueList.getHead ();
	rtl::BoxIterator <Value> externalArg = argValueList.getHead ();

	for (;;)
	{
		while (internalArg && !internalArg->isEmpty ())
			internalArg++;

		if (!internalArg)
			break;

		*internalArg = *externalArg;

		internalArg++;
		externalArg++;

		if (!externalArg)
			return m_argValueList.getCount ();
	}

	for (; externalArg; externalArg++)
		m_argValueList.insertTail (*externalArg);

	return m_argValueList.getCount ();
}

bool
Closure::apply (rtl::BoxList <Value>* argValueList)
{
	if (m_argValueList.isEmpty ())
		return true;

	rtl::BoxIterator <Value> closureArg = m_argValueList.getHead ();
	rtl::BoxIterator <Value> targetArg = argValueList->getHead ();

	for (size_t i = 0; closureArg; closureArg++, i++)
	{
		if (!closureArg->isEmpty ())
		{
			argValueList->insertBefore (*closureArg, targetArg);
		}
		else if (targetArg)
		{
			targetArg++;
		}
		else
		{
			err::setFormatStringError ("closure call misses argument #%d", i + 1);
			return false;
		}
	}

	return true;
}

Type*
Closure::getClosureType (Type* type)
{
	TypeKind typeKind = type->getTypeKind ();

	switch (typeKind)
	{
	case TypeKind_FunctionPtr:
	case TypeKind_FunctionRef:
		return getFunctionClosureType ((FunctionPtrType*) type);

	case TypeKind_PropertyPtr:
	case TypeKind_PropertyRef:
		return getPropertyClosureType ((PropertyPtrType*) type);

	default:
		return type;
	}
}

FunctionPtrType*
Closure::getFunctionClosureType (Function* function)
{
	if (!function->isOverloaded ())
		return getFunctionClosureType (function->getType ()->getFunctionPtrType (TypeKind_FunctionRef, FunctionPtrTypeKind_Thin));

	err::setFormatStringError ("function overload closures are not implemented yet");
	return NULL;
}

bool
Closure::getArgTypeArray (
	Module* module,
	rtl::Array <FunctionArg*>* argArray
	)
{
	bool result;

	size_t closureArgCount = m_argValueList.getCount ();
	size_t argCount = argArray->getCount ();

	if (closureArgCount > argCount)
	{
		err::setFormatStringError ("closure with %d arguments for function with %d arguments", closureArgCount, argCount);
		return NULL;
	}

	rtl::BoxIterator <Value> closureArg = m_argValueList.getHead ();
	for (size_t i = 0; closureArg; closureArg++)
	{
		if (closureArg->isEmpty ())
		{
			i++;
			continue;
		}

		ASSERT (i < argCount);

		result = module->m_operatorMgr.checkCastKind (closureArg->getType (), (*argArray) [i]->getType ());
		if (!result)
			return false;

		argArray->remove (i);
		argCount--;
	}

	return true;
}

FunctionPtrType*
Closure::getFunctionClosureType (FunctionPtrType* ptrType)
{
	bool result;

	Module* module = ptrType->getModule ();
	FunctionType* type = ptrType->getTargetType ();

	if (type->getFlags () & FunctionTypeFlag_VarArg)
	{
		err::setFormatStringError ("function closures cannot be applied to vararg functions");
		return NULL;
	}

	rtl::Array <FunctionArg*> argArray = type->getArgArray ();
	result = getArgTypeArray (module, &argArray);
	if (!result)
		return NULL;

	FunctionType* closureType = module->m_typeMgr.getFunctionType (
		type->getCallConv (),
		type->getReturnType (),
		argArray
		);

	return closureType->getFunctionPtrType (
		ptrType->getTypeKind (),
		ptrType->getPtrTypeKind (),
		ptrType->getFlags ()
		);
}

PropertyPtrType*
Closure::getPropertyClosureType (PropertyPtrType* ptrType)
{
	bool result;

	Module* module = ptrType->getModule ();
	PropertyType* type = ptrType->getTargetType ();
	FunctionType* getterType = type->getGetterType ();
	FunctionTypeOverload* setterType = type->getSetterType ();

	rtl::Array <FunctionArg*> argArray = getterType->getArgArray ();
	result = getArgTypeArray (module, &argArray);
	if (!result)
		return NULL;

	FunctionType* closureGetterType = module->m_typeMgr.getFunctionType (
		getterType->getCallConv (),
		getterType->getReturnType (),
		argArray
		);

	FunctionTypeOverload closureSetterType;

	size_t setterCount = setterType->getOverloadCount ();
	for (size_t i = 0; i < setterCount; i++)
	{
		FunctionType* overloadType = setterType->getOverload (i);
		ASSERT (!overloadType->getArgArray ().isEmpty ());

		argArray.append (overloadType->getArgArray ().getBack ());

		FunctionType* closureOverloadType = module->m_typeMgr.getFunctionType (
			overloadType->getCallConv (),
			overloadType->getReturnType (),
			argArray
			);

		argArray.pop ();

		result = closureSetterType.addOverload (closureOverloadType) != -1;
		if (!result)
			return NULL;
	}

	PropertyType* closureType = module->m_typeMgr.getPropertyType (
		closureGetterType,
		closureSetterType,
		type->getFlags ()
		);

	return closureType->getPropertyPtrType (
		ptrType->getTypeKind (),
		ptrType->getPtrTypeKind (),
		ptrType->getFlags ()
		);
}

//.............................................................................

} // namespace jnc {
