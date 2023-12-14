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
#include "jnc_ct_CallConv_gcc32.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

void
CallConv_gcc32::prepareFunctionType(FunctionType* functionType) {
	Type* returnType = functionType->getReturnType();
	if (!(returnType->getFlags() & TypeFlag_StructRet)) {
		CallConv::prepareFunctionType(functionType);
		return;
	}

	sl::Array<FunctionArg*> argArray = functionType->getArgArray();
	size_t argCount = argArray.getCount() + 1;

	char buffer[256];
	sl::Array<llvm::Type*> llvmArgTypeArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	llvmArgTypeArray.setCount(argCount);

	llvmArgTypeArray[0] = returnType->getDataPtrType_c()->getLlvmType();

	for (size_t i = 0, j = 1; j < argCount; i++, j++)
		llvmArgTypeArray[j] = argArray[i]->getType()->getLlvmType();

	functionType->m_llvmType = llvm::FunctionType::get(
		m_module->m_typeMgr.getPrimitiveType(TypeKind_Void)->getLlvmType(),
		llvm::ArrayRef<llvm::Type*> (llvmArgTypeArray, argCount),
		(functionType->getFlags() & FunctionTypeFlag_VarArg) != 0
	);
}

llvm::Function*
CallConv_gcc32::createLlvmFunction(
	FunctionType* functionType,
	const sl::StringRef& name
) {
	llvm::Function* llvmFunction = CallConv::createLlvmFunction(functionType, name);

	Type* returnType = functionType->getReturnType();
	if (returnType->getFlags() & TypeFlag_StructRet)
		m_module->m_llvmIrBuilder.addTypedAttribute(llvmFunction, 1, llvm::Attribute::StructRet, returnType);

	return llvmFunction;
}

llvm::CallInst*
CallConv_gcc32::call(
	const Value& calleeValue,
	FunctionType* functionType,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	Type* returnType = functionType->getReturnType();
	if (!(returnType->getFlags() & TypeFlag_StructRet))
		return CallConv::call(calleeValue, functionType, argValueList, resultValue);

	Value tmpReturnValue;
	m_module->m_llvmIrBuilder.createAlloca(returnType, returnType->getDataPtrType_c(), &tmpReturnValue);
	argValueList->insertHead(tmpReturnValue);

	llvm::CallInst* llvmInst = m_module->m_llvmIrBuilder.createCall(
		calleeValue,
		functionType,
		*argValueList,
		m_module->m_typeMgr.getPrimitiveType(TypeKind_Void),
		NULL
	);

	m_module->m_llvmIrBuilder.addTypedAttribute(llvmInst, 1, llvm::Attribute::StructRet, returnType);
	m_module->m_llvmIrBuilder.createLoad(tmpReturnValue, returnType, resultValue);
	return llvmInst;
}

llvm::ReturnInst*
CallConv_gcc32::ret(
	Function* function,
	const Value& value
) {
	Type* returnType = function->getType()->getReturnType();
	if (!(returnType->getFlags() & TypeFlag_StructRet))
		return CallConv::ret(function, value);

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();

	Value returnPtrValue;
	returnPtrValue.setLlvmValue(&*llvmArg, returnType->getDataPtrType_c());
	m_module->m_llvmIrBuilder.createStore(value, returnPtrValue);
	return m_module->m_llvmIrBuilder.createRet();
}

Value
CallConv_gcc32::getThisArgValue(Function* function) {
	ASSERT(function->isMember());

	Type* returnType = function->getType()->getReturnType();
	if (!(returnType->getFlags() & TypeFlag_StructRet))
		return CallConv::getThisArgValue(function);

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	llvmArg++;
	return Value(&*llvmArg, function->getThisArgType());
}

void
CallConv_gcc32::getArgValueArray(
	Function* function,
	Value* argValueArray,
	size_t count
) {
	Type* returnType = function->getType()->getReturnType();
	CallConv::getArgValueArrayImpl(
		function,
		argValueArray,
		count,
		(returnType->getFlags() & TypeFlag_StructRet) ? 1 : 0
	);
}

void
CallConv_gcc32::createArgVariables(Function* function) {
	Type* returnType = function->getType()->getReturnType();
	CallConv::createArgVariablesImpl(
		function,
		(returnType->getFlags() & TypeFlag_StructRet) ? 1 : 0
	);
}

//..............................................................................

} // namespace ct
} // namespace jnc
