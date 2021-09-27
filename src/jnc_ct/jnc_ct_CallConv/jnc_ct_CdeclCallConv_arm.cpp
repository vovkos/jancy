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
#include "jnc_ct_CdeclCallConv_arm.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"

namespace jnc {
namespace ct {

//..............................................................................

Type*
CdeclCallConv_arm::getArgCoerceType(Type* type) {
	size_t size = type->getSize();
	size_t atomSize = m_regType->getSize();

	if (size <= atomSize)
		return m_regType;

	Type* atomType;

	if (type->getAlignment() <= atomSize) {
		atomType = m_regType;
	} else {
		atomType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64);
		atomSize = sizeof(int64_t);
	}

	size_t atomCount = size / atomSize;
	if (size % atomSize)
		atomCount++;

	Type* coerceType = atomType->getArrayType(atomCount);
	bool result = coerceType->ensureLayout();
	ASSERT(result);
	return coerceType;
}

void
CdeclCallConv_arm::prepareFunctionType(FunctionType* functionType) {
	size_t regSize = m_regType->getSize();

	Type* returnType = functionType->getReturnType();
	sl::Array<FunctionArg*> argArray = functionType->getArgArray();
	size_t argCount = argArray.getCount();

	char buffer[256];
	sl::Array<llvm::Type*> llvmArgTypeArray(rc::BufKind_Stack, buffer, sizeof(buffer));
	llvmArgTypeArray.setCount(argCount);
	functionType->m_argFlagArray.setCountZeroConstruct(argCount);

	size_t j = 0;

	if (returnType->getFlags() & TypeFlag_StructRet) {
		if (returnType->getSize() > regSize) { // return in memory
			argCount++;
			llvmArgTypeArray.setCount(argCount);
			llvmArgTypeArray[0] = returnType->getDataPtrType_c()->getLlvmType();
			j = 1;

			returnType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
		} else { // coerce
			returnType = getArgCoerceType(returnType);
		}
	}

	bool hasCoercedArgs = false;

	for (size_t i = 0; j < argCount; i++, j++) {
		Type* type = argArray[i]->getType();
		llvm::Type* llvmType;

		if (!(type->getFlags() & TypeFlag_StructRet)) {
			llvmType = type->getLlvmType();
		} else { // coerce
			llvmType = getArgCoerceType(type)->getLlvmType();
			functionType->m_argFlagArray[i] = ArgFlag_Coerced;
			hasCoercedArgs = true;
		}

		llvmArgTypeArray[j] = llvmType;
	}

	if (hasCoercedArgs)
		functionType->m_flags |= FunctionTypeFlag_CoercedArgs;

	functionType->m_llvmType = llvm::FunctionType::get(
		returnType->getLlvmType(),
		llvm::ArrayRef<llvm::Type*> (llvmArgTypeArray, argCount),
		(functionType->getFlags() & FunctionTypeFlag_VarArg) != 0
	);
}

llvm::Function*
CdeclCallConv_arm::createLlvmFunction(
	FunctionType* functionType,
	const sl::StringRef& name
) {
	llvm::Function* llvmFunction = CallConv::createLlvmFunction(functionType, name);

	Type* returnType = functionType->getReturnType();

	if ((returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > m_regType->getSize()) // return in memory
		llvmFunction->addAttribute(1, llvm::Attribute::StructRet);

	return llvmFunction;
}

void
CdeclCallConv_arm::call(
	const Value& calleeValue,
	FunctionType* functionType,
	sl::BoxList<Value>* argValueList,
	Value* resultValue
) {
	size_t regSize = m_regType->getSize();
	Type* returnType = functionType->getReturnType();

	if (!(returnType->getFlags() & TypeFlag_StructRet) &&
		!(functionType->getFlags() & (FunctionTypeFlag_CoercedArgs | FunctionTypeFlag_VarArg))) {
		CallConv::call(calleeValue, functionType, argValueList, resultValue);
		return;
	}

	Value tmpReturnValue;

	if ((returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > regSize) { // return in memory
		m_module->m_llvmIrBuilder.createAlloca(
			returnType,
			"tmpRetVal",
			returnType->getDataPtrType_c(),
			&tmpReturnValue
		);

		argValueList->insertHead(tmpReturnValue);
	}

	unsigned j = 1;

	sl::BoxIterator<Value> it = argValueList->getHead();
	for (; it; it++, j++) {
		Type* type = it->getType();
		if (!(type->getFlags() & TypeFlag_StructRet))
			continue;

		Type* coerceType = getArgCoerceType(type);
		m_module->m_operatorMgr.forceCast(it.p(), coerceType);
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

	if (returnType->getFlags() & TypeFlag_StructRet) {
		if (returnType->getSize() > regSize) { // return in memory
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
CdeclCallConv_arm::ret(
	Function* function,
	const Value& value
) {
	Type* returnType = function->getType()->getReturnType();
	if (!(returnType->getFlags() & TypeFlag_StructRet)) {
		CallConv::ret(function, value);
		return;
	}

	if (returnType->getSize() > m_regType->getSize()) { // return in memory
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
CdeclCallConv_arm::getThisArgValue(Function* function) {
	ASSERT(function->isMember());

	FunctionType* functionType = function->getType();
	Type* returnType = functionType->getReturnType();

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	if ((returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > m_regType->getSize())
		llvmArg++;

	return getArgValue(&*llvmArg, functionType, 0);
}

Value
CdeclCallConv_arm::getArgValue(
	llvm::Value* llvmValue,
	FunctionType* functionType,
	size_t argIdx
) {
	Type* type = functionType->m_argArray[argIdx]->getType();
	uint_t flags = functionType->m_argFlagArray[argIdx];

	Value value;
	if (flags & ArgFlag_Coerced) {
		Type* coerceType = getArgCoerceType(type);
		m_module->m_operatorMgr.forceCast(Value(llvmValue, coerceType), type, &value);
	} else {
		value.setLlvmValue(llvmValue, type);
	}

	return value;
}

void
CdeclCallConv_arm::getArgValueArray(
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
		returnType->getSize() > m_regType->getSize() ? 1 : 0
	);
}

void
CdeclCallConv_arm::createArgVariables(Function* function) {
	FunctionType* functionType = function->getType();
	Type* returnType = functionType->getReturnType();

	llvm::Function::arg_iterator llvmArg = function->getLlvmFunction()->arg_begin();
	if ((returnType->getFlags() & TypeFlag_StructRet) &&
		returnType->getSize() > m_regType->getSize())
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

CdeclCallConv_arm32::CdeclCallConv_arm32() {
	m_callConvKind = CallConvKind_Cdecl_arm32;
	m_regType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int32);
}

CdeclCallConv_arm64::CdeclCallConv_arm64() {
	m_callConvKind = CallConvKind_Cdecl_arm32;
	m_regType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int64);
}

//..............................................................................

} // namespace ct
} // namespace jnc
