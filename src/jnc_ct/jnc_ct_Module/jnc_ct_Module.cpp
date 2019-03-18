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

Module::Module()
{
	m_compileFlags = ModuleCompileFlag_StdFlags;
	m_compileState = ModuleCompileState_Idle;

	m_llvmContext = NULL;
	m_llvmModule = NULL;
	m_llvmExecutionEngine = NULL;
	m_constructor = NULL;
	m_destructor = NULL;

	finalizeConstruction();
}

Module::~Module()
{
	clear();
}

void
Module::clear()
{
	m_typeMgr.clear();
	m_namespaceMgr.clear();
	m_functionMgr.clear();
	m_variableMgr.clear();
	m_constMgr.clear();
	m_controlFlowMgr.clear();
	m_operatorMgr.clear();
	m_gcShadowStackMgr.clear();
	m_regexMgr.clear();
	m_unitMgr.clear();
	m_importMgr.clear();
	m_extensionLibMgr.clear();
	m_doxyMgr.clear();

	m_name.clear();
	m_llvmIrBuilder.clear();
	m_llvmDiBuilder.clear();
	m_calcLayoutArray.clear();
	m_compileArray.clear();
	m_sourceList.clear();
	m_filePathSet.clear();
	m_functionMap.clear();

	if (m_llvmExecutionEngine)
		delete m_llvmExecutionEngine;
	else if (m_llvmModule)
		delete m_llvmModule;

	if (m_llvmContext)
		delete m_llvmContext;

	m_llvmContext = NULL;
	m_llvmModule = NULL;
	m_llvmExecutionEngine = NULL;
	m_constructor = NULL;
	m_destructor = NULL;

	m_compileFlags = ModuleCompileFlag_StdFlags;
	m_compileState = ModuleCompileState_Idle;
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

#if (LLVM_VERSION >= 0x0306)
	compileFlags |= ModuleCompileFlag_McJit;
#endif

	m_name = name;
	m_compileFlags = compileFlags;
	m_compileState = ModuleCompileState_Idle;

	m_llvmContext = new llvm::LLVMContext;
	m_llvmModule = new llvm::Module("jncModule", *m_llvmContext);

	m_llvmIrBuilder.create();

	if (compileFlags & ModuleCompileFlag_DebugInfo)
		m_llvmDiBuilder.create();

	if (!(compileFlags & ModuleCompileFlag_StdLibDoc))
	{
		m_extensionLibMgr.addStaticLib(jnc_CoreLib_getLib());
		m_variableMgr.createStdVariables();
		m_namespaceMgr.addStdItems();
	}
}

#if (JNC_PTR_BITS == 32)
#	if (_JNC_OS_POSIX)
extern "C" int64_t __divdi3 (int64_t, int64_t);
extern "C" int64_t __moddi3 (int64_t, int64_t);
extern "C" uint64_t __udivdi3 (uint64_t, uint64_t);
extern "C" uint64_t __umoddi3 (uint64_t, uint64_t);
#		if (_JNC_CPU_ARM32)
struct DivModRetVal
{
	uint64_t m_quotient;
	uint64_t m_remainder;
};
extern "C" int32_t __aeabi_idiv (int32_t);
extern "C" uint32_t __aeabi_uidiv (uint32_t);
extern "C" DivModRetVal __aeabi_ldivmod (int64_t, int64_t);
extern "C" DivModRetVal __aeabi_uldivmod (uint64_t, uint64_t);
extern "C" float __aeabi_i2f (int32_t);
extern "C" float __aeabi_l2f (int64_t);
extern "C" float __aeabi_ui2f (uint32_t);
extern "C" float __aeabi_ul2f (uint64_t);
extern "C" double __aeabi_i2d (int32_t);
extern "C" double __aeabi_l2d (int64_t);
extern "C" double __aeabi_ui2d (uint32_t);
extern "C" double __aeabi_ul2d (uint64_t);
#		endif
#	elif (_JNC_OS_WIN)
extern "C" int64_t _alldiv (int64_t, int64_t);
extern "C" int64_t _allrem (int64_t, int64_t);
extern "C" int64_t _aulldiv (int64_t, int64_t);
extern "C" int64_t _aullrem (int64_t, int64_t);
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
	llvm::cl::opt<bool>* enableMerge = (llvm::cl::opt<bool>*) options.find("global-merge")->second;
	ASSERT(llvm::isa<llvm::cl::opt<bool> > (enableMerge));

	enableMerge->setValue(false);
#endif

#if (LLVM_VERSION < 0x0306)
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
#if (LLVM_VERSION < 0x0306)
		engineBuilder.setUseMCJIT(true);
		engineBuilder.setMCJITMemoryManager(jitMemoryMgr);
		targetOptions.JITEmitDebugInfo = true;
