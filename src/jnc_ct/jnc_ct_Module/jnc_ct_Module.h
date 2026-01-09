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

#include "jnc_Module.h"
#include "jnc_ct_TypeMgr.h"
#include "jnc_ct_AttributeMgr.h"
#include "jnc_ct_NamespaceMgr.h"
#include "jnc_ct_FunctionMgr.h"
#include "jnc_ct_VariableMgr.h"
#include "jnc_ct_ConstMgr.h"
#include "jnc_ct_DynamicLayoutMgr.h"
#include "jnc_ct_ControlFlowMgr.h"
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_GcShadowStackMgr.h"
#include "jnc_ct_UnitMgr.h"
#include "jnc_ct_ImportMgr.h"
#include "jnc_ct_PragmaMgr.h"
#include "jnc_ct_TemplateMgr.h"
#include "jnc_ct_ExtensionLibMgr.h"
#include "jnc_ct_CodeAssistMgr.h"
#include "jnc_ct_DoxyHost.h"
#include "jnc_ct_LlvmIrBuilder.h"
#include "jnc_ct_LlvmDiBuilder.h"
#include "jnc_ct_Jit.h"

namespace jnc {
namespace ct {

class Jit;

//..............................................................................

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
		AuxCompileFlag_IntrospectionLib  = 0x80000000,
		AuxCompileFlag_DynamicLayout     = 0x40000000,
		AuxCompileFlag_ConstOperatorOnly = 0x20000000,
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
	size_t m_uniqueLinkId;
	size_t m_tryCompileLevel;
	size_t m_disableAccessCheckLevel;
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
	DynamicLayoutMgr m_dynamicLayoutMgr;
	ControlFlowMgr m_controlFlowMgr;
	OperatorMgr m_operatorMgr;
	GcShadowStackMgr m_gcShadowStackMgr;
	UnitMgr m_unitMgr;
	ImportMgr m_importMgr;
	PragmaMgr m_pragmaMgr;
	TemplateMgr m_templateMgr;
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
		return m_disableAccessCheckLevel == 0;
	}

	void
	disableAccessChecks() {
		m_disableAccessCheckLevel++;
	}

	void
	enableAccessChecks() {
		m_disableAccessCheckLevel--;
	}

	bool
	isConstOperatorOnly() {
		return (m_compileFlags & AuxCompileFlag_ConstOperatorOnly) != 0;
	}

	uint_t
	setConstOperatorOnly();

