#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
OperatorMgr::callTraceFunction (
	const char* functionName,
	const char* string
	)
{
	ModuleItem* item = m_module->m_namespaceMgr.getGlobalNamespace ()->findItem (functionName);
	if (item && item->getItemKind () == ModuleItemKind_Function)
	{
		Value literalValue;
		literalValue.setCharArray (string, m_module);
		m_module->m_operatorMgr.callOperator ((Function*) item, literalValue);
	}
}

Type*
OperatorMgr::getFunctionType (
	const Value& opValue,
	FunctionType* functionType
	)
{
	FunctionPtrType* functionPtrType = functionType->getFunctionPtrType (
		TypeKind_FunctionRef,
		FunctionPtrTypeKind_Thin
		);

	Closure* closure = opValue.getClosure ();
	if (!closure)
		return functionPtrType;

	return getClosureOperatorResultType (functionPtrType, closure->getArgValueList ());
}

Type*
OperatorMgr::getClosureOperatorResultType (
	const Value& rawOpValue,
	rtl::BoxList <Value>* argValueList
	)
{
	Value opValue;
	bool result = prepareOperand (rawOpValue, &opValue);
	if (!result)
		return NULL;

	TypeKind typeKind = opValue.getType ()->getTypeKind ();
	if (typeKind != TypeKind_FunctionRef && typeKind != TypeKind_FunctionPtr)
	{
		err::setFormatStringError (
			"closure operator cannot be applied to '%s'",
			opValue.getType ()->getTypeString ().cc () // thanks a lot gcc
			);
		return NULL;
	}

	ref::Ptr <Closure> closure = AXL_REF_NEW (Closure);
	closure->append (*argValueList);
	return closure->getClosureType (opValue.getType ());
}

bool
OperatorMgr::getClosureOperatorResultType (
	const Value& rawOpValue,
	rtl::BoxList <Value>* argValueList,
	Value* resultValue
	)
{
	Type* resultType = getClosureOperatorResultType (rawOpValue, argValueList);
	if (!resultType)
		return false;

	resultValue->setType (resultType);
	return true;
}

bool
OperatorMgr::closureOperator (
	const Value& rawOpValue,
	rtl::BoxList <Value>* argValueList,
	Value* resultValue
	)
{
	Value opValue;
	bool result = prepareOperand (rawOpValue, &opValue);
	if (!result)
		return false;

	TypeKind typeKind = opValue.getType ()->getTypeKind ();
	if (typeKind != TypeKind_FunctionRef && typeKind != TypeKind_FunctionPtr)
	{
		err::setFormatStringError (
			"closure operator cannot be applied to '%s'",
			opValue.getType ()->getTypeString ().cc ()
			);
		return false;
	}

	*resultValue = opValue;

	Closure* closure = resultValue->getClosure ();
	if (!closure)
		closure = resultValue->createClosure ();

	closure->append (*argValueList);
	return true;
}

Type*
OperatorMgr::getCdeclVarArgType (Type* type)
{
	for (;;)
	{
		Type* prevType = type;

		TypeKind typeKind = type->getTypeKind ();
		switch (typeKind)
		{
		case TypeKind_PropertyRef:
			type = ((PropertyPtrType*) type)->getTargetType ()->getReturnType ();
			break;

		case TypeKind_DataRef:
			type = ((DataPtrType*) type)->getTargetType ();
			break;

		case TypeKind_BitField:
			type = ((BitFieldType*) type)->getBaseType ();
			break;

		case TypeKind_Enum:
			type = ((EnumType*) type)->getBaseType ();
			break;

		case TypeKind_Float:
			type = m_module->m_typeMgr.getPrimitiveType (TypeKind_Double);
			break;

		case TypeKind_Array:
			type = ((ArrayType*) type)->getElementType ()->getDataPtrType_c (TypeKind_DataPtr, PtrTypeFlag_Const);
			break;

		case TypeKind_DataPtr:
			type = ((DataPtrType*) type)->getTargetType ()->getDataPtrType_c (TypeKind_DataPtr, PtrTypeFlag_Const);
			break;

		default:
			if (type->getTypeKindFlags () & TypeKindFlag_Integer)
			{
				type = type->getSize () > 4 ?
					m_module->m_typeMgr.getPrimitiveType (TypeKind_Int64) :
					m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
			}
		}

		if (type == prevType)
			return type;
	}
}

