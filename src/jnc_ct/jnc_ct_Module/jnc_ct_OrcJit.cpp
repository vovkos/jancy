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
			symbolMap[it->first] = llvm::JITEvaluatedSymbol(
				(llvm::JITTargetAddress)(void*)p,
				llvm::JITSymbolFlags::Exported
			);
	}

	return JD.define(llvm::orc::absoluteSymbols(std::move(symbolMap)));
}

//..............................................................................

OrcJit::OrcJit(Module* module):
	Jit(module) {

	m_llvmExecutionSession = NULL;
	m_llvmIrCompileLayer = NULL;
	m_llvmObjectLinkingLayer = NULL;
	m_llvmJitDylib = NULL;
	m_llvmDataLayout = NULL;
}

OrcJit::~OrcJit() {
	if (m_llvmExecutionSession) {
		llvm::Error error = m_llvmExecutionSession->endSession();
		if (error)
			TRACE("llvm::orc::ExecutionSession::endSession failed: %s\n", (std::move(error) >> toAxl).getDescription().sz());

		delete m_llvmExecutionSession;
	}

	if (m_llvmIrCompileLayer)
		delete m_llvmIrCompileLayer;

	if (m_llvmObjectLinkingLayer)
		delete m_llvmObjectLinkingLayer;

	if (m_llvmDataLayout)
		delete m_llvmDataLayout;

	if (m_llvmModule) {
		clearLlvmModule(); // compile layer takes ownership of module & context
		clearLlvmContext();;
	}

	m_llvmModule = llvm::orc::ThreadSafeModule();
	m_llvmExecutionSession = NULL;
	m_llvmIrCompileLayer = NULL;
	m_llvmObjectLinkingLayer = NULL;
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
	llvm::Expected<std::unique_ptr<llvm::orc::SelfExecutorProcessControl> > processControl = llvm::orc::SelfExecutorProcessControl::Create();
	if (!processControl) {
		err::setError(processControl.takeError() >> toAxl);
		return false;
	}

	m_llvmExecutionSession = new llvm::orc::ExecutionSession(std::move(*processControl));
#endif

	llvm::Expected<llvm::orc::JITTargetMachineBuilder> targetMachineBuilder = llvm::orc::JITTargetMachineBuilder::detectHost();
	if (!targetMachineBuilder) {
		err::setError(targetMachineBuilder.takeError() >> toAxl);
		return false;
	}


	targetMachineBuilder->setCodeGenOptLevel((llvm::CodeGenOpt::Level)optLevel);

	llvm::Expected<llvm::DataLayout> dataLayout = targetMachineBuilder->getDefaultDataLayoutForTarget();
	if (!dataLayout) {
		err::setError(dataLayout.takeError() >> toAxl);
		return false;
	}

	m_llvmJitDylib = &m_llvmExecutionSession->createBareJITDylib(m_module->getName().sz());
	m_llvmJitDylib->addGenerator(std::make_unique<JitDefinitionGenerator>(this));

	std::function<std::unique_ptr<llvm::RuntimeDyld::MemoryManager>()> getMemoryMgr;

	m_llvmDataLayout = new llvm::DataLayout(*dataLayout);
	m_llvmObjectLinkingLayer = new llvm::orc::RTDyldObjectLinkingLayer(
		*m_llvmExecutionSession,
			[] () {
				return std::make_unique<llvm::SectionMemoryManager>();
			}
	);

	m_module->getLlvmModule()->setDataLayout(*m_llvmDataLayout);

	m_llvmIrCompileLayer = new llvm::orc::IRCompileLayer(
		*m_llvmExecutionSession,
		*m_llvmObjectLinkingLayer,
		std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(*targetMachineBuilder))
	);

	return true;
}

bool
OrcJit::prepare() { // called after everything is mapped and before jitting
	m_llvmModule = llvm::orc::ThreadSafeModule(
		std::move(std::unique_ptr<llvm::Module>(m_module->getLlvmModule())),
		std::move(std::unique_ptr<llvm::LLVMContext>(m_module->getLlvmContext()))
	);

	llvm::Error error = m_llvmIrCompileLayer->add(
		m_llvmJitDylib->getDefaultResourceTracker(),
		llvm::orc::cloneToNewContext(m_llvmModule)
	);

	if (error) {
		err::setError(std::move(error) >> toAxl);
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
		err::setFormatStringError("attempt to map non-global variable: %s", variable->getQualifiedName().sz());
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
		err::setFormatStringError("attempt to re-map variable: %s", variable->getQualifiedName().sz());
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
		err::setFormatStringError("attempt to re-map function: %s/%s", function->getQualifiedName().sz(), llvmFunction->getName().data());
		return false;
	}

	it->m_value = p;
	return true;
}

void*
OrcJit::lookup(const llvm::StringRef& name) {
	llvm::Expected<llvm::JITEvaluatedSymbol> symbol = m_llvmExecutionSession->lookup(
		llvm::ArrayRef<llvm::orc::JITDylib*>(m_llvmJitDylib),
		name
	);

	if (!symbol) {
		err::setError(symbol.takeError() >> toAxl);
		return NULL;
	}

	return (void*)symbol->getAddress();
}

//..............................................................................

} // namespace ct
} // namespace jnc
