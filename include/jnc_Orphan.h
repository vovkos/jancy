// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Function.h"

namespace jnc {

class CFunctionType;

//.............................................................................

enum EOrphan
{
	EOrphan_Undefined,
	EOrphan_Function,
	EOrphan_Reactor
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class COrphan:
	public CUserModuleItem,
	public CFunctionName
{
	friend class CNamespaceMgr;
	friend class CParser;

protected:
	EOrphan m_OrphanKind;
	CFunctionType* m_pFunctionType;
	rtl::CBoxListT <CToken> m_Body;

public:
	COrphan ();

	EOrphan
	GetOrphanKind ()
	{
		return m_OrphanKind;
	}

	CFunctionType*
	GetFunctionType ()
	{
		return m_pFunctionType;
	}

	rtl::CConstBoxListT <CToken>
	GetBody ()
	{
		return m_Body;
	}

	bool
	SetBody (rtl::CBoxListT <CToken>* pTokenList);

	bool
	ResolveOrphan ();

protected:
	bool
	AdoptOrphanFunction (CModuleItem* pItem);

	bool
	AdoptOrphanReactor (CModuleItem* pItem);

	bool
	VerifyStorageKind (CModuleItemDecl* pTargetDecl);

	void
	CopySrcPos (CModuleItemDecl* pTargetDecl);

	bool
	CopyArgNames (CFunctionType* pTargetFunctionType);

	CFunction*
	GetItemUnnamedMethod (CModuleItem* pItem);
};

//.............................................................................

} // namespace jnc {
