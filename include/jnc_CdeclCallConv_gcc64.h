// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CallConv.h"

namespace jnc {

//.............................................................................

class CdeclCallConv_gcc64: public CallConv
{
public:
	CdeclCallConv_gcc64 ()
	{
		m_callConvKind = CallConvKind_Cdecl_gcc64;
	}

	virtual
	llvm::FunctionType*
	getLlvmFunctionType (FunctionType* functionType);

	virtual
	llvm::Function*
	createLlvmFunction (
		FunctionType* functionType,
		const char* tag
		);

	virtual
	void
	call (
		const Value& calleeValue,
		FunctionType* functionType,
		rtl::BoxList <Value>* argValueList,
		Value* resultValue
		);

	virtual
	void
	ret (
		Function* function,
		const Value& value
		);

	virtual
	Value
	getArgValue (
		FunctionArg* arg,
		llvm::Value* llvmValue
		);

	virtual
	Value
	getThisArgValue (Function* function);

	virtual
	void
	createArgVariables (Function* function);

protected:
	Type*
	getArgCoerceType (Type* type);
};

//.............................................................................

} // namespace jnc {
