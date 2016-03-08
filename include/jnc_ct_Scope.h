// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Namespace.h"
#include "jnc_ct_Value.h"
#include "jnc_ct_LlvmIrInsertPoint.h"

namespace jnc {
namespace ct {

class BasicBlock;
class Function;
class Variable;
class GcShadowStackFrameMap;

//.............................................................................

enum ScopeFlag
{
	ScopeFlag_Function     = 0x000100,
	ScopeFlag_Unsafe       = 0x000200,	
	ScopeFlag_Try          = 0x000400,
	ScopeFlag_Catch        = 0x000800,
	ScopeFlag_Finally      = 0x001000,
	ScopeFlag_Nested       = 0x002000,
	ScopeFlag_CatchAhead   = 0x004000,
	ScopeFlag_FinallyAhead = 0x008000,	
	ScopeFlag_Finalizable  = 0x010000, // scope or one of its parents has finally
	ScopeFlag_Disposable   = 0x020000, // this scope contains disposable variables
	ScopeFlag_StaticThrow  = 0x040000, // this scope or its parents have catch or function is errorcode
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
	Variable* m_disposeLevelVariable;
	sl::Array <Variable*> m_disposableVariableArray;
	llvm::DIScope m_llvmDiScope;

public:
	BasicBlock* m_breakBlock;
	BasicBlock* m_continueBlock;
	BasicBlock* m_catchBlock;
	BasicBlock* m_finallyBlock;

	GcShadowStackFrameMap* m_gcShadowStackFrameMap;
	Variable* m_firstStackVariable; // we have to set frame map BEFORE the very first stack variable init
	Value m_prevSjljFrameValue;

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

	Variable* 
	getDisposeLevelVariable ()
	{
		return m_disposeLevelVariable;
	}

	sl::Array <Variable*>
	getDisposableVariableArray ()
	{
		return m_disposableVariableArray;
	}

	size_t
	addDisposableVariable (Variable* variable)
	{
		m_disposableVariableArray.append (variable);
		return m_disposableVariableArray.getCount ();
	}

	llvm::DIScope
	getLlvmDiScope ()
	{
		return m_llvmDiScope;
	}
};

//.............................................................................

} // namespace ct
} // namespace jnc