Type*
OperatorMgr::getCallOperatorResultType (
	const Value& rawOpValue,
	rtl::BoxList <Value>* argValueList
	)
{
	bool result;

	Value opValue;
	prepareOperandType (rawOpValue, &opValue);

	if (opValue.getType ()->getTypeKind () == TypeKind_ClassPtr)
	{
		Function* callOperator = ((ClassPtrType*) opValue.getType ())->getTargetType ()->getCallOperator ();
		if (!callOperator)
		{
			err::setFormatStringError ("cannot call '%s'", opValue.getType ()->getTypeString ().cc ());
			return NULL;
		}

		Value objValue = opValue;

		opValue.setFunctionTypeOverload (callOperator->getTypeOverload ());
		opValue.insertToClosureTail (objValue);
	}

	ref::Ptr <Closure> closure = opValue.getClosure ();
	if (closure)
	{
		result = closure->apply (argValueList);
		if (!result)
			return NULL;
	}

	if (rawOpValue.getValueKind () == ValueKind_FunctionTypeOverload)
	{
		size_t i = rawOpValue.getFunctionTypeOverload ()->chooseOverload (*argValueList);
		if (i == -1)
			return NULL;

		FunctionType* functionType = rawOpValue.getFunctionTypeOverload ()->getOverload (i);
		return functionType->getReturnType ();
	}

	FunctionType* functionType;

	Type* opType = opValue.getType ();
	TypeKind typeKind = opType->getTypeKind ();

	switch (typeKind)
	{
	case TypeKind_Function:
		functionType = (FunctionType*) opType;
		break;

	case TypeKind_FunctionRef:
	case TypeKind_FunctionPtr:
		functionType = ((FunctionPtrType*) opType)->getTargetType ();
		break;

	default:
		err::setFormatStringError ("cannot call '%s'", opType->getTypeString ().cc ());
		return NULL;
	}

	return functionType->getReturnType ();
}

bool
OperatorMgr::getCallOperatorResultType (
	const Value& rawOpValue,
	rtl::BoxList <Value>* argValueList,
	Value* resultValue
	)
{
	Type* resultType = getCallOperatorResultType (rawOpValue, argValueList);
	if (!resultType)
		return false;

	resultValue->setType (resultType);
	return true;
}

bool
OperatorMgr::callOperator (
	const Value& rawOpValue,
	rtl::BoxList <Value>* argValueList,
	Value* resultValue
	)
{
	bool result;

	Value opValue;
	Value unusedReturnValue;
	rtl::BoxList <Value> emptyArgValueList;

	if (!resultValue)
		resultValue = &unusedReturnValue;

	if (!argValueList)
		argValueList = &emptyArgValueList;

	result = prepareOperand (rawOpValue, &opValue, 0);
	if (!result)
		return false;

	if (opValue.getType ()->getTypeKind () == TypeKind_ClassPtr)
	{
		ClassPtrType* ptrType = (ClassPtrType*) opValue.getType ();

		Function* callOperator = ptrType->getTargetType ()->getCallOperator ();
		if (!callOperator)
		{
			err::setFormatStringError ("cannot call '%s'", ptrType->getTypeString ().cc ());
			return false;
		}

		if ((callOperator->getFlags () & MulticastMethodFlag_InaccessibleViaEventPtr) && ptrType->isEventPtrType ())
		{
			err::setFormatStringError ("'call' is inaccessible via 'event' pointer");
			return false;
		}

		Value objValue = opValue;

		opValue.setFunction (callOperator);
		opValue.insertToClosureTail (objValue);
	}

	ref::Ptr <Closure> closure = opValue.getClosure ();
	if (closure)
	{
		result = closure->apply (argValueList);
		if (!result)
			return false;
	}

	if (opValue.getValueKind () == ValueKind_Function && opValue.getFunction ()->isOverloaded ())
	{
		Function* function = opValue.getFunction ()->chooseOverload (*argValueList);
		if (!function)
			return false;

		opValue.setFunction (function);
		opValue.setClosure (closure);
	}

	if (opValue.getValueKind () == ValueKind_Function)
	{
		Function* function = opValue.getFunction ();

		if (function->isVirtual ())
		{
			result = getVirtualMethod (function, closure, &opValue);
			if (!result)
				return false;
		}

		return callImpl (opValue, function->getType (), argValueList, resultValue);
	}

	Type* opType = opValue.getType ();
	if (!(opType->getTypeKindFlags () & TypeKindFlag_FunctionPtr) ||
		((FunctionPtrType*) opType)->getPtrTypeKind () == FunctionPtrTypeKind_Weak)
	{
		err::setFormatStringError ("cannot call '%s'", opType->getTypeString ().cc ());
		return false;
	}

	FunctionPtrType* functionPtrType = ((FunctionPtrType*) opType);
	return functionPtrType->hasClosure () ?
		callClosureFunctionPtr (opValue, argValueList, resultValue) :
		callImpl (opValue, functionPtrType->getTargetType (), argValueList, resultValue);
}

