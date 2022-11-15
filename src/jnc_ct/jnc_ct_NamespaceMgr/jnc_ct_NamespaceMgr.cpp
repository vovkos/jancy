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

#include "pch.h"
#include "jnc_ct_NamespaceMgr.h"
#include "jnc_ct_ExtensionNamespace.h"
#include "jnc_ct_DynamicLibNamespace.h"
#include "jnc_ct_Module.h"

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// jancy sources

;static char g_jnc_gcSrc[] =
#include "jnc_gc.jnc.cpp"
;static char g_jnc_dataPtrSrc[] =
#include "jnc_DataPtr.jnc.cpp"
;static char g_jnc_dynamicLibSrc[] =
#include "jnc_DynamicLib.jnc.cpp"
;static char g_jnc_promiseSrc[] =
#include "jnc_Promise.jnc.cpp"
;static char g_jnc_regexSrc[] =
#include "jnc_Regex.jnc.cpp"
;static char g_jnc_schedulerSrc[] =
#include "jnc_Scheduler.jnc.cpp"
;static char g_jnc_introSrc[] =
#include "jnc_intro.jnc.cpp"
;

#include "jnc_StdFunctions.jnc.cpp"
#include "jnc_StdTypes.jnc.cpp"

namespace jnc {
namespace ct {

//..............................................................................

NamespaceMgr::NamespaceMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	sl::String jncName("jnc");
	sl::String stdName("std");

	GlobalNamespace* global = &m_stdNamespaceArray[StdNamespace_Global];
	GlobalNamespace* jnc = &m_stdNamespaceArray[StdNamespace_Jnc];
	GlobalNamespace* std = &m_stdNamespaceArray[StdNamespace_Std];
	GlobalNamespace* internal = &m_stdNamespaceArray[StdNamespace_Internal];

	global->m_module = m_module;
	global->m_namespaceStatus = NamespaceStatus_Ready;

	jnc->m_module = m_module;
	jnc->m_namespaceStatus = NamespaceStatus_Ready;
	jnc->m_parentNamespace = global;
	jnc->m_name = jncName;
	jnc->m_qualifiedName = jncName;

	std->m_module = m_module;
	std->m_parentNamespace = global;
	std->m_name = stdName;
	std->m_qualifiedName = stdName;

	internal->m_module = m_module;
	internal->m_namespaceStatus = NamespaceStatus_Ready;
	internal->m_parentNamespace = global;
	internal->m_name = jncName;
	internal->m_qualifiedName = jncName;

