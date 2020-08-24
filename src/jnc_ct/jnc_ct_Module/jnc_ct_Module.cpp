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

#include "pch.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_JitMemoryMgr.h"
#include "jnc_ct_Parser.llk.h"

#if (_AXL_DEBUG)
#	define _JNC_TEST_NO_CODE_GEN 1
#endif

namespace jnc {

//..............................................................................

axl::sl::String*
getTlsStringBuffer()
{
	static int32_t flag = 0;
	sys::TlsSlot* slot = sl::getSimpleSingleton<sys::TlsSlot> (&flag);

	sl::String* oldStringBuffer = (sl::String*)sys::getTlsMgr()->getSlotValue(*slot).p();
	if (oldStringBuffer)
		return oldStringBuffer;

	ref::Ptr<sl::String> newStringBuffer = AXL_REF_NEW(ref::Box<sl::String>);
	sys::getTlsMgr()->setSlotValue(*slot, newStringBuffer);
	return newStringBuffer;
}

namespace ct {

//..............................................................................

Module::Module():
	m_doxyModule(&m_doxyHost)
{
	m_compileFlags = ModuleCompileFlag_StdFlags;
	m_compileState = ModuleCompileState_Idle;
	m_tryCompileLevel = 0;
	m_compileErrorCount = 0;
	m_compileErrorCountLimit = DefaultErrorCountLimit;
	m_compileErrorHandler = NULL;
	m_compileErrorHandlerContext = NULL;
	m_constructor = NULL;

	m_llvmContext = NULL;
	m_llvmModule = NULL;
	m_llvmExecutionEngine = NULL;

	finalizeConstruction();
}

Module::~Module()
{
	clear();
}

bool
Module::processCompileError(ModuleCompileErrorKind errorKind)
{
	if (m_tryCompileLevel)
		return false;

	if (++m_compileErrorCount > m_compileErrorCountLimit)
	{
		err::setFormatStringError("%d errors; error limit reached", m_compileErrorCount);
		return false;
	}

	if (m_compileErrorCount == 1) // stop code generation after the very first error
		clearLlvm();

	bool result =
		m_compileErrorHandler &&
		m_compileErrorHandler(m_compileErrorHandlerContext, errorKind);

	if (!result)
		return false;

	if (errorKind >= ModuleCompileErrorKind_PostParse)
	{
		m_namespaceMgr.closeAllNamespaces();
		m_functionMgr.setCurrentFunction(NULL);
		m_controlFlowMgr.setCurrentBlock(NULL);

		// probably, need more cleanup
	}

	return true;
}

void
Module::clear()
{
	m_name.clear();
	m_compileArray.clear();
	m_sourceList.clear();
	m_filePathSet.clear();
	m_functionMap.clear();
	m_requireSet.clear();

	m_typeMgr.clear();
	m_namespaceMgr.clear();
	m_functionMgr.clear();
	m_variableMgr.clear();
	m_constMgr.clear();
	m_operatorMgr.clear();
	m_unitMgr.clear();
	m_importMgr.clear();
	m_extensionLibMgr.clear();
	m_doxyModule.clear();
	m_controlFlowMgr.clear();
	m_gcShadowStackMgr.clear();
	m_regexMgr.clear();
	m_codeAssistMgr.clear();

	clearLlvm();

	m_constructor = NULL;
	m_compileFlags = ModuleCompileFlag_StdFlags;
	m_compileState = ModuleCompileState_Idle;
	m_compileErrorCount = 0;
	m_tryCompileLevel = 0;
}

void
Module::clearLlvm()
{
	m_llvmIrBuilder.clear();
	m_llvmDiBuilder.clear();

	if (m_llvmExecutionEngine)
		delete m_llvmExecutionEngine;
	else if (m_llvmModule)
		delete m_llvmModule;

	if (m_llvmContext)
		delete m_llvmContext;

	m_llvmContext = NULL;
	m_llvmModule = NULL;
	m_llvmExecutionEngine = NULL;

	m_compileFlags &= ~(
		ModuleCompileFlag_DebugInfo |
		ModuleCompileFlag_GcSafePointInPrologue |
		ModuleCompileFlag_GcSafePointInInternalPrologue
		);
}

void
Module::initialize(
	const sl::StringRef& name,
	uint_t compileFlags
	)
{
	clear();

#if (_AXL_GCC_ASAN)
	// GC guard page safe points do not work with address sanitizer
	compileFlags |= ModuleCompileFlag_SimpleGcSafePoint;
#endif

#if (LLVM_VERSION >= 0x030600)
	compileFlags |= ModuleCompileFlag_McJit;
#endif

	m_name = name;
	m_compileFlags = compileFlags;
	m_compileState = ModuleCompileState_Idle;
	m_compileErrorCount = 0;

	if (!(compileFlags & ModuleCompileFlag_DisableCodeGen))
	{
		m_llvmContext = new llvm::LLVMContext;
		m_llvmModule = new llvm::Module("jncModule", *m_llvmContext);
		m_llvmIrBuilder.create();

		if (compileFlags & ModuleCompileFlag_DebugInfo)
			m_llvmDiBuilder.create();
	}

	if (!(compileFlags & ModuleCompileFlag_StdLibDoc))
	{
		m_extensionLibMgr.addStaticLib(jnc_CoreLib_getLib());
		m_extensionLibMgr.addStaticLib(jnc_IntrospectionLib_getLib());
		m_typeMgr.createStdTypes();
		m_variableMgr.createStdVariables();
		m_namespaceMgr.addStdItems();
	}
}

CodeAssist*
Module::generateCodeAssist(
	jnc_CodeAssistKind kind,
	Module* cacheModule,
	size_t offset,
	const sl::StringRef& source
	)
{
	m_compileFlags |= ModuleCompileFlag_DisableCodeGen;
	m_codeAssistMgr.initialize(kind, cacheModule, offset);
	parse("code-assist-source", source);
	parseImports();
	return m_codeAssistMgr.generateCodeAssist();
}

#if (JNC_PTR_BITS == 32)
#	if (_JNC_OS_POSIX)
extern "C" int64_t __divdi3(int64_t, int64_t);
extern "C" int64_t __moddi3(int64_t, int64_t);
extern "C" uint64_t __udivdi3(uint64_t, uint64_t);
extern "C" uint64_t __umoddi3(uint64_t, uint64_t);
#		if (_JNC_CPU_ARM32)
struct DivModRetVal
{
	uint64_t m_quotient;
	uint64_t m_remainder;
};
extern "C" int32_t __aeabi_idiv(int32_t);
extern "C" uint32_t __aeabi_uidiv(uint32_t);
extern "C" DivModRetVal __aeabi_ldivmod(int64_t, int64_t);
extern "C" DivModRetVal __aeabi_uldivmod(uint64_t, uint64_t);
extern "C" float __aeabi_i2f(int32_t);
extern "C" float __aeabi_l2f(int64_t);
extern "C" float __aeabi_ui2f(uint32_t);
extern "C" float __aeabi_ul2f(uint64_t);
extern "C" double __aeabi_i2d(int32_t);
extern "C" double __aeabi_l2d(int64_t);
extern "C" double __aeabi_ui2d(uint32_t);
extern "C" double __aeabi_ul2d(uint64_t);
extern "C" void __aeabi_memcpy(void*, const void*, size_t);
extern "C" void __aeabi_memmove(void*, const void*, size_t);
extern "C" void __aeabi_memset(void*, int, size_t);
#		endif
#	elif (_JNC_OS_WIN)
extern "C" int64_t _alldiv(int64_t, int64_t);
extern "C" int64_t _allrem(int64_t, int64_t);
extern "C" int64_t _aulldiv(int64_t, int64_t);
extern "C" int64_t _aullrem(int64_t, int64_t);
#	endif
#endif

bool
Module::createLlvmExecutionEngine()
{
	ASSERT(!m_llvmExecutionEngine);

#if (_JNC_CPU_ARM32 || _JNC_CPU_ARM64)
	// disable the GlobalMerge pass (on by default) on ARM because
	// it will dangle GlobalVariable::m_llvmVariable pointers

	llvm::StringMap<llvm::cl::Option*> options;
	llvm::cl::getRegisteredOptions(options);
	llvm::cl::opt<bool>* enableMerge = (llvm::cl::opt<bool>*)options.find("global-merge")->second;
	ASSERT(llvm::isa<llvm::cl::opt<bool> > (enableMerge));

	enableMerge->setValue(false);
#endif

#if (LLVM_VERSION < 0x030600)
	llvm::EngineBuilder engineBuilder(m_llvmModule);
#else
	llvm::EngineBuilder engineBuilder(std::move(std::unique_ptr<llvm::Module> (m_llvmModule)));
#endif

	std::string errorString;
	engineBuilder.setErrorStr(&errorString);
	engineBuilder.setEngineKind(llvm::EngineKind::JIT);

	llvm::TargetOptions targetOptions;

#if (_JNC_CPU_ARM32 || _JNC_CPU_ARM64)
	targetOptions.FloatABIType = llvm::FloatABI::Hard;
#endif

	if (m_compileFlags & ModuleCompileFlag_McJit)
	{
		JitMemoryMgr* jitMemoryMgr = new JitMemoryMgr(this);
#if (LLVM_VERSION < 0x030600)
		engineBuilder.setUseMCJIT(true);
		engineBuilder.setMCJITMemoryManager(jitMemoryMgr);
		targetOptions.JITEmitDebugInfo = true;
#elif (LLVM_VERSION < 0x030700)
		engineBuilder.setMCJITMemoryManager(std::move(std::unique_ptr<JitMemoryMgr> (jitMemoryMgr)));
		targetOptions.JITEmitDebugInfo = true;
#else
		engineBuilder.setMCJITMemoryManager(std::move(std::unique_ptr<JitMemoryMgr> (jitMemoryMgr)));
#endif

		m_functionMap["memset"] = (void*)memset;
		m_functionMap["memcpy"] = (void*)memcpy;
		m_functionMap["memmove"] = (void*)memmove;
#if (_JNC_OS_DARWIN)
		m_functionMap["___bzero"] = (void*)bzero;
#endif
#if (JNC_PTR_BITS == 32)
#	if (_JNC_OS_POSIX)
		m_functionMap["__divdi3"] = (void*)__divdi3;
		m_functionMap["__moddi3"] = (void*)__moddi3;
		m_functionMap["__udivdi3"] = (void*)__udivdi3;
		m_functionMap["__umoddi3"] = (void*)__umoddi3;
#		if (_JNC_CPU_ARM32)
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
#		endif
#	elif (_JNC_OS_WIN)
		m_functionMap["_alldiv"] = (void*)_alldiv;
		m_functionMap["_allrem"] = (void*)_allrem;
		m_functionMap["_aulldiv"] = (void*)_aulldiv;
		m_functionMap["_aullrem"] = (void*)_aullrem;
#	endif
#endif
	}
	else
	{
#if (LLVM_VERSION >= 0x030600) // legacy JIT is gone in LLVM 3.6
		ASSERT(false); // should have been checked earlier
#else
#	if (_JNC_OS_WIN && _JNC_CPU_AMD64)
		// legacy JIT uses relative call to __chkstk
		// it worked just fine before windows 10 which loads ntdll.dll too far away

		// the fix should go to LLVM, of course, but
		// a) applying a patch to LLVM before building Jancy would be a pain in the ass
		// b) legacy JIT is a gonner anyway

		// therefore, a simple workaround is used: allocate a proxy for __chkstk
		// which would reside close enough to the generated code

		void* chkstk = ::GetProcAddress(::GetModuleHandleA("ntdll.dll"), "__chkstk");
		if (!chkstk)
		{
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
#	endif

		engineBuilder.setUseMCJIT(false);
#endif
	}

	engineBuilder.setTargetOptions(targetOptions);

	// disable CPU feature auto-detection
	// alas, making use of certain CPU features leads to JIT crashes (e.g. test114.jnc)

	engineBuilder.setMCPU("generic");

#if (_JNC_CPU_X86)
	engineBuilder.setMArch("x86");
#endif

	sys::ScopedTlsPtrSlot<Module> scopeModule(this); // for GcShadowStack

	m_llvmExecutionEngine = engineBuilder.create();
	if (!m_llvmExecutionEngine)
	{
		err::setFormatStringError("cannot create execution engine: %s", errorString.c_str());
		return false;
	}

	return true;
}

bool
Module::mapVariable(
	Variable* variable,
	void* p
	)
{
	if (variable->getStorageKind() != StorageKind_Static)
	{
		err::setFormatStringError("attempt to map non-global variable: %s", variable->getQualifiedName().sz());
		return false;
	}

	llvm::GlobalVariable* llvmVariable = !variable->m_llvmGlobalVariableName.isEmpty() ?
		m_llvmModule->getGlobalVariable(variable->m_llvmGlobalVariableName >> toLlvm) :
		variable->getLlvmGlobalVariable();

	if (!llvmVariable) // optimized out
	{
		variable->m_staticData = p;
		return true;
	}

	if (m_compileFlags & ModuleCompileFlag_McJit)
	{
		std::string name = llvmVariable->getName();
		name += ".mapping";

		llvm::GlobalVariable* llvmMapping = new llvm::GlobalVariable(
			*m_llvmModule,
			variable->getType()->getLlvmType(),
			false,
			llvm::GlobalVariable::ExternalWeakLinkage,
			NULL,
			name
			);

		llvmVariable->replaceAllUsesWith(llvmMapping);
		llvmVariable->eraseFromParent();

#if (LLVM_VERSION >= 0x040000)
		ASSERT(m_llvmExecutionEngine);
		m_llvmExecutionEngine->addGlobalMapping(llvmMapping, p);
#else
		sl::StringHashTableIterator<void*> it = m_functionMap.visit(llvmMapping->getName().data());
		if (it->m_value)
		{
			err::setFormatStringError("attempt to re-map variable: %s", variable->getQualifiedName().sz());
			return false;
		}

		it->m_value = p;
#endif
	}
	else
	{
		ASSERT(m_llvmExecutionEngine);
		m_llvmExecutionEngine->addGlobalMapping(llvmVariable, p);
	}

	variable->m_staticData = p;
	return true;
}

bool
Module::mapFunction(
	Function* function,
	void* p
	)
{
	if (!function->hasLlvmFunction()) // never used
	{
		function->m_machineCode = p;
		return true;
	}

	llvm::Function* llvmFunction = !function->m_llvmFunctionName.isEmpty() ?
		m_llvmModule->getFunction(function->m_llvmFunctionName >> toLlvm) :
		function->getLlvmFunction();

	if (!llvmFunction) // optimized out
	{
		function->m_machineCode = p;
		return true;
	}

	if (m_compileFlags & ModuleCompileFlag_McJit)
	{
		sl::StringHashTableIterator<void*> it = m_functionMap.visit(llvmFunction->getName().data());
		if (it->m_value)
		{
			err::setFormatStringError("attempt to re-map function: %s/%s", function->getQualifiedName().sz(), llvmFunction->getName().data());
			return false;
		}

		it->m_value = p;
	}
	else
	{
		ASSERT(m_llvmExecutionEngine);
		m_llvmExecutionEngine->addGlobalMapping(llvmFunction, p);
	}

	function->m_machineCode = p;
	return true;
}

void*
Module::findFunctionMapping(const sl::StringRef& name)
{
	sl::StringHashTableIterator<void*> it;

#if (_JNC_OS_WIN && _JNC_CPU_X86)
	bool isUnderscorePrefix = name.isPrefix(sl::StringRef("_", 1, true));
#else
	bool isUnderscorePrefix = name.isPrefix(sl::StringRef("_?", 2, true));
#endif

	it = isUnderscorePrefix ?
		m_functionMap.find(name.getSubString(1)) :
		m_functionMap.find(name);

	return it ? it->m_value : NULL;
}

bool
Module::setFunctionPointer(
	llvm::ExecutionEngine* llvmExecutionEngine,
	const sl::StringRef& name,
	void* p
	)
{
	QualifiedName qualifiedName;
	qualifiedName.parse(name);
	return setFunctionPointer(llvmExecutionEngine, qualifiedName, p);
}

bool
Module::setFunctionPointer(
	llvm::ExecutionEngine* llvmExecutionEngine,
	const QualifiedName& name,
	void* p
	)
{
	FindModuleItemResult findResult = m_namespaceMgr.getGlobalNamespace()->findItem(name);
	if (!findResult.m_item)
		return false;

	if (!findResult.m_item)
	{
		err::setFormatStringError("'%s' not found", name.getFullName().sz());
		return false;
	}

	if (findResult.m_item->getItemKind() != ModuleItemKind_Function)
	{
		err::setFormatStringError("'%s' is not a function", name.getFullName().sz());
		return false;
	}

	llvm::Function* llvmFunction = ((Function*)findResult.m_item)->getLlvmFunction();
	if (!llvmFunction)
		return false;

	llvmExecutionEngine->addGlobalMapping(llvmFunction, p);
	return true;
}

void
Module::markForCompile(Function* function)
{
	if (function->m_flags & ModuleItemFlag_NeedCompile)
		return;

	function->m_flags |= ModuleItemFlag_NeedCompile;
	m_compileArray.append(function);
}

bool
Module::parseImpl(
	ExtensionLib* lib,
	const sl::StringRef& fileName,
	const sl::StringRef& source
	)
{
	ASSERT(m_compileState < ModuleCompileState_Compiled);

	bool result;

	Unit* unit = m_unitMgr.createUnit(lib, fileName);
	m_unitMgr.setCurrentUnit(unit);

	Lexer lexer(LexerMode_Parse);
	lexer.create(fileName, source);

#if (0)
	for (;;)
	{
		const Token* token = lexer.getToken();
		switch (token->m_token)
		{
		case TokenKind_Error:
			printf("lexer error: %s\n", err::getLastErrorDescription().sz());
			return false;

		case TokenKind_Eof:
			printf("EOF\n");
			err::setError("lexer-test-exit");
			return false;
		}

		printf(
			"token(%3d) @%3d: %s\n",
			token->m_tokenKind,
			token->m_pos.m_line + 1,
			sl::StringRef(token->m_pos.m_p, token->m_pos.m_length).sz()
			);

		lexer.nextToken();
	}

	err::setError("lexer-test-exit");
	return false;
#endif

	if ((m_compileFlags & ModuleCompileFlag_Documentation) && !lib)
		lexer.m_channelMask = TokenChannelMask_All; // also include doxy-comments (but not for libs!)

	Parser parser(this);
	parser.create(fileName, Parser::StartSymbol);

	CodeAssistKind codeAssistKind = m_codeAssistMgr.getCodeAssistKind();
	if (!codeAssistKind || !unit->isRootUnit())
	{
		for (;;)
		{
			const Token* token = lexer.getToken();
			if (token->m_token == TokenKind_Error)
			{
				err::setFormatStringError("invalid character '\\x%02x'", (uchar_t) token->m_data.m_integer);
				lex::pushSrcPosError(fileName, token->m_pos);
				return false;
			}

			result = parser.parseToken(token);
			if (!result)
			{
				lex::ensureSrcPosError(fileName, token->m_pos);
				return false;
			}

			if (token->m_token == TokenKind_Eof) // EOF token must be parsed
				break;

			lexer.nextToken();
		}
	}
	else
	{
		size_t offset = m_codeAssistMgr.getOffset();

		for (;;)
		{
			const Token* token = lexer.getToken();
			if (token->m_token == TokenKind_Error)
				return false;

			markCodeAssistToken((Token*)token, offset);

			if (token->m_flags & TokenFlag_PostCodeAssist)
			{
				m_codeAssistMgr.prepareAutoCompleteFallback();
				offset = -1; // not needed anymore
			}

			result = parser.parseToken(token);
			if (!result)
				return false;

			if (token->m_token == TokenKind_Eof) // EOF token must be parsed
				break;

			lexer.nextToken();
		}
	}

	m_namespaceMgr.getGlobalNamespace()->getUsingSet()->clear();
	return true;
}

bool
Module::parseFile(const sl::StringRef& fileName)
{
	ASSERT(m_compileState < ModuleCompileState_Compiled);

	sl::String filePath = io::getFullFilePath(fileName);
	sl::StringHashTableIterator<bool> it = m_filePathSet.find(filePath);
	if (it)
		return true; // already parsed

	io::SimpleMappedFile file;
	bool result = file.open(filePath, io::FileFlag_ReadOnly);
	if (!result)
		return false;

	size_t length = file.getMappingSize();
	sl::String source((const char*)file.p(), length);

	m_sourceList.insertTail(source);
	m_filePathSet.visit(filePath);
	return parseImpl(NULL, filePath, source);
}

bool
Module::parseImports()
{
	ASSERT(m_compileState < ModuleCompileState_Compiled);

	bool result;

	for (;;)
	{
		sl::List<Import> importList;
		m_importMgr.takeOverImports(&importList);
		if (importList.isEmpty())
			break;

		sl::ConstIterator<Import> importIt = importList.getHead();
		for (; importIt; importIt++)
		{
			result = importIt->m_importKind == ImportKind_Source ?
				parseImpl(
					importIt->m_lib,
					importIt->m_filePath,
					importIt->m_source
					) :
				parseFile(importIt->m_filePath);

			if (!result)
				return false;
		}
	}

	m_extensionLibMgr.closeDynamicLibZipReaders();
	m_compileState = ModuleCompileState_Parsed;
	return true;
}

bool
Module::compile()
{
	bool result;

	ASSERT(m_compileState < ModuleCompileState_Compiled);
	if (m_compileState < ModuleCompileState_Parsed)
	{
		result = parseImports();
		if (!result)
			return false;
	}

	result =
		m_namespaceMgr.getGlobalNamespace()->resolveOrphans() &&
		m_variableMgr.allocateNamespaceVariables(sl::ConstIterator<Variable>()) &&
		m_functionMgr.finalizeNamespaceProperties(sl::ConstIterator<Property>()) &&
		processRequireSet() &&
		processCompileArray();

	if (!result)
		return false;

	if (m_compileErrorCount)
	{
		err::setFormatStringError("%d error(s); compilation failed", m_compileErrorCount);
		return false;
	}

	if (hasCodeGen())
	{
		createConstructor();

		result = m_variableMgr.createTlsStructType();
		if (!result)
			return false;

		m_functionMgr.injectTlsPrologues();
		m_functionMgr.replaceAsyncAllocas(); // after replacing TLS allocas!

		result = m_controlFlowMgr.deleteUnreachableBlocks();
		if (!result)
			return false;
	}

	if (m_compileFlags & ModuleCompileFlag_DebugInfo)
		m_llvmDiBuilder.finalize();

	m_compileState = ModuleCompileState_Compiled;
	return true;
}

bool
Module::optimize(uint_t level)
{
	// optimization requires knowledge of the DataLayout for TargetMachine

	if (!m_llvmExecutionEngine)
	{
		bool result = createLlvmExecutionEngine();
		if (!result)
			return false;
	}

	// before running LLVM optimization passes, save LLVM names of declarations --
	// corresponding llvm::Function*/llvm::GlobalVariable* may be optimized out

	// things would be much easier if we could just derive from llvm::Function
	// and override eraseFromParent -- then we could update jnc::ct::Function when
	// llvm::Function is optimized out

	// alas, we can't -- the constructor of llvm::Function is private

	sl::Iterator<Function> it = m_functionMgr.m_functionList.getHead();
	for (; it; it++)
	{
		if (!it->hasLlvmFunction())
			continue;

		llvm::Function* llvmFunction = it->getLlvmFunction();
		if (llvmFunction->isDeclaration())
			it->m_llvmFunctionName = llvmFunction->getName() >> toAxl;
	}

	size_t count = m_variableMgr.m_staticVariableArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = m_variableMgr.m_staticVariableArray[i];
		llvm::GlobalVariable* llvmGlobalVariable = variable->getLlvmGlobalVariable();
		if (llvmGlobalVariable->isDeclaration())
			variable->m_llvmGlobalVariableName = llvmGlobalVariable->getName() >> toAxl;
	}

	llvm::PassManagerBuilder passManagerBuilder;
	passManagerBuilder.OptLevel = level;
	passManagerBuilder.SizeLevel = 0;
	passManagerBuilder.Inliner = llvm::createFunctionInliningPass();

	llvm::legacy::PassManager llvmModulePassMgr;
	llvm::legacy::FunctionPassManager llvmFunctionPassMgr(m_llvmModule);
	passManagerBuilder.populateModulePassManager(llvmModulePassMgr);
	passManagerBuilder.populateFunctionPassManager(llvmFunctionPassMgr);

	llvmModulePassMgr.run(*m_llvmModule);

	llvmFunctionPassMgr.doInitialization();

	it = m_functionMgr.m_functionList.getHead();
	for (; it; it++)
		if (!it->isEmpty())
			llvmFunctionPassMgr.run(*it->getLlvmFunction());

	llvmFunctionPassMgr.doFinalization();
	return true;
}

bool
Module::jit()
{
	bool result;

	ASSERT(m_compileState < ModuleCompileState_Jitted);
	if (m_compileState < ModuleCompileState_Compiled)
	{
		result = compile();
		if (!result)
			return false;
	}

	if (!m_llvmExecutionEngine)
	{
		result = createLlvmExecutionEngine();
		if (!result)
			return false;
	}

	result =
		m_extensionLibMgr.mapAddresses() &&
		m_functionMgr.jitFunctions();

	if (!result)
		return false;

	m_compileState = ModuleCompileState_Jitted;
	return true;
}

bool
Module::requireIntrospectionLib()
{
	ASSERT(!(m_compileFlags & AuxCompileFlag_IntrospectionLib));

	static StdType introspectionTypeTable[] =
	{
		StdType_ModuleItem,
		StdType_ModuleItemDecl,
		StdType_ModuleItemInitializer,
		StdType_Attribute,
		StdType_AttributeBlock,
		StdType_Namespace,
		StdType_GlobalNamespace,
		StdType_Type,
		StdType_DataPtrType,
		StdType_NamedType,
		StdType_MemberBlock,
		StdType_BaseTypeSlot,
		StdType_DerivableType,
		StdType_ArrayType,
		StdType_BitFieldType,
		StdType_FunctionArg,
		StdType_FunctionType,
		StdType_FunctionPtrType,
		StdType_PropertyType,
		StdType_PropertyPtrType,
		StdType_EnumConst,
		StdType_EnumType,
		StdType_ClassType,
		StdType_ClassPtrType,
		StdType_Field,
		StdType_StructType,
		StdType_UnionType,
		StdType_Alias,
		StdType_Const,
		StdType_Variable,
		StdType_Function,
		StdType_FunctionOverload,
		StdType_Property,
		StdType_Typedef,
		StdType_Module,
		StdType_Unit,
	};

	for (size_t i = 0; i < countof(introspectionTypeTable); i++)
	{
		bool result = m_typeMgr.getStdType(introspectionTypeTable[i])->require();
		if (!result)
			return false;
	}

	m_compileFlags |= AuxCompileFlag_IntrospectionLib;
	return true;
}

bool
Module::processRequireSet()
{
	bool result;

	sl::StringHashTableIterator<RequiredItem> requireIt = m_requireSet.getHead();
	for (; requireIt; requireIt++)
	{
		FindModuleItemResult findResult = m_namespaceMgr.getGlobalNamespace()->findItem(requireIt->getKey());
		if (!findResult.m_result)
			return false;

		if (!findResult.m_item)
		{
			if (!requireIt->m_value.m_isEssential) // not essential
				continue;

			err::setFormatStringError("required module item '%s' not found", requireIt->getKey().sz());
			return false;
		}

		if (requireIt->m_value.m_itemKind != ModuleItemKind_Undefined)
		{
			if (findResult.m_item->getItemKind() != requireIt->m_value.m_itemKind)
			{
				err::setFormatStringError(
					"required module item '%s' item kind mismatch: '%s'",
					requireIt->getKey().sz(),
					getModuleItemKindString(findResult.m_item->getItemKind())
					);
				return false;
			}

			if (requireIt->m_value.m_itemKind == ModuleItemKind_Type &&
				requireIt->m_value.m_typeKind != TypeKind_Void &&
				requireIt->m_value.m_typeKind != ((Type*)findResult.m_item)->getTypeKind())
			{
				err::setFormatStringError(
					"required type '%s' type mismatch: '%s'",
					requireIt->getKey().sz(),
					((Type*)findResult.m_item)->getTypeString().sz()
					);
				return false;
			}
		}

		result = findResult.m_item->require();
		if (!result)
			return false;
	}

	return true;
}

bool
Module::processCompileArray()
{
	bool result;

	// new items could be added in the process, so we need a loop

	while (!m_compileArray.isEmpty())
	{
		sl::Array<Function*> compileArray;
		sl::takeOver(&compileArray, &m_compileArray);

		size_t count = compileArray.getCount();
		for (size_t i = 0; i < compileArray.getCount(); i++)
		{
			Function* function = compileArray[i];
			result = function->compile();
			if (!result)
			{
				lex::ensureSrcPosError(
					function->m_parentUnit ? function->m_parentUnit->getFilePath() : m_name,
					function->m_pos
					);

				result = processCompileError(ModuleCompileErrorKind_PostParse);
				if (!result)
					return false;

				m_namespaceMgr.closeAllNamespaces();
			}

			ASSERT(!m_namespaceMgr.getCurrentScope());
		}

		if (hasCodeGen() && !m_variableMgr.getGlobalVariablePrimeArray().isEmpty())
		{
			Function* function = createGlobalPrimerFunction();
			m_functionMgr.addGlobalCtorDtor(GlobalCtorDtorKind_VariablePrimer, function);
		}

		if (!m_variableMgr.getGlobalVariableInitializeArray().isEmpty())
		{
			Function* function = createGlobalInitializerFunction();
			if (!function)
			{
				result = processCompileError(ModuleCompileErrorKind_PostParse);
				if (!result)
					return false;
			}
			else
			{
				m_functionMgr.addGlobalCtorDtor(GlobalCtorDtorKind_VariableInitializer, function);
			}
		}

		result = m_typeMgr.requireExternalReturnTypes();
		if (!result)
		{
			result = processCompileError(ModuleCompileErrorKind_PostParse);
			if (!result)
				return false;

			return false;
		}
	}

	return true;
}

void
Module::createConstructor()
{
	ASSERT(!m_constructor);

	const sl::Array<Function*>& primerArray = m_functionMgr.getGlobalCtorDtorArray(GlobalCtorDtorKind_VariablePrimer);
	const sl::Array<Function*>& initializerArray = m_functionMgr.getGlobalCtorDtorArray(GlobalCtorDtorKind_VariableInitializer);
	const sl::Array<Function*>& constructorArray = m_functionMgr.getGlobalCtorDtorArray(GlobalCtorDtorKind_Constructor);
	const sl::Array<Function*>& destructorArray = m_functionMgr.getGlobalCtorDtorArray(GlobalCtorDtorKind_Destructor);

	if (primerArray.isEmpty() &&
		initializerArray.isEmpty() &&
		constructorArray.isEmpty() &&
		destructorArray.isEmpty())
		return;

	FunctionType* constructorType = (FunctionType*)m_typeMgr.getStdType(StdType_SimpleFunction);
	m_constructor = m_functionMgr.createInternalFunction("module.construct", constructorType);
	m_constructor->m_storageKind = StorageKind_Static;

	uint_t prevFlags = m_compileFlags;
	m_compileFlags &= ~ModuleCompileFlag_GcSafePointInInternalPrologue;
	m_functionMgr.internalPrologue(m_constructor);
	m_compileFlags = prevFlags;

	size_t count = primerArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_llvmIrBuilder.createCall(primerArray[i], primerArray[i]->getType(), NULL);

	count = initializerArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_llvmIrBuilder.createCall(initializerArray[i], initializerArray[i]->getType(), NULL);

	count = constructorArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_llvmIrBuilder.createCall(constructorArray[i], constructorArray[i]->getType(), NULL);

	count = destructorArray.getCount();
	if (count)
	{
		Function* addStaticDestructor = m_functionMgr.getStdFunction(StdFunc_AddStaticDestructor);
		Type* voidPtrType = m_typeMgr.getStdType(StdType_BytePtr);

		for (size_t i = 0; i < count; i++)
		{
			Value argValue;
			m_llvmIrBuilder.createBitCast(destructorArray[i], voidPtrType, &argValue);
			m_llvmIrBuilder.createCall(addStaticDestructor, addStaticDestructor->getType(), argValue, NULL);
		}
	}

	m_functionMgr.internalEpilogue();
}

Function*
Module::createGlobalPrimerFunction()
{
	FunctionType* type = (FunctionType*)m_typeMgr.getStdType(StdType_SimpleFunction);
	Function* function = m_functionMgr.createInternalFunction("module.primeGlobals", type);
	function->m_storageKind = StorageKind_Static;

	m_functionMgr.internalPrologue(function);
	m_variableMgr.primeGlobalVariables();
	m_functionMgr.internalEpilogue();
	return function;
}

Function*
Module::createGlobalInitializerFunction()
{
	FunctionType* type = (FunctionType*)m_typeMgr.getStdType(StdType_SimpleFunction);
	Function* function = m_functionMgr.createInternalFunction("module.initializeGlobals", type);
	function->m_storageKind = StorageKind_Static;

	m_functionMgr.internalPrologue(function);

	bool result = m_variableMgr.initializeGlobalVariables();
	if (!result)
		return NULL;

	m_functionMgr.internalEpilogue();
	return function;
}

sl::String
Module::getLlvmIrString()
{
	::std::string string;
	llvm::raw_string_ostream stream(string);
	m_llvmModule->print(stream, NULL);
	return string.c_str();
}

//..............................................................................

} // namespace ct
} // namespace jnc
