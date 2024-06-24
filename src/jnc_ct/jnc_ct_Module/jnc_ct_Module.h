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

class ParseContext {
protected:
	Module* m_module;
	Unit* m_prevUnit;
	ReactorBody* m_prevReactorBody;
	bool m_isNamespaceOpened;

public:
	ParseContext(
		Module* module,
		Unit* unit,
		Namespace* nspace
	) {
		set(module, unit, nspace);
	}

	ParseContext(
		Module* module,
		ModuleItemDecl* decl
	) {
		set(module, decl->getParentUnit(), decl->getParentNamespace());
	}

	~ParseContext() {
		restore();
	}

protected:
	void
	set(
		Module* module,
		Unit* unit,
		Namespace* nspace
	);

	void
	restore();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// makes it convenient to initialize childs (especially operators)

class PreModule {
protected:
	PreModule() {
		Module* prevModule = sys::setTlsPtrSlotValue<Module>((Module*)this);
		ASSERT(prevModule == NULL);
	}

public:
	static
	Module*
	getCurrentConstructedModule() {
		return sys::getTlsPtrSlotValue<Module>();
	}

protected:
	void
	finalizeConstruction() {
		sys::setTlsPtrSlotValue<Module>(NULL);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Module: public PreModule {
	friend class Jit;  // JITters take ownership of llvm::Module & llvm::LLVMContext
	friend class Parser;

protected:
	enum AuxCompileFlag {
		AuxCompileFlag_SkipAccessChecks = 0x80000000,
		AuxCompileFlag_IntrospectionLib = 0x40000000,
		AuxCompileFlag_DynamicLayout    = 0x20000000,
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
		uint_t m_flags;

		RequiredItem();

		RequiredItem(
			ModuleItemKind itemKind,
			uint_t flags
		);

		RequiredItem(
			TypeKind typeKind,
			uint_t flags
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
	AttributeObserverFunc* m_attributeObserver;
	void* m_attributeObserverContext;
	uint_t m_attributeObserverItemKindMask;
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

	void
	disableAccessChecks() {
		m_compileFlags |= AuxCompileFlag_SkipAccessChecks;
	}

	void
	enableAccessChecks() {
		m_compileFlags &= ~AuxCompileFlag_SkipAccessChecks;
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
	setAttributeObserver(
		AttributeObserverFunc* observer,
		void* context,
		uint_t itemKindMask
	);

	void
	notifyAttributeObserver(
		ModuleItem* item,
		AttributeBlock* attributeBlock
	) {
		if (m_attributeObserver && (m_attributeObserverItemKindMask & (1 << item->getItemKind())))
			m_attributeObserver(m_attributeObserverContext, item, attributeBlock);
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
		uint_t flags = jnc_ModuleRequireFlag_Essential
	) {
		m_requireSet[name] = RequiredItem(itemKind, flags);
	}

	void
	require(
		TypeKind typeKind,
		const sl::StringRef& name,
		uint_t flags = jnc_ModuleRequireFlag_Essential
	) {
		m_requireSet[name] = RequiredItem(typeKind, flags);
	}

	bool
	compile();

	bool
	optimize(uint_t level);

	bool
	createJit();

	bool
	ensureJitCreated() {
		return m_jit || createJit();
	}

	bool
	jit();

	bool
	ensureIntrospectionLibRequired() {
		return (m_compileFlags & AuxCompileFlag_IntrospectionLib) || requireIntrospectionLib();
	}

	bool
	ensureDynamicLayoutRequired() {
		return (m_compileFlags & AuxCompileFlag_DynamicLayout) || requireDynamicLayout();
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
	compileImpl();

	bool
	requireIntrospectionLib();

	bool
	requireDynamicLayout();

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
Module::RequiredItem::RequiredItem() {
	m_itemKind = ModuleItemKind_Undefined;
	m_typeKind = TypeKind_Void;
	m_flags = 0;
}

inline
Module::RequiredItem::RequiredItem(
	ModuleItemKind itemKind,
	uint_t flags
) {
	m_itemKind = itemKind;
	m_typeKind = TypeKind_Void;
	m_flags = flags;
}

inline
Module::RequiredItem::RequiredItem(
	TypeKind typeKind,
	uint_t flags
) {
	m_itemKind = ModuleItemKind_Type;
	m_typeKind = typeKind;
	m_flags = flags;
}

inline
void
Module::setAttributeObserver(
	AttributeObserverFunc* observer,
	void* context,
	uint_t itemKindMask
) {
	m_attributeObserver = observer;
	m_attributeObserverContext = context;
	m_attributeObserverItemKindMask = itemKindMask;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
void
ParseContext::set(
	Module* module,
	Unit* unit,
	Namespace* nspace
) {
	m_module = module;
	m_prevUnit = module->m_unitMgr.setCurrentUnit(unit);
	m_prevReactorBody = module->m_controlFlowMgr.setCurrentReactor(NULL);
	m_isNamespaceOpened = module->m_namespaceMgr.openNamespaceIf(nspace);
}

inline
void
ParseContext::restore() {
	m_module->m_unitMgr.setCurrentUnit(m_prevUnit);
	m_module->m_controlFlowMgr.setCurrentReactor(m_prevReactorBody);
	if (m_isNamespaceOpened)
		m_module->m_namespaceMgr.closeNamespace();
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

inline
bool
OperatorMgr::parseReactiveInitializer(
	const Value& value,
	sl::List<Token>* tokenList
) {
	m_module->m_controlFlowMgr.enterReactiveExpression();
	bool result = m_module->m_operatorMgr.parseInitializer(value, tokenList);
	m_module->m_controlFlowMgr.finalizeReactiveExpressionStmt();
	return result;
}

//..............................................................................

inline
void
ControlFlowMgr::enterReactiveExpression() {
	if (!isReactor())
		return;

	m_reactorBody->m_reactionBlock = m_currentBlock;
	m_reactorBody->m_reactionBindingCount = 0;

	if (m_module->hasCodeGen() && !m_currentBlock->getLlvmBlock()->empty())
		m_reactorBody->m_llvmReactionIt = m_currentBlock->getLlvmBlock()->back();
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

inline
void
Value::init() {
	m_valueKind = ValueKind_Void;
	m_type = NULL;
	m_variable = NULL;
	m_llvmValue = NULL;
}

inline
void
Value::clear() {
	m_valueKind = ValueKind_Void;
	m_type = NULL;
	m_item = NULL;
	m_llvmValue = NULL;
	m_closure = rc::g_nullPtr;
	m_leanDataPtrValidator = rc::g_nullPtr;
}

inline
void
Value::setVoid(Module* module) {
	clear();
	m_valueKind = ValueKind_Void;
	m_type = module->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

inline
void
Value::setNull(Module* module) {
	clear();
	m_valueKind = ValueKind_Null;
	m_type = module->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

inline
void
Value::setType(Type* type) {
	clear();
	m_valueKind = type->getTypeKind() != TypeKind_Void ? ValueKind_Type : ValueKind_Void;
	m_type = type;
}

inline
void
Value::setNamespace(GlobalNamespace* nspace) {
	clear();
	Module* module = nspace->getModule();
	m_valueKind = ValueKind_Namespace;
	m_namespace = nspace;
	m_type = module->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

inline
void
Value::setNamespace(NamedType* type) {
	clear();
	Module* module = type->getModule();
	m_valueKind = ValueKind_Namespace;
	m_namespace = type;
	m_type = module->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

inline
void
Value::setFunctionOverload(FunctionOverload* functionOverload) {
	clear();
	m_valueKind = ValueKind_FunctionOverload;
	m_functionOverload = functionOverload;
	m_type = functionOverload->getModule()->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

inline
bool
Value::trySetOverloadableFunction(OverloadableFunction function) {
	if (function->getItemKind() == ModuleItemKind_Function)
		return trySetFunction(function.getFunction());

	setFunctionOverload(function.getFunctionOverload());
	return true;
}

inline
void
Value::setFunctionTypeOverload(FunctionTypeOverload* functionTypeOverload) {
	clear();
	m_valueKind = ValueKind_FunctionTypeOverload;
	m_functionTypeOverload = functionTypeOverload;
	m_type = functionTypeOverload->getModule()->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

inline
void
Value::setProperty(Property* prop) {
	clear();

	m_valueKind = ValueKind_Property;
	m_property = prop;
	m_type = prop->getType()->getPropertyPtrType(
		TypeKind_PropertyRef,
		PropertyPtrTypeKind_Thin,
		PtrTypeFlag_Safe
	);

	// don't assign LlvmValue (property LlvmValue is only needed for pointers)
}

inline
void
Value::setField(
	Field* field,
	size_t baseOffset
) {
	clear();
	m_valueKind = ValueKind_Field;
	m_field = field;
	m_type = field->getModule()->m_typeMgr.getPrimitiveType(TypeKind_Void);
	m_constData.setCount(sizeof(size_t));
	*(size_t*)m_constData.p() = baseOffset + field->getOffset();
}

inline
void
Value::setLlvmValue(
	llvm::Value* llvmValue,
	Type* type,
	ValueKind valueKind
) {
	clear();
	m_valueKind = valueKind;
	m_type = type;
	m_llvmValue = llvmValue;
}

//..............................................................................

} // namespace ct
} // namespace jnc
