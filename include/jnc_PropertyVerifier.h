// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_PropertyType.h"
#include "jnc_Function.h"

namespace jnc {

//.............................................................................

class CPropertyVerifier
{
protected:
	rtl::CString m_IndexArgSignature; // all accessors must have matching index arg signature
	
public:
	bool
	CheckGetter (CFunctionType* pFunctionType)
	{
		return CheckIndexSignature (EFunction_Getter, pFunctionType);
	}

	bool
	CheckSetter (CFunctionType* pFunctionType);

protected:
	bool 
	CheckIndexSignature (
		EFunction FunctionKind,
		CFunctionType* pFunctionType
		);

	static
	rtl::CString 
	CreateIndexArgSignature (
		EFunction FunctionKind,
		CFunctionType* pFunctionType
		);
};

//.............................................................................

} // namespace jnc {
