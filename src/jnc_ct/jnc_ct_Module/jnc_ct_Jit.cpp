#include "pch.h"
#include "jnc_ct_Jit.h"
#include "jnc_ct_Module.h"

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
	void* p = m_jit->findFunctionMapping(name.c_str());
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
	void* p = m_jit->findFunctionMapping(name.c_str());
	if (p)
		return (uint64_t)p;

	return 0;
}

//..............................................................................

#if (_JNC_JIT == JNC_JIT_LLVM_ORC)

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
		void* p = m_jit->findFunctionMapping(*it->first >> toAxl);
		if (p)
			symbolMap[it->first] = llvm::JITEvaluatedSymbol(
				(llvm::JITTargetAddress)(void*)p,
				llvm::JITSymbolFlags::Exported
			);
	}

	return JD.define(llvm::orc::absoluteSymbols(std::move(symbolMap)));
}

//..............................................................................

#endif // JNC_JIT_LLVM_ORC

Jit::Jit() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

#if (_JNC_JIT == JNC_JIT_LLVM_ORC)
	m_llvmExecutionSession = NULL;
	m_llvmIrCompileLayer = NULL;
	m_llvmObjectLinkingLayer = NULL;
	m_llvmJitDylib = NULL;
	m_llvmDataLayout = NULL;
#else // JNC_JIT_LLVM_MCJIT or JNC_JIT_LLVM_LEGACY_JIT
	m_llvmExecutionEngine = NULL;
#endif
}

Jit::~Jit() {
	clear();
}

bool
Jit::isCreated() const {
#if (_JNC_JIT == JNC_JIT_LLVM_ORC)
	return m_llvmExecutionSession != NULL;
#else
	return m_llvmExecutionEngine != NULL;
#endif
}

void
Jit::clear() {
#if (_JNC_JIT != JNC_JIT_LLVM_LEGACY_JIT)
	m_functionMap.clear();
#endif

#if (_JNC_JIT == JNC_JIT_LLVM_ORC)
	if (m_llvmExecutionSession)
		delete m_llvmExecutionSession;

	if (m_llvmIrCompileLayer)
		delete m_llvmIrCompileLayer;

	if (m_llvmObjectLinkingLayer)
		delete m_llvmObjectLinkingLayer;

	if (m_llvmDataLayout)
		delete m_llvmDataLayout;

	if (m_llvmModule) {
		m_module->m_llvmModule = NULL; // compile layer takes ownership of module & context
		m_module->m_llvmContext = NULL;
	}

	m_llvmModule = llvm::orc::ThreadSafeModule();
	m_llvmExecutionSession = NULL;
	m_llvmIrCompileLayer = NULL;
	m_llvmObjectLinkingLayer = NULL;
	m_llvmDataLayout = NULL;
	m_llvmJitDylib = NULL;
#else // JNC_JIT_LLVM_MCJIT or JNC_JIT_LLVM_LEGACY_JIT
	if (!m_llvmExecutionEngine)
		return;

	delete m_llvmExecutionEngine;
	m_module->m_llvmModule = NULL; // execution engine takes ownership of module
	m_llvmExecutionEngine = NULL;
#endif // _JNC_JIT != JNC_JIT_LLVM_ORC
}

