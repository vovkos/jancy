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

enum LambdaCaptureFlag {
	LambdaCaptureFlag_ByValue = 0x01, // listed in the lambda's [] -- must be used
	LambdaCaptureFlag_Used    = 0x02, // referenced from the body; the fields below are valid
	LambdaCaptureFlag_Ptr     = 0x04, // the field is a pointer to the outer variable
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// an outer local (or the outer 'this') referenced from a lambda body; the closure
// layout is unknown while the body compiles, so fields are accessed via placeholder
// allocas which are later replaced with field GEPs (same as TLS/reactor variables)

struct LambdaCapture {
	Type* m_fieldType;
	Field* m_field;
	llvm::AllocaInst* m_llvmFieldAlloca; // placeholder for the field address
	lex::LineCol m_pos; // [] entries only
	uint_t m_flags;

	LambdaCapture();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
LambdaCapture::LambdaCapture() {
	m_fieldType = NULL;
	m_field = NULL;
	m_llvmFieldAlloca = NULL;
	m_flags = 0;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// a method of its (internal) closure class; the parent namespace is the scope
// where the lambda is created

class LambdaFunction: public Function {
	friend class FunctionMgr;
	friend class VariableMgr;
	friend class OperatorMgr;
	friend class Parser;

protected:
	Type* m_outerThisType; // type of the closure field for the outer 'this'
	Value m_outerThisValue; // the outer 'this' (captured on demand)
	LlvmIrInsertPoint m_captureInsertPoint; // top of the lambda body
	sl::SimpleHashTable<Variable*, LambdaCapture> m_captureMap; // outer variable -> capture; NULL -> outer 'this'

public:
	LambdaFunction();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
LambdaFunction::LambdaFunction() {
	m_functionKind = FunctionKind_Lambda;
	m_outerThisType = NULL;
}

//..............................................................................

} // namespace ct
} // namespace jnc
