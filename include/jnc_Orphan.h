// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Function.h"

namespace jnc {

class FunctionType;

//.............................................................................

enum OrphanKind
{
	OrphanKind_Undefined,
	OrphanKind_Function,
	OrphanKind_Reactor
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Orphan:
	public UserModuleItem,
	public FunctionName
{
	friend class NamespaceMgr;
	friend class Parser;

protected:
	OrphanKind m_orphanKind;
	FunctionType* m_functionType;
	rtl::BoxList <Token> m_body;

public:
	Orphan ();

	OrphanKind
	getOrphanKind ()
	{
		return m_orphanKind;
	}

	FunctionType*
	getFunctionType ()
	{
		return m_functionType;
	}

	rtl::ConstBoxList <Token>
	getBody ()
	{
		return m_body;
	}

	bool
	setBody (rtl::BoxList <Token>* tokenList);

	bool
	resolveOrphan ();

protected:
	bool
	adoptOrphanFunction (ModuleItem* item);

	bool
	adoptOrphanReactor (ModuleItem* item);

	bool
	verifyStorageKind (ModuleItemDecl* targetDecl);

	void
	copySrcPos (ModuleItemDecl* targetDecl);

	bool
	copyArgNames (FunctionType* targetFunctionType);

	Function*
	getItemUnnamedMethod (ModuleItem* item);
};

//.............................................................................

} // namespace jnc {