bool
Jit::create() {
#if (_JNC_JIT != JNC_JIT_LLVM_LEGACY_JIT)
	m_functionMap["memset"] = (void*)memset;
	m_functionMap["memcpy"] = (void*)memcpy;
	m_functionMap["memmove"] = (void*)memmove;
#	if (_JNC_OS_DARWIN)
	m_functionMap["_bzero"] = (void*)bzero;
	m_functionMap["___bzero"] = (void*)bzero;
#	endif
#	if (JNC_PTR_BITS == 32)
#		if (_JNC_OS_POSIX)
	m_functionMap["__divdi3"] = (void*)__divdi3;
	m_functionMap["__moddi3"] = (void*)__moddi3;
	m_functionMap["__udivdi3"] = (void*)__udivdi3;
	m_functionMap["__umoddi3"] = (void*)__umoddi3;
#			if (_JNC_CPU_ARM32)
	m_functionMap["__modsi3"] = (void*)__modsi3;
	m_functionMap["__umodsi3"] = (void*)__umodsi3;
	m_functionMap["__aeabi_idiv"] = (void*)__aeabi_idiv;
	m_functionMap["__aeabi_uidiv"] = (void*)__aeabi_uidiv;
	m_functionMap["__aeabi_ldivmod"] = (void*)__aeabi_ldivmod;
	m_functionMap["__aeabi_uldivmod"] = (void*)__aeabi_uldivmod;
	m_functionMap["__aeabi_i2f"] = (void*)__aeabi_i2f;
	m_functionMap["__aeabi_l2f"] = (void*)__aeabi_l2f;
	m_functionMap["__aeabi_ui2f"] = (void*)__aeabi_ui2f;
	m_functionMap["__aeabi_ul2f"] = (void*)__aeabi_ul2f;
	m_functionMap["__aeabi_i2d"] = (void*)__aeabi_i2d;
	m_functionMap["__aeabi_l2d"] = (void*)__aeabi_l2d;
	m_functionMap["__aeabi_ui2d"] = (void*)__aeabi_ui2d;
	m_functionMap["__aeabi_ul2d"] = (void*)__aeabi_ul2d;
	m_functionMap["__aeabi_memcpy"] = (void*)__aeabi_memcpy;
	m_functionMap["__aeabi_memmove"] = (void*)__aeabi_memmove;
	m_functionMap["__aeabi_memset"] = (void*)__aeabi_memset;
#			endif // _JNC_CPU_ARM32
#		elif (_JNC_OS_WIN)
	m_functionMap["_alldiv"] = (void*)_alldiv;
	m_functionMap["_allrem"] = (void*)_allrem;
	m_functionMap["_aulldiv"] = (void*)_aulldiv;
	m_functionMap["_aullrem"] = (void*)_aullrem;
#		endif // _JNC_OS_WIN
#	endif // JNC_PTR_BITS == 32
#endif // _JNC_JIT != JNC_JIT_LLVM_LEGACY_JIT

#if (_JNC_JIT == JNC_JIT_LLVM_ORC)
	ASSERT(!m_llvmExecutionSession);

	llvm::Expected<std::unique_ptr<llvm::orc::SelfExecutorProcessControl> > processControl = llvm::orc::SelfExecutorProcessControl::Create();
	if (!processControl) {
		err::setError(processControl.takeError() >> toAxl);
		return false;
	}

	m_llvmExecutionSession = new llvm::orc::ExecutionSession(std::move(*processControl));
	llvm::orc::JITTargetMachineBuilder targetMachineBuilder(m_llvmExecutionSession->getExecutorProcessControl().getTargetTriple());
	targetMachineBuilder.setCodeGenOptLevel((llvm::CodeGenOpt::Level)0);

	llvm::Expected<llvm::DataLayout> dataLayout = targetMachineBuilder.getDefaultDataLayoutForTarget();
	if (!dataLayout) {
		err::setError(processControl.takeError() >> toAxl);
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
		std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(targetMachineBuilder))
	);

#else
	ASSERT(!m_llvmExecutionEngine);

#	if (_JNC_CPU_ARM32 || _JNC_CPU_ARM64)
	// disable the GlobalMerge pass (on by default) on ARM because
	// it will dangle GlobalVariable::m_llvmVariable pointers

#		if (LLVM_VERSION < 0x030700)
	llvm::StringMap<llvm::cl::Option*> options;
	llvm::cl::getRegisteredOptions(options);
#		else
	llvm::StringMap<llvm::cl::Option*>& options = llvm::cl::getRegisteredOptions();
#		endif

	llvm::StringMap<llvm::cl::Option*>::iterator globalMergeIt = options.find("global-merge");
	if (globalMergeIt != options.end()) {
		llvm::cl::opt<bool>* globalMerge = (llvm::cl::opt<bool>*)globalMergeIt->second;
		ASSERT(llvm::isa<llvm::cl::opt<bool> >(globalMerge));
		globalMerge->setValue(false);
	}
#	endif

#	if (LLVM_VERSION < 0x030600)
	llvm::EngineBuilder engineBuilder(m_module->getLlvmModule());
#	else
	llvm::EngineBuilder engineBuilder(std::move(std::unique_ptr<llvm::Module>(m_module->getLlvmModule())));
#	endif

	std::string errorString;
	engineBuilder.setErrorStr(&errorString);
	engineBuilder.setEngineKind(llvm::EngineKind::JIT);
	engineBuilder.setOptLevel((llvm::CodeGenOpt::Level)0);

	llvm::TargetOptions targetOptions;
#	if (_JNC_CPU_ARM32 || _JNC_CPU_ARM64)
	targetOptions.FloatABIType = llvm::FloatABI::Hard;
