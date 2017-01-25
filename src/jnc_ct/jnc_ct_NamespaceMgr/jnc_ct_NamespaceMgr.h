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

#include "jnc_ct_Namespace.h"
#include "jnc_ct_Scope.h"
#include "jnc_ct_Orphan.h"

namespace jnc {
namespace ct {

class Module;
class ClassType;

//..............................................................................

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
	sl::StdList <GlobalNamespace> m_globalNamespaceList;
	sl::StdList <ExtensionNamespace> m_extensionNamespaceList;
	sl::StdList <DynamicLibNamespace> m_dynamicLibNamespaceList;
	sl::StdList <Scope> m_scopeList;
	sl::StdList <Orphan> m_orphanList;

	sl::Array <NamespaceStackEntry> m_namespaceStack;

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

	void
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
	closeScope ();

	AccessKind
	getAccessKind (Namespace* nspace);

	sl::String
	createQualifiedName (const sl::StringRef& name)
	{
		return m_currentNamespace->createQualifiedName (name);
	}

	GlobalNamespace*
	createGlobalNamespace (
		const sl::StringRef& name,
		Namespace* parentNamespace = NULL
		);

	ExtensionNamespace*
	createExtensionNamespace (
		const sl::StringRef& name,
		DerivableType* type,
		Namespace* parentNamespace = NULL
		);

	DynamicLibNamespace*
	createDynamicLibNamespace (ClassType* dynamicLibType);

	Scope*
	findBreakScope (size_t level);

	Scope*
	findContinueScope (size_t level);

	Scope*
	findCatchScope ();
};

//..............................................................................

} // namespace ct
} // namespace jnc
