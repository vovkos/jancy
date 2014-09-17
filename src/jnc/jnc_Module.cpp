#include "pch.h"
#include "jnc_Module.h"
#include "jnc_JitMemoryMgr.h"
#include "jnc_Parser.llk.h"

namespace jnc {

//.............................................................................

CModule::CModule ()
{
	m_pLlvmModule = NULL;
	m_pLlvmExecutionEngine = NULL;
	m_Flags = 0;
}

void
CModule::Clear ()
{
	m_TypeMgr.Clear ();
	m_NamespaceMgr.Clear ();
	m_FunctionMgr.Clear ();
	m_VariableMgr.Clear ();
	m_ConstMgr.Clear ();
	m_ControlFlowMgr.Clear ();
	m_OperatorMgr.Clear ();
	m_UnitMgr.Clear ();
	m_CalcLayoutArray.Clear ();
	m_CompileArray.Clear ();
	m_ApiItemArray.Clear ();
	m_LlvmDiBuilder.Clear ();
	m_SourceList.Clear ();
	m_FunctionMap.Clear ();

	if (m_pLlvmExecutionEngine)
		delete m_pLlvmExecutionEngine;
	else if (m_pLlvmModule)
		delete m_pLlvmModule;

	m_pConstructor = NULL;
	m_pDestructor = NULL;
	m_pLlvmModule = NULL;
	m_pLlvmExecutionEngine = NULL;

	m_Flags = 0;
}

bool
CModule::Create (
	const rtl::CString& Name,
	uint_t Flags
	)
{
	Clear ();

	m_Flags = Flags;
	m_Name = Name;

	llvm::LLVMContext* pLlvmContext = new llvm::LLVMContext;
	m_pLlvmModule = new llvm::Module ("jncc_module", *pLlvmContext);

	m_LlvmIrBuilder.Create ();

	if (Flags & EModuleFlag_DebugInfo)
		m_LlvmDiBuilder.Create ();

	bool Result = m_NamespaceMgr.AddStdItems ();
	if (!Result)
		return false;

	return true;
}

#if (_AXL_ENV == AXL_ENV_POSIX)
extern "C" int64_t __divdi3 (int64_t, int64_t);
extern "C" int64_t __moddi3 (int64_t, int64_t);
extern "C" uint64_t __udivdi3 (uint64_t, uint64_t);
extern "C" uint64_t __umoddi3 (uint64_t, uint64_t);
#endif

bool
CModule::CreateLlvmExecutionEngine ()
{
	ASSERT (!m_pLlvmExecutionEngine);

	llvm::EngineBuilder EngineBuilder (m_pLlvmModule);

	std::string errorString;
	EngineBuilder.setErrorStr (&errorString);
	EngineBuilder.setEngineKind(llvm::EngineKind::JIT);

	llvm::TargetOptions TargetOptions;
#if (LLVM_VERSION < 0x0304) // they removed JITExceptionHandling in 3.4
	TargetOptions.JITExceptionHandling = true;
#endif

	if (m_Flags & EModuleFlag_McJit)
	{
		CJitMemoryMgr* pJitMemoryMgr = new CJitMemoryMgr (this);
		EngineBuilder.setUseMCJIT (true);
#if (LLVM_VERSION < 0x0304) // they distinguish between JIT & MCJIT memory managers in 3.4
		EngineBuilder.setJITMemoryManager (pJitMemoryMgr);
#else
		EngineBuilder.setMCJITMemoryManager (pJitMemoryMgr);
#endif

		TargetOptions.JITEmitDebugInfo = true;

#if (_AXL_ENV == AXL_ENV_POSIX)
		m_FunctionMap ["__divdi3"]  = (void*) __divdi3;
		m_FunctionMap ["__moddi3"]  = (void*) __moddi3;
		m_FunctionMap ["__udivdi3"] = (void*) __udivdi3;
		m_FunctionMap ["__umoddi3"] = (void*) __umoddi3;
#endif
	}

	EngineBuilder.setTargetOptions (TargetOptions);

#if (_AXL_CPU == AXL_CPU_X86)
	EngineBuilder.setMArch ("x86");
#endif

	m_pLlvmExecutionEngine = EngineBuilder.create ();
	if (!m_pLlvmExecutionEngine)
	{
		err::SetFormatStringError ("cannot create execution engine: %s\n", errorString.c_str());
		return false;
	}

	return true;
}

void
CModule::MapFunction (
	llvm::Function* pLlvmFunction,
	void* pf
	)
{
	if (m_Flags & EModuleFlag_McJit)
	{
		m_FunctionMap [pLlvmFunction->getName ().data ()] = pf;
	}
	else
	{
		ASSERT (m_pLlvmExecutionEngine);
		m_pLlvmExecutionEngine->addGlobalMapping (pLlvmFunction, pf);
	}
}

CModuleItem*
CModule::GetApiItem (
	size_t Slot,
	const char* pName
	)
{
	size_t Count = m_ApiItemArray.GetCount ();
	if (Count <= Slot)
		m_ApiItemArray.SetCount (Slot + 1);

	CModuleItem* pItem = m_ApiItemArray [Slot];
	if (pItem)
		return pItem;

	pItem = GetItemByName (pName);
	m_ApiItemArray [Slot] = pItem;
	return pItem;
}

bool
CModule::SetConstructor (CFunction* pFunction)
{
	if (!pFunction->GetType ()->GetArgArray ().IsEmpty ())
	{
		err::SetFormatStringError ("module 'construct' cannot have arguments");
		return false;
	}

	if (m_pConstructor)
	{
		err::SetFormatStringError ("module already has 'construct' method");
		return false;
	}

	pFunction->m_FunctionKind = EFunction_ModuleConstructor;
	pFunction->m_StorageKind = EStorage_Static;
	pFunction->m_Tag = "module.construct";
	m_pConstructor = pFunction;
	return true;
}

bool
CModule::SetDestructor (CFunction* pFunction)
{
	if (m_pDestructor)
	{
		err::SetFormatStringError ("module already has 'destruct' method");
		return false;
	}

	pFunction->m_FunctionKind = EFunction_ModuleDestructor;
	pFunction->m_StorageKind = EStorage_Static;
	pFunction->m_Tag = "module.destruct";
	m_pDestructor = pFunction;
	return true;
}

bool
CModule::SetFunctionPointer (
	llvm::ExecutionEngine* pLlvmExecutionEngine,
	const char* pName,
	void* pf
	)
{
	CFunction* pFunction = GetFunctionByName (pName);
	if (!pFunction)
		return false;

	llvm::Function* pLlvmFunction = pFunction->GetLlvmFunction ();
	if (!pLlvmFunction)
		return false;

	pLlvmExecutionEngine->addGlobalMapping (pLlvmFunction, pf);
	return true;
}

bool
CModule::SetFunctionPointer (
	llvm::ExecutionEngine* pLlvmExecutionEngine,
	const CQualifiedName& Name,
	void* pf
	)
{
	CModuleItem* pItem = m_NamespaceMgr.GetGlobalNamespace ()->FindItem (Name);
	if (!pItem || pItem->GetItemKind () != EModuleItem_Function)
		return false;

	llvm::Function* pLlvmFunction = ((CFunction*) pItem)->GetLlvmFunction ();
	if (!pLlvmFunction)
		return false;

	pLlvmExecutionEngine->addGlobalMapping (pLlvmFunction, pf);
	return true;
}

bool
CModule::Link (CModule* pModule)
{
	err::SetFormatStringError ("module link is not yet implemented");
	return false;
}

void
CModule::MarkForLayout (
	CModuleItem* pItem,
	bool IsForced
	)
{
	if (!IsForced && (pItem->m_Flags & EModuleItemFlag_NeedLayout))
		return;

	pItem->m_Flags |= EModuleItemFlag_NeedLayout;
	m_CalcLayoutArray.Append (pItem);
}

void
CModule::MarkForCompile (CModuleItem* pItem)
{
	if (pItem->m_Flags & EModuleItemFlag_NeedCompile)
		return;

	pItem->m_Flags |= EModuleItemFlag_NeedCompile;
	m_CompileArray.Append (pItem);
}

bool
CModule::Parse (
	const char* pFilePath,
	const char* pSource,
	size_t Length
	)
{
	bool Result;

	CScopeThreadModule ScopeModule (this);

	m_UnitMgr.CreateUnit (pFilePath);

	CLexer Lexer;
	Lexer.Create (pFilePath, pSource, Length);

	CParser Parser;
	Parser.Create (CParser::StartSymbol, true);

	for (;;)
	{
		const CToken* pToken = Lexer.GetToken ();
		if (pToken->m_Token == EToken_Error)
		{
			err::SetFormatStringError ("invalid character '\\x%02x'", (uchar_t) pToken->m_Data.m_Integer);
			err::PushSrcPosError (pFilePath, pToken->m_Pos);
			return false;
		}

		Result = Parser.ParseToken (pToken);
		if (!Result)
		{
			err::EnsureSrcPosError (pFilePath, pToken->m_Pos);
			return false;
		}

		if (pToken->m_Token == EToken_Eof) // EOF token must be parsed
			break;

		Lexer.NextToken ();
	}

	return true;
}

bool
CModule::ParseFile (const char* pFilePath)
{
	io::CMappedFile File;
	bool Result = File.Open (pFilePath, io::EFileFlag_ReadOnly);
	if (!Result)
		return false;

	size_t Length = (size_t) File.GetSize ();
	const char* p = (const char*) File.View (0, Length);
	if (!p)
		return false;

	rtl::CString Source (p, Length);
	m_SourceList.InsertTail (Source);
	return p != NULL && Parse (pFilePath, Source, Length);
}

bool
CModule::CalcLayout ()
{
	bool Result;

	for (size_t i = 0; i < m_CalcLayoutArray.GetCount (); i++) // new items could be added in process
	{
		Result = m_CalcLayoutArray [i]->EnsureLayout ();
		if (!Result)
			return false;
	}

	return true;
}

bool
CModule::Compile ()
{
	bool Result;

	// step 1: resolve imports & orphans

	Result =
		m_TypeMgr.ResolveImportTypes () &&
		m_NamespaceMgr.ResolveOrphans ();

	if (!Result)
		return false;

	// step 2: calc layout

	Result = CalcLayout ();
	if (!Result)
		return false;

	// step 3: ensure module constructor (always! cause static variable might appear during compilation)

	if (m_pConstructor)
	{
		if (!m_pConstructor->HasBody ())
		{
			err::SetFormatStringError ("unresolved module constructor");
			return false;
		}

		Result = m_pConstructor->Compile ();
		if (!Result)
			return false;
	}
	else
	{
		Result = CreateDefaultConstructor ();
		if (!Result)
			return false;
	}

	// step 4: compile the rest

	for (size_t i = 0; i < m_CompileArray.GetCount (); i++) // new items could be added in process
	{
		Result = m_CompileArray [i]->Compile ();
		if (!Result)
			return false;
	}

	// step 5: ensure module destructor (if needed)

	if (!m_pDestructor && !m_VariableMgr.m_StaticDestructList.IsEmpty ())
		CreateDefaultDestructor ();

	// step 6: deal with tls

	Result =
		m_VariableMgr.CreateTlsStructType () &&
		m_FunctionMgr.InjectTlsPrologues ();

	if (!Result)
		return false;

	// step 7: delete unreachable blocks

	m_ControlFlowMgr.DeleteUnreachableBlocks ();

	// step 8: finalize debug information

	if (m_Flags & EModuleFlag_DebugInfo)
		m_LlvmDiBuilder.Finalize ();

	return true;
}

bool
CModule::Jit ()
{
	#pragma AXL_TODO ("move JITting logic to CModule")
	return m_FunctionMgr.JitFunctions ();
}

bool
CModule::CreateDefaultConstructor ()
{
	bool Result;

	ASSERT (!m_pConstructor);

	CFunctionType* pType = (CFunctionType*) GetSimpleType (EStdType_SimpleFunction);
	CFunction* pFunction = m_FunctionMgr.CreateFunction (EFunction_ModuleConstructor, pType);
	pFunction->m_StorageKind = EStorage_Static;
	pFunction->m_Tag = "module.construct";

	m_pConstructor = pFunction;

	m_FunctionMgr.InternalPrologue (pFunction);

	CBasicBlock* pBlock = m_ControlFlowMgr.SetCurrentBlock (pFunction->GetEntryBlock ());

	Result = m_VariableMgr.AllocatePrimeStaticVariables ();
	if (!Result)
		return false;

	m_ControlFlowMgr.SetCurrentBlock (pBlock);

	Result = m_VariableMgr.InitializeGlobalStaticVariables ();
	if (!Result)
		return false;

	m_FunctionMgr.InternalEpilogue ();

	return true;
}

void
CModule::CreateDefaultDestructor ()
{
	ASSERT (!m_pDestructor);

	CFunctionType* pType = (CFunctionType*) GetSimpleType (EStdType_SimpleFunction);
	CFunction* pFunction = m_FunctionMgr.CreateFunction (EFunction_ModuleDestructor, "module.destruct", pType);
	pFunction->m_StorageKind = EStorage_Static;

	m_pDestructor = pFunction;

	m_FunctionMgr.InternalPrologue (pFunction);
	m_VariableMgr.m_StaticDestructList.RunDestructors ();
	m_FunctionMgr.InternalEpilogue ();
}

rtl::CString
CModule::GetLlvmIrString ()
{
	std::string String;
	llvm::raw_string_ostream Stream (String);
	m_pLlvmModule->print (Stream, NULL);
	return String.c_str ();
}

//.............................................................................

} // namespace jnc {
