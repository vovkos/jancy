// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#define _JNC_CALLCONV_GCC32

#include "jnc_CallConv.h"

namespace jnc {

//.............................................................................

class CCallConv_gcc32: public CCallConv
{
public:
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
	CreateArgVariables (CFunction* pFunction);
};

//.............................................................................

class CCdeclCallConv_gcc32: public CCallConv_gcc32
{
public:
	CCdeclCallConv_gcc32 ()
	{
		m_CallConvKind = ECallConv_Cdecl_gcc32;
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CStdcallCallConv_gcc32: public CCallConv_gcc32
{
public:
	CStdcallCallConv_gcc32 ()
	{
		m_CallConvKind = ECallConv_Stdcall_gcc32;
	}
};

//.............................................................................

} // namespace jnc {
