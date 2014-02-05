// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Namespace.h"
#include "jnc_DestructList.h"

namespace jnc {

class CBasicBlock;
class CFunction;

//.............................................................................

enum EScopeFlag
{
	EScopeFlag_Try            = 0x0100,
	EScopeFlag_CatchDefined   = 0x0200,
	EScopeFlag_FinallyDefined = 0x0400,
	EScopeFlag_CanThrow       = 0x0800, // function throws, or parent has catch
	EScopeFlag_HasFinally     = 0x1000, // this scope or its parent has finally
};

//.............................................................................

class CScope:
	public CModuleItem,
	public CNamespace
{
	friend class CNamespaceMgr;
	friend class CControlFlowMgr;
	friend class CFunctionMgr;
	friend class CParser;

protected:
	size_t m_Level;

	CToken::CPos m_Pos;
	CFunction* m_pFunction;

	rtl::CBoxListT <CValue> m_GcRootList;
	llvm::DIScope m_LlvmDiScope;

public:
	CBasicBlock* m_pBreakBlock;
	CBasicBlock* m_pContinueBlock;
	CBasicBlock* m_pCatchBlock;
	CBasicBlock* m_pFinallyBlock;
	CVariable* m_pFinallyReturnAddress;
	rtl::CArrayT <CBasicBlock*> m_FinallyReturnBlockArray;
	CDestructList m_DestructList;

public:
	CScope ();

	size_t
	GetLevel ()
	{
		return m_Level;
	}

	bool
	IsFunctionScope ()
	{
		return m_Level == 2;
	}

	const CToken::CPos*
	GetPos ()
	{
		return &m_Pos;
	}

	CFunction*
	GetFunction ()
	{
		return m_pFunction;
	}

	CScope*
	GetParentScope ()
	{
		return m_pParentNamespace && m_pParentNamespace->GetNamespaceKind () == ENamespace_Scope ? (CScope*) m_pParentNamespace : NULL;
	}

	rtl::CConstBoxListT <CValue>
	GetGcRootList ()
	{
		return m_GcRootList;
	}

	void
	AddToGcRootList (const CValue& Value)
	{
		m_GcRootList.InsertTail (Value);
	}

	llvm::DIScope
	GetLlvmDiScope ()
	{
		return m_LlvmDiScope;
	}
};

//.............................................................................

} // namespace jnc {
