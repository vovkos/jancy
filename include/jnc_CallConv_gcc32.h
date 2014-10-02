// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#define _JNC_CALLCONV_GCC32

#include "jnc_CallConv.h"

namespace jnc {

//.............................................................................

class CallConv_gcc32: public CallConv
{
public:
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
	getThisArgValue (Function* function);

	virtual
	void
	createArgVariables (Function* function);
};

//.............................................................................

class CdeclCallConv_gcc32: public CallConv_gcc32
{
public:
	CdeclCallConv_gcc32 ()
	{
		m_callConvKind = CallConvKind_Cdecl_gcc32;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class StdcallCallConv_gcc32: public CallConv_gcc32
{
public:
	StdcallCallConv_gcc32 ()
	{
		m_callConvKind = CallConvKind_Stdcall_gcc32;
	}
};

//.............................................................................

} // namespace jnc {
