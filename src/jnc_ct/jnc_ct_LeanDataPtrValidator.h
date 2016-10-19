// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class Variable;
class OperatorMgr;

//..............................................................................

class LeanDataPtrValidator: public ref::RefCount
{
	friend class Value;
	friend class Variable;
	friend class VariableMgr;
	friend class OperatorMgr;

protected:
	Value m_originValue;
	Value m_rangeBeginValue;
	size_t m_rangeLength;
	Value m_validatorValue;

public:
	LeanDataPtrValidator ()
	{
		m_rangeLength = 0;
	}

	bool
	isDynamicRange ()
	{
		return m_rangeLength == 0;
	}

	bool
	hasValidatorValue ()
	{
		return m_validatorValue;
	}

	Value
	getOriginValue ()
	{
		return m_originValue;
	}

	Value
	getRangeBeginValue ()
	{
		return m_rangeBeginValue;
	}

	size_t
	getRangeLength ()
	{
		ASSERT (m_rangeLength); // should be checked with isDynamicRange ()
		return m_rangeLength;
	}

	Value
	getValidatorValue ();

protected:
	void
	createValidator ();

	void
	createValidator (const Value& boxValue);

	void
	createClassFieldValidator ();
};

//..............................................................................

} // namespace ct
} // namespace jnc
