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
#include "jnc_ct_UnitMgr.h"
#include "jnc_ct_PragmaMgr.h"
#include "jnc_ct_ImportMgr.h"
#include "jnc_ct_ExtensionLibMgr.h"
#include "jnc_ct_CodeAssistMgr.h"
#include "jnc_ct_DoxyHost.h"
#include "jnc_ct_LlvmIrBuilder.h"
#include "jnc_ct_LlvmDiBuilder.h"

namespace jnc {
namespace ct {

class Jit;

//..............................................................................

// makes it convenient to initialize childs (especially operators)

class PreModule {
protected:
	PreModule() {
		Module* prevModule = sys::setTlsPtrSlotValue<Module> ((Module*)this);
		ASSERT(prevModule == NULL);
	}

public:
	static
	Module*
	getCurrentConstructedModule() {
		return sys::getTlsPtrSlotValue<Module> ();
	}

protected:
	void
	finalizeConstruction() {
		sys::setTlsPtrSlotValue<Module> (NULL);
	}
};

//..............................................................................

class Module: public PreModule {
	friend class Jit;  // JITters take ownership of llvm::Module & llvm::LLVMContext
	friend class Parser;

protected:
	enum AuxCompileFlag {
		AuxCompileFlag_IntrospectionLib = 0x80000000,
		AuxCompileFlag_SkipAccessChecks = 0x40000000,
	};

	enum AsyncFlag {
		AsyncFlag_CancelCodeAssist = 0x01,
	};

	enum {
		DefaultErrorCountLimit = 100,
	};

	struct RequiredItem {
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
	ModuleConfig m_config;
	uint_t m_compileFlags;
	ModuleCompileState m_compileState;
	size_t m_tryCompileLevel;
	size_t m_compileErrorCount;
	ModuleCompileErrorHandlerFunc* m_compileErrorHandler;
	void* m_compileErrorHandlerContext;
	ModuleDynamicSectionObserverFunc* m_dynamicSectionObserver;
	void* m_dynamicSectionObserverContext;
	volatile int32_t m_asyncFlags;

	sl::Array<Function*> m_compileArray;
	sl::BoxList<sl::String> m_sourceList; // need to keep all sources in-memory during compilation
	sl::StringHashTable<bool> m_filePathSet;
	sl::StringHashTable<RequiredItem> m_requireSet;

	Function* m_constructor;

	// codegen-only

	llvm::LLVMContext* m_llvmContext;
	llvm::Module* m_llvmModule;

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
	UnitMgr m_unitMgr;
	PragmaMgr m_pragmaMgr;
	ImportMgr m_importMgr;
	ExtensionLibMgr m_extensionLibMgr;
	CodeAssistMgr m_codeAssistMgr;
	DoxyHost m_doxyHost;
	dox::Module m_doxyModule;
	Jit* m_jit;

	// codegen-only

	LlvmIrBuilder m_llvmIrBuilder;
	LlvmDiBuilder m_llvmDiBuilder;

	size_t m_compileErrorCountLimit; // freely adjustible

public:
	Module();
	~Module();

	bool
	hasCodeGen() {
		return m_llvmIrBuilder;
	}

	bool
	hasAccessChecks() {
		return (m_compileFlags & AuxCompileFlag_SkipAccessChecks) == 0;
	}

	const sl::String&
	getName() {
		return m_name;
	}

	JitKind
	getJitKind() {
		return m_config.m_jitKind;
	}

	uint_t
	getCompileFlags() {
		return m_compileFlags;
	}

	ModuleCompileState
	getCompileState() {
		return m_compileState;
	}

	size_t
	getCompileErrorCount() {
		return m_compileErrorCount;
	}

	void
	enterTryCompile() {
		m_tryCompileLevel++;
	}

	void
	leaveTryCompile() {
		m_tryCompileLevel--;
	}

	bool
	processCompileError(ModuleCompileErrorKind errorKind);

	void
	setCompileErrorHandler(
		ModuleCompileErrorHandlerFunc* handler,
		void* context
	) {
		m_compileErrorHandler = handler;
		m_compileErrorHandlerContext = context;
	}

	void
	setDynamicSectionObserver(
		ModuleDynamicSectionObserverFunc* observer,
		void* context
	) {
		m_dynamicSectionObserver = observer;
		m_dynamicSectionObserverContext = context;
	}

	llvm::LLVMContext*
	getLlvmContext() {
		ASSERT(m_llvmModule);
		return &m_llvmModule->getContext();
	}

	llvm::Module*
	getLlvmModule() {
		ASSERT(m_llvmModule);
		return m_llvmModule;
	}

	Function*
	getConstructor() {
		return m_constructor;
	}

	void
	markForCompile(Function* function);

	void
	clear();

	void
	initialize(
		const sl::StringRef& name,
		const ModuleConfig* config
	);

	CodeAssist*
	generateCodeAssist(
		jnc_CodeAssistKind kind,
		Module* cacheModule,
		size_t offset,
		const sl::StringRef& source
	);

	void
	cancelCodeAssist() {
		setAsyncFlag(AsyncFlag_CancelCodeAssist);
	}

	bool
	parse(
		const sl::StringRef& fileName,
		const sl::StringRef& source
	);

	bool
	parseFile(const sl::StringRef& fileName);

	bool
	parseImports();

	void
	require(
		ModuleItemKind itemKind,
		const sl::StringRef& name,
		bool isEssential = true
	) {
		m_requireSet[name] = RequiredItem(itemKind, isEssential);
	}

	void
	require(
		TypeKind typeKind,
		const sl::StringRef& name,
		bool isEssential = true
	) {
		m_requireSet[name] = RequiredItem(typeKind, isEssential);
	}

