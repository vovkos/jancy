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

#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

//..............................................................................

struct LambdaCapture {
	Variable* m_variable; // pseudo-variable we use in lambda body
	Field* m_field; // final field (known only after closure layout is ready)

	LambdaCapture();

	void
	finalize(const Value& closurePtrValue);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
LambdaCapture::LambdaCapture() {
	m_variable = NULL;
	m_field = NULL;
}

//..............................................................................

struct LambdaByValCapture: LambdaCapture {
	Value m_value; // evaluated inside the [...] block
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct LambdaByRefCapture: LambdaCapture {
	Variable* m_outerVariable; // after we create the capture, we add it to scope
};

//..............................................................................

// a method of its (internal) closure class; the parent namespace is the scope
// where the lambda is created

class LambdaFunction: public Function {
	friend class FunctionMgr;
	friend class OperatorMgr;
	friend class Parser;

protected:
	ClassType* m_closureType; // NULL if no captures or only a class 'this' (passed as the closure itself)
	Type* m_outerThisType;
	LambdaCapture m_outerThisCapture;
	sl::StringHashTable<LambdaByValCapture> m_byValCaptureMap; // [...] entries
	sl::Array<LambdaByRefCapture> m_byRefCaptureArray; // local vars captured by reference

public:
	LambdaFunction();

	Type*
	getOuterThisType() {
		return m_outerThisType;
	}

	Variable*
	getOuterThisVariable() {
		return m_outerThisCapture.m_variable ? m_outerThisCapture.m_variable : createOuterThisVariable();
	}

	Variable*
	createByRefCaptureVariable(Variable* variable);

	virtual
	bool
	compile(); // nested into the compilation of the outer function

	bool
	createFunctionPtrValue(Value* resultValue); // in the outer function, after compile()

protected:
	Variable*
	createOuterThisVariable();

	void
	addByValCapturesToScope(); // called from the prologue
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
LambdaFunction::LambdaFunction() {
	m_functionKind = FunctionKind_Lambda;
	m_closureType = NULL;
	m_outerThisType = NULL;
}

//..............................................................................

} // namespace ct
} // namespace jnc
