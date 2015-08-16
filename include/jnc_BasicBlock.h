// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

class Function;

//.............................................................................

enum BasicBlockFlag
{
	BasicBlockFlag_Reachable = 0x01,
	BasicBlockFlag_Jumped    = 0x02,
	BasicBlockFlag_Entry     = 0x04,
	BasicBlockFlag_Return    = 0x08,
	BasicBlockFlag_Catch     = 0x10,
	BasicBlockFlag_Finally   = 0x20,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class BasicBlock: public rtl::ListLink
{
	friend class ControlFlowMgr;
	friend class LlvmIrBuilder;

protected:
	Module* m_module;

	rtl::String m_name;
	Function* m_function;
	llvm::BasicBlock* m_llvmBlock;
	llvm::DebugLoc m_llvmDebugLoc;

	rtl::HashTableMap <size_t, BasicBlock*, rtl::HashId <size_t> > m_finallyRouteMap;

	uint_t m_flags;

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

	rtl::String
	getName ()
	{
		return m_name;
	}

	Function*
	getFunction ()
	{
		return m_function;
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

//.............................................................................

} // namespace jnc {