	bool
	compile();

	bool
	optimize(uint_t level);

	bool
	jit();

	bool
	ensureIntrospectionLibRequired() {
		return (m_compileFlags & AuxCompileFlag_IntrospectionLib) || requireIntrospectionLib();
	}

	sl::String
	getLlvmIrString();

protected:
	void
	setAsyncFlag(AsyncFlag flag);

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
	processRequireSet();

	bool
	processCompileArray();

	bool
	createJit();

	void
	createConstructor();

	Function*
	createGlobalPrimerFunction();

	Function*
	createGlobalInitializerFunction();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
Module::RequiredItem::RequiredItem() {
	m_itemKind = ModuleItemKind_Undefined;
	m_typeKind = TypeKind_Void;
	m_isEssential = false;
}

inline
Module::RequiredItem::RequiredItem(
	ModuleItemKind itemKind,
	bool isEssential
) {
	m_itemKind = itemKind;
	m_typeKind = TypeKind_Void;
	m_isEssential = isEssential;
}

inline
Module::RequiredItem::RequiredItem(
	TypeKind typeKind,
	bool isEssential
) {
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
) {
	T* function = m_parent->getModule()->m_functionMgr.createFunction<T>(
		name,
		getParentNamespaceImpl()->createQualifiedName(name),
		shortType
	);
	return addMethod(function) ? function : NULL;
}

template <typename T>
T*
MemberBlock::createUnnamedMethod(
	FunctionKind functionKind,
	FunctionType* shortType
) {
	T* function = m_parent->getModule()->m_functionMgr.createFunction<T>(shortType);
	function->m_functionKind = functionKind;
	return addMethod(function) ? function : NULL;
}

template <typename T>
T*
MemberBlock::createDefaultMethod() {
	Module* module = m_parent->getModule();
	FunctionType* type = (FunctionType*)module->m_typeMgr.getStdType(StdType_SimpleFunction);
	T* function = module->m_functionMgr.createFunction<T>(sl::StringRef(), sl::StringRef(), type);
	bool result = addMethod(function);
	return result ? function : NULL;
}


//..............................................................................

inline
bool
Unit::isRootUnit() {
	return this == m_module->m_unitMgr.getRootUnit();
}

//..............................................................................

inline
void
CodeAssistMgr::prepareQualifiedNameFallback(
	const QualifiedName& namePrefix,
	const Token& token
) {
	m_fallbackMode = FallbackMode_QualifiedName;
	m_fallbackNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	m_fallbackNamePrefix = namePrefix;
	m_fallbackToken = token;
}

inline
void
CodeAssistMgr::prepareExpressionFallback(const sl::List<Token>& expression) {
	ASSERT(!expression.isEmpty());
	m_fallbackMode = FallbackMode_Expression;
	m_fallbackNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	cloneTokenList(&m_fallbackExpression, expression);
}

inline
void
CodeAssistMgr::prepareIdentifierFallback(const Token& token) {
	if (m_fallbackMode <= FallbackMode_Identifier) { // only if no better fallbacks
		m_fallbackMode = FallbackMode_Identifier;
		m_fallbackNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
		m_fallbackToken = token;
	}
}

inline
void
CodeAssistMgr::prepareNamespaceFallback() {
	if (m_fallbackMode <= FallbackMode_Namespace) { // only if no better fallbacks
		m_fallbackMode = FallbackMode_Namespace;
		m_fallbackNamespace = m_module->m_namespaceMgr.getCurrentNamespace();
	}
}

//..............................................................................

inline
bool
OperatorMgr::checkAccess(ModuleItemDecl* decl) {
	Namespace* nspace = decl->getParentNamespace();
	if (m_module->hasAccessChecks() &&
		decl->getAccessKind() != AccessKind_Public &&
		m_module->m_namespaceMgr.getAccessKind(nspace) == AccessKind_Public
	) {
		err::setFormatStringError("'%s' is protected", decl->getQualifiedName().sz());
		return false;
	}

	return true;
}

//..............................................................................

inline
void
LlvmIrBuilder::addTypedAttribute(
	llvm::Function* llvmFunction,
	unsigned i,
	llvm::Attribute::AttrKind attrKind,
	Type* type
) {
#if (LLVM_VERSION_MAJOR < 9)
	llvmFunction->addAttribute(i, attrKind);
#elif (LLVM_VERSION_MAJOR < 14)
	llvm::Attribute llvmAttr = llvm::Attribute::get(*m_module->getLlvmContext(), attrKind, type->getLlvmType());
    llvmFunction->addAttribute(i, llvmAttr);
#else
    llvm::Attribute llvmAttr = llvm::Attribute::get(*m_module->getLlvmContext(), attrKind, type->getLlvmType());
    llvmFunction->addAttributeAtIndex(i, llvmAttr);
#endif
}

inline
void
LlvmIrBuilder::addTypedAttribute(
	llvm::CallInst* llvmCallInst,
	unsigned i,
	llvm::Attribute::AttrKind attrKind,
	Type* type
) {
#if (LLVM_VERSION_MAJOR < 9)
	llvmCallInst->addAttribute(i, attrKind);
#elif (LLVM_VERSION_MAJOR < 14)
	llvm::Attribute llvmAttr = llvm::Attribute::get(*m_module->getLlvmContext(), attrKind, type->getLlvmType());
	llvmCallInst->addAttribute(i, llvmAttr);
#else
	llvm::Attribute llvmAttr = llvm::Attribute::get(*m_module->getLlvmContext(), attrKind, type->getLlvmType());
    llvmCallInst->addAttributeAtIndex(i, llvmAttr);
#endif
}

//..............................................................................

} // namespace ct
} // namespace jnc
