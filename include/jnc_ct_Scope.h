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
struct TryExpr;

//.............................................................................

enum ScopeFlag
{
	ScopeFlag_Function     = 0x000100,
	ScopeFlag_Unsafe       = 0x000200,
	ScopeFlag_Nested       = 0x000400,
	ScopeFlag_Try          = 0x001000,
	ScopeFlag_Catch        = 0x002000,
	ScopeFlag_Finally      = 0x004000,
	ScopeFlag_CatchAhead   = 0x020000,
	ScopeFlag_FinallyAhead = 0x040000,
	ScopeFlag_Finalizable  = 0x100000, // scope or one of its parents has finally
	ScopeFlag_Disposable   = 0x200000, // this scope contains disposable variables
	ScopeFlag_HasCatch     = 0x400000, // this scope or some of its parents have catch
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
	
	TryExpr* m_tryExpr;
	size_t m_sjljFrameIdx;

	LlvmIrInsertPoint m_gcShadowStackFrameMapInsertPoint;
	GcShadowStackFrameMap* m_gcShadowStackFrameMap;
	Variable* m_firstStackVariable; // we have to set frame map BEFORE the very first stack variable lift point

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

	bool
	canStaticThrow ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
