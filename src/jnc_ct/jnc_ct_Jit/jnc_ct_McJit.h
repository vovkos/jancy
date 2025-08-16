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

#include "jnc_ct_Jit.h"

namespace jnc {
namespace ct {

//..............................................................................

class McJit: public ExecutionEngineJit {
public:
	McJit(Module* module):
		ExecutionEngineJit(module) {
	}

	virtual
	bool
	create(uint_t optLevel);

	virtual
	bool
	mapVariable(
		Variable* variable,
		void* p
	);

	virtual
	bool
	mapFunction(
		Function* function,
		void* p
	);

	virtual
	void*
	jit(Function* function) {
		return m_llvmExecutionEngine->getPointerToFunction(function->getLlvmFunction());
	}

	virtual
	void*
	getStaticData(Variable* variable) {
		return (void*)m_llvmExecutionEngine->getGlobalValueAddress(variable->getLlvmGlobalVariable()->getName().str());
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
