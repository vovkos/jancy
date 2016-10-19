// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_PropertyType.h"
#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

//..............................................................................

class PropertyVerifier
{
protected:
	sl::String m_indexArgSignature; // all accessors must have matching index arg signature

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
	sl::String
	createIndexArgSignature (
		FunctionKind functionKind,
		FunctionType* functionType
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
