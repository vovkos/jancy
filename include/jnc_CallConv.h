// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#define _JNC_CALLCONV

#include "jnc_Value.h"

namespace jnc {

class CFunctionType;

//.............................................................................

// jnccall is basically cdecl with the following 2 differences:
// - arrays are passed by value (like if it were wrapped in a struct)
// - varargs are wrapped into variants and prepended with vararg count

enum ECallConv
{
	ECallConv_Undefined = 0,
	ECallConv_Jnccall_msc32,
	ECallConv_Jnccall_msc64,
	ECallConv_Jnccall_gcc32,
	ECallConv_Jnccall_gcc64,
	ECallConv_Cdecl_msc32,
	ECallConv_Cdecl_msc64,
	ECallConv_Cdecl_gcc32,
	ECallConv_Cdecl_gcc64,
	ECallConv_Stdcall_msc32,
	ECallConv_Stdcall_gcc32,
	ECallConv__Count,

#if (_AXL_CPP == AXL_CPP_MSC)
#	if (_AXL_CPU == AXL_CPU_AMD64)
	ECallConv_Jnccall = ECallConv_Jnccall_msc64,
	ECallConv_Cdecl   = ECallConv_Cdecl_msc64,
#	else
	ECallConv_Jnccall = ECallConv_Jnccall_msc32,
	ECallConv_Cdecl   = ECallConv_Cdecl_msc32,
	ECallConv_Stdcall = ECallConv_Stdcall_msc32,

#	endif
#else
#	if (_AXL_CPU == AXL_CPU_AMD64)
	ECallConv_Jnccall = ECallConv_Jnccall_gcc64,
	ECallConv_Cdecl   = ECallConv_Cdecl_gcc64,
#	else
	ECallConv_Jnccall = ECallConv_Jnccall_gcc32,
	ECallConv_Cdecl   = ECallConv_Cdecl_gcc32,
	ECallConv_Stdcall = ECallConv_Stdcall_gcc32,
#	endif
#endif

	ECallConv_Default = ECallConv_Jnccall,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ECallConvFlag
{
	// vararg

	ECallConvFlag_NoVarArg     = 0x0001,
	ECallConvFlag_UnsafeVarArg = 0x0002,

	// family

	ECallConvFlag_Jnccall      = 0x0010,
	ECallConvFlag_Cdecl        = 0x0020,
	ECallConvFlag_Stdcall      = 0x0040,

	// compiler

	ECallConvFlag_Msc          = 0x0100,
	ECallConvFlag_Gcc          = 0x0200,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::CallingConv::ID
GetLlvmCallConv (ECallConv CallConvKind);

uint_t
GetCallConvFlags (ECallConv CallConvKind);

const char*
GetCallConvString (ECallConv CallConvKind);

const char*
GetCallConvDisplayString (ECallConv CallConvKind);

const char*
GetCallConvSignature (ECallConv CallConvKind);

ECallConv
GetCallConvKindFromModifiers (uint_t Modifiers);

//.............................................................................

class CCallConv
{
protected:
	CModule* m_pModule;
	ECallConv m_CallConvKind;

public:
	CCallConv ();

	ECallConv
	GetCallConvKind ()
	{
		return m_CallConvKind;
	}

	uint_t
	GetFlags ()
	{
		return jnc::GetCallConvFlags (m_CallConvKind);
	}

	llvm::CallingConv::ID
	GetLlvmCallConv ()
	{
		return jnc::GetLlvmCallConv (m_CallConvKind);
	}

	const char*
	GetCallConvString ()
	{
		return jnc::GetCallConvString (m_CallConvKind);
	}

	const char*
	GetCallConvDisplayString ()
	{
		return jnc::GetCallConvDisplayString (m_CallConvKind);
	}

	bool
	IsDefault ()
	{
		return m_CallConvKind == ECallConv_Default;
	}

	virtual
	llvm::FunctionType*
	GetLlvmFunctionType (CFunctionType* pFunctionType);

	virtual
	llvm::Function*
	CreateLlvmFunction (
		CFunctionType* pFunctionType,
		const char* pTag
		);

	virtual
	void
	Call (
		const CValue& CalleeValue,
		CFunctionType* pFunctionType,
		rtl::CBoxListT <CValue>* pArgValueList,
		CValue* pResultValue
		);

	virtual
	void
	Return (
		CFunction* pFunction,
		const CValue& Value
		);

	virtual
	CValue
	GetThisArgValue (CFunction* pFunction);

	virtual
	void
	CreateArgVariables (CFunction* pFunction)
	{
		CreateArgVariablesImpl (pFunction, 0);
	}

protected:
	void
	CreateArgVariablesImpl (
		CFunction* pFunction,
		size_t BaseLlvmArgIdx = 0
		);
};

//.............................................................................

} // namespace jnc {
