// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

//.............................................................................

enum LeanDataPtrValidatorKind
{
	LeanDataPtrValidatorKind_Undefined,
	LeanDataPtrValidatorKind_Simple,
	LeanDataPtrValidatorKind_Complex,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class LeanDataPtrValidator: public ref::RefCount
{
	friend class Value;
	friend class ConstMgr;

protected:
	LeanDataPtrValidatorKind m_validatorKind;

	Value m_scopeValidatorValue;
	Value m_rangeBeginValue;
	Value m_sizeValue;

public:
	LeanDataPtrValidator ()
	{
		m_validatorKind = LeanDataPtrValidatorKind_Undefined;
	}

	LeanDataPtrValidatorKind 
	getValidatorKind ()
	{
		return m_validatorKind;
	}

	Value 
	getScopeValidator ()
	{
		return m_scopeValidatorValue;
	}

	Value 
	getRangeBegin ()
	{
		return m_rangeBeginValue;
	}

	Value
	getSizeValue ()
	{
		return m_sizeValue;
	}
};

//.............................................................................

} // namespace jnc {
