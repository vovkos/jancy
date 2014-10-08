#include "pch.h"
#include "jnc_NamespaceMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

NamespaceMgr::NamespaceMgr ()
{
	m_module = getCurrentThreadModule ();
	ASSERT (m_module);

	m_currentNamespace = &m_globalNamespace;
	m_currentScope = NULL;
	m_currentAccessKind = AccessKind_Public;
	m_sourcePosLockCount = 0;
	m_scopeLevelStack.m_module = m_module;
}

void
NamespaceMgr::clear ()
{
	m_globalNamespace.clear ();
	m_namespaceList.clear ();
	m_scopeList.clear ();
	m_orphanList.clear ();
	m_namespaceStack.clear ();
	m_currentNamespace = &m_globalNamespace;
	m_jncNamespace = NULL;
	m_currentScope = NULL;
	m_sourcePosLockCount = 0;
	m_scopeLevelStack.clear ();
	m_staticObjectValue.clear ();
}

GlobalNamespace* 
NamespaceMgr::getJncNamespace ()
{
	if (m_jncNamespace)
		return m_jncNamespace;

	m_jncNamespace = createGlobalNamespace ("jnc", &m_globalNamespace);
	m_jncNamespace->m_flags |= GlobalNamespaceFlag_Sealed;
	return m_jncNamespace;
}

bool
NamespaceMgr::addStdItems ()
{
	GlobalNamespace* jnc = getJncNamespace ();

	return
		m_globalNamespace.addItem (jnc) &&
		m_globalNamespace.addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_StrLen)) &&
		m_globalNamespace.addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_MemCpy)) &&
		m_globalNamespace.addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_MemCat)) &&
		m_globalNamespace.addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_Rand)) &&
		m_globalNamespace.addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_Printf)) &&
		m_globalNamespace.addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_Atoi)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdTypeKind_Scheduler)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdTypeKind_Guid)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdTypeKind_Error)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdTypeKind_String)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdTypeKind_StringBuilder)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdTypeKind_SmartPtr)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdTypeKind_SmartConstPtr)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdTypeKind_DynamicArray)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_GetDataPtrSpan)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_RunGc)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_CreateThread)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_Sleep)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_GetCurrentThreadId)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_GetTimestamp)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_GetLastError)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFuncKind_Format));
}

Value
NamespaceMgr::getStaticObjHdr ()
{
	if (m_staticObjectValue)
		return m_staticObjectValue;

	static ObjHdr* staticObjHdr = jnc::getStaticObjHdr ();
	m_staticObjectValue.createConst (&staticObjHdr, m_module->m_typeMgr.getStdType (StdTypeKind_ObjHdrPtr));
	return m_staticObjectValue;
}

Orphan*
NamespaceMgr::createOrphan (
	OrphanKind orphanKind,
	FunctionType* functionType
	)
{
	Orphan* orphan = AXL_MEM_NEW (Orphan);
	orphan->m_orphanKind = orphanKind;
	orphan->m_functionType = functionType;
	m_orphanList.insertTail (orphan);
	return orphan;
}

bool
NamespaceMgr::resolveOrphans ()
{
	bool result;

	rtl::Iterator <Orphan> it = m_orphanList.getHead ();
	for (; it; it++)
	{
		result = it->resolveOrphan ();
		if (!result)
			return false;
	}

	return true;
}

void
NamespaceMgr::setSourcePos (const Token::Pos& pos)
{
	if (!(m_module->getFlags () & ModuleFlag_DebugInfo) ||
		!m_currentScope ||
		m_sourcePosLockCount)
		return;

	llvm::DebugLoc llvmDebugLoc = m_module->m_llvmDiBuilder.getDebugLoc (m_currentScope, pos);
	m_module->m_llvmIrBuilder.setCurrentDebugLoc (llvmDebugLoc);
}

void
NamespaceMgr::openNamespace (Namespace* nspace)
{
	NamespaceStackEntry entry =
	{
		m_currentNamespace,
		m_currentAccessKind
	};

	m_namespaceStack.append (entry);
	m_currentNamespace = nspace;
	m_currentScope = nspace->m_namespaceKind == NamespaceKind_Scope ? (Scope*) nspace : NULL;
	m_currentAccessKind = AccessKind_Public; // always start with 'public'
}

void
NamespaceMgr::closeNamespace ()
{
	if (m_namespaceStack.isEmpty ())
		return;

	NamespaceStackEntry entry = m_namespaceStack.getBackAndPop ();

	m_currentNamespace = entry.m_namespace;
	m_currentScope = m_currentNamespace->m_namespaceKind == NamespaceKind_Scope ? (Scope*) m_currentNamespace : NULL;
	m_currentAccessKind = entry.m_accessKind;
}