	m_currentNamespace = global;
	m_currentScope = NULL;
	m_currentAccessKind = AccessKind_Public;
	m_sourcePosLockCount = 0;
}

void
NamespaceMgr::clear() {
	for (size_t i = 0; i < StdNamespace__Count; i++)
		m_stdNamespaceArray[i].clear();

	m_stdNamespaceArray[StdNamespace_Std].m_namespaceStatus = NamespaceStatus_ParseRequired; // `std` must be reparsed
	m_globalNamespaceList.clear();
	m_scopeList.clear();
	m_orphanList.clear();
	m_aliasList.clear();
	m_namespaceStack.clear();
	m_sourcePos.clear();
	m_currentNamespace = &m_stdNamespaceArray[StdNamespace_Global];
	m_currentScope = NULL;
	m_sourcePosLockCount = 0;
	m_staticObjectValue.clear();
}

void
NamespaceMgr::addStdItems() {
	GlobalNamespace* globalNspace = &m_stdNamespaceArray[StdNamespace_Global];
	GlobalNamespace* jncNspace = &m_stdNamespaceArray[StdNamespace_Jnc];
	GlobalNamespace* stdNspace = &m_stdNamespaceArray[StdNamespace_Std];

	ExtensionLib* coreLib = jnc_CoreLib_getLib();
	ExtensionLib* introLib = jnc_IntrospectionLib_getLib();

	LazyImport* gcImport = m_module->m_importMgr.createLazyImport(coreLib, "jnc_gc.jnc", g_jnc_gcSrc);
	LazyImport* dataPtrImport = m_module->m_importMgr.createLazyImport(coreLib, "jnc_DataPtr.jnc", g_jnc_dataPtrSrc);
	LazyImport* dynamicLibImport = m_module->m_importMgr.createLazyImport(coreLib, "jnc_DynamicLib.jnc", g_jnc_dynamicLibSrc);
	LazyImport* promiseImport = m_module->m_importMgr.createLazyImport(coreLib, "jnc_Promise.jnc", g_jnc_promiseSrc);
	LazyImport* schedulerImport = m_module->m_importMgr.createLazyImport(coreLib, "jnc_Scheduler.jnc", g_jnc_schedulerSrc);
	LazyImport* regexImport = m_module->m_importMgr.createLazyImport(coreLib, "jnc_Regex.jnc", g_jnc_regexSrc);
	LazyImport* introImport = m_module->m_importMgr.createLazyImport(coreLib, "jnc_intro.jnc", g_jnc_introSrc);

	bool result =
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uint_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_intptr_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uintptr_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_size_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_int8_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_utf8_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uint8_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uchar_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_byte_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_int16_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_utf16_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uint16_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_ushort_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_word_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_int32_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_utf32_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uint32_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_dword_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_int64_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uint64_t)) &&
		globalNspace->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_qword_t)) &&
		globalNspace->addItem(jncNspace) &&
		globalNspace->addItem(stdNspace) &&
		jncNspace->addItem("GcStats", gcImport) &&
		jncNspace->addItem("GcTriggers", gcImport) &&
		jncNspace->addItem("getGcStats", gcImport) &&
		jncNspace->addItem("g_gcTriggers", gcImport) &&
		jncNspace->addItem("collectGarbage", gcImport) &&
		jncNspace->addItem("createDataPtr", dataPtrImport) &&
		jncNspace->addItem("resetDynamicLayout", dataPtrImport) &&
		jncNspace->addItem("Scheduler", schedulerImport) &&
		jncNspace->addItem("RegexMatch", regexImport) &&
		jncNspace->addItem("RegexComplileFlags", regexImport) &&
		jncNspace->addItem("RegexExecFlags", regexImport) &&
		jncNspace->addItem("RegexExecResult", regexImport) &&
		jncNspace->addItem("RegexState", regexImport) &&
		jncNspace->addItem("RegexKind", regexImport) &&
		jncNspace->addItem("RegexEof", regexImport) &&
		jncNspace->addItem("Regex", regexImport) &&
		jncNspace->addItem("Promise", promiseImport) &&
		jncNspace->addItem("Promisifier", promiseImport) &&
		jncNspace->addItem("DynamicLib", dynamicLibImport) &&
		jncNspace->addItem("ModuleItemKind", introImport) &&
		jncNspace->addItem("ModuleItemFlags", introImport) &&
		jncNspace->addItem("StorageKind", introImport) &&
		jncNspace->addItem("AccessKind", introImport) &&
		jncNspace->addItem("ModuleItem", introImport) &&
		jncNspace->addItem("ModuleItemDecl", introImport) &&
		jncNspace->addItem("ModuleItemInitializer", introImport) &&
		jncNspace->addItem("Attribute", introImport) &&
		jncNspace->addItem("AttributeBlock", introImport) &&
		jncNspace->addItem("NamespaceKind", introImport) &&
		jncNspace->addItem("Namespace", introImport) &&
		jncNspace->addItem("GlobalNamespace", introImport) &&
		jncNspace->addItem("UnOpKind", introImport) &&
		jncNspace->addItem("BinOpKind", introImport) &&
		jncNspace->addItem("TypeKind", introImport) &&
		jncNspace->addItem("TypeKindFlags", introImport) &&
		jncNspace->addItem("TypeFlags", introImport) &&
		jncNspace->addItem("PtrTypeFlags", introImport) &&
		jncNspace->addItem("DataPtrTypeKind", introImport) &&
		jncNspace->addItem("Type", introImport) &&
		jncNspace->addItem("DataPtrType", introImport) &&
		jncNspace->addItem("NamedType", introImport) &&
		jncNspace->addItem("MemberBlock", introImport) &&
		jncNspace->addItem("BaseTypeSlot", introImport) &&
		jncNspace->addItem("DerivableType", introImport) &&
		jncNspace->addItem("ArrayType", introImport) &&
		jncNspace->addItem("BitFieldType", introImport) &&
		jncNspace->addItem("FunctionTypeFlags", introImport) &&
		jncNspace->addItem("FunctionPtrTypeKind", introImport) &&
		jncNspace->addItem("FunctionArg", introImport) &&
		jncNspace->addItem("FunctionType", introImport) &&
		jncNspace->addItem("FunctionPtrType", introImport) &&
		jncNspace->addItem("PropertyTypeFlags", introImport) &&
		jncNspace->addItem("PropertyPtrTypeKind", introImport) &&
		jncNspace->addItem("PropertyType", introImport) &&
		jncNspace->addItem("PropertyPtrType", introImport) &&
		jncNspace->addItem("EnumTypeFlags", introImport) &&
		jncNspace->addItem("EnumConst", introImport) &&
		jncNspace->addItem("EnumType", introImport) &&
		jncNspace->addItem("ClassTypeKind", introImport) &&
		jncNspace->addItem("ClassTypeFlags", introImport) &&
		jncNspace->addItem("ClassPtrTypeKind", introImport) &&
		jncNspace->addItem("ClassType", introImport) &&
		jncNspace->addItem("ClassPtrType", introImport) &&
		jncNspace->addItem("StructTypeKind", introImport) &&
		jncNspace->addItem("Field", introImport) &&
		jncNspace->addItem("StructType", introImport) &&
		jncNspace->addItem("UnionType", introImport) &&
		jncNspace->addItem("Alias", introImport) &&
		jncNspace->addItem("Const", introImport) &&
		jncNspace->addItem("Variable", introImport) &&
		jncNspace->addItem("FunctionKind", introImport) &&
		jncNspace->addItem("FunctionKindFlags", introImport) &&
		jncNspace->addItem("Function", introImport) &&
		jncNspace->addItem("FunctionOverload", introImport) &&
		jncNspace->addItem("PropertyKind", introImport) &&
		jncNspace->addItem("PropertyFlag", introImport) &&
		jncNspace->addItem("Property", introImport) &&
		jncNspace->addItem("Typedef", introImport) &&
		jncNspace->addItem("ModuleCompileFlags", introImport) &&
		jncNspace->addItem("ModuleCompileState", introImport) &&
		jncNspace->addItem("Module", introImport) &&
		jncNspace->addItem("Unit", introImport);

	ASSERT(result);
}