	void
	restoreConstOperatorOnly(uint_t prevFlags) {
		m_compileFlags &= ~AuxCompileFlag_ConstOperatorOnly;
		m_compileFlags |= prevFlags & AuxCompileFlag_ConstOperatorOnly;
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

	size_t
	createUniqueLinkId() {
		return m_uniqueLinkId++;
	}

	sl::String
	createUniqueName(const sl::StringRef& prefix) {
		return sl::formatString("%s-%d", prefix.sz(), createUniqueLinkId());
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
uint_t
Module::setConstOperatorOnly() {
	uint_t prevFlags = m_compileFlags;
	m_compileFlags |= AuxCompileFlag_ConstOperatorOnly;
	return prevFlags;
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

inline
void
Module::markForCompile(Function* function) {
	if (function->m_flags & FunctionFlag_CompilePending)
		return;

	function->m_flags |= FunctionFlag_CompilePending;
	m_compileArray.append(function);
}

//..............................................................................

inline
void
ModuleItemContext::captureContext(Module* module) {
	setup(module->m_unitMgr.getCurrentUnit(), module->m_namespaceMgr.getCurrentNamespace());
}

//..............................................................................

inline
ImportTypeNameAnchor*
TypeMgr::createImportTypeNameAnchor() {
	ImportTypeNameAnchor* anchor = new ImportTypeNameAnchor;
	anchor->m_linkId = m_module->createUniqueLinkId();
	m_importTypeNameAnchorArray.append(anchor);
	return anchor;
}

//..............................................................................

inline
Type*
TypeName::lookupType(Namespace* nspace) const {
	return lookupTypeImpl(
		ModuleItemContext(m_parentUnit, nspace),
		nspace,
		m_parentUnit->getModule()->getCompileFlags(),
		false
	);
}

//..............................................................................

template <typename T>
T*
MemberBlock::createMethod(
	const sl::StringRef& name,
	FunctionType* shortType
) {
	T* function = m_parent->getModule()->m_functionMgr.createFunction<T>(name, shortType);
	bool result = addMethod(function);
	ASSERT(result);
	return function;
}

template <typename T>
T*
MemberBlock::createUnnamedMethod(
	FunctionKind functionKind,
	FunctionType* shortType
) {
	T* function = m_parent->getModule()->m_functionMgr.createFunction<T>(shortType);
	function->m_functionKind = functionKind;
	bool result = addMethod(function);
	ASSERT(result);
	return function;
}

template <typename T>
T*
MemberBlock::createDefaultMethod() {
	Module* module = m_parent->getModule();
	FunctionType* type = (FunctionType*)module->m_typeMgr.getStdType(StdType_SimpleFunction);
	T* function = module->m_functionMgr.createFunction<T>(sl::StringRef(), type);
	bool result = addMethod(function);
	ASSERT(result);
	return function;
}

//..............................................................................

inline
ArrayType*
Type::getArrayType(size_t elementCount) {
	return m_module->m_typeMgr.getArrayType(this, elementCount);
}

inline
DataPtrType*
Type::getDataPtrType(
	uint_t bitOffset,
	uint_t bitCount,
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return m_module->m_typeMgr.getDataPtrType(this, bitOffset, bitCount, typeKind, ptrTypeKind, flags);
}

inline
DataPtrType*
Type::getDataPtrType(
	TypeKind typeKind,
	DataPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return m_module->m_typeMgr.getDataPtrType(this, typeKind, ptrTypeKind, flags);
}

inline
FunctionArg*
Type::getSimpleFunctionArg(uint_t ptrTypeFlags) {
	return m_module->m_typeMgr.getSimpleFunctionArg(this, ptrTypeFlags);
}

inline
void
Type::prepareSimpleTypeVariable(StdType stdType) {
	ASSERT(!m_typeVariable);
	m_typeVariable = m_module->m_variableMgr.createRtlItemVariable(
		stdType,
		"jnc.g_type_" + getSignature(),
		this
	);
}

//..............................................................................

inline
FunctionPtrType*
FunctionType::getFunctionPtrType(
	TypeKind typeKind,
	FunctionPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return m_module->m_typeMgr.getFunctionPtrType(this, typeKind, ptrTypeKind, flags);
}

inline
ClassType*
FunctionType::getMulticastType() {
	return m_module->m_typeMgr.getMulticastType(this);
}

inline
FunctionType*
FunctionType::getMemberMethodType(
	DerivableType* parentType,
	uint_t thisArgTypeFlags
) {
	return m_module->m_typeMgr.getMemberMethodType(parentType, this, thisArgTypeFlags);
}

inline
FunctionType*
FunctionType::getStdObjectMemberMethodType() {
	return m_module->m_typeMgr.getStdObjectMemberMethodType(this);
}

//..............................................................................

inline
ClassType*
FunctionPtrType::getMulticastType() {
	return m_module->m_typeMgr.getMulticastType(this);
}

//..............................................................................

inline
PropertyPtrType*
PropertyType::getPropertyPtrType(
	TypeKind typeKind,
	PropertyPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return m_module->m_typeMgr.getPropertyPtrType(this, typeKind, ptrTypeKind, flags);
}

inline
PropertyType*
PropertyType::getMemberPropertyType(ClassType* classType) {
	return m_module->m_typeMgr.getMemberPropertyType(classType, this);
}

inline
PropertyType*
PropertyType::getStdObjectMemberPropertyType() {
	return m_module->m_typeMgr.getStdObjectMemberPropertyType(this);
}

inline
PropertyType*
PropertyType::getShortType() {
	return m_module->m_typeMgr.getShortPropertyType(this);
}

inline
StructType*
PropertyType::getVtableStructType() {
	return m_module->m_typeMgr.getPropertyVtableStructType(this);
}

//..............................................................................

inline
FunctionType*
DerivableType::getMemberMethodType(
	FunctionType* shortType,
	uint_t thisArgTypeFlags
) {
	return m_module->m_typeMgr.getMemberMethodType(this, shortType, thisArgTypeFlags);
}

inline
PropertyType*
DerivableType::getMemberPropertyType(PropertyType* shortType) {
	return m_module->m_typeMgr.getMemberPropertyType(this, shortType);
}

//..............................................................................

inline
ClassPtrType*
ClassType::getClassPtrType(
	TypeKind typeKind,
	ClassPtrTypeKind ptrTypeKind,
	uint_t flags
) {
	return m_module->m_typeMgr.getClassPtrType(this, typeKind, ptrTypeKind, flags);
}

//..............................................................................

inline
void
Typedef::prepareShadowType() {
	ASSERT(!m_shadowType);
	m_shadowType = m_module->m_typeMgr.createTypedefShadowType(this);
}

//..............................................................................

inline
Variable*
Variable::getDeclVariable() {
	if (!m_declVariable)
		m_declVariable = m_module->m_variableMgr.createRtlItemVariable(
			StdType_Variable,
			"jnc.g_decl." + getLinkId(),
			this
		);

	return m_declVariable;
}

//..............................................................................

inline
Variable*
Function::getDeclVariable() {
	if (!m_declVariable)
		m_declVariable = m_module->m_variableMgr.createRtlItemVariable(
			StdType_Function,
			"jnc.g_decl." + getLinkId(),
			this
		);

	return m_declVariable;
}

//..............................................................................

inline
Variable*
Property::getDeclVariable() {
	if (!m_declVariable)
		m_declVariable = m_module->m_variableMgr.createRtlItemVariable(
			StdType_Property,
			"jnc.g_decl." + getLinkId(),
			this
		);

	return m_declVariable;
}

//..............................................................................

inline
Variable*
EnumConst::getDeclVariable() {
	if (!m_declVariable)
		m_declVariable = m_module->m_variableMgr.createRtlItemVariable(
			StdType_EnumConst,
			"jnc.g_decl." + getLinkId(),
			this
		);

	return m_declVariable;
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
Jit::clearLlvmModule() {
	m_module->m_llvmModule = NULL;
}

inline
void
Jit::clearLlvmContext() {
	m_module->m_llvmContext = NULL;
}

inline
llvm::Function*
Jit::getLlvmFunction(Function* function) {
	return
		!function->hasLlvmFunction() ? NULL : // never used
		!function->getLlvmFunctionName().isEmpty() ?
			m_module->getLlvmModule()->getFunction(function->getLlvmFunctionName() >> toLlvm) :
			function->getLlvmFunction();
}

inline
llvm::GlobalVariable*
Jit::getLlvmGlobalVariable(Variable* variable) {
	return !variable->getLlvmGlobalVariableName().isEmpty() ?
		m_module->getLlvmModule()->getGlobalVariable(variable->getLlvmGlobalVariableName() >> toLlvm) :
		variable->getLlvmGlobalVariable();
}

//..............................................................................

inline
void
CodeAssistMgr::prepareQualifiedNameFallback(
	const QualifiedName& namePrefix,
	const Token& token
) {
	m_fallbackMode = FallbackMode_QualifiedName;
	m_fallbackContext.captureContext(m_module);
	m_fallbackNamePrefix.copy(namePrefix);
	m_fallbackToken = token;
}

inline
void
CodeAssistMgr::prepareExpressionFallback(const sl::List<Token>& expression) {
	ASSERT(!expression.isEmpty());
	m_fallbackMode = FallbackMode_Expression;
	m_fallbackContext.captureContext(m_module);
	cloneTokenList(&m_fallbackExpression, expression);
}

inline
void
CodeAssistMgr::prepareIdentifierFallback(const Token& token) {
	if (m_fallbackMode <= FallbackMode_Identifier) { // only if no better fallbacks
		m_fallbackMode = FallbackMode_Identifier;
		m_fallbackContext.captureContext(m_module);
		m_fallbackToken = token;
	}
}

inline
void
CodeAssistMgr::prepareNamespaceFallback() {
	if (m_fallbackMode <= FallbackMode_Namespace) { // only if no better fallbacks
		m_fallbackMode = FallbackMode_Namespace;
		m_fallbackContext.captureContext(m_module);
	}
}

//..............................................................................

inline
bool
OperatorMgr::checkAccess(
	ModuleItemDecl* decl,
	Namespace* viaNamespace
) {
	ASSERT(viaNamespace);

	if (!m_module->hasAccessChecks() || decl->getAccessKind() == AccessKind_Public)
		return true;

	Namespace* parentNamespace = decl->getParentNamespace();
	if (m_module->m_namespaceMgr.getAccessKind(parentNamespace) == AccessKind_Protected)
		return true;

	if (viaNamespace != parentNamespace &&
		m_module->m_namespaceMgr.getAccessKind(viaNamespace) == AccessKind_Protected
	)
		return true;

	err::setFormatStringError("'%s' is protected", decl->getDeclItem()->getItemName().sz());
	return false;
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
		m_reactorBody->m_llvmReactionInst = &m_currentBlock->getLlvmBlock()->back();
}

//..............................................................................

inline
const Value&
ConstMgr::saveValue(const Value& value) {
	ASSERT(m_module->getCompileState() < ModuleCompileState_Compiled);
	sl::BoxIterator<Value> it = m_valueList.insertTail(value);
	return *it;
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
Value::setNamespace(
	Namespace* nspace,
	Module* module
) {
	clear();
	m_valueKind = ValueKind_Namespace;
	m_namespace = nspace;
	m_type = module->m_typeMgr.getPrimitiveType(TypeKind_Void);
}

inline
void
Value::setTemplate(Template* templ) {
	clear();
	m_valueKind = ValueKind_Template;
	m_template = templ;
	m_type = templ->getModule()->m_typeMgr.getPrimitiveType(TypeKind_Void);
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
