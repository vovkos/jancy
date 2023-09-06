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
#include "jnc_ct_CdeclCallConv_msc64.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

llvm::FunctionType*
CdeclCallConv_msc64::prepareFunctionType(FunctionType* functionType) {
	Type* returnType = functionType->getReturnType();
	sl::Array<FunctionArg*> argArray = functionType->getArgArray();
	size_t argCount = argArray.getCount();

	char buffer[256];
	sl::Array<llvm::Type*> llvmArgTypeArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	llvmArgTypeArray.setCount(argCount);

	size_t j = 0;

	if (returnType->getFlags() & TypeFlag_StructRet) {
		if (returnType->getSize() <= sizeof(uint64_t)) {
			returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64);
		} else {
			returnType = returnType->getDataPtrType_c();

			argCount++;
			llvmArgTypeArray.setCount(argCount);
			llvmArgTypeArray[0] = returnType->getLlvmType();
			j = 1;
		}
	}

	bool hasCoercedArgs = false;

	for (size_t i = 0; j < argCount; i++, j++) {
		Type* type = argArray[i]->getType();
		if (!(type->getFlags() & TypeFlag_StructRet)) {
			llvmArgTypeArray[j] = type->getLlvmType();
		} else if (type->getSize() <= sizeof(uint64_t)) {
			llvmArgTypeArray[j] = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64)->getLlvmType();
			hasCoercedArgs = true;
		} else {
			llvmArgTypeArray[j] = type->getDataPtrType_c()->getLlvmType();
			hasCoercedArgs = true;
		}
	}

	if (hasCoercedArgs)
		functionType->m_flags |= FunctionTypeFlag_CoercedArgs;

	functionType->m_llvmType = llvm::FunctionType::get(
		returnType->getLlvmType(),
		llvm::ArrayRef<llvm::Type*> (llvmArgTypeArray, argCount),
		(functionType->getFlags() & FunctionTypeFlag_VarArg) != 0
	);

	return (llvm::FunctionType*)functionType->m_llvmType;
}

llvm::CallInst*
CdeclCallConv_msc64::call(
	const Value& calleeValue,
	FunctionType* functionType,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	Type* returnType = functionType->getReturnType();

	if (!(functionType->getFlags() & FunctionTypeFlag_CoercedArgs) &&
		!(returnType->getFlags() & TypeFlag_StructRet)
	)
		return CallConv::call(calleeValue, functionType, argValueList, resultValue);

	Value tmpReturnValue;

	if (returnType->getFlags() & TypeFlag_StructRet) {
		m_module->m_llvmIrBuilder.createAlloca(returnType, returnType->getDataPtrType_c(), &tmpReturnValue);
		if (returnType->getSize() > sizeof(uint64_t))
			argValueList->insertHead(tmpReturnValue);
	}

	if (functionType->getFlags() & FunctionTypeFlag_CoercedArgs) {
		sl::BoxIterator<Value> it = argValueList->getHead();
		for (; it; it++) {
			Type* type = it->getType();
			if (!(type->getFlags() & TypeFlag_StructRet))
				continue;

			if (type->getSize() > sizeof(uint64_t)) {
				Value tmpValue;
				m_module->m_llvmIrBuilder.createAlloca(type, NULL, &tmpValue);
				m_module->m_llvmIrBuilder.createStore(*it, tmpValue);
				*it = tmpValue;
			} else {
				Type* coerceType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64);

				Value tmpValue, tmpValue2;
				m_module->m_llvmIrBuilder.createAlloca(coerceType, NULL, &tmpValue);
				m_module->m_llvmIrBuilder.createBitCast(tmpValue, type->getDataPtrType_c(), &tmpValue2);
				m_module->m_llvmIrBuilder.createStore(*it, tmpValue2);
				m_module->m_llvmIrBuilder.createLoad(tmpValue, NULL, &tmpValue);
				*it = tmpValue;
			}
		}
	}

	llvm::CallInst* llvmInst = m_module->m_llvmIrBuilder.createCall(
		calleeValue,
		functionType,
		*argValueList,
		resultValue
	);

	if (returnType->getFlags() & TypeFlag_StructRet) {
		if (returnType->getSize() <= sizeof(uint64_t)) {
			Value tmpValue;
			Type* type = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64)->getDataPtrType_c();
			m_module->m_llvmIrBuilder.createBitCast(tmpReturnValue, type, &tmpValue);
			m_module->m_llvmIrBuilder.createStore(*resultValue, tmpValue);
		}

		m_module->m_llvmIrBuilder.createLoad(tmpReturnValue, returnType, resultValue);
	}

	return llvmInst;
}

