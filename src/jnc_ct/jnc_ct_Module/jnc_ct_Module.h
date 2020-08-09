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

#include "jnc_ct_TypeMgr.h"
#include "jnc_ct_AttributeMgr.h"
#include "jnc_ct_NamespaceMgr.h"
#include "jnc_ct_FunctionMgr.h"
#include "jnc_ct_VariableMgr.h"
#include "jnc_ct_ConstMgr.h"
#include "jnc_ct_ControlFlowMgr.h"
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_GcShadowStackMgr.h"
#include "jnc_ct_RegexMgr.h"
#include "jnc_ct_UnitMgr.h"
#include "jnc_ct_ImportMgr.h"
#include "jnc_ct_ExtensionLibMgr.h"
#include "jnc_ct_DoxyHost.h"
#include "jnc_ct_LlvmIrBuilder.h"
#include "jnc_ct_LlvmDiBuilder.h"

namespace jnc {
namespace ct {

//..............................................................................

// makes it convenient to initialize childs (especially operators)

class PreModule
{
protected:
	PreModule()
	{
		Module* prevModule = sys::setTlsPtrSlotValue<Module> ((Module*)this);
		ASSERT(prevModule == NULL);
	}

public:
	static
	Module*
	getCurrentConstructedModule()
	{
		return sys::getTlsPtrSlotValue<Module> ();
	}

protected:
	void
	finalizeConstruction()
	{
		sys::setTlsPtrSlotValue<Module> (NULL);
	}
};

//..............................................................................

class Module: public PreModule
{
protected:
	enum AuxCompileFlag
	{
		AuxCompileFlag_IntrospectionLib = 0x80000000,
	};

	enum
	{
		DefaultErrorCountLimit = 100,
	};

	struct RequiredItem
	{
		ModuleItemKind m_itemKind;
		TypeKind m_typeKind;
		bool m_isEssential;

		RequiredItem();

		RequiredItem(
			ModuleItemKind itemKind,
			bool isEssential
			);

		RequiredItem(
			TypeKind typeKind,
			bool isEssential
			);
	};

protected:
	sl::String m_name;

	uint_t m_compileFlags;
	ModuleCompileState m_compileState;
	size_t m_tryCompileLevel;
	size_t m_compileErrorCount;
	ModuleCompileErrorHandlerFunc* m_compileErrorHandler;
	void* m_compileErrorHandlerContext;

	sl::Array<Function*> m_compileArray;
	sl::BoxList<sl::String> m_sourceList; // need to keep all sources in-memory during compilation
	sl::StringHashTable<bool> m_filePathSet;
	sl::StringHashTable<void*> m_functionMap;
	sl::StringHashTable<RequiredItem> m_requireSet;

	Function* m_constructor;

	// codegen-only

	llvm::LLVMContext* m_llvmContext;
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
	RegexMgr m_regexMgr;
	UnitMgr m_unitMgr;
	ImportMgr m_importMgr;
	ExtensionLibMgr m_extensionLibMgr;
	DoxyHost m_doxyHost;
	dox::Module m_doxyModule;

	// codegen-only

	LlvmIrBuilder m_llvmIrBuilder;
	LlvmDiBuilder m_llvmDiBuilder;

	size_t m_compileErrorCountLimit; // freely adjustible

public:
	Module();
	~Module();

	bool
	hasCodeGen()
	{
		return m_llvmIrBuilder;
	}

	const sl::String&
	getName()
	{
		return m_name;
	}

	uint_t
	getCompileFlags()
	{
		return m_compileFlags;
	}

	ModuleCompileState
	getCompileState()
	{
		return m_compileState;
	}

	size_t
	getCompileErrorCount()
	{
		return m_compileErrorCount;
	}

	void
	enterTryCompile()
	{
		m_tryCompileLevel++;
	}

	void
	leaveTryCompile()
	{
		m_tryCompileLevel--;
	}

	bool
	processCompileError(ModuleCompileErrorKind errorKind);

	void
	setCompileErrorHandler(
		ModuleCompileErrorHandlerFunc* handler,
		void* context
		)
	{
		m_compileErrorHandler = handler;
		m_compileErrorHandlerContext = context;
	}

	llvm::LLVMContext*
	getLlvmContext()
	{
		ASSERT(m_llvmModule);
		return &m_llvmModule->getContext();
	}

	llvm::Module*
	getLlvmModule()
	{
		ASSERT(m_llvmModule);
		return m_llvmModule;
	}

	llvm::ExecutionEngine*
	getLlvmExecutionEngine()
	{
		ASSERT(m_llvmExecutionEngine);
		return m_llvmExecutionEngine;
	}

