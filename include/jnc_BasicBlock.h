// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

class CFunction;

//.............................................................................

enum EBasicBlockFlag
{
	EBasicBlockFlag_Reachable = 0x01,
	EBasicBlockFlag_Jumped    = 0x02,
	EBasicBlockFlag_Entry     = 0x04,
	EBasicBlockFlag_Return    = 0x08,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CBasicBlock: public rtl::TListLink
{
	friend class CControlFlowMgr;
	friend class CLlvmIrBuilder;

protected:
	CModule* m_pModule;

	rtl::CString m_Name;
	rtl::CString m_LeadingComment;
	CFunction* m_pFunction;
	llvm::BasicBlock* m_pLlvmBlock;
	llvm::DebugLoc m_LlvmDebugLoc;

	uint_t m_Flags;

public:
	CBasicBlock ();

	int
	GetFlags ()
	{
		return m_Flags;
	}

	bool
	IsEmpty ()
	{
		return m_pLlvmBlock->getInstList ().empty ();
	}

	size_t
	GetInstructionCount ()
	{
		return m_pLlvmBlock->getInstList ().size ();
	}

	bool
	HasTerminator ()
	{
		return m_pLlvmBlock->getTerminator () != NULL;
	}

	bool
	HasReturn ()
	{
		llvm::TerminatorInst* pInst = m_pLlvmBlock->getTerminator ();
		return pInst && pInst->getOpcode () == llvm::Instruction::Ret;
	}

	rtl::CString
	GetName ()
	{
		return m_Name;
	}

	rtl::CString
	GetLeadingComment ()
	{
		return m_LeadingComment;
	}

	CFunction*
	GetFunction ()
	{
		return m_pFunction;
	}

	llvm::BasicBlock*
	GetLlvmBlock ()
	{
		return m_pLlvmBlock;
	}

	CValue
	GetBlockAddressValue ();

	void
	MarkEntry ()
	{
		m_Flags |= (EBasicBlockFlag_Entry | EBasicBlockFlag_Reachable);
	}
};

//.............................................................................

} // namespace jnc {
