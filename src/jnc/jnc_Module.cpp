#include "pch.h"
#include "jnc_Module.h"
#include "jnc_JitMemoryMgr.h"
#include "jnc_Parser.llk.h"

namespace jnc {

//.............................................................................

Module::Module ()
{
	m_llvmModule = NULL;
	m_llvmExecutionEngine = NULL;
	m_flags = 0;
	m_compileState = ModuleCompileState_Idle;
}

void
Module::clear ()
{
	m_typeMgr.clear ();
	m_namespaceMgr.clear ();
	m_functionMgr.clear ();
	m_variableMgr.clear ();
	m_constMgr.clear ();
	m_controlFlowMgr.clear ();
	m_operatorMgr.clear ();
	m_unitMgr.clear ();
	m_calcLayoutArray.clear ();
	m_compileArray.clear ();
	m_apiItemArray.clear ();
	m_llvmDiBuilder.clear ();
	m_sourceList.clear ();
	m_functionMap.clear ();

	if (m_llvmExecutionEngine)
		delete m_llvmExecutionEngine;
	else if (m_llvmModule)
		delete m_llvmModule;

	m_constructor = NULL;
	m_destructor = NULL;
	m_llvmModule = NULL;
	m_llvmExecutionEngine = NULL;

	m_flags = 0;
	m_compileState = ModuleCompileState_Idle;
}

bool
Module::create (
	const rtl::String& name,
	uint_t flags
	)
{
	clear ();

	m_flags = flags;
	m_name = name;

	llvm::LLVMContext* llvmContext = new llvm::LLVMContext;
	m_llvmModule = new llvm::Module ("jncc_module", *llvmContext);

	m_llvmIrBuilder.create ();

	if (flags & ModuleFlag_DebugInfo)
		m_llvmDiBuilder.create ();

	bool result = m_namespaceMgr.addStdItems ();
	if (!result)
		return false;

	return true;
}

#if (_AXL_CPU == AXL_CPU_X86 && _AXL_ENV == AXL_ENV_POSIX)
extern "C" int64_t __divdi3 (int64_t, int64_t);
extern "C" int64_t __moddi3 (int64_t, int64_t);
extern "C" uint64_t __udivdi3 (uint64_t, uint64_t);
extern "C" uint64_t __umoddi3 (uint64_t, uint64_t);
#endif

bool
Module::createLlvmExecutionEngine ()
{
	ASSERT (!m_llvmExecutionEngine);

	llvm::EngineBuilder engineBuilder (m_llvmModule);

	std::string errorString;
	engineBuilder.setErrorStr (&errorString);
	engineBuilder.setEngineKind(llvm::EngineKind::JIT);

	llvm::TargetOptions targetOptions;
#if (LLVM_VERSION < 0x0304) // they removed JITExceptionHandling in 3.4
	targetOptions.JITExceptionHandling = true;
#endif

	if (m_flags & ModuleFlag_McJit)
	{
		JitMemoryMgr* jitMemoryMgr = new JitMemoryMgr (this);
		engineBuilder.setUseMCJIT (true);
#if (LLVM_VERSION < 0x0304) // they distinguish between JIT & MCJIT memory managers in 3.4
		engineBuilder.setJITMemoryManager (jitMemoryMgr);
#else
		engineBuilder.setMCJITMemoryManager (jitMemoryMgr);
#endif

		targetOptions.JITEmitDebugInfo = true;

#if (_AXL_CPU == AXL_CPU_X86 && _AXL_ENV == AXL_ENV_POSIX)
		m_functionMap ["__divdi3"]  = (void*) __divdi3;
		m_functionMap ["__moddi3"]  = (void*) __moddi3;
		m_functionMap ["__udivdi3"] = (void*) __udivdi3;
		m_functionMap ["__umoddi3"] = (void*) __umoddi3;
#endif
	}

	engineBuilder.setTargetOptions (targetOptions);

#if (_AXL_CPU == AXL_CPU_X86)
	engineBuilder.setMArch ("x86");
#endif

	m_llvmExecutionEngine = engineBuilder.create ();
	if (!m_llvmExecutionEngine)
	{
		err::setFormatStringError ("cannot create execution engine: %s\n", errorString.c_str());
		return false;
	}

	return true;
}

void
Module::mapFunction (
	llvm::Function* llvmFunction,
	void* pf
	)
{
	if (m_flags & ModuleFlag_McJit)
	{
		m_functionMap [llvmFunction->getName ().data ()] = pf;
	}
	else
	{
		ASSERT (m_llvmExecutionEngine);
		m_llvmExecutionEngine->addGlobalMapping (llvmFunction, pf);
	}
}

ModuleItem*
Module::getApiItem (
	size_t slot,
	const char* name
	)
{
	size_t count = m_apiItemArray.getCount ();
	if (count <= slot)
		m_apiItemArray.setCount (slot + 1);

	ModuleItem* item = m_apiItemArray [slot];
	if (item)
		return item;

	item = getItemByName (name);
	m_apiItemArray [slot] = item;
	return item;
}

bool
Module::setConstructor (Function* function)
{
	if (!function->getType ()->getArgArray ().isEmpty ())
	{
		err::setFormatStringError ("module 'construct' cannot have arguments");
		return false;
	}

	if (m_constructor)
	{
		err::setFormatStringError ("module already has 'construct' method");
		return false;
	}

	function->m_functionKind = FunctionKind_ModuleConstructor;
	function->m_storageKind = StorageKind_Static;
	function->m_tag = "module.construct";
	m_constructor = function;
	return true;
}

bool
Module::setDestructor (Function* function)
{
	if (m_destructor)
	{
		err::setFormatStringError ("module already has 'destruct' method");
		return false;
	}

	function->m_functionKind = FunctionKind_ModuleDestructor;
	function->m_storageKind = StorageKind_Static;
	function->m_tag = "module.destruct";
	m_destructor = function;
	return true;
}

bool
Module::setFunctionPointer (
	llvm::ExecutionEngine* llvmExecutionEngine,
	const char* name,
	void* pf
	)
{
	Function* function = getFunctionByName (name);
	if (!function)
		return false;

	llvm::Function* llvmFunction = function->getLlvmFunction ();
	if (!llvmFunction)
		return false;

	llvmExecutionEngine->addGlobalMapping (llvmFunction, pf);
	return true;
}

bool
Module::setFunctionPointer (
	llvm::ExecutionEngine* llvmExecutionEngine,
	const QualifiedName& name,
	void* pf
	)
{
	ModuleItem* item = m_namespaceMgr.getGlobalNamespace ()->findItem (name);
	if (!item || item->getItemKind () != ModuleItemKind_Function)
		return false;

	llvm::Function* llvmFunction = ((Function*) item)->getLlvmFunction ();
	if (!llvmFunction)
		return false;

	llvmExecutionEngine->addGlobalMapping (llvmFunction, pf);
	return true;
}

bool
Module::link (Module* module)
{
	err::setFormatStringError ("module link is not yet implemented");
	return false;
}

void
Module::markForLayout (
	ModuleItem* item,
	bool isForced
	)
{
	if (!isForced && (item->m_flags & ModuleItemFlag_NeedLayout))
		return;

	item->m_flags |= ModuleItemFlag_NeedLayout;
	m_calcLayoutArray.append (item);
}

void
Module::markForCompile (ModuleItem* item)
{
	if (item->m_flags & ModuleItemFlag_NeedCompile)
		return;

	item->m_flags |= ModuleItemFlag_NeedCompile;
	m_compileArray.append (item);
}

bool
Module::parse (
	const char* filePath,
	const char* source,
	size_t length
	)
{
	bool result;

	ScopeThreadModule scopeModule (this);

	m_unitMgr.createUnit (filePath);

	Lexer lexer;
	lexer.create (filePath, source, length);

	Parser parser;
	parser.create (Parser::StartSymbol, true);

	for (;;)
	{
		const Token* token = lexer.getToken ();
		if (token->m_token == TokenKind_Error)
		{
			err::setFormatStringError ("invalid character '\\x%02x'", (uchar_t) token->m_data.m_integer);
			err::pushSrcPosError (filePath, token->m_pos);
			return false;
		}

		result = parser.parseToken (token);
		if (!result)
		{
			err::ensureSrcPosError (filePath, token->m_pos);
			return false;
		}

		if (token->m_token == TokenKind_Eof) // EOF token must be parsed
			break;

		lexer.nextToken ();
	}

	return true;
}

bool
Module::parseFile (const char* filePath)
{
	io::MappedFile file;
	bool result = file.open (filePath, io::FileFlag_ReadOnly);
	if (!result)
		return false;

	size_t length = (size_t) file.getSize ();
	const char* p = (const char*) file.view (0, length);
	if (!p)
		return false;

	rtl::String source (p, length);
	m_sourceList.insertTail (source);
	return p != NULL && parse (filePath, source, length);
}

bool
Module::calcLayout ()
{
	bool result;

	for (size_t i = 0; i < m_calcLayoutArray.getCount (); i++) // new items could be added in process
	{
		result = m_calcLayoutArray [i]->ensureLayout ();
		if (!result)
			return false;
	}

	return true;
}

bool
Module::compile ()
{
	bool result;

	// step 1: resolve imports & orphans

	m_compileState = ModuleCompileState_Resolving;

	result =
		m_typeMgr.resolveImportTypes () &&
		m_namespaceMgr.resolveOrphans ();

	if (!result)
		return false;

	// step 2: calc layout

	m_compileState = ModuleCompileState_CalcLayout;

	result = calcLayout ();
	if (!result)
		return false;

	// step 3: ensure module constructor (always! cause static variable might appear during compilation)

	m_compileState = ModuleCompileState_Compiling;

	if (m_constructor)
	{
		if (!m_constructor->hasBody ())
		{
			err::setFormatStringError ("unresolved module constructor");
			return false;
		}

		result = m_constructor->compile ();
		if (!result)
			return false;
	}
	else
	{
		result = createDefaultConstructor ();
		if (!result)
			return false;
	}

	// step 4: compile the rest

	for (size_t i = 0; i < m_compileArray.getCount (); i++) // new items could be added in process
	{
		result = m_compileArray [i]->compile ();
		if (!result)
			return false;
	}

	// step 5: ensure module destructor (if needed)

	if (!m_destructor && !m_variableMgr.m_staticDestructList.isEmpty ())
		createDefaultDestructor ();

	// step 6: deal with tls

	result =
		m_variableMgr.createTlsStructType () &&
		m_functionMgr.injectTlsPrologues ();

	if (!result)
		return false;

	// step 7: delete unreachable blocks

	m_controlFlowMgr.deleteUnreachableBlocks ();

	// step 8: finalize debug information

	if (m_flags & ModuleFlag_DebugInfo)
		m_llvmDiBuilder.finalize ();

	m_compileState = ModuleCompileState_Compiled;

	return true;
}

bool
Module::jit ()
{
	#pragma AXL_TODO ("move JITting logic to CModule")

	ASSERT (m_compileState = ModuleCompileState_Compiled);

	m_compileState = ModuleCompileState_Jitting;
	bool result = m_functionMgr.jitFunctions ();
	if (!result)
		return false;

	m_compileState = ModuleCompileState_Jitted;
	return true;
}

bool
Module::createDefaultConstructor ()
{
	bool result;

	ASSERT (!m_constructor);

	FunctionType* type = (FunctionType*) getSimpleType (StdTypeKind_SimpleFunction);
	Function* function = m_functionMgr.createFunction (FunctionKind_ModuleConstructor, type);
	function->m_storageKind = StorageKind_Static;
	function->m_tag = "module.construct";

	m_constructor = function;

	m_functionMgr.internalPrologue (function);

	BasicBlock* block = m_controlFlowMgr.setCurrentBlock (function->getEntryBlock ());

	result = m_variableMgr.allocatePrimeStaticVariables ();
	if (!result)
		return false;

	m_controlFlowMgr.setCurrentBlock (block);

	result = m_variableMgr.initializeGlobalStaticVariables ();
	if (!result)
		return false;

	m_functionMgr.internalEpilogue ();

	return true;
}

void
Module::createDefaultDestructor ()
{
	ASSERT (!m_destructor);

	FunctionType* type = (FunctionType*) getSimpleType (StdTypeKind_SimpleFunction);
	Function* function = m_functionMgr.createFunction (FunctionKind_ModuleDestructor, "module.destruct", type);
	function->m_storageKind = StorageKind_Static;

	m_destructor = function;

	m_functionMgr.internalPrologue (function);
	m_variableMgr.m_staticDestructList.runDestructors ();
	m_functionMgr.internalEpilogue ();
}

rtl::String
Module::getLlvmIrString ()
{
	std::string string;
	llvm::raw_string_ostream stream (string);
	m_llvmModule->print (stream, NULL);
	return string.c_str ();
}

//.............................................................................

} // namespace jnc {
