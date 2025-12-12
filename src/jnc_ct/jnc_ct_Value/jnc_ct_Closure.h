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

class FunctionType;
class FunctionPtrType;
class PropertyType;
class PropertyPtrType;

//..............................................................................

class Closure: public rc::RefCount {
protected:
	sl::BoxList<Value> m_argValueList;
	Value* m_thisArgValue;
	size_t m_thisArgIdx;

public:
	Closure() {
		m_thisArgValue = NULL;
		m_thisArgIdx = -1;
	}

	sl::BoxList<Value>*
	getArgValueList() {
		return &m_argValueList;
	}

	bool
	isMemberClosure() {
		return m_thisArgIdx != -1;
	}

	size_t
	countNonEmptyArgs();

	size_t
	getThisArgIdx() {
		return m_thisArgIdx;
	}

	Value
	getThisArgValue();

	void
	setThisArgIdx(size_t thisArgIdx);

	void
	insertThisArgValue(const Value& thisValue);

	bool
	isSimpleClosure() {
		return isMemberClosure() && m_argValueList.getCount() == 1;
	}

	size_t
	append(const Value& argValue);

	size_t
	append(const sl::ConstBoxList<Value>& argValueList);

	size_t
	append(sl::BoxList<Value>* argValueList); // destructive

	size_t
	append(const sl::ArrayRef<Type*>& argTypeArray);

	bool
	apply(sl::BoxList<Value>* argValueList);

	Type*
	getClosureType(Type* type);

	FunctionPtrType*
	getFunctionClosureType(Function* function); // choose the best overload

	FunctionPtrType*
	getFunctionClosureType(FunctionPtrType* ptrType);

	PropertyPtrType*
	getPropertyClosureType(PropertyPtrType* ptrType);

	bool
	getArgTypeArray(
		Module* module,
		sl::Array<FunctionArg*>* argArray
	);

protected:
	sl::ConstBoxIterator<Value>
	prepend(const sl::ConstBoxList<Value>& argValueList);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
size_t
Closure::countNonEmptyArgs() {
	size_t count = 0;
	sl::BoxIterator<Value> it = m_argValueList.getHead();
	for (; it; it++) {
		if (!it->isEmpty())
			count++;
	}

	return count;
}

//..............................................................................

inline
Closure*
Value::createClosure() {
	m_closure = AXL_RC_NEW(Closure);
	return m_closure;
}

inline
void
Value::setClosure(Closure* closure) {
	m_closure = closure;
}

inline
Type*
Value::getClosureAwareType() const {
	return m_closure ? m_closure->getClosureType(m_type) : m_type;
}

//..............................................................................

} // namespace ct
} // namespace jnc
