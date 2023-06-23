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

class OrcJit: public Jit {
protected:
	llvm::orc::ThreadSafeModule m_llvmModule;
	llvm::orc::ExecutionSession* m_llvmExecutionSession;
	llvm::orc::IRCompileLayer* m_llvmIrCompileLayer;
	llvm::orc::RTDyldObjectLinkingLayer* m_llvmObjectLinkingLayer;
	llvm::orc::JITDylib* m_llvmJitDylib;
	llvm::DataLayout* m_llvmDataLayout;
	sl::StringHashTable<void*> m_symbolMap;

public:
	OrcJit(Module*);

	virtual
	~OrcJit();

	virtual
	bool
	create();

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
	bool
	prepare();

	virtual
	void*
	jit(Function* function) {
		return lookup(function->getLlvmFunction()->getName());
	}

	virtual
	void*
	getStaticData(Variable* variable) {
		return lookup(variable->getLlvmGlobalVariable()->getName());
	}

protected:
	void*
	lookup(const llvm::StringRef& name);
};

//..............................................................................

} // namespace ct
} // namespace jnc
