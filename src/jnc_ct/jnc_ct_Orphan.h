// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

class FunctionType;

//..............................................................................

enum OrphanKind
{
	OrphanKind_Undefined,
	OrphanKind_Function,
	OrphanKind_Reactor
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Orphan:
	public ModuleItem,
	public ModuleItemDecl,
	public FunctionName
{
	friend class NamespaceMgr;
	friend class Parser;

protected:
	OrphanKind m_orphanKind;
	FunctionType* m_functionType;
	sl::BoxList <Token> m_body;

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

	sl::ConstBoxList <Token>
	getBody ()
	{
		return m_body;
	}

	bool
	setBody (sl::BoxList <Token>* tokenList);

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

//..............................................................................

} // namespace ct
} // namespace jnc
