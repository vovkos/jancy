#include "pch.h"
#include "jnc_CdeclCallConv_gcc64.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

Type*
CdeclCallConv_gcc64::getArgCoerceType (Type* type)
{
	#pragma AXL_TODO ("implement proper coercion for structures with floating point fields")

	return type->getSize () > sizeof (uint64_t) ?
		m_module->m_typeMgr.getStdType (StdType_Int64Int64) :
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Int64);
}

llvm::FunctionType*
CdeclCallConv_gcc64::getLlvmFunctionType (FunctionType* functionType)
{
	Type* returnType = functionType->getReturnType ();
	rtl::Array <FunctionArg*> argArray = functionType->getArgArray ();
	size_t argCount = argArray.getCount ();

	char buffer [256];
	rtl::Array <llvm::Type*> llvmArgTypeArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	llvmArgTypeArray.setCount (argCount);

	size_t j = 0;

	if (returnType->getFlags () & TypeFlag_StructRet)
	{
		if (returnType->getSize () > sizeof (uint64_t) * 2) // return in memory
		{
			argCount++;
			llvmArgTypeArray.setCount (argCount);
			llvmArgTypeArray [0] = returnType->getDataPtrType_c ()->getLlvmType ();
			j = 1;

			returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		}
		else // coerce
		{
			returnType = getArgCoerceType (returnType);
		}
	}

	bool hasCoercedArgs = false;

	for (size_t i = 0; j < argCount; i++, j++)
	{
		Type* type = argArray [i]->getType ();
		if (!(type->getFlags () & TypeFlag_StructRet))
		{
			llvmArgTypeArray [j] = type->getLlvmType ();
		}
		else if (type->getSize () > sizeof (uint64_t) * 2) // pass in memory
		{
			llvmArgTypeArray [j] = type->getDataPtrType_c ()->getLlvmType ();
			hasCoercedArgs = true;
		}
		else // coerce
		{
			llvmArgTypeArray [j] = getArgCoerceType (type)->getLlvmType ();
			hasCoercedArgs = true;
		}
	}

	if (hasCoercedArgs)
		functionType->m_flags |= FunctionTypeFlag_CoercedArgs;

	return llvm::FunctionType::get (
		returnType->getLlvmType (),
		llvm::ArrayRef <llvm::Type*> (llvmArgTypeArray, argCount),
		(functionType->getFlags () & FunctionTypeFlag_VarArg) != 0
		);
}

llvm::Function*
CdeclCallConv_gcc64::createLlvmFunction (
	FunctionType* functionType,
	const char* tag
	)
{
	llvm::Function* llvmFunction = CallConv::createLlvmFunction (functionType, tag);

	size_t j = 1;

	Type* returnType = functionType->getReturnType ();
	if ((returnType->getFlags () & TypeFlag_StructRet) &&
		returnType->getSize () > sizeof (uint64_t) * 2) // return in memory
	{
		llvmFunction->addAttribute (1, llvm::Attribute::StructRet);
		j = 2;
	}

	if (functionType->getFlags () & FunctionTypeFlag_CoercedArgs)
	{
		rtl::Array <FunctionArg*> argArray = functionType->getArgArray ();
		size_t argCount = argArray.getCount ();

		for (size_t i = 0; i < argCount; i++, j++)
		{
			Type* type = argArray [i]->getType ();
			if ((type->getFlags () & TypeFlag_StructRet) &&
				type->getSize () > sizeof (uint64_t) * 2) // pass in memory
			{
				llvmFunction->addAttribute (j, llvm::Attribute::ByVal);
			}
		}
	}

	return llvmFunction;
}

