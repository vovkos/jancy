// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#define _JNC_CDECLCALLCONV_MSC64

#include "jnc_CallConv.h"

namespace jnc {

//.............................................................................

class CCdeclCallConv_msc64: public CCallConv
{
public:
	CCdeclCallConv_msc64 ()
	{
		m_CallConvKind = ECallConv_Cdecl_msc64;
	}

	virtual
	llvm::FunctionType*
	GetLlvmFunctionType (CFunctionType* pFunctionType);

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
	GetArgValue (
		CFunctionArg* pArg,
		llvm::Value* pLlvmValue
		);

	virtual
	CValue
	GetThisArgValue (CFunction* pFunction);

	virtual
	void
	CreateArgVariables (CFunction* pFunction);
};

//.............................................................................

} // namespace jnc {
