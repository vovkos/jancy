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

#include "jnc_ct_Value.h"
#include "jnc_ct_FunctionArg.h"

namespace jnc {
namespace ct {

class FunctionType;

//..............................................................................

// jnccall is basically cdecl with the following 2 differences:
// - arrays are passed by value (like if it were wrapped in a struct)
// - varargs are wrapped into variants and prepended with vararg count

enum CallConvKind
{
	CallConvKind_Undefined = 0,
	CallConvKind_Jnccall_msc32,
	CallConvKind_Jnccall_msc64,
	CallConvKind_Jnccall_gcc32,
	CallConvKind_Jnccall_gcc64,
	CallConvKind_Jnccall_arm32,
	CallConvKind_Jnccall_arm64,
	CallConvKind_Cdecl_msc32,
	CallConvKind_Cdecl_msc64,
	CallConvKind_Cdecl_gcc32,
	CallConvKind_Cdecl_gcc64,
	CallConvKind_Cdecl_arm32,
	CallConvKind_Cdecl_arm64,
	CallConvKind_Stdcall_msc32,
	CallConvKind_Stdcall_gcc32,
	CallConvKind_Thiscall_msc32,
	CallConvKind__Count,

#if (_JNC_CPP_MSC)
#	if (_JNC_CPU_AMD64)
	CallConvKind_Jnccall  = CallConvKind_Jnccall_msc64,
	CallConvKind_Cdecl    = CallConvKind_Cdecl_msc64,
#	else
	CallConvKind_Jnccall  = CallConvKind_Jnccall_msc32,
	CallConvKind_Cdecl    = CallConvKind_Cdecl_msc32,
	CallConvKind_Stdcall  = CallConvKind_Stdcall_msc32,
	CallConvKind_Thiscall = CallConvKind_Thiscall_msc32,
#	endif
#else
#	if (_JNC_CPU_ARM64)
	CallConvKind_Jnccall  = CallConvKind_Jnccall_arm64,
	CallConvKind_Cdecl    = CallConvKind_Cdecl_arm64,
#	elif (_JNC_CPU_ARM32)
	CallConvKind_Jnccall  = CallConvKind_Jnccall_arm32,
	CallConvKind_Cdecl    = CallConvKind_Cdecl_arm32,
#	elif (_JNC_CPU_AMD64)
	CallConvKind_Jnccall  = CallConvKind_Jnccall_gcc64,
	CallConvKind_Cdecl    = CallConvKind_Cdecl_gcc64,
#	else
	CallConvKind_Jnccall  = CallConvKind_Jnccall_gcc32,
	CallConvKind_Cdecl    = CallConvKind_Cdecl_gcc32,
	CallConvKind_Stdcall  = CallConvKind_Stdcall_gcc32,
	CallConvKind_Thiscall = CallConvKind_Cdecl_gcc32,
#	endif
#endif

	CallConvKind_Default = CallConvKind_Jnccall,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum CallConvFlag
{
	// vararg

	CallConvFlag_NoVarArg     = 0x0001,
	CallConvFlag_UnsafeVarArg = 0x0002,

	// family

	CallConvFlag_Jnccall      = 0x0010,
	CallConvFlag_Cdecl        = 0x0020,
	CallConvFlag_Stdcall      = 0x0040,

	// compiler

	CallConvFlag_Msc          = 0x0100,
	CallConvFlag_Gcc          = 0x0200,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::CallingConv::ID
getLlvmCallConv (CallConvKind callConvKind);

uint_t
getCallConvFlags (CallConvKind callConvKind);

const char*
getCallConvString (CallConvKind callConvKind);

const char*
getCallConvDisplayString (CallConvKind callConvKind);

const char*
getCallConvSignature (CallConvKind callConvKind);

CallConvKind
getCallConvKindFromModifiers (uint_t modifiers);

//..............................................................................

class CallConv
{
protected:
	Module* m_module;
	CallConvKind m_callConvKind;

public:
	CallConv ();

	CallConvKind
	getCallConvKind ()
	{
		return m_callConvKind;
	}

	uint_t
	getFlags ()
	{
		return ct::getCallConvFlags (m_callConvKind);
	}

	llvm::CallingConv::ID
	getLlvmCallConv ()
	{
		return ct::getLlvmCallConv (m_callConvKind);
	}

	const char*
	getCallConvString ()
	{
		return ct::getCallConvString (m_callConvKind);
	}

	const char*
	getCallConvDisplayString ()
	{
		return ct::getCallConvDisplayString (m_callConvKind);
	}

	bool
	isDefault ()
	{
		return m_callConvKind == CallConvKind_Default;
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
	getThisArgValue (Function* function);

	virtual
	Value
	getArgValue (
		llvm::Value* llvmValue,
		FunctionType* functionType,
		size_t argIdx
		);

	virtual
	void
	createArgVariables (Function* function)
	{
		createArgVariablesImpl (function, 0);
	}

protected:
	void
	createArgVariablesImpl (
		Function* function,
		size_t baseLlvmArgIdx = 0
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