#elif (LLVM_VERSION < 0x0307)
		engineBuilder.setMCJITMemoryManager(std::move(std::unique_ptr<JitMemoryMgr> (jitMemoryMgr)));
		targetOptions.JITEmitDebugInfo = true;
#else
		engineBuilder.setMCJITMemoryManager(std::move(std::unique_ptr<JitMemoryMgr> (jitMemoryMgr)));
#endif

		m_functionMap["memset"] = (void*) memset;
		m_functionMap["memcpy"] = (void*) memcpy;
		m_functionMap["memmove"] = (void*) memmove;

#if (JNC_PTR_BITS == 32)
#	if (_JNC_OS_POSIX)
		m_functionMap["__divdi3"] = (void*) __divdi3;
		m_functionMap["__moddi3"] = (void*) __moddi3;
		m_functionMap["__udivdi3"] = (void*) __udivdi3;
		m_functionMap["__umoddi3"] = (void*) __umoddi3;
#		if (_JNC_CPU_ARM32)
		m_functionMap["__aeabi_idiv"] = (void*) __aeabi_idiv;
		m_functionMap["__aeabi_uidiv"] = (void*) __aeabi_uidiv;
		m_functionMap["__aeabi_ldivmod"] = (void*) __aeabi_ldivmod;
		m_functionMap["__aeabi_uldivmod"] = (void*) __aeabi_uldivmod;
		m_functionMap["__aeabi_i2f"] = (void*) __aeabi_i2f;
		m_functionMap["__aeabi_l2f"] = (void*) __aeabi_l2f;
		m_functionMap["__aeabi_ui2f"] = (void*) __aeabi_ui2f;
		m_functionMap["__aeabi_ul2f"] = (void*) __aeabi_ul2f;
		m_functionMap["__aeabi_i2d"] = (void*) __aeabi_i2d;
		m_functionMap["__aeabi_l2d"] = (void*) __aeabi_l2d;
		m_functionMap["__aeabi_ui2d"] = (void*) __aeabi_ui2d;
		m_functionMap["__aeabi_ul2d"] = (void*) __aeabi_ul2d;
#		endif
#	elif (_JNC_OS_WIN)
		m_functionMap["_alldiv"] = (void*) _alldiv;
		m_functionMap["_allrem"] = (void*) _allrem;
		m_functionMap["_aulldiv"] = (void*) _aulldiv;
		m_functionMap["_aullrem"] = (void*) _aullrem;
