#include "pch.h"
#include "jnc_ct_McJit.h"

namespace jnc {
namespace ct {

//..............................................................................

class JitMemoryMgr: public llvm::SectionMemoryManager {
protected:
	Jit* m_jit;

public:
	JitMemoryMgr(Jit* jit) {
		m_jit = jit;
	}

	virtual
	void*
	getPointerToNamedFunction(
		const std::string &name,
		bool abortOnFailure
	);

	virtual
	uint64_t
	getSymbolAddress(const std::string &name);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void*
JitMemoryMgr::getPointerToNamedFunction(
	const std::string& name,
	bool abortOnFailure
) {
	void* p = m_jit->findSymbol(name.c_str());
	if (p)
		return p;

	if (abortOnFailure) {
		std::string errorString = "JitMemoryManager::getPointerToNamedFunction: unresolved external function '" + name + "'";
		llvm::report_fatal_error(errorString.c_str());
	}

	return NULL;
}

uint64_t
JitMemoryMgr::getSymbolAddress(const std::string &name) {
	void* p = m_jit->findSymbol(name.c_str());
	if (p)
		return (uint64_t)p;

	return 0;
}

//..............................................................................

bool
McJit::create(uint_t optLevel) {
	addStdSymbols();

#if (_JNC_CPU_ARM32 || _JNC_CPU_ARM64)
	// disable the GlobalMerge pass (on by default) on ARM because
	// it will dangle GlobalVariable::m_llvmVariable pointers

	disableLlvmGlobalMerge();
#endif

#if (LLVM_VERSION < 0x030600)
	llvm::EngineBuilder engineBuilder(m_module->getLlvmModule());
#else
	llvm::EngineBuilder engineBuilder(std::move(std::unique_ptr<llvm::Module>(m_module->getLlvmModule())));
#endif

	std::string errorString;
	engineBuilder.setErrorStr(&errorString);
	engineBuilder.setEngineKind(llvm::EngineKind::JIT);
	engineBuilder.setOptLevel((llvm::CodeGenOpt::Level)optLevel);

	llvm::TargetOptions targetOptions;
#if (_JNC_CPU_ARM32 || _JNC_CPU_ARM64)
	targetOptions.FloatABIType = llvm::FloatABI::Hard;
#endif

	JitMemoryMgr* jitMemoryMgr = new JitMemoryMgr(this);
#if (LLVM_VERSION < 0x030600)
	engineBuilder.setUseMCJIT(true);
	engineBuilder.setMCJITMemoryManager(jitMemoryMgr);
	targetOptions.JITEmitDebugInfo = true;
#elif (LLVM_VERSION < 0x030700)
	engineBuilder.setMCJITMemoryManager(std::move(std::unique_ptr<JitMemoryMgr>(jitMemoryMgr)));
	targetOptions.JITEmitDebugInfo = true;
#else
	engineBuilder.setMCJITMemoryManager(std::move(std::unique_ptr<JitMemoryMgr>(jitMemoryMgr)));
#endif

	engineBuilder.setTargetOptions(targetOptions);

	// disable CPU feature auto-detection
	// alas, making use of certain CPU features leads to JIT crashes (e.g. test114.jnc)

	engineBuilder.setMCPU("generic");
#if (_JNC_CPU_X86)
	engineBuilder.setMArch("x86");
#endif

	m_llvmExecutionEngine = engineBuilder.create();
	if (!m_llvmExecutionEngine) {
		err::setFormatStringError("cannot create execution engine: %s", errorString.c_str());
		return false;
	}

	return true;
}

bool
McJit::mapVariable(
	Variable* variable,
	void* p
) {
	if (variable->getStorageKind() != StorageKind_Static) {
		err::setFormatStringError("attempt to map non-global variable: %s", variable->getQualifiedName().sz());
		return false;
	}

	setVariableStaticData(variable, p);
	llvm::GlobalVariable* llvmMapping = createLlvmGlobalVariableMapping(variable);
	if (!llvmMapping)
		return true; // optimized out

	sl::StringHashTableIterator<void*> it = m_symbolMap.visit(llvmMapping->getName().data());
	if (it->m_value) {
		err::setFormatStringError("attempt to re-map variable: %s", variable->getQualifiedName().sz());
		return false;
	}

	it->m_value = p;

#if (LLVM_VERSION >= 0x040000)
	ASSERT(m_llvmExecutionEngine);
	m_llvmExecutionEngine->addGlobalMapping(llvmMapping, p);
#endif
	return true;
}

bool
McJit::mapFunction(
	Function* function,
	void* p
) {
	setFunctionMachineCode(function, p);

	llvm::Function* llvmFunction = getLlvmFunction(function);
	if (!llvmFunction)
		return true;

	sl::StringHashTableIterator<void*> it = m_symbolMap.visit(llvmFunction->getName().data());
	if (it->m_value) {
		err::setFormatStringError("attempt to re-map function: %s/%s", function->getQualifiedName().sz(), llvmFunction->getName().data());
		return false;
	}

	it->m_value = p;
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
