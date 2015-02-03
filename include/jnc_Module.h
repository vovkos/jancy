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
#include "jnc_CallFunction.h"

namespace jnc {

class Module;

//.............................................................................

enum ModuleFlag
{
	ModuleFlag_DebugInfo  = 0x0001,
	ModuleFlag_IrComments = 0x0002,
	ModuleFlag_McJit      = 0x0004,
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
	Module* m_prevModule;

	PreModule ()
	{
		m_prevModule = mt::setTlsSlotValue <Module> ((Module*) this);
	}

	void
	restorePrevModule ()
	{
		mt::setTlsSlotValue <Module> (m_prevModule);
	}
};

//.............................................................................

class Module: PreModule
{
protected:
	rtl::String m_name;

	uint_t m_flags;
	ModuleCompileState m_compileState;

	rtl::BoxList <rtl::String> m_importList;
	rtl::BoxList <rtl::String> m_shadowImportList;
	rtl::StringHashTable m_importSet;

	Function* m_constructor;
	Function* m_destructor;

	rtl::Array <ModuleItem*> m_calcLayoutArray;
	rtl::Array <ModuleItem*> m_compileArray;
	rtl::Array <ModuleItem*> m_apiItemArray;
	rtl::BoxList <rtl::String> m_sourceList;
	rtl::StringHashTableMap <void*> m_functionMap;

	llvm::Module* m_llvmModule;
	llvm::ExecutionEngine* m_llvmExecutionEngine;

public:
	rtl::BoxList <rtl::String> m_importDirList;

	TypeMgr m_typeMgr;
	AttributeMgr m_attributeMgr;
	NamespaceMgr m_namespaceMgr;
	FunctionMgr m_functionMgr;
	VariableMgr m_variableMgr;
	ConstMgr m_constMgr;
	ControlFlowMgr m_controlFlowMgr;
	OperatorMgr m_operatorMgr;
	UnitMgr m_unitMgr;
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
	getFlags ()
	{
		return m_flags;
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

	Type*
	getSimpleType (TypeKind typeKind)
	{
		return m_typeMgr.getPrimitiveType (typeKind);
	}

	Type*
	getSimpleType (StdType stdType)
	{
		return m_typeMgr.getStdType (stdType);
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
		void* pf
		)
	{
		llvmExecutionEngine->addGlobalMapping (function->getLlvmFunction (), pf);
	}

	void
	setFunctionPointer (
		llvm::ExecutionEngine* llvmExecutionEngine,
		StdFunction funcKind,
		void* pf
		)
	{
		setFunctionPointer (llvmExecutionEngine, m_functionMgr.getStdFunction (funcKind), pf);
	}

	bool
	setFunctionPointer (
		llvm::ExecutionEngine* llvmExecutionEngine,
		const char* name,
		void* pf
		);

	bool
	setFunctionPointer (
		llvm::ExecutionEngine* llvmExecutionEngine,
		const QualifiedName& name,
		void* pf
		);

	void
	markForLayout (
		ModuleItem* item,
		bool isForced = false
		);

	void
	markForCompile (ModuleItem* item);

	ModuleItem*
	getItemByName (const char* name)
	{
		return m_namespaceMgr.getGlobalNamespace ()->getItemByName (name);
	}

	Type*
	getTypeByName (const char* name)
	{
		return m_namespaceMgr.getGlobalNamespace ()->getTypeByName (name);
	}

	DerivableType*
	getDerivableTypeByName (const char* name)
	{
		return m_namespaceMgr.getGlobalNamespace ()->getDerivableTypeByName (name);
	}

	ClassType*
	getClassTypeByName (const char* name)
	{
		return m_namespaceMgr.getGlobalNamespace ()->getClassTypeByName (name);
	}

	Function*
	getFunctionByName (const char* name)
	{
		return m_namespaceMgr.getGlobalNamespace ()->getFunctionByName (name);
	}

	Function*
	getFunctionByName (
		const char* name,
		size_t overloadIdx
		)
	{
		return m_namespaceMgr.getGlobalNamespace ()->getFunctionByName (name, overloadIdx);
	}

	Property*
	getPropertyByName (const char* name)
	{
		return m_namespaceMgr.getGlobalNamespace ()->getPropertyByName (name);
	}

	ModuleItem*
	getApiItem (
		size_t slot,
		const char* name
		);

	Type*
	getApiType (
		size_t slot,
		const char* name
		)
	{
		return verifyModuleItemIsType (getApiItem (slot, name), name);
	}

	DerivableType*
	getApiDerivableType (
		size_t slot,
		const char* name
		)
	{
		return verifyModuleItemIsDerivableType (getApiItem (slot, name), name);
	}

	ClassType*
	getApiClassType (
		size_t slot,
		const char* name
		)
	{
		return verifyModuleItemIsClassType (getApiItem (slot, name), name);
	}

	Function*
	getApiFunction (
		size_t slot,
		const char* name
		)
	{
		return verifyModuleItemIsFunction (getApiItem (slot, name), name);
	}

	Function*
	getApiFunction (
		size_t slot,
		const char* name,
		size_t overloadIdx
		)
	{
		Function* function = getApiFunction (slot, name);
		return function ? function->getOverload (overloadIdx) : NULL;
	}

	Property*
	getApiProperty (
		size_t slot,
		const char* name
		)
	{
		return verifyModuleItemIsProperty (getApiItem (slot, name), name);
	}

	bool
	create (
		const rtl::String& name,
		uint_t flags = 0
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
	import (const char* fileName);

	bool
	link (Module* module);

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

	bool
	construct ()
	{
		return jnc::callVoidFunction (m_constructor);
	}

protected:
	bool
	createDefaultConstructor ();
};

//.............................................................................

typedef mt::ScopeTlsSlot <Module> ScopeThreadModule;

inline
Module*
getCurrentThreadModule ()
{
	return mt::getTlsSlotValue <Module> ();
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

ModuleItem*
getStockModuleItem (
	size_t slot,
	const char* name
	);

//.............................................................................

} // namespace jnc {
