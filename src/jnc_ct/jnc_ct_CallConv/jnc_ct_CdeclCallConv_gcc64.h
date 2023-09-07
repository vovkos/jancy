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

class CdeclCallConv_gcc64: public CallConv {
protected:
	enum ArgFlag {
		ArgFlag_ByVal   = 0x01,
		ArgFlag_Coerced = 0x02,
	};

public:
	CdeclCallConv_gcc64() {
		m_callConvKind = CallConvKind_Cdecl_gcc64;
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
	);

	virtual
	void
	createArgVariables(Function* function);

protected:
	Type*
	getArgCoerceType(Type* type);
};

//..............................................................................

} // namespace ct
} // namespace jnc