Orphan*
NamespaceMgr::createOrphan(
	OrphanKind orphanKind,
	FunctionType* functionType
) {
	Orphan* orphan = AXL_MEM_NEW(Orphan);
	orphan->m_module = m_module;
	orphan->m_orphanKind = orphanKind;
	orphan->m_functionType = functionType;
	m_orphanList.insertTail(orphan);
	return orphan;
}

Alias*
NamespaceMgr::createAlias(
	const sl::StringRef& name,
	const sl::StringRef& qualifiedName,
	sl::BoxList<Token>* initializer
) {
	Alias* alias = AXL_MEM_NEW(Alias);
	alias->m_module = m_module;
	alias->m_name = name;
	alias->m_qualifiedName = qualifiedName;
	sl::takeOver(&alias->m_initializer, initializer);
	m_aliasList.insertTail(alias);
	return alias;
}

void
NamespaceMgr::setSourcePos(const lex::LineCol& pos) {
	if (!(m_module->getCompileFlags() & ModuleCompileFlag_DebugInfo) ||
		!m_currentScope ||
		m_sourcePosLockCount)
		return;

	llvm::DebugLoc llvmDebugLoc = m_module->m_llvmDiBuilder.getDebugLoc(m_currentScope, pos);
	m_module->m_llvmIrBuilder.setCurrentDebugLoc(llvmDebugLoc);
}

void
NamespaceMgr::openNamespace(Namespace* nspace) {
	NamespaceStackEntry entry = {
		m_currentNamespace,
		m_currentScope,
		m_currentAccessKind
	};

	m_namespaceStack.append(entry);
	m_currentNamespace = nspace;
	m_currentScope = nspace->m_namespaceKind == NamespaceKind_Scope ? (Scope*)nspace : NULL;
	m_currentAccessKind = AccessKind_Public; // always start with 'public'
}

void
NamespaceMgr::closeNamespace() {
	if (m_namespaceStack.isEmpty())
		return;

	NamespaceStackEntry entry = m_namespaceStack.getBackAndPop();

	if (m_currentNamespace->m_namespaceKind == NamespaceKind_Global) // for others not really needed
		m_currentNamespace->m_usingSet.clear();

	m_currentNamespace = entry.m_namespace;
	m_currentScope = entry.m_scope;
	m_currentAccessKind = entry.m_accessKind;
}

