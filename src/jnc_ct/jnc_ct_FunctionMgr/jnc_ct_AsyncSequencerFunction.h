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

class AsyncSequencerFunction: public CompilableFunction {
	friend class AsyncLauncherFunction;

protected:
	ClassType* m_promiseType;
	BasicBlock* m_catchBlock;

public:
	AsyncSequencerFunction();

	Type*
	getAsyncReturnType() {
		return m_asyncLauncher->getType()->getAsyncReturnType();
	}

	ClassType*
	getPromiseType() {
		return m_promiseType;
	}

	BasicBlock*
	getCatchBlock() {
		return m_catchBlock;
	}

	virtual
	bool
	compile();

	void
	replaceAllocas();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
AsyncSequencerFunction::AsyncSequencerFunction() {
	m_functionKind = FunctionKind_AsyncSequencer;
	m_promiseType = NULL;
	m_catchBlock = NULL;
	m_flags |= ModuleItemFlag_User;
}

//..............................................................................

} // namespace ct
} // namespace jnc
