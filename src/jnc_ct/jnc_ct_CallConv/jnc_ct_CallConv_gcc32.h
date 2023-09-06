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

#include "jnc_ct_CallConv.h"

namespace jnc {
namespace ct {

//..............................................................................

class CallConv_gcc32: public CallConv {
public:
	virtual
	llvm::FunctionType*
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
	void
	getArgValueArray(
		Function* function,
		Value* argValueArray,
		size_t count
	);

	virtual
	void
	createArgVariables(Function* function);
};

//..............................................................................

class CdeclCallConv_gcc32: public CallConv_gcc32 {
public:
	CdeclCallConv_gcc32() {
		m_callConvKind = CallConvKind_Cdecl_gcc32;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class StdcallCallConv_gcc32: public CallConv_gcc32 {
public:
	StdcallCallConv_gcc32() {
		m_callConvKind = CallConvKind_Stdcall_gcc32;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
