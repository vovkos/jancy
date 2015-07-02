// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Namespace.h"
#include "jnc_DestructList.h"

namespace jnc {

class BasicBlock;
class Function;

//.............................................................................

enum ScopeFlag
{
	ScopeFlag_Try            = 0x0100,
	ScopeFlag_CatchDefined   = 0x0200,
	ScopeFlag_FinallyDefined = 0x0400,
	ScopeFlag_CanThrow       = 0x0800, // function throws, or parent has catch
	ScopeFlag_HasFinally     = 0x1000, // this scope or its parent has finally
	ScopeFlag_FunctionScope  = 0x2000, // this scope is from compound_stmt
};

//.............................................................................

class Scope:
	public ModuleItem,
	public Namespace
{
	friend class NamespaceMgr;
	friend class ControlFlowMgr;
	friend class FunctionMgr;
	friend class Parser;

protected:
	Token::Pos m_pos;
	Function* m_function;

	rtl::BoxList <Value> m_gcRootList;
	llvm::DIScope m_llvmDiScope;

public:
	BasicBlock* m_breakBlock;
	BasicBlock* m_continueBlock;
	BasicBlock* m_catchBlock;
	BasicBlock* m_catchFollowBlock;
	BasicBlock* m_finallyBlock;
	Variable* m_finallyReturnAddress;
	rtl::Array <BasicBlock*> m_finallyReturnBlockArray;
	DestructList m_destructList;

public:
	Scope ();

	bool
	isFunctionScope ()
	{
		return (m_flags & ScopeFlag_FunctionScope) != 0;
	}

	const Token::Pos*
	getPos ()
	{
		return &m_pos;
	}

	Function*
	getFunction ()
	{
		return m_function;
	}

	Scope*
	getParentScope ()
	{
		return m_parentNamespace && m_parentNamespace->getNamespaceKind () == NamespaceKind_Scope ? (Scope*) m_parentNamespace : NULL;
	}

	rtl::ConstBoxList <Value>
	getGcRootList ()
	{
		return m_gcRootList;
	}

	void
	addToGcRootList (const Value& value)
	{
		m_gcRootList.insertTail (value);
	}

	llvm::DIScope
	getLlvmDiScope ()
	{
		return m_llvmDiScope;
	}
};

//.............................................................................

} // namespace jnc {
