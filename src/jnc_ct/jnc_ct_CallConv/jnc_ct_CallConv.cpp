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
#include "jnc_ct_CallConv.h"
#include "jnc_ct_Decl.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

CallConv::CallConv() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_callConvKind = CallConvKind_Undefined;
}

void
CallConv::prepareFunctionType(FunctionType* functionType) {
	sl::Array<FunctionArg*> argArray = functionType->getArgArray();
	size_t argCount = argArray.getCount();

	char buffer[256];
	sl::Array<llvm::Type*> llvmArgTypeArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	llvmArgTypeArray.setCount(argCount);
	sl::Array<llvm::Type*>::Rwi rwi = llvmArgTypeArray;

	for (size_t i = 0; i < argCount; i++)
		rwi[i] = argArray[i]->getType()->getLlvmType();

	functionType->m_llvmType = llvm::FunctionType::get(
		functionType->getReturnType()->getLlvmType(),
		llvm::ArrayRef<llvm::Type*>(llvmArgTypeArray, argCount),
		(functionType->getFlags() & FunctionTypeFlag_VarArg) != 0
	);
}

llvm::Function*
CallConv::createLlvmFunction(
	FunctionType* functionType,
	const sl::StringRef& name
) {
	llvm::FunctionType* llvmType = (llvm::FunctionType*)functionType->getLlvmType();
	llvm::Function* llvmFunction = llvm::Function::Create(
		llvmType,
		llvm::Function::ExternalLinkage,
		name >> toLlvm,
		m_module->getLlvmModule()
	);

	llvm::CallingConv::ID llvmCallConv = getLlvmCallConv();
	if (llvmCallConv)
		llvmFunction->setCallingConv(llvmCallConv);

	return llvmFunction;
}

llvm::CallInst*
CallConv::call(
	const Value& calleeValue,
	FunctionType* functionType,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	return m_module->m_llvmIrBuilder.createCall(
		calleeValue,
		functionType,
		*argValueList,
		resultValue
	);
}

llvm::ReturnInst*
CallConv::ret(
	Function* function,
	const Value& value
) {
	return m_module->m_llvmIrBuilder.createRet(value);
}

Value
CallConv::getThisArgValue(Function* function) {
	ASSERT(function->isMember());

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	return Value(&*llvmArg, function->getThisArgType());
}

Value
CallConv::getArgValue(
	llvm::Value* llvmValue,
	FunctionType* functionType,
	size_t argIdx
) {
	FunctionArg* arg = functionType->m_argArray[argIdx];
	return Value(llvmValue, arg->getType());
}

void
CallConv::getArgValueArrayImpl(
	Function* function,
	Value* argValueArray,
	size_t argCount,
	size_t baseLlvmArgIdx
) {
	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	for (size_t i = 0; i < baseLlvmArgIdx; i++)
		llvmArg++;

	FunctionType* functionType = function->getType();
	for (size_t i = 0; i < argCount; i++, llvmArg++) {
		Value argValue = getArgValue(&*llvmArg, functionType, i);
		argValueArray[i] = argValue;
	}
}

void
CallConv::createArgVariablesImpl(
	Function* function,
	size_t baseLlvmArgIdx
) {
	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	for (size_t i = 0; i < baseLlvmArgIdx; i++)
		llvmArg++;

	size_t i = 0;
	if (function->isMember()) { // skip this
		i++;
		llvmArg++;
	}

	sl::Array<FunctionArg*> argArray = function->getType()->getArgArray();
	size_t argCount = argArray.getCount();
	for (; i < argCount; i++, llvmArg++) {
		FunctionArg* arg = argArray[i];
		if (!arg->isNamed())
			continue;

		llvm::Value* llvmArgValue = &*llvmArg;

		Variable* argVariable = m_module->m_variableMgr.createArgVariable(arg, i);
		function->getScope()->addItem(argVariable);

		Value argValue(llvmArgValue, arg->getType());
		m_module->m_llvmIrBuilder.createStore(argValue, argVariable);
	}
}

void
CallConv::addIntExtAttributes(
	llvm::CallInst* llvmInst,
	const sl::BoxList<Value>& argValueList
) {
	sl::ConstBoxIterator<Value> it = argValueList.getHead();
	for (size_t i = 1; it; it++, i++) {
		Type* type = it->getType();
		if (!(type->getTypeKindFlags() & TypeKindFlag_Integer) || type->getSize() >= sizeof(int))
			continue;

		if (type->getTypeKind() == TypeKind_Enum)
			type = ((EnumType*)type)->getBaseType();

		llvm::Attribute::AttrKind llvmAttrKind = (type->getTypeKindFlags() & TypeKindFlag_Unsigned) ?
			llvm::Attribute::ZExt :
			llvm::Attribute::SExt;

#if (LLVM_VERSION_MAJOR < 14)
		llvmInst->addAttribute(i, llvmAttrKind);
#else
		llvmInst->addAttributeAtIndex(i, llvmAttrKind);
#endif
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
