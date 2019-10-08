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

namespace jnc {
namespace ct {

//..............................................................................

NamespaceMgr::NamespaceMgr()
{
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	sl::String jncName("jnc");

	GlobalNamespace* global = &m_stdNamespaceArray[StdNamespace_Global];
	GlobalNamespace* jnc = &m_stdNamespaceArray[StdNamespace_Jnc];
	GlobalNamespace* internal = &m_stdNamespaceArray[StdNamespace_Internal];

	global->m_module = m_module;
	global->m_namespaceStatus = NamespaceStatus_Ready;

	jnc->m_module = m_module;
	jnc->m_namespaceStatus = NamespaceStatus_Ready;
	jnc->m_parentNamespace = global;
	jnc->m_name = jncName;
	jnc->m_qualifiedName = jncName;

	if (!(m_module->getCompileFlags() & ModuleCompileFlag_StdLibDoc))
		jnc->m_flags |= ModuleItemFlag_Sealed;

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
NamespaceMgr::clear()
{
	for (size_t i = 0; i < StdNamespace__Count; i++)
		m_stdNamespaceArray[i].clear();

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
NamespaceMgr::addStdItems()
{
	GlobalNamespace* global = &m_stdNamespaceArray[StdNamespace_Global];
	GlobalNamespace* jnc = &m_stdNamespaceArray[StdNamespace_Jnc];

	bool result =
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uint_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_intptr_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uintptr_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_size_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_int8_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_utf8_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uint8_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uchar_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_byte_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_int16_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_utf16_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uint16_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_ushort_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_word_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_int32_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_utf32_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uint32_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_dword_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_int64_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_uint64_t)) &&
		global->addItem(m_module->m_typeMgr.getStdTypedef(StdTypedef_qword_t)) &&
		global->addItem(jnc) &&
		jnc->addItem("g_gcTriggers", m_module->m_functionMgr.getLazyStdProperty(StdProp_GcTriggers)) &&
		jnc->addItem("getGcStats", m_module->m_functionMgr.getLazyStdFunction(StdFunc_GetGcStats)) &&
		jnc->addItem("collectGarbage", m_module->m_functionMgr.getLazyStdFunction(StdFunc_CollectGarbage)) &&
		jnc->addItem("createDataPtr", m_module->m_functionMgr.getLazyStdFunction(StdFunc_CreateDataPtr)) &&
		jnc->addItem("GcTriggers", m_module->m_typeMgr.getLazyStdType(StdType_GcTriggers)) &&
		jnc->addItem("GcStats", m_module->m_typeMgr.getLazyStdType(StdType_GcStats)) &&
		jnc->addItem("Scheduler", m_module->m_typeMgr.getLazyStdType(StdType_Scheduler)) &&
		jnc->addItem("RegexMatch", m_module->m_typeMgr.getLazyStdType(StdType_RegexMatch)) &&
		jnc->addItem("RegexState", m_module->m_typeMgr.getLazyStdType(StdType_RegexState)) &&
		jnc->addItem("RegexDfa", m_module->m_typeMgr.getLazyStdType(StdType_RegexDfa)) &&
		jnc->addItem("Promise", m_module->m_typeMgr.getLazyStdType(StdType_Promise)) &&
		jnc->addItem("Promisifier", m_module->m_typeMgr.getLazyStdType(StdType_Promisifier)) &&
		jnc->addItem("DynamicLib", m_module->m_typeMgr.getLazyStdType(StdType_DynamicLib)) &&
		jnc->addItem("ModuleItemKind", m_module->m_typeMgr.getLazyStdType(StdType_ModuleItemKind)) &&
		jnc->addItem("ModuleItemFlags", m_module->m_typeMgr.getLazyStdType(StdType_ModuleItemFlags)) &&
		jnc->addItem("StorageKind", m_module->m_typeMgr.getLazyStdType(StdType_StorageKind)) &&
		jnc->addItem("AccessKind", m_module->m_typeMgr.getLazyStdType(StdType_AccessKind)) &&
		jnc->addItem("ModuleItem", m_module->m_typeMgr.getLazyStdType(StdType_ModuleItem)) &&
		jnc->addItem("ModuleItemDecl", m_module->m_typeMgr.getLazyStdType(StdType_ModuleItemDecl)) &&
		jnc->addItem("ModuleItemInitializer", m_module->m_typeMgr.getLazyStdType(StdType_ModuleItemInitializer)) &&
		jnc->addItem("Attribute", m_module->m_typeMgr.getLazyStdType(StdType_Attribute)) &&
		jnc->addItem("AttributeBlock", m_module->m_typeMgr.getLazyStdType(StdType_AttributeBlock)) &&
		jnc->addItem("NamespaceKind", m_module->m_typeMgr.getLazyStdType(StdType_NamespaceKind)) &&
		jnc->addItem("Namespace", m_module->m_typeMgr.getLazyStdType(StdType_Namespace)) &&
		jnc->addItem("GlobalNamespace", m_module->m_typeMgr.getLazyStdType(StdType_GlobalNamespace)) &&
		jnc->addItem("UnOpKind", m_module->m_typeMgr.getLazyStdType(StdType_UnOpKind)) &&
		jnc->addItem("BinOpKind", m_module->m_typeMgr.getLazyStdType(StdType_BinOpKind)) &&
		jnc->addItem("TypeKind", m_module->m_typeMgr.getLazyStdType(StdType_TypeKind)) &&
		jnc->addItem("TypeKindFlags", m_module->m_typeMgr.getLazyStdType(StdType_TypeKindFlags)) &&
		jnc->addItem("TypeFlags", m_module->m_typeMgr.getLazyStdType(StdType_TypeFlags)) &&
		jnc->addItem("PtrTypeFlags", m_module->m_typeMgr.getLazyStdType(StdType_PtrTypeFlags)) &&
		jnc->addItem("DataPtrTypeKind", m_module->m_typeMgr.getLazyStdType(StdType_DataPtrTypeKind)) &&
		jnc->addItem("Type", m_module->m_typeMgr.getLazyStdType(StdType_Type)) &&
		jnc->addItem("DataPtrType", m_module->m_typeMgr.getLazyStdType(StdType_DataPtrType)) &&
		jnc->addItem("NamedType", m_module->m_typeMgr.getLazyStdType(StdType_NamedType)) &&
		jnc->addItem("MemberBlock", m_module->m_typeMgr.getLazyStdType(StdType_MemberBlock)) &&
		jnc->addItem("BaseTypeSlot", m_module->m_typeMgr.getLazyStdType(StdType_BaseTypeSlot)) &&
		jnc->addItem("DerivableType", m_module->m_typeMgr.getLazyStdType(StdType_DerivableType)) &&
		jnc->addItem("ArrayType", m_module->m_typeMgr.getLazyStdType(StdType_ArrayType)) &&
		jnc->addItem("BitFieldType", m_module->m_typeMgr.getLazyStdType(StdType_BitFieldType)) &&
		jnc->addItem("FunctionTypeFlags", m_module->m_typeMgr.getLazyStdType(StdType_FunctionTypeFlags)) &&
		jnc->addItem("FunctionPtrTypeKind", m_module->m_typeMgr.getLazyStdType(StdType_FunctionPtrTypeKind)) &&
		jnc->addItem("FunctionArg", m_module->m_typeMgr.getLazyStdType(StdType_FunctionArg)) &&
		jnc->addItem("FunctionType", m_module->m_typeMgr.getLazyStdType(StdType_FunctionType)) &&
		jnc->addItem("FunctionPtrType", m_module->m_typeMgr.getLazyStdType(StdType_FunctionPtrType)) &&
		jnc->addItem("PropertyTypeFlags", m_module->m_typeMgr.getLazyStdType(StdType_PropertyTypeFlags)) &&
		jnc->addItem("PropertyPtrTypeKind", m_module->m_typeMgr.getLazyStdType(StdType_PropertyPtrTypeKind)) &&
		jnc->addItem("PropertyType", m_module->m_typeMgr.getLazyStdType(StdType_PropertyType)) &&
		jnc->addItem("PropertyPtrType", m_module->m_typeMgr.getLazyStdType(StdType_PropertyPtrType)) &&
		jnc->addItem("EnumTypeFlags", m_module->m_typeMgr.getLazyStdType(StdType_EnumTypeFlags)) &&
		jnc->addItem("EnumConst", m_module->m_typeMgr.getLazyStdType(StdType_EnumConst)) &&
		jnc->addItem("EnumType", m_module->m_typeMgr.getLazyStdType(StdType_EnumType)) &&
		jnc->addItem("ClassTypeKind", m_module->m_typeMgr.getLazyStdType(StdType_ClassTypeKind)) &&
		jnc->addItem("ClassTypeFlags", m_module->m_typeMgr.getLazyStdType(StdType_ClassTypeFlags)) &&
		jnc->addItem("ClassPtrTypeKind", m_module->m_typeMgr.getLazyStdType(StdType_ClassPtrTypeKind)) &&
		jnc->addItem("ClassType", m_module->m_typeMgr.getLazyStdType(StdType_ClassType)) &&
		jnc->addItem("ClassPtrType", m_module->m_typeMgr.getLazyStdType(StdType_ClassPtrType)) &&
		jnc->addItem("StructTypeKind", m_module->m_typeMgr.getLazyStdType(StdType_StructTypeKind)) &&
		jnc->addItem("Field", m_module->m_typeMgr.getLazyStdType(StdType_Field)) &&
		jnc->addItem("StructType", m_module->m_typeMgr.getLazyStdType(StdType_StructType)) &&
		jnc->addItem("UnionType", m_module->m_typeMgr.getLazyStdType(StdType_UnionType)) &&
		jnc->addItem("Alias", m_module->m_typeMgr.getLazyStdType(StdType_Alias)) &&
		jnc->addItem("Const", m_module->m_typeMgr.getLazyStdType(StdType_Const)) &&
		jnc->addItem("Variable", m_module->m_typeMgr.getLazyStdType(StdType_Variable)) &&
		jnc->addItem("FunctionKind", m_module->m_typeMgr.getLazyStdType(StdType_FunctionKind)) &&
		jnc->addItem("FunctionKindFlags", m_module->m_typeMgr.getLazyStdType(StdType_FunctionKindFlags)) &&
		jnc->addItem("Function", m_module->m_typeMgr.getLazyStdType(StdType_Function)) &&
		jnc->addItem("FunctionOverload", m_module->m_typeMgr.getLazyStdType(StdType_FunctionOverload)) &&
		jnc->addItem("PropertyKind", m_module->m_typeMgr.getLazyStdType(StdType_PropertyKind)) &&
		jnc->addItem("PropertyFlag", m_module->m_typeMgr.getLazyStdType(StdType_PropertyFlag)) &&
		jnc->addItem("Property", m_module->m_typeMgr.getLazyStdType(StdType_Property)) &&
		jnc->addItem("Typedef", m_module->m_typeMgr.getLazyStdType(StdType_Typedef)) &&
		jnc->addItem("ModuleCompileFlags", m_module->m_typeMgr.getLazyStdType(StdType_ModuleCompileFlags)) &&
		jnc->addItem("ModuleCompileState", m_module->m_typeMgr.getLazyStdType(StdType_ModuleCompileState)) &&
		jnc->addItem("Module", m_module->m_typeMgr.getLazyStdType(StdType_Module)) &&
		jnc->addItem("Unit", m_module->m_typeMgr.getLazyStdType(StdType_Unit));

	ASSERT(result);
}

Orphan*
NamespaceMgr::createOrphan(
	OrphanKind orphanKind,
	FunctionType* functionType
	)
{
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
	)
{
	Alias* alias = AXL_MEM_NEW(Alias);
	alias->m_module = m_module;
	alias->m_name = name;
	alias->m_qualifiedName = qualifiedName;
	sl::takeOver(&alias->m_initializer, initializer);
	m_aliasList.insertTail(alias);
	return alias;
}

void
NamespaceMgr::setSourcePos(const lex::LineCol& pos)
{
	if (!(m_module->getCompileFlags() & ModuleCompileFlag_DebugInfo) ||
		!m_currentScope ||
		m_sourcePosLockCount)
		return;

	llvm::DebugLoc llvmDebugLoc = m_module->m_llvmDiBuilder.getDebugLoc(m_currentScope, pos);
	m_module->m_llvmIrBuilder.setCurrentDebugLoc(llvmDebugLoc);
}

void
NamespaceMgr::openNamespace(Namespace* nspace)
{
	NamespaceStackEntry entry =
	{
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
NamespaceMgr::closeNamespace()
{
	if (m_namespaceStack.isEmpty())
		return;

	NamespaceStackEntry entry = m_namespaceStack.getBackAndPop();

	if (m_currentNamespace->m_namespaceKind == NamespaceKind_Global) // for others not really needed
		m_currentNamespace->m_usingSet.clear();

	m_currentNamespace = entry.m_namespace;
	m_currentScope = entry.m_scope;
	m_currentAccessKind = entry.m_accessKind;
}

Scope*
NamespaceMgr::openInternalScope()
{
	Function* function = m_module->m_functionMgr.getCurrentFunction();
	ASSERT(function);

	Scope* scope = AXL_MEM_NEW(Scope);
	scope->m_module = m_module;
	scope->m_function = function;
	scope->m_parentNamespace = m_currentNamespace;

	if (m_currentScope)
	{
		// propagate parent scope traits
		scope->m_flags |= m_currentScope->m_flags & (ScopeFlag_Finalizable | ScopeFlag_HasCatch);
		scope->m_sjljFrameIdx = m_currentScope->m_sjljFrameIdx;
	}
	else
	{
		scope->m_flags = ScopeFlag_Function;
	}

	// if this scope creates any gc roots, frame map should be set before any of those

	m_module->m_llvmIrBuilder.saveInsertPoint(&scope->m_gcShadowStackFrameMapInsertPoint);

	m_scopeList.insertTail(scope);

	openNamespace(scope);
	return scope;
}

Scope*
NamespaceMgr::openScope(
	const lex::LineCol& pos,
	uint_t flags
	)
{
	Scope* parentScope = m_currentScope;
	Scope* scope = openInternalScope();
	scope->m_pos = pos;
	scope->m_flags |= flags;

	bool isFunctionScope = scope->m_parentNamespace == scope->m_function->getScope();
	if (isFunctionScope)
		scope->m_flags |= ScopeFlag_Function;

	if (m_module->getCompileFlags() & ModuleCompileFlag_DebugInfo)
	{
		scope->m_llvmDiScope = isFunctionScope ?
			(llvm::DIScope_vn)scope->m_function->getLlvmDiSubprogram() :
			(llvm::DIScope_vn)m_module->m_llvmDiBuilder.createLexicalBlock(parentScope, pos);
	}

	setSourcePos(pos);

	if (flags & ScopeFlag_Disposable)
	{
		scope->m_finallyBlock = m_module->m_controlFlowMgr.createBlock("dispose_block");
		scope->m_sjljFrameIdx++;
		m_module->m_controlFlowMgr.setJmpFinally(scope->m_finallyBlock, scope->m_sjljFrameIdx);

		Type* type = m_module->m_typeMgr.getPrimitiveType(TypeKind_Int);
		scope->m_disposeLevelVariable = m_module->m_variableMgr.createSimpleStackVariable("dispose_level", type);
		m_module->m_llvmIrBuilder.createStore(type->getZeroValue(), scope->m_disposeLevelVariable);
	}
	else if (flags & (ScopeFlag_Try | ScopeFlag_CatchAhead))
	{
		scope->m_catchBlock = m_module->m_controlFlowMgr.createBlock("catch_block");
		scope->m_sjljFrameIdx++;
		m_module->m_controlFlowMgr.setJmp(scope->m_catchBlock, scope->m_sjljFrameIdx);

		if (flags & ScopeFlag_FinallyAhead)
			scope->m_finallyBlock = m_module->m_controlFlowMgr.createBlock("catch_finally_block");
	}
	else if (flags & ScopeFlag_FinallyAhead)
	{
		scope->m_finallyBlock = m_module->m_controlFlowMgr.createBlock("finally_block");
		scope->m_sjljFrameIdx++;
		m_module->m_controlFlowMgr.setJmpFinally(scope->m_finallyBlock, scope->m_sjljFrameIdx);
	}

	if (flags & ScopeFlag_Nested)
	{
		if (parentScope->m_flags & (ScopeFlag_Catch | ScopeFlag_Finally | ScopeFlag_Nested))
		{
			err::setFormatStringError("'nestedscope' can only be used before other scope labels");
			return NULL;
		}

		scope->m_flags |= parentScope->m_flags & ScopeFlag_Function; // propagate function flag
	}

	return scope;
}

void
NamespaceMgr::closeScope()
{
	ASSERT(m_currentScope);
	m_module->m_gcShadowStackMgr.finalizeScope(m_currentScope);

	uint_t flags = m_currentScope->m_flags;

	if (flags & ScopeFlag_Disposable)
	{
		m_currentScope->m_flags &= ~ScopeFlag_Disposable; // prevent recursion
		m_module->m_controlFlowMgr.finalizeDisposableScope(m_currentScope);
	}
	else if ((flags & ScopeFlag_Try) && !(flags & (ScopeFlag_CatchAhead | ScopeFlag_FinallyAhead)))
	{
		m_currentScope->m_flags &= ~ScopeFlag_Try; // prevent recursion
		m_module->m_controlFlowMgr.finalizeTryScope(m_currentScope);
	}
	else if ((flags & ScopeFlag_Catch) && !(flags & ScopeFlag_FinallyAhead))
	{
		m_currentScope->m_flags &= ~ScopeFlag_Catch; // prevent recursion
		m_module->m_controlFlowMgr.finalizeCatchScope(m_currentScope);
	}
	else if (flags & ScopeFlag_Finally)
	{
		m_currentScope->m_flags &= ~ScopeFlag_Finally; // prevent recursion
		m_module->m_controlFlowMgr.finalizeFinallyScope(m_currentScope);
	}

	closeNamespace();

	if ((flags & ScopeFlag_Nested) && !(flags & (ScopeFlag_CatchAhead | ScopeFlag_FinallyAhead)))
		closeScope();
}

AccessKind
NamespaceMgr::getAccessKind(Namespace* targetNamespace)
{
	Namespace* nspace = m_currentNamespace;

	if (!targetNamespace->isNamed())
	{
		for (; nspace; nspace = nspace->m_parentNamespace)
		{
			if (nspace == targetNamespace)
				return AccessKind_Protected;
		}

		return AccessKind_Public;
	}

	if (targetNamespace->m_namespaceKind != NamespaceKind_Type)
	{
		for (; nspace; nspace = nspace->m_parentNamespace)
		{
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

	for (; nspace; nspace = nspace->m_parentNamespace)
	{
		if (!nspace->isNamed())
			continue;

		if (nspace == targetNamespace ||
			targetNamespace->getQualifiedName().cmp(nspace->getQualifiedName()) == 0 ||
			targetNamespace->m_friendSet.find(nspace->getQualifiedName()))
			return AccessKind_Protected;

		if (nspace->m_namespaceKind == NamespaceKind_Type)
		{
			NamedType* type = (NamedType*)nspace;
			TypeKind typeKind = type->getTypeKind();
			if (typeKind == TypeKind_Class || typeKind == TypeKind_Struct)
			{
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
	)
{
	if (!parentNamespace)
		parentNamespace = &m_stdNamespaceArray[StdNamespace_Global];

	nspace->m_module = m_module;
	nspace->m_name = name;
	nspace->m_parentNamespace = parentNamespace;
	m_globalNamespaceList.insertTail(nspace);
}

Scope*
NamespaceMgr::findBreakScope(size_t level)
{
	size_t i = 0;
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope())
	{
		if (scope->m_breakBlock)
		{
			i++;
			if (i >= level)
				break;
		}
	}

	return scope;
}

Scope*
NamespaceMgr::findContinueScope(size_t level)
{
	size_t i = 0;
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope())
	{
		if (scope->m_continueBlock)
		{
			i++;
			if (i >= level)
				break;
		}
	}

	return scope;
}

Scope*
NamespaceMgr::findCatchScope()
{
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope())
	{
		if (scope->m_tryExpr || scope->m_catchBlock)
			break;
	}

	return scope;
}

//..............................................................................

} // namespace ct
} // namespace jnc
