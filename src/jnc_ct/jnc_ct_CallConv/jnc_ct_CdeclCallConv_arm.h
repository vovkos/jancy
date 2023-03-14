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

class CdeclCallConv_arm: public CallConv {
protected:
	enum ArgFlag {
		ArgFlag_Coerced = 0x01,
	};

protected:
	Type* m_regType;
	size_t m_coerceSizeLimit;

public:
	CdeclCallConv_arm() {
		m_regType = NULL;
		m_coerceSizeLimit = 0;
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
	void
	call(
		const Value& calleeValue,
		FunctionType* functionType,
		sl::BoxList<Value>* argValueList,
		Value* resultValue
	);

	virtual
	void
	ret(
		Function* function,
		const Value& value
	);

	virtual
	Value
	getArgValue(
		llvm::Value* llvmValue,
		FunctionType* functionType,
		size_t argIdx
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

protected:
	Type*
	getArgCoerceType(Type* type);
};

//..............................................................................

class CdeclCallConv_arm32: public CdeclCallConv_arm {
public:
	CdeclCallConv_arm32();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CdeclCallConv_arm64: public CdeclCallConv_arm {
public:
	CdeclCallConv_arm64();
};

//..............................................................................

} // namespace ct
} // namespace jnc
