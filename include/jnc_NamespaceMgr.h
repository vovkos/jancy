// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Namespace.h"
#include "jnc_Scope.h"
#include "jnc_Orphan.h"
#include "jnc_ScopeLevelStack.h"

namespace jnc {

class CModule;
class CClassType;


//.............................................................................

class CNamespaceMgr
{
	friend class CModule;
	friend class CParser;
	friend class CFunctionMgr;

protected:
	struct TNamespaceStackEntry
	{
		CNamespace* m_pNamespace;
		EAccess m_AccessKind;
	};

protected:
	CModule* m_pModule;

	CGlobalNamespace m_GlobalNamespace;
	rtl::CStdListT <CGlobalNamespace> m_NamespaceList;
	rtl::CStdListT <CScope> m_ScopeList;
	rtl::CStdListT <COrphan> m_OrphanList;

	rtl::CArrayT <TNamespaceStackEntry> m_NamespaceStack;

	CNamespace* m_pCurrentNamespace;
	CScope* m_pCurrentScope;
	EAccess m_CurrentAccessKind;

	intptr_t m_SourcePosLockCount;

	CValue m_StaticObjectValue;
	CScopeLevelStack m_ScopeLevelStack;

public:
	CNamespaceMgr ();
	
	~CNamespaceMgr ()
	{
		Clear ();
	}

	CModule*
	GetModule ()
	{
		return m_pModule;
	}

	void
	Clear ();

	bool
	AddStdItems ();

	COrphan*
	CreateOrphan (
		EOrphan OrphanKind,
		CFunctionType* pFunctionType
		);

	bool
	ResolveOrphans ();

	void
	LockSourcePos ()
	{
		m_SourcePosLockCount++;
	}

	void
	UnlockSourcePos ()
	{
		m_SourcePosLockCount--;
	}

	void
	SetSourcePos (const CToken::CPos& Pos);

	CGlobalNamespace*
	GetGlobalNamespace ()
	{
		return &m_GlobalNamespace;
	}

	CNamespace*
	GetCurrentNamespace ()
	{
		return m_pCurrentNamespace; 
	}

	CScope*
	GetCurrentScope ()
	{
		return m_pCurrentScope;
	}

	EAccess
	GetCurrentAccessKind ()
	{
		return m_CurrentAccessKind;
	}

	CValue
	GetScopeLevel (CScope* pScope)
	{
		return pScope ? m_ScopeLevelStack.GetScopeLevel (pScope->GetLevel ()) : CValue ((int64_t) 0, EType_SizeT);
	}

	CValue
	GetCurrentScopeLevel ()
	{
		return GetScopeLevel (m_pCurrentScope);
	}

	CValue
	GetScopeLevelObjHdr (CScope* pScope)
	{
		return pScope ? m_ScopeLevelStack.GetObjHdr (pScope->GetLevel ()) : GetStaticObjHdr ();
	}

	CValue
	GetCurrentScopeObjHdr ()
	{
		return GetScopeLevelObjHdr (m_pCurrentScope);
	}

	CValue
	GetStaticObjHdr ();

	void
	OpenNamespace (CNamespace* pNamespace);

	void
	CloseNamespace ();

	CScope*
	OpenInternalScope ();

	CScope*
	OpenScope (const CToken::CPos& Pos);

	void
	CloseScope ();

	EAccess
	GetAccessKind (CNamespace* pNamespace);

	rtl::CString
	CreateQualifiedName (const char* pName)
	{
		return m_pCurrentNamespace->CreateQualifiedName (pName);
	}

	CGlobalNamespace*
	CreateGlobalNamespace (
		const rtl::CString& Name,
		CNamespace* pParentNamespace = NULL
		);

	CScope*
	FindBreakScope (size_t Level);

	CScope*
	FindContinueScope (size_t Level);

	CScope*
	FindCatchScope ();
};

//.............................................................................

} // namespace jnc {
