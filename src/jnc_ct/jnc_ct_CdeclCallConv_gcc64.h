// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_CallConv.h"

namespace jnc {
namespace ct {


//.............................................................................

class CdeclCallConv_gcc64: public CallConv
{
public:
	enum ArgFlag
	{
		ArgFlag_ByVal   = 0x01,
		ArgFlag_Coerced = 0x02,
	};


public:
	CdeclCallConv_gcc64 ()
	{
		m_callConvKind = CallConvKind_Cdecl_gcc64;
	}

	virtual
	void
	prepareFunctionType (FunctionType* functionType);

	virtual
	llvm::Function*
	createLlvmFunction (
		FunctionType* functionType,
		const sl::StringRef& tag
		);

	virtual
	void
	call (
		const Value& calleeValue,
		FunctionType* functionType,
		sl::BoxList <Value>* argValueList,
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
		llvm::Value* llvmValue,
		FunctionType* functionType,
		size_t argIdx
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

} // namespace ct
} // namespace jnc
