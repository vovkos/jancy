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
#include "jnc_ct_CdeclCallConv_gcc64.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

Type*
CdeclCallConv_gcc64::getArgCoerceType(Type* type) {
	AXL_TODO("implement proper coercion for structures with floating point fields")

	if (type->getSize() <= sizeof(uint64_t))
		return m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64);

	Type* coerceType = m_module->m_typeMgr.getStdType(StdType_Int64Int64);
	bool result = coerceType->ensureLayout();
	ASSERT(result);
	return coerceType;
}

void
CdeclCallConv_gcc64::prepareFunctionType(FunctionType* functionType) {
	Type* returnType = functionType->getReturnType();
	sl::Array<FunctionArg*> argArray = functionType->getArgArray();
	size_t argCount = argArray.getCount();
	size_t argRegCount = 6; // rdi, rsi, rdx, rcx, r8, r9

	char buffer[256];
	sl::Array<llvm::Type*> llvmArgTypeArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	llvmArgTypeArray.setCount(argCount);
	functionType->m_argFlagArray.setCountZeroConstruct(argCount);

	size_t j = 0;

	if (returnType->getFlags() & TypeFlag_StructRet) {
		if (returnType->getSize() > sizeof(uint64_t)* 2) { // return in memory
			argCount++;
			llvmArgTypeArray.setCount(argCount);
			llvmArgTypeArray[0] = returnType->getDataPtrType_c()->getLlvmType();
			j = 1;
			argRegCount--;

			returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		} else { // coerce
			returnType = getArgCoerceType(returnType);
		}
	}

	bool hasByValArgs = false;
	bool hasCoercedArgs = false;

	for (size_t i = 0; j < argCount; i++, j++) {
		Type* type = argArray[i]->getType();
		size_t size = type->getSize();
		size_t regCount = size > sizeof(uint64_t) ? 2 : 1;

		llvm::Type* llvmType;

		if (!(type->getFlags() & TypeFlag_StructRet)) {
			llvmType = type->getLlvmType();
			if (argRegCount)
				argRegCount--;
		} else if (size > sizeof(uint64_t)* 2 || argRegCount < regCount) { // pass on stack
			llvmType = type->getDataPtrType_c()->getLlvmType();
			functionType->m_argFlagArray[i] = ArgFlag_ByVal;
			hasByValArgs = true;
		} else { // coerce
			llvmType = getArgCoerceType(type)->getLlvmType();
			functionType->m_argFlagArray[i] = ArgFlag_Coerced;
			argRegCount -= regCount;
			hasCoercedArgs = true;
		}

		llvmArgTypeArray[j] = llvmType;
	}

	if (hasByValArgs)
		functionType->m_flags |= FunctionTypeFlag_ByValArgs;

	if (hasCoercedArgs)
		functionType->m_flags |= FunctionTypeFlag_CoercedArgs;

	functionType->m_llvmType = llvm::FunctionType::get(
		returnType->getLlvmType(),
		llvm::ArrayRef<llvm::Type*> (llvmArgTypeArray, argCount),
		(functionType->getFlags() & FunctionTypeFlag_VarArg) != 0
	);
}