void
NamespaceMgr::closeAllNamespaces() {
	m_namespaceStack.clear();
	m_currentNamespace = &m_stdNamespaceArray[StdNamespace_Global];
	m_currentScope = NULL;
	m_currentAccessKind = AccessKind_Public;
	m_sourcePosLockCount = 0;
}

Scope*
NamespaceMgr::openInternalScope() {
	Function* function = m_module->m_functionMgr.getCurrentFunction();
	ASSERT(function);

	Scope* scope = AXL_MEM_NEW(Scope);
	scope->m_module = m_module;
	scope->m_function = function;
	scope->m_parentNamespace = m_currentNamespace;

	if (m_currentScope) {
		// propagate parent scope traits
		scope->m_flags |= m_currentScope->m_flags & (ScopeFlag_Finalizable | ScopeFlag_HasCatch);
		scope->m_sjljFrameIdx = m_currentScope->m_sjljFrameIdx;
	} else {
		scope->m_flags = ScopeFlag_Function;
	}

	// if this scope creates any gc roots, frame map should be set before any of those

	if (m_module->hasCodeGen())
		m_module->m_llvmIrBuilder.saveInsertPoint(&scope->m_gcShadowStackFrameMapInsertPoint);

	m_scopeList.insertTail(scope);

	openNamespace(scope);
	return scope;
}

Scope*
NamespaceMgr::openScope(
	const lex::LineCol& pos,
	uint_t flags
) {
	Scope* parentScope = m_currentScope;
	Scope* scope = openInternalScope();
	scope->m_pos = pos;
	scope->m_flags |= flags;

	bool isFunctionScope = scope->m_parentNamespace == scope->m_function->getScope();
	if (isFunctionScope)
		scope->m_flags |= ScopeFlag_Function;

	if (m_module->getCompileFlags() & ModuleCompileFlag_DebugInfo) {
		scope->m_llvmDiScope = isFunctionScope ?
			(llvm::DIScope_vn)scope->m_function->getLlvmDiSubprogram() :
			(llvm::DIScope_vn)m_module->m_llvmDiBuilder.createLexicalBlock(parentScope, pos);
	}

	setSourcePos(pos);

	if (flags & ScopeFlag_Disposable) {
		scope->m_finallyBlock = m_module->m_controlFlowMgr.createBlock("dispose_block");
		scope->m_sjljFrameIdx++;
		scope->m_flags |= parentScope->m_flags & ScopeFlag_Function; // propagate function flag
		m_module->m_controlFlowMgr.setJmpFinally(scope->m_finallyBlock, scope->m_sjljFrameIdx);

		Type* type = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		scope->m_disposeLevelVariable = m_module->m_variableMgr.createSimpleStackVariable("dispose_level", type);

		if (m_module->hasCodeGen())
			m_module->m_llvmIrBuilder.createStore(type->getZeroValue(), scope->m_disposeLevelVariable);
	} else if (flags & (ScopeFlag_Try | ScopeFlag_CatchAhead)) {
		scope->m_catchBlock = m_module->m_controlFlowMgr.createBlock("catch_block");
		scope->m_sjljFrameIdx++;
		m_module->m_controlFlowMgr.setJmp(scope->m_catchBlock, scope->m_sjljFrameIdx);

		if (flags & ScopeFlag_FinallyAhead)
			scope->m_finallyBlock = m_module->m_controlFlowMgr.createBlock("catch_finally_block");
	} else if (flags & ScopeFlag_FinallyAhead) {
		scope->m_finallyBlock = m_module->m_controlFlowMgr.createBlock("finally_block");
		scope->m_sjljFrameIdx++;
		m_module->m_controlFlowMgr.setJmpFinally(scope->m_finallyBlock, scope->m_sjljFrameIdx);
	}

	if (flags & ScopeFlag_Nested) {
		if (parentScope->m_flags & (ScopeFlag_Catch | ScopeFlag_Finally | ScopeFlag_Nested)) {
			err::setFormatStringError("'nestedscope' can only be used before other scope labels");
			return NULL;
		}

		scope->m_flags |= parentScope->m_flags & ScopeFlag_Function; // propagate function flag
	}

	return scope;
}