llvm::ReturnInst*
CdeclCallConv_msc64::ret(
	Function* function,
	const Value& value
) {
	Type* returnType = function->getType()->getReturnType();
	if (!(returnType->getFlags() & TypeFlag_StructRet))
		return CallConv::ret(function, value);

	if (returnType->getSize() > sizeof(uint64_t)) {
		Value returnPtrValue(&*function->getLlvmFunction()->arg_begin());

		m_module->m_llvmIrBuilder.createStore(value, returnPtrValue);
		return m_module->m_llvmIrBuilder.createRet(returnPtrValue);
	} else {
		Type* type = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64);

		Value tmpValue, tmpValue2;
		m_module->m_llvmIrBuilder.createAlloca(type, NULL, &tmpValue);
		m_module->m_llvmIrBuilder.createBitCast(tmpValue, returnType->getDataPtrType_c(), &tmpValue2);
		m_module->m_llvmIrBuilder.createStore(value, tmpValue2);
		m_module->m_llvmIrBuilder.createLoad(tmpValue, NULL, &tmpValue);
		return m_module->m_llvmIrBuilder.createRet(tmpValue);
	}
}

Value
CdeclCallConv_msc64::getThisArgValue(Function* function) {
	ASSERT(function->isMember());

	FunctionType* functionType = function->getType();
	Type* returnType = functionType->getReturnType();

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	if ((returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > sizeof(uint64_t))
		llvmArg++;

	return getArgValue(&*llvmArg, functionType, 0);
}

Value
CdeclCallConv_msc64::getArgValue(
	llvm::Value* llvmValue,
	FunctionType* functionType,
	size_t argIdx
) {
	Type* type = functionType->m_argArray[argIdx]->getType();

	if (!(type->getFlags() & TypeFlag_StructRet))
		return Value(llvmValue, type);

	if (type->getSize() > sizeof(uint64_t)) {
		Value value;
		m_module->m_llvmIrBuilder.createLoad(llvmValue, type, &value);
		return value;
	}

	Type* int64Type = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64);

	Value value;
	m_module->m_llvmIrBuilder.createAlloca(int64Type, NULL, &value);
	m_module->m_llvmIrBuilder.createStore(llvmValue, value);
	m_module->m_llvmIrBuilder.createBitCast(value, type->getDataPtrType_c(), &value);
	m_module->m_llvmIrBuilder.createLoad(value, type, &value);

	return value;
}

void
CdeclCallConv_msc64::getArgValueArray(
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
		returnType->getSize() > sizeof(uint64_t) ? 1 : 0
	);
}

void
CdeclCallConv_msc64::createArgVariables(Function* function) {
	Type* returnType = function->getType()->getReturnType();

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	if ((returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > sizeof(uint64_t))
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

		Type* type = arg->getType();
		llvm::Value* llvmArgValue = &*llvmArg;

		if (!(type->getFlags() & TypeFlag_StructRet)) {
			Variable* argVariable = m_module->m_variableMgr.createArgVariable(arg, i);
			function->getScope()->addItem(argVariable);

			m_module->m_llvmIrBuilder.createStore(llvmArgValue, argVariable);
		} else {
			Variable* argVariable = m_module->m_variableMgr.createArgVariable(arg, i);
			function->getScope()->addItem(argVariable);

			if (type->getSize() > sizeof(uint64_t)) {
				Value tmpValue;
				m_module->m_llvmIrBuilder.createLoad(llvmArgValue, NULL, &tmpValue);
				m_module->m_llvmIrBuilder.createStore(tmpValue, argVariable);
			} else {
				Type* int64Type = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64);

				Value tmpValue;
				m_module->m_llvmIrBuilder.createAlloca(int64Type, NULL, &tmpValue);
				m_module->m_llvmIrBuilder.createStore(llvmArgValue, tmpValue);

				m_module->m_llvmIrBuilder.createBitCast(tmpValue, type->getDataPtrType_c(), &tmpValue);
				m_module->m_llvmIrBuilder.createLoad(tmpValue, NULL, &tmpValue);
				m_module->m_llvmIrBuilder.createStore(tmpValue, argVariable);
			}
		}
	}
}

//..............................................................................

} // namespace ct
} // namespace jnc
