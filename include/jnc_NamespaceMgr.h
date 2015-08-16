// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Namespace.h"
#include "jnc_Scope.h"
#include "jnc_Orphan.h"

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
		Scope* m_scope;
		AccessKind m_accessKind;
	};

protected:
	Module* m_module;

	GlobalNamespace m_stdNamespaceArray [StdNamespace__Count];
	rtl::StdList <GlobalNamespace> m_globalNamespaceList;
	rtl::StdList <ExtensionNamespace> m_extensionNamespaceList;
	rtl::StdList <LibraryNamespace> m_libraryNamespaceList;
	rtl::StdList <Scope> m_scopeList;
	rtl::StdList <Orphan> m_orphanList;

	rtl::Array <NamespaceStackEntry> m_namespaceStack;

	Namespace* m_currentNamespace;
	Scope* m_currentScope;
	AccessKind m_currentAccessKind;

	intptr_t m_sourcePosLockCount;

	Value m_staticObjectValue;

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
		return &m_stdNamespaceArray [StdNamespace_Global];
	}

	GlobalNamespace* 
	getStdNamespace (StdNamespace stdNamespace)
	{
		ASSERT (stdNamespace < StdNamespace__Count);
		return &m_stdNamespaceArray [stdNamespace];
	}

	bool
	isGlobalNamespace ()
	{
		return m_currentNamespace->m_namespaceKind == NamespaceKind_Global;
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

	void
	openNamespace (Namespace* nspace);

	void
	openStdNamespace (StdNamespace stdNamepace)
	{
		ASSERT (stdNamepace < StdNamespace__Count);
		openNamespace (&m_stdNamespaceArray [stdNamepace]);
	}

	void
	closeNamespace ();

	Scope*
	openInternalScope ();

	Scope*
	openScope (
		const Token::Pos& pos,
		uint_t flags = 0
		);

	void
	closeScope ()
	{
		ASSERT (m_currentScope);
		closeNamespace ();
	}

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

	ExtensionNamespace*
	createExtensionNamespace (
		const rtl::String& name,
		DerivableType* type,
		Namespace* parentNamespace = NULL
		);

	LibraryNamespace*
	createLibraryNamespace (ClassType* libraryType);

	Scope*
	findBreakScope (size_t level);

	Scope*
	findContinueScope (size_t level);

	Scope*
	findCatchScope ();
};

//.............................................................................

} // namespace jnc {
