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

#pragma once

#include "jnc_ct_CallConvKind.h"
#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class FunctionType;

//..............................................................................

class CallConv {
protected:
	Module* m_module;
	CallConvKind m_callConvKind;

public:
	CallConv();

	CallConvKind
	getCallConvKind() {
		return m_callConvKind;
	}

	uint_t
	getFlags() {
		return ct::getCallConvFlags(m_callConvKind);
	}

	llvm::CallingConv::ID
	getLlvmCallConv() {
		return ct::getLlvmCallConv(m_callConvKind);
	}

	const char*
	getCallConvString() {
		return ct::getCallConvString(m_callConvKind);
	}

	const char*
	getCallConvDisplayString() {
		return ct::getCallConvDisplayString(m_callConvKind);
	}

	bool
	isDefault() {
		return m_callConvKind == CallConvKind_Default;
	}

	virtual
	void
	prepareFunctionType(FunctionType* functionType);

	virtual
	llvm::Function*
	createLlvmFunction(
		FunctionType* functionType,
		const sl::StringRef& name
	);

	virtual
	llvm::CallInst*
	call(
		const Value& calleeValue,
		FunctionType* functionType,
		sl::BoxList<Value>* argValueList,
		Value* resultValue
	);

	virtual
	llvm::ReturnInst*
	ret(
		Function* function,
		const Value& value
	);

	virtual
	Value
	getThisArgValue(Function* function);

	virtual
	Value
	getArgValue(
		llvm::Value* llvmValue,
		FunctionType* functionType,
		size_t argIdx
	);

	virtual
	void
	getArgValueArray(
		Function* function,
		Value* argValueArray,
		size_t count
	) {
		getArgValueArrayImpl(function, argValueArray, count, 0);
	}

	virtual
	void
	createArgVariables(Function* function) {
		createArgVariablesImpl(function, 0);
	}

	static
	void
	addIntExtAttributes(
		llvm::CallInst* llvmInst,
		const sl::BoxList<Value>& argValueList
	);

protected:
	void
	getArgValueArrayImpl(
		Function* function,
		Value* argValueArray,
		size_t argCount,
		size_t baseLlvmArgIdx = 0
	);

	void
	createArgVariablesImpl(
		Function* function,
		size_t baseLlvmArgIdx = 0
	);
};

//..............................................................................

} // namespace ct
} // namespace jnc
