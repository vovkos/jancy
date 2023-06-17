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

#include "jnc_ct_GlobalNamespace.h"
#include "jnc_ct_Scope.h"
#include "jnc_ct_Orphan.h"
#include "jnc_ct_Alias.h"

namespace jnc {
namespace ct {

class Module;
class ClassType;
class ExtensionNamespace;
class DynamicLibNamespace;

//..............................................................................

class NamespaceMgr {
	friend class Module;
	friend class Parser;
	friend class FunctionMgr;
	friend class VariableMgr;

protected:
	struct NamespaceStackEntry {
		Namespace* m_namespace;
		Scope* m_scope;
		AccessKind m_accessKind;
	};

protected:
	Module* m_module;

	GlobalNamespace m_stdNamespaceArray[StdNamespace__Count];
	sl::List<GlobalNamespace> m_globalNamespaceList;
	sl::List<Scope> m_scopeList;
	sl::List<Orphan> m_orphanList;
	sl::List<Alias> m_aliasList;

	sl::Array<NamespaceStackEntry> m_namespaceStack;

	lex::LineCol m_sourcePos;
	Namespace* m_currentNamespace;
	Scope* m_currentScope;
	AccessKind m_currentAccessKind;

	intptr_t m_sourcePosLockCount;

	Value m_staticObjectValue;

public:
	NamespaceMgr();

	~NamespaceMgr() {
		clear();
	}

	Module*
	getModule() {
		return m_module;
	}

	void
	clear();

	void
	addStdItems();

	Orphan*
	createOrphan(
		OrphanKind orphanKind,
		FunctionType* functionType
	);

	Alias*
	createAlias(
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		sl::List<Token>* initializer
	);

	void
	lockSourcePos() {
		m_sourcePosLockCount++;
	}

	void
	unlockSourcePos() {
		m_sourcePosLockCount--;
	}

	const lex::LineCol&
	getSourcePos() {
		return m_sourcePos;
	}

	void
	setSourcePos(const lex::LineCol& pos);

	GlobalNamespace*
	getGlobalNamespace() {
		return &m_stdNamespaceArray[StdNamespace_Global];
	}

	GlobalNamespace*
	getStdNamespace(StdNamespace stdNamespace) {
		ASSERT(stdNamespace < StdNamespace__Count);
		return &m_stdNamespaceArray[stdNamespace];
	}

	bool
	isGlobalNamespace() {
		return m_currentNamespace->m_namespaceKind == NamespaceKind_Global;
	}

	Namespace*
	getCurrentNamespace() {
		return m_currentNamespace;
	}

	Scope*
	getCurrentScope() {
		return m_currentScope;
	}

	AccessKind
	getCurrentAccessKind() {
		return m_currentAccessKind;
	}

	void
	openNamespace(Namespace* nspace);

	void
	openStdNamespace(StdNamespace stdNamepace) {
		ASSERT(stdNamepace < StdNamespace__Count);
		openNamespace(&m_stdNamespaceArray[stdNamepace]);
	}

	void
	closeNamespace();

	void
	closeAllNamespaces();

	Scope*
	openInternalScope();

	Scope*
	openScope(
		const lex::LineCol& pos,
		uint_t flags = 0
	);

	void
	closeScope();

	AccessKind
	getAccessKind(Namespace* nspace);

	sl::String
	createQualifiedName(const sl::StringRef& name) {
		return m_currentNamespace->createQualifiedName(name);
	}

	template <typename T>
	T*
	createGlobalNamespace(
		const sl::StringRef& name,
		Namespace* parentNamespace = NULL
	);

	GlobalNamespace*
	createGlobalNamespace(
		const sl::StringRef& name,
		Namespace* parentNamespace = NULL
	) {
		return createGlobalNamespace<GlobalNamespace>(name, parentNamespace);
	}

	Scope*
	findBreakScope(size_t level);

	Scope*
	findContinueScope(size_t level);

	Scope*
	findCatchScope();

	Scope*
	findRegexScope();

protected:
	void
	addGlobalNamespace(
		GlobalNamespace* nspace,
		const sl::StringRef& name,
		Namespace* parentNamespace
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

template <typename T>
T*
NamespaceMgr::createGlobalNamespace(
	const sl::StringRef& name,
	Namespace* parentNamespace
) {
	T* nspace = AXL_MEM_NEW(T);
	addGlobalNamespace(nspace, name, parentNamespace);
	return nspace;
}

//..............................................................................

} // namespace ct
} // namespace jnc
