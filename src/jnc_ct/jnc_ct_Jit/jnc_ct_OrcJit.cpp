#include "pch.h"
#include "jnc_ct_OrcJit.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

class JitDefinitionGenerator: public llvm::orc::DefinitionGenerator {
protected:
	Jit* m_jit;

public:
	JitDefinitionGenerator(Jit* jit) {
		m_jit = jit;
	}

	virtual
	llvm::Error
	tryToGenerate(
		llvm::orc::LookupState &LS,
		llvm::orc::LookupKind K,
		llvm::orc::JITDylib &JD,
		llvm::orc::JITDylibLookupFlags JDLookupFlags,
		const llvm::orc::SymbolLookupSet &LookupSet
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

llvm::Error
JitDefinitionGenerator::tryToGenerate(
	llvm::orc::LookupState &LS,
	llvm::orc::LookupKind K,
	llvm::orc::JITDylib &JD,
	llvm::orc::JITDylibLookupFlags JDLookupFlags,
	const llvm::orc::SymbolLookupSet &LookupSet
) {
	llvm::orc::SymbolMap symbolMap;
	llvm::orc::SymbolLookupSet::const_iterator it = LookupSet.begin();
	for (; it != LookupSet.end(); it++) {
		void* p = m_jit->findSymbol(*it->first >> toAxl);
		if (p)
#if (LLVM_VERSION_MAJOR < 17)
			symbolMap[it->first] = llvm::JITEvaluatedSymbol(
				(llvm::JITTargetAddress)(void*)p,
				llvm::JITSymbolFlags::Exported
			);
#else
			symbolMap[it->first] = llvm::orc::ExecutorSymbolDef(
				llvm::orc::ExecutorAddr((size_t)p),
				llvm::JITSymbolFlags::Exported
			);
#endif
	}

	return JD.define(llvm::orc::absoluteSymbols(std::move(symbolMap)));
}

//..............................................................................

OrcJit::OrcJit(Module* module):
	Jit(module) {

	m_llvmExecutionSession = NULL;
	m_llvmIrCompileLayer = NULL;
	m_llvmObjectLinkingLayer = NULL;
	m_llvmMangle = NULL;
	m_llvmJitDylib = NULL;
	m_llvmDataLayout = NULL;
}

OrcJit::~OrcJit() {
	if (m_llvmExecutionSession) {
		llvm::Error llvmError = m_llvmExecutionSession->endSession();
		if (llvmError)
			TRACE("llvm::orc::ExecutionSession::endSession failed: %s\n", (std::move(llvmError) >> toAxl).getDescription().sz());

		delete m_llvmExecutionSession;
	}

	delete m_llvmIrCompileLayer;
	delete m_llvmObjectLinkingLayer;
	delete m_llvmMangle;
	delete m_llvmDataLayout;

	if (m_llvmThreadSafeModule) {
		clearLlvmContext();
		clearLlvmModule();
	}

	m_llvmThreadSafeModule = llvm::orc::ThreadSafeModule();
	m_llvmExecutionSession = NULL;
	m_llvmIrCompileLayer = NULL;
	m_llvmObjectLinkingLayer = NULL;
	m_llvmMangle = NULL;
	m_llvmDataLayout = NULL;
	m_llvmJitDylib = NULL;
}

bool
OrcJit::create(uint_t optLevel) {
	addStdSymbols();

#if (_JNC_CPU_ARM32 || _JNC_CPU_ARM64)
	// disable the GlobalMerge pass (on by default) on ARM because
	// it will dangle GlobalVariable::m_llvmVariable pointers

	disableLlvmGlobalMerge();
#endif

	ASSERT(!m_llvmExecutionSession);

#	if (LLVM_VERSION_MAJOR < 13)
	m_llvmExecutionSession = new llvm::orc::ExecutionSession();
#else
	llvm::Expected<std::unique_ptr<llvm::orc::SelfExecutorProcessControl> > llvmEpc = llvm::orc::SelfExecutorProcessControl::Create();
	if (!llvmEpc) {
		err::setError(llvmEpc.takeError() >> toAxl);
		return false;
	}

	m_llvmExecutionSession = new llvm::orc::ExecutionSession(std::move(*llvmEpc));
#endif

	llvm::Expected<llvm::orc::JITTargetMachineBuilder> llvmTmb = llvm::orc::JITTargetMachineBuilder::detectHost();
	if (!llvmTmb) {
		err::setError(llvmTmb.takeError() >> toAxl);
		return false;
	}

	llvmTmb->setCodeGenOptLevel((llvm::CodeGenOptLevel)optLevel);

	llvm::Expected<llvm::DataLayout> llvmDl = llvmTmb->getDefaultDataLayoutForTarget();
	if (!llvmDl) {
		err::setError(llvmDl.takeError() >> toAxl);
		return false;
	}

	m_llvmDataLayout = new llvm::DataLayout(*llvmDl);
	m_llvmMangle = new llvm::orc::MangleAndInterner(*m_llvmExecutionSession, *m_llvmDataLayout);
	m_module->getLlvmModule()->setDataLayout(*m_llvmDataLayout);

	m_llvmJitDylib = &m_llvmExecutionSession->createBareJITDylib(m_module->getName().sz());
	m_llvmJitDylib->addGenerator(std::make_unique<JitDefinitionGenerator>(this));

	m_llvmObjectLinkingLayer = new llvm::orc::RTDyldObjectLinkingLayer(
		*m_llvmExecutionSession,
		[]() {
			return std::make_unique<llvm::SectionMemoryManager>();
		}
	);

	m_llvmIrCompileLayer = new llvm::orc::IRCompileLayer(
		*m_llvmExecutionSession,
		*m_llvmObjectLinkingLayer,
		std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(*llvmTmb))
	);

	return true;
}

bool
OrcJit::prepare() { // called after everything is mapped and before jitting
	m_llvmThreadSafeModule = llvm::orc::ThreadSafeModule(
		std::unique_ptr<llvm::Module>(m_module->getLlvmModule()),
		std::unique_ptr<llvm::LLVMContext>(m_module->getLlvmContext())
	);

	llvm::Error llvmError = m_llvmIrCompileLayer->add(
		m_llvmJitDylib->getDefaultResourceTracker(),
		llvm::orc::cloneToNewContext(m_llvmThreadSafeModule)
	);

	if (llvmError) {
		err::setError(std::move(llvmError) >> toAxl);
		return false;
	}

	return true;
}

bool
OrcJit::mapVariable(
	Variable* variable,
	void* p
) {
	if (variable->getStorageKind() != StorageKind_Static) {
		err::setFormatStringError("attempt to map non-global variable: %s", variable->getItemName().sz());
		return false;
	}

	setVariableStaticData(variable, p);

	llvm::GlobalVariable* llvmMapping = createLlvmGlobalVariableMapping(variable);
	if (!llvmMapping)
		return true;

	if (!llvmMapping)
		return true; // optimized out

	sl::StringHashTableIterator<void*> it = m_symbolMap.visit(llvmMapping->getName().data());
	if (it->m_value) {
		err::setFormatStringError("attempt to re-map variable: %s", variable->getItemName().sz());
		return false;
	}

	it->m_value = p;
	return true;
}

bool
OrcJit::mapFunction(
	Function* function,
	void* p
) {
	setFunctionMachineCode(function, p);

	if (!function->hasLlvmFunction()) // never used
		return true;

	llvm::Function* llvmFunction = getLlvmFunction(function);
	if (!llvmFunction)
		return true;

	sl::StringHashTableIterator<void*> it = m_symbolMap.visit(llvmFunction->getName().data());
	if (it->m_value) {
		err::setFormatStringError("attempt to re-map function: %s/%s", function->getItemName().sz(), llvmFunction->getName().data());
		return false;
	}

	it->m_value = p;
	return true;
}

void*
OrcJit::lookup(const llvm::StringRef& name) {
#if (LLVM_VERSION_MAJOR < 17)
	typedef llvm::Expected<llvm::JITEvaluatedSymbol> LookupResult;
#else
	typedef llvm::Expected<llvm::orc::ExecutorSymbolDef> LookupResult;
#endif
	LookupResult symbol = m_llvmExecutionSession->lookup(
		llvm::ArrayRef<llvm::orc::JITDylib*>(m_llvmJitDylib),
		(*m_llvmMangle)(name)
	);

	if (!symbol) {
		err::setError(symbol.takeError() >> toAxl);
		return NULL;
	}

#if (LLVM_VERSION_MAJOR < 17)
	return (void*)symbol->getAddress();
#else
	return (void*)symbol->getAddress().getValue();
#endif
}

//..............................................................................

} // namespace ct
} // namespace jnc
