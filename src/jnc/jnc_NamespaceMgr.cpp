#include "pch.h"
#include "jnc_NamespaceMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

NamespaceMgr::NamespaceMgr ()
{
	m_module = getCurrentThreadModule ();
	ASSERT (m_module);

	rtl::String jncName ("jnc");

	GlobalNamespace* global = &m_stdNamespaceArray [StdNamespace_Global];
	GlobalNamespace* jnc = &m_stdNamespaceArray [StdNamespace_Jnc];
	GlobalNamespace* internal = &m_stdNamespaceArray [StdNamespace_Internal];

	global->m_module = m_module;

	jnc->m_module = m_module;
	jnc->m_parentNamespace = global;
	jnc->m_name = jncName;
	jnc->m_qualifiedName = jncName;
	jnc->m_tag = jncName;
	jnc->m_flags |= ModuleItemFlag_Sealed;

	internal->m_module = m_module;
	internal->m_parentNamespace = global;
	internal->m_name = jncName;
	internal->m_qualifiedName = jncName;
	internal->m_tag = jncName;

	m_currentNamespace = global;
	m_currentScope = NULL;
	m_currentAccessKind = AccessKind_Public;
	m_sourcePosLockCount = 0;
	m_scopeLevelStack.m_module = m_module;
}

void
NamespaceMgr::clear ()
{
	for (size_t i = 0; i < StdNamespace__Count; i++)
		m_stdNamespaceArray [i].clear ();

	m_globalNamespaceList.clear ();
	m_extensionNamespaceList.clear ();
	m_scopeList.clear ();
	m_orphanList.clear ();
	m_namespaceStack.clear ();
	m_currentNamespace = &m_stdNamespaceArray [StdNamespace_Global];
	m_currentScope = NULL;
	m_sourcePosLockCount = 0;
	m_scopeLevelStack.clear ();
	m_staticObjectValue.clear ();
}

bool
NamespaceMgr::addStdItems ()
{
	GlobalNamespace* global = &m_stdNamespaceArray [StdNamespace_Global];
	GlobalNamespace* jnc = &m_stdNamespaceArray [StdNamespace_Jnc];

	return
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uint_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uintptr_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_size_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uint8_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uchar_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_byte_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uint16_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_ushort_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_word_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uint32_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_dword_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uint64_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_qword_t)) &&
		global->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_StrLen)) &&
		global->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_MemCpy)) &&
		global->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_MemCat)) &&
		global->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_Rand)) &&
		global->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_Printf)) &&
		global->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_Atoi)) &&
		global->addItem (jnc) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdType_Scheduler)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdType_Guid)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdType_Error)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdType_String)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdType_StringRef)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdType_StringBuilder)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdType_ConstBuffer)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdType_ConstBufferRef)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdType_BufferRef)) &&
		jnc->addItem (m_module->m_typeMgr.getLazyStdType (StdType_Buffer)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_RunGc)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_CreateThread)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_Sleep)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_GetCurrentThreadId)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_GetTimestamp)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_GetLastError)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_SetPosixError)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_SetStringError)) &&
		jnc->addItem (m_module->m_functionMgr.getLazyStdFunction (StdFunction_Format));
}

Value
NamespaceMgr::getStaticObjHdr ()
{
	if (m_staticObjectValue)
		return m_staticObjectValue;

	static ObjHdr* staticObjHdr = jnc::getStaticObjHdr ();
	m_staticObjectValue.createConst (&staticObjHdr, m_module->m_typeMgr.getStdType (StdType_ObjHdrPtr));
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

	if (m_currentNamespace->m_namespaceKind == NamespaceKind_Global) // for others not really needed
		m_currentNamespace->m_usingSet.clear ();

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

Scope*
NamespaceMgr::openTryScope (const Token::Pos& pos)
{
	Scope* scope = openScope (pos);
	scope->m_flags |= ScopeFlag_Try;
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
	else if (scope->m_flags & ScopeFlag_CatchDefined)
		m_module->m_controlFlowMgr.endCatch ();
	else if (scope->m_flags & ScopeFlag_Try)
		m_module->m_controlFlowMgr.endTry ();

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
		parentNamespace = &m_stdNamespaceArray [StdNamespace_Global];

	rtl::String qualifiedName = parentNamespace->createQualifiedName (name);

	GlobalNamespace* nspace = AXL_MEM_NEW (GlobalNamespace);
	nspace->m_module = m_module;
	nspace->m_name = name;
	nspace->m_qualifiedName = qualifiedName;
	nspace->m_tag = qualifiedName;
	nspace->m_parentNamespace = parentNamespace;
	m_globalNamespaceList.insertTail (nspace);
	return nspace;
}

ExtensionNamespace*
NamespaceMgr::createExtensionNamespace (
	const rtl::String& name,
	DerivableType* type,
	Namespace* parentNamespace
	)
{
	if (!parentNamespace)
		parentNamespace = &m_stdNamespaceArray [StdNamespace_Global];

	rtl::String qualifiedName = type->createQualifiedName (name);

	ExtensionNamespace* nspace = AXL_MEM_NEW (ExtensionNamespace);
	nspace->m_module = m_module;
	nspace->m_name = name;
	nspace->m_qualifiedName = qualifiedName;
	nspace->m_tag = qualifiedName;
	nspace->m_parentNamespace = parentNamespace;
	nspace->m_type = type;
	m_extensionNamespaceList.insertTail (nspace);
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