llvm::Function*
CdeclCallConv_gcc64::createLlvmFunction(
	FunctionType* functionType,
	const sl::StringRef& name
) {
	llvm::Function* llvmFunction = CallConv::createLlvmFunction(functionType, name);

	Type* returnType = functionType->getReturnType();

	size_t j = 1;

	if ((returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > sizeof(uint64_t)* 2) { // return in memory
		llvmFunction->addAttribute(1, llvm::Attribute::StructRet);
		j = 2;
	}

	if (functionType->getFlags() & FunctionTypeFlag_ByValArgs) {
		sl::Array<FunctionArg*> argArray = functionType->getArgArray();
		size_t argCount = argArray.getCount();

		for (size_t i = 0; i < argCount; i++, j++) {
			if (functionType->m_argFlagArray[i] & ArgFlag_ByVal)
				llvmFunction->addAttribute(j, llvm::Attribute::ByVal);
		}
	}

	return llvmFunction;
}

void
CdeclCallConv_gcc64::call(
	const Value& calleeValue,
	FunctionType* functionType,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	Type* returnType = functionType->getReturnType();

	if (!(returnType->getFlags() & TypeFlag_StructRet) &&
		!(functionType->getFlags() & (FunctionTypeFlag_ByValArgs | FunctionTypeFlag_CoercedArgs | FunctionTypeFlag_VarArg))) {
		CallConv::call(calleeValue, functionType, argValueList, resultValue);
		return;
	}

	size_t argRegCount = 6; // rdi, rsi, rdx, rcx, r8, r9

	Value tmpReturnValue;

	if ((returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > sizeof(uint64_t)* 2) { // return in memory
		m_module->m_llvmIrBuilder.createAlloca(
			returnType,
			"tmpRetVal",
			returnType->getDataPtrType_c(),
			&tmpReturnValue
		);

		argValueList->insertHead(tmpReturnValue);
		argRegCount--;
	}

	unsigned j = 1;
	char buffer[256];
	sl::Array<unsigned> byValArgIdxArray(rc::BufKind_Stack, buffer, sizeof(buffer));

	sl::BoxIterator<Value> it = argValueList->getHead();
	for (; it; it++, j++) {
		Type* type = it->getType();
		if (!(type->getFlags() & TypeFlag_StructRet)) {
			if (argRegCount)
				argRegCount--;

			continue;
		}

		size_t size = type->getSize();
		size_t regCount = size > sizeof(uint64_t) ? 2 : 1;

		if (size > sizeof(uint64_t)* 2 || argRegCount < regCount) { // pass on stack
			Value tmpValue;
			m_module->m_llvmIrBuilder.createAlloca(type, "tmpArg", NULL, &tmpValue);
			m_module->m_llvmIrBuilder.createStore(*it, tmpValue);

			*it = tmpValue;
			byValArgIdxArray.append(j);
		} else { // coerce
			Type* coerceType = getArgCoerceType(type);
			m_module->m_operatorMgr.forceCast(it.p(), coerceType);
			argRegCount -= regCount;
		}
	}

	llvm::CallInst* llvmInst = m_module->m_llvmIrBuilder.createCall(
		calleeValue,
		this,
		*argValueList,
		tmpReturnValue ?
			m_module->m_typeMgr.getPrimitiveType(TypeKind_Void) :
			returnType,
		resultValue
	);

	size_t byValArgCount = byValArgIdxArray.getCount();
	for (size_t i = 0; i < byValArgCount; i++)
		llvmInst->addAttribute(byValArgIdxArray[i], llvm::Attribute::ByVal);

	if (returnType->getFlags() & TypeFlag_StructRet) {
		if (returnType->getSize() > sizeof(uint64_t)* 2) { // return in memory
			llvmInst->addAttribute(1, llvm::Attribute::StructRet);
			m_module->m_llvmIrBuilder.createLoad(tmpReturnValue, returnType, resultValue);
		} else { // coerce
			Type* coerceType = getArgCoerceType(returnType);
			resultValue->overrideType(coerceType);
			m_module->m_operatorMgr.forceCast(resultValue, returnType);
		}
	}
}

void
CdeclCallConv_gcc64::ret(
	Function* function,
	const Value& value
) {
	Type* returnType = function->getType()->getReturnType();
	if (!(returnType->getFlags() & TypeFlag_StructRet)) {
		CallConv::ret(function, value);
		return;
	}

	if (returnType->getSize() > sizeof(uint64_t)* 2) { // return in memory
		Value returnPtrValue(&*function->getLlvmFunction()->arg_begin());

		m_module->m_llvmIrBuilder.createStore(value, returnPtrValue);
		m_module->m_llvmIrBuilder.createRet();
	} else { // coerce
		Type* coerceType = getArgCoerceType(returnType);

		Value tmpValue;
		m_module->m_operatorMgr.forceCast(value, coerceType, &tmpValue);
		m_module->m_llvmIrBuilder.createRet(tmpValue);
	}
}

Value
CdeclCallConv_gcc64::getThisArgValue(Function* function) {
	ASSERT(function->isMember());

	FunctionType* functionType = function->getType();
	Type* returnType = functionType->getReturnType();

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	if ((returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > sizeof(uint64_t)* 2)
		llvmArg++;

	return getArgValue(&*llvmArg, functionType, 0);
}

Value
CdeclCallConv_gcc64::getArgValue(
	llvm::Value* llvmValue,
	FunctionType* functionType,
	size_t argIdx
) {
	Type* type = functionType->m_argArray[argIdx]->getType();
	uint_t flags = functionType->m_argFlagArray[argIdx];

	Value value;
	if (flags & ArgFlag_ByVal) {
		m_module->m_llvmIrBuilder.createLoad(llvmValue, type, &value);
	} else if (flags & ArgFlag_Coerced) {
		Type* coerceType = getArgCoerceType(type);
		m_module->m_operatorMgr.forceCast(Value(llvmValue, coerceType), type, &value);
	} else {
		value.setLlvmValue(llvmValue, type);
	}

	return value;
}

void
CdeclCallConv_gcc64::getArgValueArray(
	Function* function,
	Value* argValueArray,
	size_t count
) {
	Type* returnType = function->getType()->getReturnType();
	CallConv::getArgValueArrayImpl(
		function,
		argValueArray,
		count,
		(returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > sizeof(uint64_t) * 2 ? 1 : 0
	);
}

void
CdeclCallConv_gcc64::createArgVariables(Function* function) {
	FunctionType* functionType = function->getType();
	Type* returnType = functionType->getReturnType();

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	if ((returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > sizeof(uint64_t) * 2)
		llvmArg++;

	size_t i = 0;
	if (function->isMember()) { // skip this
		i++;
		llvmArg++;
	}

	sl::Array<FunctionArg*> argArray = functionType->getArgArray();
	size_t argCount = argArray.getCount();

	for (; i < argCount; i++, llvmArg++) {
		FunctionArg* arg = argArray[i];
		if (!arg->isNamed())
			continue;

		Variable* argVariable = m_module->m_variableMgr.createArgVariable(arg, i);
		function->getScope()->addItem(argVariable);

		Value argValue = getArgValue(&*llvmArg, functionType, i);
		m_module->m_llvmIrBuilder.createStore(argValue, argVariable);
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
