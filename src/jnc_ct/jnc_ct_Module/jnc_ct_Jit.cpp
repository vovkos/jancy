#include "pch.h"
#include "jnc_ct_Jit.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

void
disableLlvmGlobalMerge() {
#if (LLVM_VERSION < 0x030700)
	llvm::StringMap<llvm::cl::Option*> options;
	llvm::cl::getRegisteredOptions(options);
#else
	llvm::StringMap<llvm::cl::Option*>& options = llvm::cl::getRegisteredOptions();
#endif

	llvm::StringMap<llvm::cl::Option*>::iterator globalMergeIt = options.find("global-merge");
	if (globalMergeIt != options.end()) {
		llvm::cl::opt<bool>* globalMerge = (llvm::cl::opt<bool>*)globalMergeIt->second;
		ASSERT(llvm::isa<llvm::cl::opt<bool> >(globalMerge));
		globalMerge->setValue(false);
	}
}

//..............................................................................

void*
Jit::findSymbol(const sl::StringRef& name) {
#if (_JNC_OS_WIN && _JNC_CPU_X86)
	bool isUnderscorePrefix = name.isPrefix(sl::StringRef("_", 1, true));
#else
	bool isUnderscorePrefix = name.isPrefix(sl::StringRef("_?", 2, true));
#endif

	sl::StringHashTableIterator<void*> it = isUnderscorePrefix ?
		m_symbolMap.find(name.getSubString(1)) :
		m_symbolMap.find(name);

	return it ? it->m_value : NULL;
}

void
Jit::addStdSymbols() {
	m_symbolMap["memset"] = (void*)memset;
	m_symbolMap["memcpy"] = (void*)memcpy;
	m_symbolMap["memmove"] = (void*)memmove;
#if (_JNC_OS_DARWIN)
	m_symbolMap["_bzero"] = (void*)bzero;
	m_symbolMap["___bzero"] = (void*)bzero;
#endif

#if (JNC_PTR_BITS == 32)
#	if (_JNC_OS_POSIX)
	m_symbolMap["__divdi3"] = (void*)__divdi3;
	m_symbolMap["__moddi3"] = (void*)__moddi3;
	m_symbolMap["__udivdi3"] = (void*)__udivdi3;
	m_symbolMap["__umoddi3"] = (void*)__umoddi3;
#		if (_JNC_CPU_ARM32)
	m_symbolMap["__modsi3"] = (void*)__modsi3;
	m_symbolMap["__umodsi3"] = (void*)__umodsi3;
	m_symbolMap["__aeabi_idiv"] = (void*)__aeabi_idiv;
	m_symbolMap["__aeabi_uidiv"] = (void*)__aeabi_uidiv;
	m_symbolMap["__aeabi_ldivmod"] = (void*)__aeabi_ldivmod;
	m_symbolMap["__aeabi_uldivmod"] = (void*)__aeabi_uldivmod;
	m_symbolMap["__aeabi_i2f"] = (void*)__aeabi_i2f;
	m_symbolMap["__aeabi_l2f"] = (void*)__aeabi_l2f;
	m_symbolMap["__aeabi_ui2f"] = (void*)__aeabi_ui2f;
	m_symbolMap["__aeabi_ul2f"] = (void*)__aeabi_ul2f;
	m_symbolMap["__aeabi_i2d"] = (void*)__aeabi_i2d;
	m_symbolMap["__aeabi_l2d"] = (void*)__aeabi_l2d;
	m_symbolMap["__aeabi_ui2d"] = (void*)__aeabi_ui2d;
	m_symbolMap["__aeabi_ul2d"] = (void*)__aeabi_ul2d;
	m_symbolMap["__aeabi_memcpy"] = (void*)__aeabi_memcpy;
	m_symbolMap["__aeabi_memmove"] = (void*)__aeabi_memmove;
	m_symbolMap["__aeabi_memset"] = (void*)__aeabi_memset;
#		endif // _JNC_CPU_ARM32
#	elif (_JNC_OS_WIN)
	m_symbolMap["_alldiv"] = (void*)_alldiv;
	m_symbolMap["_allrem"] = (void*)_allrem;
	m_symbolMap["_aulldiv"] = (void*)_aulldiv;
	m_symbolMap["_aullrem"] = (void*)_aullrem;
#	endif // _JNC_OS_WIN
#endif // JNC_PTR_BITS == 32
}


llvm::GlobalVariable*
Jit::createLlvmGlobalVariableMapping(Variable* variable) {
	llvm::GlobalVariable* llvmVariable = getLlvmGlobalVariable(variable);
	if (!llvmVariable) // optimized out
		return NULL;

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
	return llvmMapping;
}

//..............................................................................

} // namespace ct
} // namespace jnc