Scope*
NamespaceMgr::openInternalScope ()
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	Scope* scope = AXL_MEM_NEW (Scope);
	scope->m_module = m_module;
	scope->m_function = function;
	scope->m_level = m_currentScope ? m_currentScope->getLevel () + 1 : 1;
	scope->m_parentNamespace = m_currentNamespace;

	if (m_currentScope)
		scope->m_flags |= m_currentScope->m_flags & (ScopeFlag_CanThrow | ScopeFlag_HasFinally);
	else if (function->getType ()->getFlags () & FunctionTypeFlag_Throws)
		scope->m_flags |= ScopeFlag_CanThrow;

	m_scopeList.insertTail (scope);

	openNamespace (scope);
	return scope;
}

Scope*
NamespaceMgr::openScope (const Token::Pos& pos)
{
	Scope* parentScope = m_currentScope;
	Scope* scope = openInternalScope ();
	scope->m_pos = pos;

	if (m_module->getFlags () & ModuleFlag_DebugInfo)
		scope->m_llvmDiScope = (llvm::DIScope) m_module->m_llvmDiBuilder.createLexicalBlock (parentScope, pos);

	setSourcePos (pos);
	return scope;
}

void
NamespaceMgr::closeScope ()
{
	Scope* scope = m_currentScope;
	ASSERT (scope);

	if (m_module->m_controlFlowMgr.getCurrentBlock ()->getFlags () & BasicBlockFlag_Reachable)
	{
		scope->m_destructList.runDestructors ();
		m_module->m_operatorMgr.nullifyGcRootList (scope->getGcRootList ());
	}

	if (scope->m_flags & ScopeFlag_FinallyDefined)
		m_module->m_controlFlowMgr.endFinally ();

	closeNamespace ();
}

AccessKind
NamespaceMgr::getAccessKind (Namespace* targetNamespace)
{
	Namespace* nspace = m_currentNamespace;

	if (!targetNamespace->isNamed ())
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
			if (!nspace->isNamed ())
				continue;

			if (nspace == targetNamespace ||
				targetNamespace->m_qualifiedName.cmp (nspace->m_qualifiedName) == 0 ||
				targetNamespace->m_friendSet.find (nspace->m_qualifiedName))
				return AccessKind_Protected;
		}

		return AccessKind_Public;
	}

	NamedType* targetType = (NamedType*) targetNamespace;

	for (; nspace; nspace = nspace->m_parentNamespace)
	{
		if (!nspace->isNamed ())
			continue;

		if (nspace == targetNamespace ||
			targetNamespace->m_qualifiedName.cmp (nspace->m_qualifiedName) == 0 ||
			targetNamespace->m_friendSet.find (nspace->m_qualifiedName))
			return AccessKind_Protected;

		if (nspace->m_namespaceKind == NamespaceKind_Type)
		{
			NamedType* type = (NamedType*) nspace;
			TypeKind typeKind = type->getTypeKind ();
			if (typeKind == TypeKind_Class || typeKind == TypeKind_Struct)
			{
				bool result = ((DerivableType*) type)->findBaseTypeTraverse (targetType);
				if (result)
					return AccessKind_Protected;
			}
		}
	}

	return AccessKind_Public;
}

GlobalNamespace*
NamespaceMgr::createGlobalNamespace (
	const rtl::String& name,
	Namespace* parentNamespace
	)
{
	if (!parentNamespace)
		parentNamespace = &m_globalNamespace;

	rtl::String qualifiedName = parentNamespace->createQualifiedName (name);

	GlobalNamespace* nspace = AXL_MEM_NEW (GlobalNamespace);
	nspace->m_module = m_module;
	nspace->m_name = name;
	nspace->m_qualifiedName = qualifiedName;
	nspace->m_tag = qualifiedName;
	nspace->m_parentNamespace = parentNamespace;
	m_namespaceList.insertTail (nspace);
	return nspace;
}

Scope*
NamespaceMgr::findBreakScope (size_t level)
{
	size_t i = 0;
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope ())
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
NamespaceMgr::findContinueScope (size_t level)
{
	size_t i = 0;
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope ())
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
NamespaceMgr::findCatchScope ()
{
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope ())
	{
		if (scope->m_catchBlock)
			break;
	}

	return scope;
}

//.............................................................................

} // namespace jnc {
