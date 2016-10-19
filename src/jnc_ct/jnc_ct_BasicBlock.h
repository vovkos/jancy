// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class Function;

//..............................................................................

enum BasicBlockFlag
{
	BasicBlockFlag_Reachable  = 0x01,
	BasicBlockFlag_Jumped     = 0x02,
	BasicBlockFlag_Entry      = 0x04,
	BasicBlockFlag_Return     = 0x08,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum LandingPadKind
{
	LandingPadKind_None,
	LandingPadKind_EscapeScope,
	LandingPadKind_Exception,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class BasicBlock: public sl::ListLink
{
	friend class ControlFlowMgr;
	friend class LlvmIrBuilder;

protected:
	Module* m_module;
	uint_t m_flags;

	sl::String m_name;
	Function* m_function;
	llvm::BasicBlock* m_llvmBlock;
	llvm::DebugLoc m_llvmDebugLoc;

	sl::HashTableMap <size_t, BasicBlock*, sl::HashId <size_t> > m_finallyRouteMap;

	LandingPadKind m_landingPadKind;
	Scope* m_landingPadScope;

public:
	BasicBlock ();

	int
	getFlags ()
	{
		return m_flags;
	}

	bool
	isEmpty ()
	{
		return m_llvmBlock->getInstList ().empty ();
	}

	size_t
	getInstructionCount ()
	{
		return m_llvmBlock->getInstList ().size ();
	}

	bool
	hasTerminator ()
	{
		return m_llvmBlock->getTerminator () != NULL;
	}

	bool
	hasReturn ()
	{
		llvm::TerminatorInst* inst = m_llvmBlock->getTerminator ();
		return inst && inst->getOpcode () == llvm::Instruction::Ret;
	}

	const sl::String&
	getName ()
	{
		return m_name;
	}

	Function*
	getFunction ()
	{
		return m_function;
	}

	LandingPadKind
	getLandingPadKind ()
	{
		return m_landingPadKind;
	}

	Scope*
	getLandingPadScope ()
	{
		return m_landingPadScope;
	}

	llvm::BasicBlock*
	getLlvmBlock ()
	{
		return m_llvmBlock;
	}

	Value
	getBlockAddressValue ();

	void
	markEntry ()
	{
		m_flags |= (BasicBlockFlag_Entry | BasicBlockFlag_Reachable);
	}

	void
	markReachable ()
	{
		m_flags |= BasicBlockFlag_Reachable;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