#	endif

#	if (_JNC_JIT == JNC_JIT_LLVM_MCJIT)
	JitMemoryMgr* jitMemoryMgr = new JitMemoryMgr(this);
#	if (LLVM_VERSION < 0x030600)
	engineBuilder.setUseMCJIT(true);
	engineBuilder.setMCJITMemoryManager(jitMemoryMgr);
	targetOptions.JITEmitDebugInfo = true;
#	elif (LLVM_VERSION < 0x030700)
	engineBuilder.setMCJITMemoryManager(std::move(std::unique_ptr<JitMemoryMgr> (jitMemoryMgr)));
	targetOptions.JITEmitDebugInfo = true;
#	else
	engineBuilder.setMCJITMemoryManager(std::move(std::unique_ptr<JitMemoryMgr> (jitMemoryMgr)));
#	endif
#	else // _JNC_JIT == JNC_JIT_LLVM_LEGACY
#		if (_JNC_OS_WIN && _JNC_CPU_AMD64)
	// legacy JIT uses relative call to __chkstk
	// it worked just fine before windows 10 which loads ntdll.dll too far away

	// the fix should go to LLVM, of course, but
	// a) applying a patch to LLVM before building Jancy would be a pain in the ass
	// b) legacy JIT is a gonner anyway

	// therefore, a simple workaround is used: allocate a proxy for __chkstk
	// which would reside close enough to the generated code

	void* chkstk = ::GetProcAddress(::GetModuleHandleA("ntdll.dll"), "__chkstk");
	if (!chkstk) {
		err::setFormatStringError("__chkstk is not found");
		return false;
	}

	llvm::JITMemoryManager* jitMemoryMgr = llvm::JITMemoryManager::CreateDefaultMemManager();
	engineBuilder.setJITMemoryManager(jitMemoryMgr);
	uchar_t* p = jitMemoryMgr->allocateCodeSection(128, 0, 0, llvm::StringRef());

	// mov r11, __chkstk

	p[0] = 0x49;
	p[1] = 0xbb;
	*(void**)(p + 2) = chkstk;

	// jmp r11

	p[10] = 0x41;
	p[11] = 0xff;
	p[12] = 0xe3;

	llvm::sys::DynamicLibrary::AddSymbol("__chkstk", p);
#		endif // _JNC_OS_WIN && _JNC_CPU_AMD64
	engineBuilder.setUseMCJIT(false);
#	endif // _JNC_JIT == JNC_JIT_LLVM_LEGACY

	engineBuilder.setTargetOptions(targetOptions);

	// disable CPU feature auto-detection
	// alas, making use of certain CPU features leads to JIT crashes (e.g. test114.jnc)

	engineBuilder.setMCPU("generic");
#	if (_JNC_CPU_X86)
	engineBuilder.setMArch("x86");
#	endif

	m_llvmExecutionEngine = engineBuilder.create();
	if (!m_llvmExecutionEngine) {
		err::setFormatStringError("cannot create execution engine: %s", errorString.c_str());
		return false;
	}
#endif // _JNC_JIT != JNC_JIT_LLVM_ORC

	return true;
}

bool
Jit::prepare() { // called after everything is mapped and before jitting
#if (_JNC_JIT == JNC_JIT_LLVM_ORC)
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
#endif

	return true;
}

bool
Jit::mapVariable(
	Variable* variable,
	void* p
) {
	if (variable->getStorageKind() != StorageKind_Static) {
		err::setFormatStringError("attempt to map non-global variable: %s", variable->getQualifiedName().sz());
		return false;
	}

	variable->m_staticData = p;

	llvm::GlobalVariable* llvmVariable = !variable->getLlvmGlobalVariableName().isEmpty() ?
		m_module->getLlvmModule()->getGlobalVariable(variable->getLlvmGlobalVariableName() >> toLlvm) :
		variable->getLlvmGlobalVariable();

	if (!llvmVariable) // optimized out
		return true;

#if (_JNC_JIT == JNC_JIT_LLVM_LEGACY_JIT)
	ASSERT(m_llvmExecutionEngine);
	m_llvmExecutionEngine->addGlobalMapping(llvmVariable, p);
#else
	std::string name = llvmVariable->getName().str();
	name += ".mapping";

	llvm::GlobalVariable* llvmMapping = new llvm::GlobalVariable(
		*m_module->getLlvmModule(),
		variable->getType()->getLlvmType(),
		false,
		llvm::GlobalVariable::ExternalWeakLinkage,
		NULL,
		name
	);

	llvmVariable->replaceAllUsesWith(llvmMapping);
	llvmVariable->eraseFromParent();

	sl::StringHashTableIterator<void*> it = m_functionMap.visit(llvmMapping->getName().data());
	if (it->m_value) {
		err::setFormatStringError("attempt to re-map variable: %s", variable->getQualifiedName().sz());
		return false;
	}

	it->m_value = p;

#	if (LLVM_VERSION >= 0x040000)
#		if (_JNC_JIT != JNC_JIT_LLVM_ORC)
	ASSERT(m_llvmExecutionEngine);
	m_llvmExecutionEngine->addGlobalMapping(llvmMapping, p);
#		endif
#	endif
#endif // _JNC_JIT != JNC_JIT_LLVM_LEGACY_JIT

	return true;
}