	Function*
	getConstructor()
	{
		return m_constructor;
	}

	void
	setFunctionPointer(
		llvm::ExecutionEngine* llvmExecutionEngine,
		Function* function,
		void* p
		)
	{
		llvmExecutionEngine->addGlobalMapping(function->getLlvmFunction(), p);
	}

	void
	setFunctionPointer(
		llvm::ExecutionEngine* llvmExecutionEngine,
		StdFunc funcKind,
		void* p
		)
	{
		setFunctionPointer(llvmExecutionEngine, m_functionMgr.getStdFunction(funcKind), p);
	}

	bool
	setFunctionPointer(
		llvm::ExecutionEngine* llvmExecutionEngine,
		const sl::StringRef& name,
		void* p
		);

	bool
	setFunctionPointer(
		llvm::ExecutionEngine* llvmExecutionEngine,
		const QualifiedName& name,
		void* p
		);

	void
	markForCompile(Function* function);

	void
	initialize(
		const sl::StringRef& name,
		uint_t compileFlags = ModuleCompileFlag_StdFlags
		);

	void
	clear();

	bool
	parse(
		const sl::StringRef& fileName,
		const sl::StringRef& source
		)
	{
		return parseImpl(NULL, fileName, source);
	}

	bool
	parseFile(const sl::StringRef& fileName);

	bool
	parseImports();

	void
	require(
		ModuleItemKind itemKind,
		const sl::StringRef& name,
		bool isEssential = true
		)
	{
		m_requireSet[name] = RequiredItem(itemKind, isEssential);
	}

	void
	require(
		TypeKind typeKind,
		const sl::StringRef& name,
		bool isEssential = true
		)
	{
		m_requireSet[name] = RequiredItem(typeKind, isEssential);
	}

	bool
	compile();

	bool
	optimize(uint_t level = 2);

	bool
	jit();

	bool
	ensureIntrospectionLibRequired()
	{
		return (m_compileFlags & AuxCompileFlag_IntrospectionLib) || requireIntrospectionLib();
	}

	bool
	mapVariable(
		Variable* variable,
		void* p
		);

	bool
	mapFunction(
		Function* function,
		void* p
		);

	void*
	findFunctionMapping(const sl::StringRef& name);

	sl::String
	getLlvmIrString();

protected:
	void
	clearLlvm();

	bool
	parseImpl(
		ExtensionLib* lib,
		const sl::StringRef& fileName,
		const sl::StringRef& source
		);

	bool
	requireIntrospectionLib();

	bool
	createLlvmExecutionEngine();

	bool
	processRequireSet();

	bool
	processCompileArray();

	void
	createConstructor();

	Function*
	createGlobalPrimerFunction();

	Function*
	createGlobalInitializerFunction();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Module::RequiredItem::RequiredItem()
{
	m_itemKind = ModuleItemKind_Undefined;
	m_typeKind = TypeKind_Void;
	m_isEssential = false;
}

inline
Module::RequiredItem::RequiredItem(
	ModuleItemKind itemKind,
	bool isEssential
	)
{
	m_itemKind = itemKind;
	m_typeKind = TypeKind_Void;
	m_isEssential = isEssential;
}

inline
Module::RequiredItem::RequiredItem(
	TypeKind typeKind,
	bool isEssential
	)
{
	m_itemKind = ModuleItemKind_Type;
	m_typeKind = typeKind;
	m_isEssential = isEssential;
}

//..............................................................................

template <typename T>
T*
MemberBlock::createMethod(
	const sl::StringRef& name,
	FunctionType* shortType
	)
{
	sl::String qualifedName = getParentNamespaceImpl()->createQualifiedName(name);
	T* function = m_parent->getModule()->m_functionMgr.createFunction<T>(name, qualifedName, shortType);
	return addMethod(function) ? function : NULL;
}

template <typename T>
T*
MemberBlock::createUnnamedMethod(
	FunctionKind functionKind,
	FunctionType* shortType
	)
{
	T* function = m_parent->getModule()->m_functionMgr.createFunction<T>(shortType);
	function->m_functionKind = functionKind;
	return addMethod(function) ? function : NULL;
}

template <typename T>
T*
MemberBlock::createDefaultMethod()
{
	Module* module = m_parent->getModule();
	FunctionType* type = (FunctionType*)module->m_typeMgr.getStdType(StdType_SimpleFunction);
	T* function = module->m_functionMgr.createFunction<T>(sl::StringRef(), sl::StringRef(), type);
	bool result = addMethod(function);
	return result ? function : NULL;
}

//..............................................................................

} // namespace ct
} // namespace jnc
