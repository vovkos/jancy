#include "pch.h"
#include "jnc_ct_LegacyJit.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
LegacyJit::create() {
	ASSERT(!m_llvmExecutionEngine);

#if (_JNC_CPU_ARM32 || _JNC_CPU_ARM64)
	// disable the GlobalMerge pass (on by default) on ARM because
	// it will dangle GlobalVariable::m_llvmVariable pointers

	disableLlvmGlobalMerge();
#endif

	llvm::EngineBuilder engineBuilder(m_module->getLlvmModule());

	std::string errorString;
	engineBuilder.setErrorStr(&errorString);
	engineBuilder.setEngineKind(llvm::EngineKind::JIT);
	engineBuilder.setOptLevel((llvm::CodeGenOpt::Level)0);

	llvm::TargetOptions targetOptions;
#if (_JNC_CPU_ARM32 || _JNC_CPU_ARM64)
	targetOptions.FloatABIType = llvm::FloatABI::Hard;
#endif

#if (_JNC_OS_WIN && _JNC_CPU_AMD64)
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
#endif // _JNC_OS_WIN && _JNC_CPU_AMD64

	engineBuilder.setUseMCJIT(false);
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
LegacyJit::mapVariable(
	Variable* variable,
	void* p
) {
	ASSERT(m_llvmExecutionEngine);

	if (variable->getStorageKind() != StorageKind_Static) {
		err::setFormatStringError("attempt to map non-global variable: %s", variable->getQualifiedName().sz());
		return false;
	}

	setVariableStaticData(variable, p);

	llvm::GlobalVariable* llvmVariable = getLlvmGlobalVariable(variable);
	if (llvmVariable)
		m_llvmExecutionEngine->addGlobalMapping(llvmVariable, p);

	return true;
}

bool
LegacyJit::mapFunction(
	Function* function,
	void* p
) {
	ASSERT(m_llvmExecutionEngine);

	setFunctionMachineCode(function, p);

	llvm::Function* llvmFunction = getLlvmFunction(function);
	if (llvmFunction)
		m_llvmExecutionEngine->addGlobalMapping(llvmFunction, p);

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
