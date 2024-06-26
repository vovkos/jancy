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
#include "jnc_ct_Variable.h"

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

inline
LeanDataPtrValidator*
Value::getLeanDataPtrValidator() const {
	if (m_leanDataPtrValidator)
		return m_leanDataPtrValidator;

	ASSERT(m_valueKind == ValueKind_Variable);
	m_leanDataPtrValidator = m_variable->getLeanDataPtrValidator();
	return m_leanDataPtrValidator;
}

inline
void
Value::setLeanDataPtrValidator(LeanDataPtrValidator* validator) {
	ASSERT(isDataPtrType(m_type, DataPtrTypeKind_Lean));
	m_leanDataPtrValidator = validator;
}

inline
void
Value::setLeanDataPtrValidator(const Value& originValue) {
	ASSERT(isDataPtrType(m_type, DataPtrTypeKind_Lean));

	if (originValue.m_leanDataPtrValidator)
		m_leanDataPtrValidator = originValue.m_leanDataPtrValidator;
	else if (originValue.m_valueKind == ValueKind_Variable)
		m_leanDataPtrValidator = originValue.m_variable->getLeanDataPtrValidator();
	else {
		m_leanDataPtrValidator = AXL_RC_NEW(LeanDataPtrValidator);
		m_leanDataPtrValidator->m_originValue = originValue;
	}
}

inline
void
Value::setLeanDataPtrValidator(
	const Value& originValue,
	const Value& rangeBeginValue,
	size_t rangeLength
) {
	ASSERT(isDataPtrType(m_type, DataPtrTypeKind_Lean));

	rc::Ptr<LeanDataPtrValidator> validator = AXL_RC_NEW(LeanDataPtrValidator);
	validator->m_originValue = originValue;
	validator->m_rangeBeginValue = rangeBeginValue;
	validator->m_rangeLength = rangeLength;
	m_leanDataPtrValidator = validator;
}

//..............................................................................

} // namespace ct
} // namespace jnc
