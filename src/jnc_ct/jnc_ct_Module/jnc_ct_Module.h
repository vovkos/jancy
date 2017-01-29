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

#pragma once

#include "jnc_ct_AttributeMgr.h"
#include "jnc_ct_TypeMgr.h"
#include "jnc_ct_NamespaceMgr.h"
#include "jnc_ct_FunctionMgr.h"
#include "jnc_ct_VariableMgr.h"
#include "jnc_ct_ConstMgr.h"
#include "jnc_ct_ControlFlowMgr.h"
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_GcShadowStackMgr.h"
#include "jnc_ct_AutomatonMgr.h"
#include "jnc_ct_UnitMgr.h"
#include "jnc_ct_LlvmIrBuilder.h"
#include "jnc_ct_LlvmDiBuilder.h"
#include "jnc_ct_ImportMgr.h"
#include "jnc_ct_ExtensionLibMgr.h"
#include "jnc_ct_DoxyMgr.h"

namespace jnc {
namespace ct {

//..............................................................................

// makes it convenient to initialize childs (especially operators)

class PreModule
{
protected:
	PreModule ()
	{
		Module* prevModule = sys::setTlsPtrSlotValue <Module> ((Module*) this);
		ASSERT (prevModule == NULL);
	}

public:
	static
	Module*
	getCurrentConstructedModule ()
	{
		return sys::getTlsPtrSlotValue <Module> ();
	}

protected:
	void
	finalizeConstruction ()
	{
		sys::setTlsPtrSlotValue <Module> (NULL);
	}
};

//..............................................................................

class Module: public PreModule
{
protected:
	sl::String m_name;

	uint_t m_compileFlags;
	ModuleCompileState m_compileState;

	Function* m_constructor;
	Function* m_destructor;

	sl::Array <ModuleItem*> m_calcLayoutArray;
	sl::Array <ModuleItem*> m_compileArray;
	sl::BoxList <sl::String> m_sourceList; // need to keep all sources in-memory during compilation
	sl::BoxList <sl::String> m_filePathList;
	sl::StringHashTable m_filePathMap;
	sl::StringHashTableMap <void*> m_functionMap;

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
	GcShadowStackMgr m_gcShadowStackMgr;
	AutomatonMgr m_automatonMgr;
	UnitMgr m_unitMgr;
	ImportMgr m_importMgr;
	ExtensionLibMgr m_extensionLibMgr;
	DoxyMgr m_doxyMgr;
	LlvmIrBuilder m_llvmIrBuilder;
	LlvmDiBuilder m_llvmDiBuilder;

public:
	Module ();

	~Module ()
	{
		clear ();
	}

	const sl::String&
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

	Function*
	getDestructor ()
	{
		return m_destructor;
	}

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
		const sl::StringRef& name,
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

	void
	initialize (
		const sl::StringRef& name,
		uint_t compileFlags = ModuleCompileFlag_StdFlags
		);

	void
	clear ();

	bool
	parse (
		ExtensionLib* lib,
		const sl::StringRef& fileName,
		const sl::StringRef& source
		);

	bool
	parseFile (const sl::StringRef& fileName);

	bool
	parseImports ();

	bool
	link ();

	bool
	calcLayout ();

	bool
	compile ();

	bool
	jit ();

	bool
	postParseStdItem ();

	void
	mapFunction (
		Function* function,
		void* p
		);

	void*
	findFunctionMapping (const sl::StringRef& name);

	sl::String
	getLlvmIrString ();

protected:
	bool
	createLlvmExecutionEngine ();

	bool
	createConstructorDestructor ();

	bool
	processCalcLayoutArray ();

	bool
	processCompileArray ();
};

//..............................................................................

} // namespace ct
} // namespace jnc