bool
Jit::mapFunction(
	Function* function,
	void* p
) {
	function->m_machineCode = p;

	if (!function->hasLlvmFunction()) // never used
		return true;

	llvm::Function* llvmFunction = !function->m_llvmFunctionName.isEmpty() ?
		m_module->getLlvmModule()->getFunction(function->m_llvmFunctionName >> toLlvm) :
		function->getLlvmFunction();

	if (!llvmFunction) // optimized out
		return true;

#if (_JNC_JIT != JNC_JIT_LLVM_LEGACY_JIT)
	sl::StringHashTableIterator<void*> it = m_functionMap.visit(llvmFunction->getName().data());
	if (it->m_value) {
		err::setFormatStringError("attempt to re-map function: %s/%s", function->getQualifiedName().sz(), llvmFunction->getName().data());
		return false;
	}

	it->m_value = p;
#elif (_JNC_JIT == JNC_JIT_LLVM_LEGACY_JIT)
	ASSERT(m_llvmExecutionEngine);
	m_llvmExecutionEngine->addGlobalMapping(llvmFunction, p);
#endif

	return true;
}

void*
Jit::findFunctionMapping(const sl::StringRef& name) {
#if (_JNC_OS_WIN && _JNC_CPU_X86)
	bool isUnderscorePrefix = name.isPrefix(sl::StringRef("_", 1, true));
#else
	bool isUnderscorePrefix = name.isPrefix(sl::StringRef("_?", 2, true));
#endif

#if (_JNC_JIT != JNC_JIT_LLVM_LEGACY_JIT)
	sl::StringHashTableIterator<void*> it = isUnderscorePrefix ?
		m_functionMap.find(name.getSubString(1)) :
		m_functionMap.find(name);

	return it ? it->m_value : NULL;
#else
	return NULL;
#endif
}

void*
Jit::jit(Function* function) {
#if (_JNC_JIT == JNC_JIT_LLVM_ORC)
	llvm::Expected<llvm::JITEvaluatedSymbol> symbol = m_llvmExecutionSession->lookup(
		llvm::ArrayRef<llvm::orc::JITDylib*>(m_llvmJitDylib),
		function->getLlvmFunction()->getName()
	);

	if (!symbol) {
		err::setError(symbol.takeError() >> toAxl);
		return NULL;
	}

	return (void*)symbol->getAddress();
#else
	return m_llvmExecutionEngine->getPointerToFunction(function->getLlvmFunction());
#endif
}

void*
Jit::getStaticData(Variable* variable) {
#if (_JNC_JIT == JNC_JIT_LLVM_ORC)
	llvm::Expected<llvm::JITEvaluatedSymbol> symbol = m_llvmExecutionSession->lookup(
		llvm::ArrayRef<llvm::orc::JITDylib*>(m_llvmJitDylib),
		variable->getLlvmGlobalVariable()->getName()
	);

	return (void*)symbol->getAddress();
#elif (_JNC_JIT == JNC_JIT_LLVM_MCJIT)
	return (void*)m_llvmExecutionEngine->getGlobalValueAddress(variable->getLlvmGlobalVariable()->getName().str());
#elif (_JNC_JIT == JNC_JIT_LLVM_LEGACY_JIT)
	return (void*)m_llvmExecutionEngine->getPointerToGlobal(variable->getLlvmGlobalVariable());
#endif
}

bool
Jit::finalizeObject() {
#if (_JNC_JIT != JNC_JIT_LLVM_ORC)
	m_llvmExecutionEngine->finalizeObject();
#endif
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