void
CdeclCallConv_gcc64::call (
	const Value& calleeValue,
	FunctionType* functionType,
	rtl::BoxList <Value>* argValueList,
	Value* resultValue
	)
{
	Type* returnType = functionType->getReturnType ();

	if (!(functionType->getFlags () & FunctionTypeFlag_CoercedArgs) &&
		!(returnType->getFlags () & TypeFlag_StructRet))
	{
		CallConv::call (calleeValue, functionType, argValueList, resultValue);
		return;
	}


	Value tmpReturnValue;

	if ((returnType->getFlags () & TypeFlag_StructRet) &&
		returnType->getSize () > sizeof (uint64_t) * 2) // return in memory
	{
		m_module->m_llvmIrBuilder.createAlloca (
			returnType,
			"tmpRetVal",
			returnType->getDataPtrType_c (),
			&tmpReturnValue
			);

		argValueList->insertHead (tmpReturnValue);
	}

	unsigned j = 1;
	char buffer [256];
	rtl::Array <unsigned> byValArgIdxArray (ref::BufKind_Stack, buffer, sizeof (buffer));

	if (functionType->getFlags () & FunctionTypeFlag_CoercedArgs)
	{
		rtl::BoxIterator <Value> it = argValueList->getHead ();
		for (; it; it++, j++)
		{
			Type* type = it->getType ();
			if (!(type->getFlags () & TypeFlag_StructRet))
				continue;

			if (type->getSize () > sizeof (uint64_t) * 2) // pass in memory
			{
				Value tmpValue;
				m_module->m_llvmIrBuilder.createAlloca (type, "tmpArg", NULL, &tmpValue);
				m_module->m_llvmIrBuilder.createStore (*it, tmpValue);

				*it = tmpValue;
				byValArgIdxArray.append (j);
			}
			else // coerce
			{
				Type* coerceType = getArgCoerceType (type);
				m_module->m_operatorMgr.forceCast (it.p (), coerceType);
			}
		}
	}

	llvm::CallInst* llvmInst = m_module->m_llvmIrBuilder.createCall (
		calleeValue,
		this,
		*argValueList,
		tmpReturnValue ?
			m_module->m_typeMgr.getPrimitiveType (TypeKind_Void) :
			returnType,
		resultValue
		);

	size_t byValArgCount = byValArgIdxArray.getCount ();
	for (size_t i = 0; i < byValArgCount; i++)
		llvmInst->addAttribute (byValArgIdxArray [i], llvm::Attribute::ByVal);

	if (returnType->getFlags () & TypeFlag_StructRet)
	{
		if (returnType->getSize () > sizeof (uint64_t) * 2) // return in memory
		{
			llvmInst->addAttribute (1, llvm::Attribute::StructRet);
			m_module->m_llvmIrBuilder.createLoad (tmpReturnValue, returnType, resultValue);
		}
		else // coerce
		{
			Type* coerceType = getArgCoerceType (returnType);
			resultValue->overrideType (coerceType);
			m_module->m_operatorMgr.forceCast (resultValue, returnType);
		}
	}
}

void
CdeclCallConv_gcc64::ret (
	Function* function,
	const Value& value
	)
{
	Type* returnType = function->getType ()->getReturnType ();
	if (!(returnType->getFlags () & TypeFlag_StructRet))
	{
		CallConv::ret (function, value);
		return;
	}

	if (returnType->getSize () > sizeof (uint64_t) * 2) // return in memory
	{
		Value returnPtrValue (function->getLlvmFunction ()->arg_begin());

		m_module->m_llvmIrBuilder.createStore (value, returnPtrValue);
		m_module->m_llvmIrBuilder.createRet ();
	}
	else // coerce
	{
		Type* coerceType = getArgCoerceType (returnType);

		Value tmpValue;
		m_module->m_operatorMgr.forceCast (value, coerceType, &tmpValue);
		m_module->m_llvmIrBuilder.createRet (tmpValue);
	}
}

Value
CdeclCallConv_gcc64::getThisArgValue (Function* function)
{
	ASSERT (function->isMember ());

	Type* returnType = function->getType ()->getReturnType ();
	if (!(returnType->getFlags () & TypeFlag_StructRet) ||
		returnType->getSize () <= sizeof (uint64_t) * 2)
		return CallConv::getThisArgValue (function);

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction ()->arg_begin();
	llvmArg++;
	return Value (llvmArg, function->getThisArgType ());
}

Value
CdeclCallConv_gcc64::getArgValue (
	FunctionArg* arg,
	llvm::Value* llvmValue
	)
{
	Type* type = arg->getType ();

	if (!(type->getFlags () & TypeFlag_StructRet))
		return Value (llvmValue, type);

	Value value;

	if (type->getSize () > sizeof (uint64_t) * 2) // passed in memory
	{
		m_module->m_llvmIrBuilder.createLoad (llvmValue, type, &value);
	}
	else // coerce
	{
		Type* coerceType = getArgCoerceType (type);
		m_module->m_operatorMgr.forceCast (Value (llvmValue, coerceType), type, &value);
	}

	return value;
}

void
CdeclCallConv_gcc64::createArgVariables (Function* function)
{
	Type* returnType = function->getType ()->getReturnType ();

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction ()->arg_begin ();
	if ((returnType->getFlags () & TypeFlag_StructRet) &&
		returnType->getSize () > sizeof (uint64_t) * 2)
		llvmArg++;

	size_t i = 0;
	if (function->isMember ()) // skip this
	{
		i++;
		llvmArg++;
	}

	rtl::Array <FunctionArg*> argArray = function->getType ()->getArgArray ();
	size_t argCount = argArray.getCount ();
	for (; i < argCount; i++, llvmArg++)
	{
		FunctionArg* arg = argArray [i];
		if (!arg->isNamed ())
			continue;

		Variable* argVariable = m_module->m_variableMgr.createArgVariable (arg);
		function->getScope ()->addItem (argVariable);

		Value argValue = CdeclCallConv_gcc64::getArgValue (arg, llvmArg);
		m_module->m_llvmIrBuilder.createStore (argValue, argVariable);
	}
}

//.............................................................................

} // namespace jnc {
