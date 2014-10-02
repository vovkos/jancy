// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_PropertyType.h"
#include "jnc_Function.h"

namespace jnc {

//.............................................................................

class PropertyVerifier
{
protected:
	rtl::String m_indexArgSignature; // all accessors must have matching index arg signature
	
public:
	bool
	checkGetter (FunctionType* functionType)
	{
		return checkIndexSignature (FunctionKind_Getter, functionType);
	}

	bool
	checkSetter (FunctionType* functionType);

protected:
	bool 
	checkIndexSignature (
		FunctionKind functionKind,
		FunctionType* functionType
		);

	static
	rtl::String 
	createIndexArgSignature (
		FunctionKind functionKind,
		FunctionType* functionType
		);
};

//.............................................................................

} // namespace jnc {