bool
OperatorMgr::callOperatorVararg (
	Function* operatorVararg,
	DerivableType* type,
	const Value& value,
	Value* resultValue
	)
{
	Value tmpValue;

	Type* valueType = value.getType ();

	if (valueType->getTypeKind () == TypeKind_DataRef && 
		((DataPtrType*) valueType)->getTargetType () == type)
	{
		return
			unaryOperator (UnOpKind_Addr, value, &tmpValue) &&
			callOperator (operatorVararg, tmpValue, resultValue);
	}
	else
	{
		Variable* tmpVariable = m_module->m_variableMgr.createStackVariable ("tmpStruct", type);

		return
			m_module->m_variableMgr.allocatePrimeInitializeVariable (tmpVariable) &&
			storeDataRef (tmpVariable, value) &&
			unaryOperator (UnOpKind_Addr, tmpVariable, &tmpValue) &&
			callOperator (operatorVararg, tmpValue, resultValue);
	}
}

bool
OperatorMgr::castArgValueList (
	FunctionType* functionType,
	Closure* closure,
	rtl::BoxList <Value>* argValueList
	)
{
	bool result;

	rtl::Array <FunctionArg*> argArray = functionType->getArgArray ();

	size_t formalArgCount = argArray.getCount ();
	size_t actualArgCount = argValueList->getCount ();

	bool isVarArg = (functionType->getFlags () & FunctionTypeFlag_VarArg) != 0;
	bool isCdeclVarArg = (functionType->getCallConv ()->getFlags () & CallConvFlag_Cdecl) != 0;

	size_t commonArgCount;

	if (actualArgCount <= formalArgCount)
	{
		commonArgCount = actualArgCount;
	}
	else if (isVarArg)
	{
		commonArgCount = formalArgCount;
	}
	else
	{
		err::setFormatStringError ("too many arguments in a call to '%s'", functionType->getTypeString ().cc ());
		return false;
	}

	size_t i = 0;
	rtl::BoxIterator <Value> argValueIt = argValueList->getHead ();

	// common for both formal and actual

	for (; i < commonArgCount; argValueIt++, i++)
	{
		Value argValue = *argValueIt;

		FunctionArg* arg = argArray [i];
		if (argValue.isEmpty ())
		{
			rtl::ConstBoxList <Token> initializer = arg->getInitializer ();
			if (initializer.isEmpty ())
			{
				err::setFormatStringError (
					"argument (%d) of '%s' has no default value",
					i + 1,
					functionType->getTypeString ().cc ()
					);
				return false;
			}

			result = evaluateAlias (arg->getItemDecl (), closure, initializer, &argValue);
			if (!result)
				return false;
		}

		Type* formalArgType = arg->getType ();

		result =
			checkCastKind (argValue, formalArgType) &&
			castOperator (argValue, formalArgType, &*argValueIt); // store it in the same list entry

		if (!result)
			return false;
	}

	// default formal arguments

	for (; i < formalArgCount; i++)
	{
		Value argValue;

		FunctionArg* arg = argArray [i];
		rtl::ConstBoxList <Token> initializer = arg->getInitializer ();
		if (initializer.isEmpty ())
		{
			err::setFormatStringError (
				"argument (%d) of '%s' has no default value",
				i + 1,
				functionType->getTypeString ().cc ()
				);
			return false;
		}

		result = evaluateAlias (arg->getItemDecl (), closure, initializer, &argValue);
		if (!result)
			return false;

		Type* formalArgType = arg->getType ();

		result =
			checkCastKind (argValue, formalArgType) &&
			castOperator (&argValue, formalArgType);

		if (!result)
			return false;

		argValueList->insertTail (argValue);
	}

	if (!isVarArg)
		return true;

	// vararg arguments

	if (!isCdeclVarArg)
	{
		err::setFormatStringError ("only 'cdecl' vararg is currently supported");
		return false;
	}

	for (; argValueIt; argValueIt++)
	{
		Value argValue = *argValueIt;
		if (argValue.isEmpty ())
		{
			err::setFormatStringError ("vararg arguments cannot be skipped");
			return false;
		}

		Type* type = prepareOperandType (argValue);
		if (type->getTypeKindFlags () & TypeKindFlag_Derivable)
		{
			DerivableType* derivableType = (DerivableType*) type;

			Function* operatorVararg = derivableType->getOperatorCdeclVararg ();
			if (!operatorVararg)
				operatorVararg = derivableType->getOperatorVararg ();

			if (operatorVararg)
			{
				result = callOperatorVararg (operatorVararg, derivableType, &argValue);
				if (!result)
					return false;
			}
		}

		Type* formalArgType = getCdeclVarArgType (argValue.getType ());
		result = castOperator (argValue, formalArgType, &*argValueIt); // store it in the same list entry
		if (!result)
			return false;
	}

	return true;
}

