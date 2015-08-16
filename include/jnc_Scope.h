// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Namespace.h"
#include "jnc_Value.h"

namespace jnc {

class BasicBlock;
class Function;
class Variable;

//.............................................................................

enum ScopeFlag
{
	ScopeFlag_Function        = 0x000100,
	ScopeFlag_Unsafe          = 0x000200,	
	ScopeFlag_Try             = 0x000400,
	ScopeFlag_Catch           = 0x000800,
	ScopeFlag_Finally         = 0x001000,
	ScopeFlag_Nested          = 0x002000,
	ScopeFlag_CatchAhead      = 0x004000,
	ScopeFlag_FinallyAhead    = 0x008000,	
	ScopeFlag_CanThrow        = 0x010000, // function throws, scope or one of its parents has catch
	ScopeFlag_Finalizable     = 0x020000, // scope or one of its parents has finally
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

	rtl::BoxList <Value> m_gcStackRootList;
	llvm::DIScope m_llvmDiScope;

public:
	BasicBlock* m_breakBlock;
	BasicBlock* m_continueBlock;
	BasicBlock* m_catchBlock;
	BasicBlock* m_finallyBlock;

public:
	Scope ();

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
	getGcStackRootList ()
	{
		return m_gcStackRootList;
	}

	void
	addToGcStackRootList (const Value& value)
	{
		m_gcStackRootList.insertTail (value);
	}

	llvm::DIScope
	getLlvmDiScope ()
	{
		return m_llvmDiScope;
	}

	BasicBlock* 
	getOrCreateCatchBlock ();

	BasicBlock* 
	getOrCreateFinallyBlock ();
};

//.............................................................................

} // namespace jnc {
