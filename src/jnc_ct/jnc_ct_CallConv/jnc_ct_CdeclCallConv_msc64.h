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

class CdeclCallConv_msc64: public CallConv
{
public:
	CdeclCallConv_msc64()
	{
		m_callConvKind = CallConvKind_Cdecl_msc64;
	}

	virtual
	void
	prepareFunctionType(FunctionType* functionType);

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
	createArgVariables(Function* function);
};

//..............................................................................

} // namespace ct
} // namespace jnc
