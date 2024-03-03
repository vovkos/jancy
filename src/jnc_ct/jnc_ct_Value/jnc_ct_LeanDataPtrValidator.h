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

namespace jnc {
namespace ct {

class Variable;
class OperatorMgr;

//..............................................................................

class LeanDataPtrValidator: public rc::RefCount {
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
	LeanDataPtrValidator() {
		m_rangeLength = 0;
	}

	bool
	isDynamicRange() {
		return m_rangeLength == 0;
	}

	bool
	hasValidatorValue() {
		return m_validatorValue;
	}

	Value
	getOriginValue() {
		return m_originValue;
	}

	Value
	getRangeBeginValue() {
		return m_rangeBeginValue;
	}

	size_t
	getRangeLength() {
		ASSERT(m_rangeLength); // should be checked with isDynamicRange()
		return m_rangeLength;
	}

	Value
	getValidatorValue();

protected:
	void
	createValidator();

	void
	createValidator(const Value& boxValue);

	void
	createClassFieldValidator();
};

//..............................................................................

} // namespace ct
} // namespace jnc
