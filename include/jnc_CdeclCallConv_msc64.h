// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#define _JNC_CDECLCALLCONV_MSC64

#include "jnc_CallConv.h"

namespace jnc {

//.............................................................................

class CdeclCallConv_msc64: public CallConv
{
public:
	CdeclCallConv_msc64 ()
	{
		m_callConvKind = CallConvKind_Cdecl_msc64;
	}

	virtual
	llvm::FunctionType*
	getLlvmFunctionType (FunctionType* functionType);

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
};

//.............................................................................

} // namespace jnc {
