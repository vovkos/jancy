#include "pch.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_JitMemoryMgr.h"
#include "jnc_ct_Parser.llk.h"
#include "jnc_rtl_CoreLib.h"
#include "jnc_ext_StdExtensionLibHost.h"

namespace jnc {
namespace rtl {

ext::ExtensionLib* 
getCoreLib (ext::ExtensionLibHost* host);

} // namespace rtl

namespace std {

ext::ExtensionLib* 
getStdLib (ext::ExtensionLibHost* host);

} // namespace std

namespace ct {

//.............................................................................

Module::Module ()
{
	m_compileFlags = ModuleCompileFlag_StdFlags;
	m_compileState = ModuleCompileState_Idle;

	m_llvmModule = NULL;
	m_llvmExecutionEngine = NULL;
	m_constructor = NULL;
	m_destructor = NULL;

	finalizeConstruction ();
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
	m_llvmIrBuilder.clear ();
	m_llvmDiBuilder.clear ();
	m_extensionLibMgr.clear ();
	m_sourceList.clear ();
	m_functionMap.clear ();
	m_extensionLibMgr.clear ();
	m_importMgr.clear ();

	if (m_llvmExecutionEngine)
		delete m_llvmExecutionEngine;
	else if (m_llvmModule)
		delete m_llvmModule;

	m_name.clear ();

	m_llvmModule = NULL;
	m_llvmExecutionEngine = NULL;
	m_constructor = NULL;
	m_destructor = NULL;

	m_compileFlags = ModuleCompileFlag_StdFlags;
	m_compileState = ModuleCompileState_Idle;
}

bool
Module::create (
	const sl::String& name,
	uint_t compileFlags
	)
{
	clear ();

	m_name = name;
	m_compileFlags = compileFlags;
	m_compileState = ModuleCompileState_Idle;

	llvm::LLVMContext* llvmContext = new llvm::LLVMContext;
	m_llvmModule = new llvm::Module ("jncModule", *llvmContext);
	
	m_llvmIrBuilder.create ();

	if (compileFlags & ModuleCompileFlag_DebugInfo)
		m_llvmDiBuilder.create ();
	
	ext::ExtensionLibHost* libHost = ext::getStdExtensionLibHost ();
	m_extensionLibMgr.addLib (rtl::getCoreLib (libHost));

	if (compileFlags & ModuleCompileFlag_StdLib)
		m_extensionLibMgr.addLib (std::getStdLib (libHost));

	bool result = m_namespaceMgr.addStdItems ();
	if (!result)
		return false;

	return true;
}

#if (_AXL_ENV == AXL_ENV_POSIX)
#	if (_AXL_PTR_BITNESS == 64)
int128_t
lockTestAndSet (
	volatile int128_t* dst,
	int128_t src
	)
{
	return __sync_lock_test_and_set_16 (dst, src);
}
#	else
extern "C" int64_t __divdi3 (int64_t, int64_t);
extern "C" int64_t __moddi3 (int64_t, int64_t);
extern "C" uint64_t __udivdi3 (uint64_t, uint64_t);
extern "C" uint64_t __umoddi3 (uint64_t, uint64_t);
#	endif

#endif

bool
Module::createLlvmExecutionEngine ()
{
	ASSERT (!m_llvmExecutionEngine);

	llvm::EngineBuilder engineBuilder (m_llvmModule);

	::std::string errorString;
	engineBuilder.setErrorStr (&errorString);
	engineBuilder.setEngineKind(llvm::EngineKind::JIT);

	llvm::TargetOptions targetOptions;
#if (LLVM_VERSION < 0x0304) // they removed JITExceptionHandling in 3.4
	targetOptions.JITExceptionHandling = true;
#endif

	if (m_compileFlags & ModuleCompileFlag_McJit)
	{
		JitMemoryMgr* jitMemoryMgr = new JitMemoryMgr (this);
		engineBuilder.setUseMCJIT (true);
#if (LLVM_VERSION < 0x0304) // they distinguish between JIT & MCJIT memory managers in 3.4
		engineBuilder.setJITMemoryManager (jitMemoryMgr);
#else
		engineBuilder.setMCJITMemoryManager (jitMemoryMgr);
#endif

		targetOptions.JITEmitDebugInfo = true;

#if (_AXL_ENV == AXL_ENV_POSIX)
		m_functionMap ["memset"] = (void*) memset;
		m_functionMap ["memcpy"] = (void*) memcpy;
		m_functionMap ["memmove"] = (void*) memmove;
#	if (_AXL_PTR_BITNESS == 64)
		m_functionMap ["__sync_lock_test_and_set_16"] = (void*) lockTestAndSet;
#	else
		m_functionMap ["__divdi3"]  = (void*) __divdi3;
		m_functionMap ["__moddi3"]  = (void*) __moddi3;
		m_functionMap ["__udivdi3"] = (void*) __udivdi3;
		m_functionMap ["__umoddi3"] = (void*) __umoddi3;
#	endif
#endif
	}
	else
	{
#if (_AXL_ENV == AXL_ENV_WIN && _AXL_CPU == AXL_CPU_AMD64)
		// legacy JIT uses relative call to __chkstk
		// it worked just fine before windows 10 which loads ntdll.dll too far away

		// the fix should go to LLVM, of course, but 
		// a) applying a patch to LLVM before building Jancy would be a pain in the ass
		// b) legacy JIT is a gonner anyway

		// therefore, a simple workaround is used: allocate a proxy for __chkstk 
		// which would reside close enough to the generated code

		void* chkstk = ::GetProcAddress (::GetModuleHandleA ("ntdll.dll"), "__chkstk");
		if (!chkstk)
		{
			err::setFormatStringError ("__chkstk is not found");
			return false;
		}

		llvm::JITMemoryManager* jitMemoryMgr = llvm::JITMemoryManager::CreateDefaultMemManager ();
		engineBuilder.setJITMemoryManager (jitMemoryMgr);
		uchar_t* p = jitMemoryMgr->allocateCodeSection (128, 0, 0, llvm::StringRef ());

		// mov r11, __chkstk 

		p [0] = 0x49;
		p [1] = 0xbb;
		*(void**) (p + 2) = chkstk;

		// jmp r11

		p [10] = 0x41;
		p [11] = 0xff;
		p [12] = 0xe3;
		
		llvm::sys::DynamicLibrary::AddSymbol ("__chkstk", p);
#endif

		engineBuilder.setUseMCJIT (false);
	}

	engineBuilder.setTargetOptions (targetOptions);

#if (_AXL_CPU == AXL_CPU_X86)
	engineBuilder.setMArch ("x86");
#endif

	mt::ScopeTlsSlot <Module> scopeModule (this); // for GcShadowStack

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
	Function* function,
	void* p
	)
{
	llvm::Function* llvmFunction = function->getLlvmFunction ();

	if (m_compileFlags & ModuleCompileFlag_McJit)
	{
		sl::StringHashTableMapIterator <void*> it = m_functionMap.visit (llvmFunction->getName ().data ());
		ASSERT (!it->m_value); // mapped twice?
		it->m_value = p;
	}
	else
	{
		ASSERT (m_llvmExecutionEngine);
		m_llvmExecutionEngine->addGlobalMapping (llvmFunction, p);
	}

	function->m_machineCode = p;
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
	void* p
	)
{
	Function* function = m_namespaceMgr.getGlobalNamespace ()->getFunctionByName (name);
	if (!function)
		return false;

	llvm::Function* llvmFunction = function->getLlvmFunction ();
	if (!llvmFunction)
		return false;

	llvmExecutionEngine->addGlobalMapping (llvmFunction, p);
	return true;
}

bool
Module::setFunctionPointer (
	llvm::ExecutionEngine* llvmExecutionEngine,
	const QualifiedName& name,
	void* p
	)
{
	ModuleItem* item = m_namespaceMgr.getGlobalNamespace ()->findItem (name);
	if (!item || item->getItemKind () != ModuleItemKind_Function)
		return false;

	llvm::Function* llvmFunction = ((Function*) item)->getLlvmFunction ();
	if (!llvmFunction)
		return false;

	llvmExecutionEngine->addGlobalMapping (llvmFunction, p);
	return true;
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

	m_unitMgr.createUnit (filePath);

	Lexer lexer;
	lexer.create (filePath, source, length);

	Parser parser (this);
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

	m_namespaceMgr.getGlobalNamespace ()->getUsingSet ()->clear ();
	return true;
}

bool
Module::parseFile (const char* filePath)
{
	io::SimpleMappedFile file;
	bool result = file.open (filePath, io::FileFlag_ReadOnly);
	if (!result)
		return false;

	size_t length = (size_t) file.getSize ();
	sl::String source ((const char*) file.p (), length);
	m_sourceList.insertTail (source);
	return parse (filePath, source, length);
}

bool
Module::parseImports ()
{
	sl::ConstList <Import> importList = m_importMgr.getImportList ();
	sl::Iterator <Import> importIt = importList.getHead ();

	for (; importIt; importIt++)
	{
		bool result = importIt->m_importKind == ImportKind_Source ? 
			parse (
				importIt->m_filePath, 
				importIt->m_source, 
				importIt->m_source.getLength ()
				) : 
			parseFile (importIt->m_filePath);

		if (!result)
			return false;
	}

	return true;
}

bool
Module::calcLayout ()
{
	bool result;

	while (!m_calcLayoutArray.isEmpty ()) // new items could be added in process
	{
		sl::Array <ModuleItem*> calcLayoutArray = m_calcLayoutArray;
		m_calcLayoutArray.clear ();

		size_t count = calcLayoutArray.getCount ();
		for (size_t i = 0; i < calcLayoutArray.getCount (); i++) 
		{
			result = calcLayoutArray [i]->ensureLayout ();
			if (!result)
				return false;
		}
	}

	return true;
}

bool
Module::processCompileArray ()
{
	bool result;

	while (!m_compileArray.isEmpty ()) // new items could be added in process
	{
		sl::Array <ModuleItem*> compileArray = m_compileArray;
		m_compileArray.clear ();

		size_t count = compileArray.getCount ();
		for (size_t i = 0; i < compileArray.getCount (); i++) 
		{
			result = compileArray [i]->compile ();
			if (!result)
				return false;
		}
	}

	return true;
}

bool
Module::postParseStdItem ()
{
	bool result = m_typeMgr.resolveImportTypes ();
	if (!result)
		return false;

	if (m_compileState > ModuleCompileState_CalcLayout)
	{
		result = calcLayout ();
		if (!result)
			return false;

		if (m_compileState >= ModuleCompileState_Compiled)
		{
			result = processCompileArray ();
			if (!result)
				return false;
		}
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

	result = processCompileArray ();
	if (!result)
		return false;

	// step 6: deal with tls

	result =
		m_variableMgr.createTlsStructType () &&
		m_functionMgr.injectTlsPrologues ();

	if (!result)
		return false;

	// step 7: delete unreachable blocks

	result = m_controlFlowMgr.deleteUnreachableBlocks ();
	if (!result)
		return false;

	// step 8: finalize debug information

	if (m_compileFlags & ModuleCompileFlag_DebugInfo)
		m_llvmDiBuilder.finalize ();

	m_compileState = ModuleCompileState_Compiled;

	return true;
}

bool
Module::jit ()
{
	ASSERT (m_compileState = ModuleCompileState_Compiled);

	m_compileState = ModuleCompileState_Jitting;
	
	bool result = 
		m_extensionLibMgr.mapFunctions () &&
		m_functionMgr.jitFunctions ();

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

	FunctionType* type = (FunctionType*) m_typeMgr.getStdType (StdType_SimpleFunction);
	Function* function = m_functionMgr.createFunction (FunctionKind_ModuleConstructor, type);
	function->m_storageKind = StorageKind_Static;
	function->m_tag = "module.construct";

	m_constructor = function;

	m_functionMgr.internalPrologue (function);

	result = 
		m_variableMgr.allocateInitializeGlobalVariables () &&
		m_functionMgr.callStaticConstructors ();

	if (!result)
		return false;

	m_functionMgr.internalEpilogue ();

	return true;
}

sl::String
Module::getLlvmIrString ()
{
	::std::string string;
	llvm::raw_string_ostream stream (string);
	m_llvmModule->print (stream, NULL);
	return string.c_str ();
}

//.............................................................................

} // namespace ct
} // namespace jnc