void
NamespaceMgr::closeScope() {
	ASSERT(m_currentScope);
	uint_t flags = m_currentScope->m_flags;

	if (m_module->hasCodeGen())
		if (flags & ScopeFlag_Disposable) {
			m_currentScope->m_flags &= ~ScopeFlag_Disposable; // prevent recursion
			m_module->m_controlFlowMgr.finalizeDisposableScope(m_currentScope);
		} else if ((flags & ScopeFlag_Try) && !(flags & (ScopeFlag_CatchAhead | ScopeFlag_FinallyAhead))) {
			m_currentScope->m_flags &= ~ScopeFlag_Try; // prevent recursion
			m_module->m_controlFlowMgr.finalizeTryScope(m_currentScope);
		} else {
			// the above two cases introduce implicit finally/catch labels
			// as such, they will finalize this scope and open a new one

			m_module->m_gcShadowStackMgr.finalizeScope(m_currentScope);

			if ((flags & ScopeFlag_Catch) && !(flags & ScopeFlag_FinallyAhead)) {
				m_currentScope->m_flags &= ~ScopeFlag_Catch; // prevent recursion
				m_module->m_controlFlowMgr.finalizeCatchScope(m_currentScope);
			} else if (flags & ScopeFlag_Finally) {
				m_currentScope->m_flags &= ~ScopeFlag_Finally; // prevent recursion
				m_module->m_controlFlowMgr.finalizeFinallyScope(m_currentScope);
			}
		}

	closeNamespace();

	if ((flags & (ScopeFlag_Nested | ScopeFlag_Disposable)) && !(flags & (ScopeFlag_CatchAhead | ScopeFlag_FinallyAhead)))
		closeScope();
}

AccessKind
NamespaceMgr::getAccessKind(Namespace* targetNamespace) {
	Namespace* nspace = m_currentNamespace;

	if (!targetNamespace->isNamed()) {
		for (; nspace; nspace = nspace->m_parentNamespace) {
			if (nspace == targetNamespace)
				return AccessKind_Protected;
		}

		return AccessKind_Public;
	}

	if (targetNamespace->m_namespaceKind != NamespaceKind_Type) {
		for (; nspace; nspace = nspace->m_parentNamespace) {
			if (!nspace->isNamed())
				continue;

			if (nspace == targetNamespace ||
				targetNamespace->getQualifiedName().cmp(nspace->getQualifiedName()) == 0 ||
				targetNamespace->m_friendSet.find(nspace->getQualifiedName()))
				return AccessKind_Protected;
		}

		return AccessKind_Public;
	}

	NamedType* targetType = (NamedType*)targetNamespace;

	for (; nspace; nspace = nspace->m_parentNamespace) {
		if (!nspace->isNamed())
			continue;

		if (nspace == targetNamespace ||
			targetNamespace->getQualifiedName().cmp(nspace->getQualifiedName()) == 0 ||
			targetNamespace->m_friendSet.find(nspace->getQualifiedName()))
			return AccessKind_Protected;

		if (nspace->m_namespaceKind == NamespaceKind_Type) {
			NamedType* type = (NamedType*)nspace;
			TypeKind typeKind = type->getTypeKind();
			if (typeKind == TypeKind_Class || typeKind == TypeKind_Struct) {
				bool result = ((DerivableType*)type)->findBaseTypeTraverse(targetType);
				if (result)
					return AccessKind_Protected;
			}
		}
	}

	return AccessKind_Public;
}

void
NamespaceMgr::addGlobalNamespace(
	GlobalNamespace* nspace,
	const sl::StringRef& name,
	Namespace* parentNamespace
) {
	if (!parentNamespace)
		parentNamespace = &m_stdNamespaceArray[StdNamespace_Global];

	nspace->m_module = m_module;
	nspace->m_name = name;
	nspace->m_parentNamespace = parentNamespace;
	m_globalNamespaceList.insertTail(nspace);
}

Scope*
NamespaceMgr::findBreakScope(size_t level) {
	size_t i = 0;
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope()) {
		if (scope->m_breakBlock) {
			i++;
			if (i >= level)
				break;
		}
	}

	return scope;
}

Scope*
NamespaceMgr::findContinueScope(size_t level) {
	size_t i = 0;
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope()) {
		if (scope->m_continueBlock) {
			i++;
			if (i >= level)
				break;
		}
	}

	return scope;
}

Scope*
NamespaceMgr::findCatchScope() {
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope()) {
		if (scope->m_tryExpr || scope->m_catchBlock)
			break;
	}

	return scope;
}

Scope*
NamespaceMgr::findRegexScope() {
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope()) {
		if (scope->m_regexStateValue)
			break;
	}

	return scope;
}

//..............................................................................

} // namespace ct
} // namespace jnc
