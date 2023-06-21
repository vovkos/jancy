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

#include "jnc_ct_Pch.h"

namespace jnc {
namespace ct {

class Module;
class Function;
class Variable;

//..............................................................................

class Jit {
protected:
	Module* m_module;
#if (_JNC_JIT == JNC_JIT_LLVM_ORC)
	llvm::orc::ThreadSafeModule m_llvmModule;
	llvm::orc::ExecutionSession* m_llvmExecutionSession;
	llvm::orc::IRCompileLayer* m_llvmIrCompileLayer;
	llvm::orc::RTDyldObjectLinkingLayer* m_llvmObjectLinkingLayer;
	llvm::orc::JITDylib* m_llvmJitDylib;
	llvm::DataLayout* m_llvmDataLayout;
	sl::StringHashTable<void*> m_functionMap;
#elif (_JNC_JIT == JNC_JIT_LLVM_MCJIT)
	llvm::ExecutionEngine* m_llvmExecutionEngine;
	sl::StringHashTable<void*> m_functionMap;
#elif (_JNC_JIT == JNC_JIT_LLVM_LEGACY)
	llvm::ExecutionEngine* m_llvmExecutionEngine;
#endif

public:
	Jit();
	~Jit();

	bool
	isCreated() const;

	llvm::DataLayout*
	getLlvmDataLayout();

	void
	clear();

	bool
	create();

	void*
	findFunctionMapping(const sl::StringRef& name);

	bool
	mapVariable(
		Variable* variable,
		void* p
	);

	bool
	mapFunction(
		Function* function,
		void* p
	);

	bool
	prepare(); // called after everything is mapped and before jitting

	void*
	jit(Function* function);

	void*
	getStaticData(Variable* variable);

	bool
	finalizeObject();
};

//..............................................................................

} // namespace ct
} // namespace jnc
