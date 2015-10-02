// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_AttributeMgr.h"
#include "jnc_TypeMgr.h"
#include "jnc_NamespaceMgr.h"
#include "jnc_FunctionMgr.h"
#include "jnc_VariableMgr.h"
#include "jnc_ConstMgr.h"
#include "jnc_ControlFlowMgr.h"
#include "jnc_OperatorMgr.h"
#include "jnc_UnitMgr.h"
#include "jnc_LlvmIrBuilder.h"
#include "jnc_LlvmDiBuilder.h"
#include "jnc_ExtensionLibMgr.h"
#include "jnc_ImportMgr.h"

namespace jnc {

class Module;

//.............................................................................

enum ModuleCompileFlag
{
	ModuleCompileFlag_DebugInfo                            = 0x0001,
	ModuleCompileFlag_McJit                                = 0x0002,
	ModuleCompileFlag_SimpleGcSafePoint                    = 0x0004,
	ModuleCompileFlag_GcSafePointInPrologue                = 0x0010,
	ModuleCompileFlag_GcSafePointInInternalPrologue        = 0x0020,
	ModuleCompileFlag_CheckStackOverflowInPrologue         = 0x0040,
	ModuleCompileFlag_CheckStackOverflowInInternalPrologue = 0x0080,
	ModuleCompileFlag_StdLib                               = 0x0100,

	ModuleCompileFlag_StdFlags = 
		ModuleCompileFlag_GcSafePointInPrologue | 
		ModuleCompileFlag_GcSafePointInInternalPrologue |
		ModuleCompileFlag_CheckStackOverflowInPrologue |
		ModuleCompileFlag_StdLib 
#if (_AXL_ENV == AXL_ENV_WIN && _AXL_CPU != AXL_CPU_X86)
		// SEH on amd64/ia64 relies on being able to walk the stack which is not as 
		// reliable as frame-based SEH on x86. therefore, use write barrier for 
		// safe points on windows if and only if it's x86 
		| ModuleCompileFlag_SimpleGcSafePoint
#elif (_AXL_ENV == AXL_ENV_POSIX)
		| ModuleCompileFlag_McJit
#endif
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ModuleCompileState
{
	ModuleCompileState_Idle,
	ModuleCompileState_Resolving,
	ModuleCompileState_CalcLayout,
	ModuleCompileState_Compiling,
	ModuleCompileState_Compiled,
	ModuleCompileState_Jitting,
	ModuleCompileState_Jitted,
};

//.............................................................................

// makes it convenient to initialize childs (especially operators)

class PreModule
{
protected:
	PreModule ()
	{
		Module* prevModule = mt::setTlsSlotValue <Module> ((Module*) this);
		ASSERT (prevModule == NULL);
	}

public:
	static
	Module* 
	getCurrentConstructedModule ()
	{
		return mt::getTlsSlotValue <Module> ();
	}

protected:
	void
	finalizeConstruction ()
	{
		mt::setTlsSlotValue <Module> (NULL);
	}
};

//.............................................................................

class Module: public PreModule
{
protected:
	rtl::String m_name;

	uint_t m_compileFlags;
	ModuleCompileState m_compileState;

	Function* m_constructor;
	Function* m_destructor;

	rtl::Array <ModuleItem*> m_calcLayoutArray;
	rtl::Array <ModuleItem*> m_compileArray;
	rtl::BoxList <rtl::String> m_sourceList; // need to keep all sources in-memory during compilation
	rtl::StringHashTableMap <void*> m_functionMap;

	llvm::Module* m_llvmModule;
	llvm::ExecutionEngine* m_llvmExecutionEngine;

public:
	TypeMgr m_typeMgr;
	AttributeMgr m_attributeMgr;
	NamespaceMgr m_namespaceMgr;
	FunctionMgr m_functionMgr;
	VariableMgr m_variableMgr;
	ConstMgr m_constMgr;
	ControlFlowMgr m_controlFlowMgr;
	OperatorMgr m_operatorMgr;
	UnitMgr m_unitMgr;
	ExtensionLibMgr m_extensionLibMgr;
	ImportMgr m_importMgr;
	LlvmIrBuilder m_llvmIrBuilder;
	LlvmDiBuilder m_llvmDiBuilder;

public:
	Module ();

	~Module ()
	{
		clear ();
	}

	rtl::String
	getName ()
	{
		return m_name;
	}

	uint_t
	getCompileFlags ()
	{
		return m_compileFlags;
	}

	ModuleCompileState
	getCompileState ()
	{
		return m_compileState;
	}

	llvm::LLVMContext*
	getLlvmContext ()
	{
		ASSERT (m_llvmModule);
		return &m_llvmModule->getContext ();
	}

	llvm::Module*
	getLlvmModule ()
	{
		ASSERT (m_llvmModule);
		return m_llvmModule;
	}

	llvm::ExecutionEngine*
	getLlvmExecutionEngine ()
	{
		ASSERT (m_llvmExecutionEngine);
		return m_llvmExecutionEngine;
	}

	Function*
	getConstructor ()
	{
		return m_constructor;
	}

	bool
	setConstructor (Function* function);

	Function*
	getDestructor ()
	{
		return m_destructor;
	}

	bool
	setDestructor (Function* function);

	void
	setFunctionPointer (
		llvm::ExecutionEngine* llvmExecutionEngine,
		Function* function,
		void* p
		)
	{
		llvmExecutionEngine->addGlobalMapping (function->getLlvmFunction (), p);
	}

	void
	setFunctionPointer (
		llvm::ExecutionEngine* llvmExecutionEngine,
		StdFunc funcKind,
		void* p
		)
	{
		setFunctionPointer (llvmExecutionEngine, m_functionMgr.getStdFunction (funcKind), p);
	}

	bool
	setFunctionPointer (
		llvm::ExecutionEngine* llvmExecutionEngine,
		const char* name,
		void* p
		);

	bool
	setFunctionPointer (
		llvm::ExecutionEngine* llvmExecutionEngine,
		const QualifiedName& name,
		void* p
		);

	void
	markForLayout (
		ModuleItem* item,
		bool isForced = false
		);

	void
	markForCompile (ModuleItem* item);

	bool
	create (
		const rtl::String& name,
		uint_t compileFlags = ModuleCompileFlag_StdFlags
		);

	bool
	createLlvmExecutionEngine ();

	void
	clear ();

	bool
	parse (
		const char* filePath,
		const char* source,
		size_t length = -1
		);

	bool
	parseFile (const char* filePath);

	bool
	parseImports ();

	bool
	calcLayout ();

	bool
	processCompileArray ();

	bool
	postParseStdItem ();

	bool
	compile ();

	bool
	jit ();

	void
	mapFunction (
		Function* function,
		void* p
		);

	void*
	findFunctionMapping (const char* name)
	{
		rtl::StringHashTableMapIterator <void*> it = m_functionMap.find (name);
		return it ? it->m_value : NULL;
	}

	rtl::String
	getLlvmIrString ();

protected:
	bool
	createDefaultConstructor ();
};

//.............................................................................

} // namespace jnc {
