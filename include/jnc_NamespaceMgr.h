// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Namespace.h"
#include "jnc_Scope.h"
#include "jnc_Orphan.h"
#include "jnc_ScopeLevelStack.h"

namespace jnc {

class Module;
class ClassType;


//.............................................................................

class NamespaceMgr
{
	friend class Module;
	friend class Parser;
	friend class FunctionMgr;

protected:
	struct NamespaceStackEntry
	{
		Namespace* m_namespace;
		AccessKind m_accessKind;
	};

protected:
	Module* m_module;

	GlobalNamespace m_globalNamespace;
	rtl::StdList <GlobalNamespace> m_namespaceList;
	rtl::StdList <Scope> m_scopeList;
	rtl::StdList <Orphan> m_orphanList;

	rtl::Array <NamespaceStackEntry> m_namespaceStack;

	Namespace* m_currentNamespace;
	Scope* m_currentScope;
	AccessKind m_currentAccessKind;

	intptr_t m_sourcePosLockCount;

	Value m_staticObjectValue;
	ScopeLevelStack m_scopeLevelStack;

public:
	NamespaceMgr ();
	
	~NamespaceMgr ()
	{
		clear ();
	}

	Module*
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	bool
	addStdItems ();

	Orphan*
	createOrphan (
		OrphanKind orphanKind,
		FunctionType* functionType
		);

	bool
	resolveOrphans ();

	void
	lockSourcePos ()
	{
		m_sourcePosLockCount++;
	}

	void
	unlockSourcePos ()
	{
		m_sourcePosLockCount--;
	}

	void
	setSourcePos (const Token::Pos& pos);

	GlobalNamespace*
	getGlobalNamespace ()
	{
		return &m_globalNamespace;
	}

	Namespace*
	getCurrentNamespace ()
	{
		return m_currentNamespace; 
	}

	Scope*
	getCurrentScope ()
	{
		return m_currentScope;
	}

	AccessKind
	getCurrentAccessKind ()
	{
		return m_currentAccessKind;
	}

	Value
	getScopeLevel (Scope* scope)
	{
		return scope ? m_scopeLevelStack.getScopeLevel (scope->getLevel ()) : Value ((int64_t) 0, TypeKind_SizeT);
	}

	Value
	getCurrentScopeLevel ()
	{
		return getScopeLevel (m_currentScope);
	}

	Value
	getScopeLevelObjHdr (Scope* scope)
	{
		return scope ? m_scopeLevelStack.getObjHdr (scope->getLevel ()) : getStaticObjHdr ();
	}

	Value
	getCurrentScopeObjHdr ()
	{
		return getScopeLevelObjHdr (m_currentScope);
	}

	Value
	getStaticObjHdr ();

	void
	openNamespace (Namespace* nspace);

	void
	closeNamespace ();

	Scope*
	openInternalScope ();

	Scope*
	openScope (const Token::Pos& pos);

	void
	closeScope ();

	AccessKind
	getAccessKind (Namespace* nspace);

	rtl::String
	createQualifiedName (const char* name)
	{
		return m_currentNamespace->createQualifiedName (name);
	}

	GlobalNamespace*
	createGlobalNamespace (
		const rtl::String& name,
		Namespace* parentNamespace = NULL
		);

	Scope*
	findBreakScope (size_t level);

	Scope*
	findContinueScope (size_t level);

	Scope*
	findCatchScope ();
};

//.............................................................................

} // namespace jnc {