bool
OperatorMgr::callClosureFunctionPtr (
	const Value& opValue,
	rtl::BoxList <Value>* argValueList,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_FunctionPtr);

	FunctionPtrType* functionPtrType = (FunctionPtrType*) opValue.getType ();
	FunctionType* functionType = functionPtrType->getTargetType ();
	FunctionType* abstractMethodType = functionType->getStdObjectMemberMethodType ();
	FunctionPtrType* functionThinPtrType = abstractMethodType->getFunctionPtrType (FunctionPtrTypeKind_Thin);

	checkNullPtr (opValue);

	Value pfnValue;
	Value ifaceValue;
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 0, NULL, &pfnValue);
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 1, m_module->m_typeMgr.getStdType (StdType_ObjectPtr), &ifaceValue);
	m_module->m_llvmIrBuilder.createBitCast (pfnValue, functionThinPtrType, &pfnValue);

	argValueList->insertHead (ifaceValue);
	return callImpl (pfnValue, abstractMethodType, argValueList, resultValue);
}

bool
OperatorMgr::callImpl (
	const Value& pfnValue,
	FunctionType* functionType,
	rtl::BoxList <Value>* argValueList,
	Value* resultValue
	)
{
	if (functionType->getFlags () & FunctionTypeFlag_Automaton)
	{
		err::setFormatStringError ("cannot call automaton functions");
		return false;
	}

	bool result = castArgValueList (functionType, pfnValue.getClosure (), argValueList);
	if (!result)
		return false;

	if (m_module->m_functionMgr.getScopeLevel ())
	{
		Value scopeLevelValue = m_module->m_namespaceMgr.getCurrentScopeLevel ();
		Variable* variable = m_module->m_variableMgr.getStdVariable (StdVariable_ScopeLevel);
		m_module->m_llvmIrBuilder.createStore (scopeLevelValue, variable);
	}

	functionType->getCallConv ()->call (
		pfnValue,
		functionType,
		argValueList,
		resultValue
		);

	if (resultValue->getType ()->getFlags () & TypeFlag_GcRoot)
		createTmpStackGcRoot (*resultValue);

	if ((functionType->getFlags () & FunctionTypeFlag_Throws) && 
		!m_module->m_controlFlowMgr.isThrowLocked ())
	{
		result = m_module->m_controlFlowMgr.throwIf (*resultValue, functionType);
		if (!result)
			return false;
	}

	return true;
}

void
OperatorMgr::gcPulse ()
{
	Function* function = m_module->m_functionMgr.getStdFunction (StdFunction_GcPulse);
	m_module->m_llvmIrBuilder.createCall (function, function->getType (), NULL);
}

//.............................................................................

} // namespace jnc {

