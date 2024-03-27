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

#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

class Module;
class Function;
class Variable;

//..............................................................................

class Jit {
protected:
	Module* m_module;
	sl::StringHashTable<void*> m_symbolMap;

public:
	Jit(Module* module) {
		ASSERT(module);
		m_module = module;
	}

	virtual
	~Jit() {
	}

	virtual
	bool
	create(uint_t optLevel) = 0;

	void*
	findSymbol(const sl::StringRef& name);

	virtual
	bool
	mapVariable(
		Variable* variable,
		void* p
	) = 0;

	virtual
	bool
	mapFunction(
		Function* function,
		void* p
	) = 0;

	virtual
	bool
	prepare() { // called after everything is mapped and before jitting
		return true;
	}

	virtual
	void*
	jit(Function* function) = 0;

	virtual
	void*
	getStaticData(Variable* variable) = 0;

	virtual
	void
	finalizeObject() {
	}

protected:
	void
	addStdSymbols();

	void
	clearLlvmModule() {
		m_module->m_llvmModule = NULL;
	}

	void
	clearLlvmContext() {
		m_module->m_llvmContext = NULL;
	}

	static
	void
	setFunctionMachineCode(
		Function* function,
		void* p
	) {
		function->m_machineCode = p;
	}

	static
	void
	setVariableStaticData(
		Variable* variable,
		void* p
	) {
		variable->m_staticData = p;
	}

	llvm::Function*
	getLlvmFunction(Function* function) {
		return
			!function->hasLlvmFunction() ? NULL : // never used
			!function->getLlvmFunctionName().isEmpty() ?
				m_module->getLlvmModule()->getFunction(function->getLlvmFunctionName() >> toLlvm) :
				function->getLlvmFunction();
	}

	llvm::GlobalVariable*
	getLlvmGlobalVariable(Variable* variable) {
		return !variable->getLlvmGlobalVariableName().isEmpty() ?
			m_module->getLlvmModule()->getGlobalVariable(variable->getLlvmGlobalVariableName() >> toLlvm) :
			variable->getLlvmGlobalVariable();
	}

	llvm::GlobalVariable*
	createLlvmGlobalVariableMapping(Variable* variable);
};

//..............................................................................

class ExecutionEngineJit: public Jit {
protected:
	llvm::ExecutionEngine* m_llvmExecutionEngine;

public:
	ExecutionEngineJit(Module* module):
		Jit(module) {
		m_llvmExecutionEngine = NULL;
	}

	virtual
	~ExecutionEngineJit() {
		delete m_llvmExecutionEngine;
		clearLlvmModule();
	}

	virtual
	void*
	jit(Function* function) {
		ASSERT(m_llvmExecutionEngine);
		return m_llvmExecutionEngine->getPointerToFunction(function->getLlvmFunction());
	}

	virtual
	void
	finalizeObject() {
		ASSERT(m_llvmExecutionEngine);
		m_llvmExecutionEngine->finalizeObject();
	}
};

//..............................................................................

void
disableLlvmGlobalMerge();

} // namespace ct
} // namespace jnc