#	endif
#endif
	}
	else
	{
#if (LLVM_VERSION >= 0x0306) // legacy JIT is gone in LLVM 3.6
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
		*(void**) (p + 2) = chkstk;

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
	if (variable->m_flags & ModuleItemFlag_Unused)
		return true;

	llvm::GlobalVariable* llvmVariable = variable->getLlvmGlobalVariable();
	if (!llvmVariable)
	{
		err::setFormatStringError("attempt to map non-global variable: %s", variable->getQualifiedName().sz());
		return false;
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

#if (LLVM_VERSION >= 0x0400)
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

	return true;
}

bool
Module::mapFunction(
	Function* function,
	void* p
	)
{
	if (function->m_flags & ModuleItemFlag_Unused)
		return true;

	llvm::Function* llvmFunction = function->getLlvmFunction();
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
	Function* function = m_namespaceMgr.getGlobalNamespace()->getFunctionByName(name);
	if (!function)
		return false;

	llvm::Function* llvmFunction = function->getLlvmFunction();
	if (!llvmFunction)
		return false;

	llvmExecutionEngine->addGlobalMapping(llvmFunction, p);
	return true;
}

bool
Module::setFunctionPointer(
	llvm::ExecutionEngine* llvmExecutionEngine,
	const QualifiedName& name,
	void* p
	)
{
	ModuleItem* item = m_namespaceMgr.getGlobalNamespace()->findItem(name);
	if (!item || item->getItemKind() != ModuleItemKind_Function)
		return false;

	llvm::Function* llvmFunction = ((Function*)item)->getLlvmFunction();
	if (!llvmFunction)
		return false;

	llvmExecutionEngine->addGlobalMapping(llvmFunction, p);
	return true;
}

void
Module::markForLayout(
	ModuleItem* item,
	bool isForced
	)
{
	if (!isForced && (item->m_flags & ModuleItemFlag_NeedLayout))
		return;

	item->m_flags |= ModuleItemFlag_NeedLayout;
	m_calcLayoutArray.append(item);
}

void
Module::markForCompile(ModuleItem* item)
{
	if (item->m_flags & ModuleItemFlag_NeedCompile)
		return;

	item->m_flags |= ModuleItemFlag_NeedCompile;
	m_compileArray.append(item);
}

bool
Module::parse(
	ExtensionLib* lib,
	const sl::StringRef& fileName,
	const sl::StringRef& source
	)
{
	bool result;

	Unit* unit = m_unitMgr.createUnit(lib, fileName);
	m_unitMgr.setCurrentUnit(unit);

	Lexer lexer;
	lexer.create(fileName, source);

	if (m_compileFlags & ModuleCompileFlag_Documentation)
		lexer.m_channelMask = TokenChannelMask_All; // also include doxy-comments

	Parser parser(this);
	parser.create(Parser::StartSymbol, true);

	for (;;)
	{
		const Token* token = lexer.getToken();
		switch(token->m_token)
		{
		case TokenKind_Error:
			err::setFormatStringError("invalid character '\\x%02x'", (uchar_t) token->m_data.m_integer);
			lex::pushSrcPosError(fileName, token->m_pos);
			return false;

		case TokenKind_DoxyComment1:
		case TokenKind_DoxyComment2:
		case TokenKind_DoxyComment3:
		case TokenKind_DoxyComment4:
			if (!(m_compileFlags & (ModuleCompileFlag_DisableDoxyComment1 << (token->m_token - TokenKind_DoxyComment1))))
			{
				sl::StringRef comment = token->m_data.m_string;
				bool isSingleLine = token->m_token <= TokenKind_DoxyComment2;
				ModuleItem* lastDeclaredItem = NULL;

				if (isSingleLine && !comment.isEmpty() && comment[0] == '<')
				{
					lastDeclaredItem = parser.m_lastDeclaredItem;
					comment = comment.getSubString(1);
				}

				parser.m_doxyParser.addComment(
					comment,
					token->m_pos,
					isSingleLine,
					lastDeclaredItem
					);
			}
			break;

		default:
			result = parser.parseToken(token);
			if (!result)
			{
				lex::ensureSrcPosError(fileName, token->m_pos);
				return false;
			}
		}

		if (token->m_token == TokenKind_Eof) // EOF token must be parsed
			break;

		lexer.nextToken();
	}

	m_namespaceMgr.getGlobalNamespace()->getUsingSet()->clear();

	return true;
}

bool
Module::parseFile(const sl::StringRef& fileName)
{
	sl::String filePath = io::getFullFilePath(fileName);

	sl::StringHashTableIterator<bool> it = m_filePathSet.find(filePath);
	if (it)
		return true; // already parsed

	io::SimpleMappedFile file;
	bool result = file.open(filePath, io::FileFlag_ReadOnly);
	if (!result)
		return false;

	size_t length = file.getMappingSize();
	sl::String source((const char*) file.p(), length);

	m_sourceList.insertTail(source);
	m_filePathSet.visit(filePath);

	return parse(NULL, filePath, source);
}

bool
Module::parseImports()
{
	sl::ConstList<Import> importList = m_importMgr.getImportList();
	sl::ConstIterator<Import> importIt = importList.getHead();

	for (; importIt; importIt++)
	{
		bool result = importIt->m_importKind == ImportKind_Source ?
			parse(
				importIt->m_lib,
				importIt->m_filePath,
				importIt->m_source
				) :
			parseFile(importIt->m_filePath);

		if (!result)
			return false;
	}

	return true;
}

bool
Module::link()
{
	bool result;

	ASSERT(m_compileState < ModuleCompileState_Linked);

	result =
		m_typeMgr.resolveImportTypes() &&
		m_namespaceMgr.resolveImportUsingSets() &&
		m_namespaceMgr.resolveOrphans();

	if (!result)
		return false;

	m_compileState = ModuleCompileState_Linked;
	return true;
}

bool
Module::calcLayout()
{
	bool result;

	ASSERT(m_compileState < ModuleCompileState_LayoutCalculated);
	if (m_compileState < ModuleCompileState_Linked)
	{
		result = link();
		if (!result)
			return false;
	}

	result = processCalcLayoutArray();
	if (!result)
		return false;

	m_compileState = ModuleCompileState_LayoutCalculated;
	return true;
}

bool
Module::compile()
{
	bool result;

	ASSERT(m_compileState < ModuleCompileState_Compiled);
	if (m_compileState < ModuleCompileState_LayoutCalculated)
	{
		result = calcLayout();
		if (!result)
			return false;
	}

	result =
		createConstructorDestructor() &&
		processCompileArray() &&
		m_variableMgr.createTlsStructType();

	if (!result)
		return false;

	m_functionMgr.injectTlsPrologues();
	m_functionMgr.replaceAsyncAllocas();

	result = m_controlFlowMgr.deleteUnreachableBlocks();
	if (!result)
		return false;

	if (m_compileFlags & ModuleCompileFlag_DebugInfo)
		m_llvmDiBuilder.finalize();

	m_compileState = ModuleCompileState_Compiled;
	return true;
}

bool
Module::optimize(uint_t level)
{
	// before running LLVM optimization passes, mark unused functions and global variables
	// later on, avoid accessing corresponding llvm::Function*/llvm::GlobalVariable* pointers

	sl::Iterator<Function> it = m_functionMgr.m_functionList.getHead();
	for (; it; it++)
		if (it->getLlvmFunction()->use_empty())
			it->m_flags |= ModuleItemFlag_Unused;

	size_t count = m_variableMgr.m_globalStaticVariableArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = m_variableMgr.m_globalStaticVariableArray[i];
		if (variable->getLlvmGlobalVariable()->use_empty())
			variable->m_flags |= ModuleItemFlag_Unused;
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
		if (it->hasBody())
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

	result = createLlvmExecutionEngine();
	if (!result)
		return false;

	result =
		m_extensionLibMgr.mapAddresses() &&
		m_functionMgr.jitFunctions();

	if (!result)
		return false;

	m_compileState = ModuleCompileState_Jitted;
	return true;
}

bool
Module::processCalcLayoutArray()
{
	bool result;

	while (!m_calcLayoutArray.isEmpty()) // new items could be added in process
	{
		sl::Array<ModuleItem*> calcLayoutArray = m_calcLayoutArray;
		m_calcLayoutArray.clear();

		size_t count = calcLayoutArray.getCount();
		for (size_t i = 0; i < calcLayoutArray.getCount(); i++)
		{
			result = calcLayoutArray[i]->ensureLayout();
			if (!result)
				return false;
		}
	}

	return true;
}

bool
Module::processCompileArray()
{
	bool result;

	while (!m_compileArray.isEmpty()) // new items could be added in process
	{
		sl::Array<ModuleItem*> compileArray = m_compileArray;
		m_compileArray.clear();

		size_t count = compileArray.getCount();
		for (size_t i = 0; i < compileArray.getCount(); i++)
		{
			result = compileArray[i]->compile();
			if (!result)
				return false;

			ASSERT(!m_namespaceMgr.getCurrentScope());
		}
	}

	return true;
}

bool
Module::postParseStdItem()
{
	bool result = m_typeMgr.resolveImportTypes();
	if (!result)
		return false;

	if (m_compileState >= ModuleCompileState_LayoutCalculated)
	{
		result = processCalcLayoutArray();
		if (!result)
			return false;

		if (m_compileState >= ModuleCompileState_Compiled)
		{
			result = processCompileArray();
			if (!result)
				return false;
		}
	}

	return true;
}
bool
Module::createConstructorDestructor()
{
	ASSERT(!m_constructor && !m_destructor);

	bool result;

	bool hasDestructors = false;
	sl::ConstList<Unit> unitList = m_unitMgr.getUnitList();

	// create constructor unconditionally -- static variable might appear during compilation

	FunctionType* type = (FunctionType*)m_typeMgr.getStdType(StdType_SimpleFunction);
	Function* function = m_functionMgr.createFunction(
		FunctionKind_StaticConstructor,
		sl::String(),
		"module.construct",
		type
		);

	function->m_storageKind = StorageKind_Static;
	m_constructor = function;

	uint_t prevFlags = m_compileFlags;
	m_compileFlags &= ~ModuleCompileFlag_GcSafePointInInternalPrologue;
	m_functionMgr.internalPrologue(function);
	m_compileFlags = prevFlags;

	result = m_variableMgr.allocateInitializeGlobalVariables();
	if (!result)
		return false;

	m_functionMgr.callStaticConstructors();

	sl::ConstIterator<Unit> it = unitList.getHead();
	for (; it; it++)
	{
		Function* constructor = it->getConstructor();
		Function* destructor = it->getDestructor();

		if (destructor)
			hasDestructors = true;

		if (constructor)
			m_llvmIrBuilder.createCall(constructor, constructor->getType(), NULL);
	}

	m_functionMgr.internalEpilogue();

	if (!hasDestructors)
		return true;

	function = m_functionMgr.createFunction(
		FunctionKind_StaticDestructor,
		sl::String(),
		"module.destruct",
		type
		);

	function->m_storageKind = StorageKind_Static;
	m_destructor = function;

	m_functionMgr.internalPrologue(function);

	it = unitList.getTail();
	for (; it; it--)
	{
		Function* destructor = it->getDestructor();
		if (destructor)
			m_llvmIrBuilder.createCall(destructor, destructor->getType(), NULL);
	}

	m_functionMgr.internalEpilogue();

	return true;
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
