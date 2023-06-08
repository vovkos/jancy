//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class Function;

//..............................................................................

enum BasicBlockFlag {
	BasicBlockFlag_Reachable  = 0x01,
	BasicBlockFlag_Jumped     = 0x02,
	BasicBlockFlag_Entry      = 0x04,
	BasicBlockFlag_Return     = 0x08,

	BasicBlockFlag_EscapeScopeLandingPad = 0x10,
	BasicBlockFlag_ExceptionLandingPad   = 0x20,
	BasicBlockFlag_AsyncLandingPad       = 0x40,
	BasicBlockFlag_SjljLandingPadMask    = 0x60,
	BasicBlockFlag_LandingPadMask        = 0xf0,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class BasicBlock: public sl::ListLink {
	friend class ControlFlowMgr;
	friend class LlvmIrBuilder;

protected:
	Module* m_module;
	uint_t m_flags;
	sl::String m_name;
	Function* m_function;
	sl::SimpleHashTable<size_t, BasicBlock*> m_finallyRouteMap;
	Scope* m_landingPadScope;

	// codegen-only

	llvm::BasicBlock* m_llvmBlock;
	llvm::DebugLoc m_llvmDebugLoc;

public:
	BasicBlock();

	Module*
	getModule() {
		return m_module;
	}

	int
	getFlags() {
		return m_flags;
	}

	bool
	isEmpty() {
		ASSERT(m_llvmBlock);
		return m_llvmBlock->empty();
	}

	size_t
	getInstructionCount() {
		ASSERT(m_llvmBlock);
		return m_llvmBlock->size();
	}

	bool
	hasTerminator() {
		ASSERT(m_llvmBlock);
		return m_llvmBlock->getTerminator() != NULL;
	}

	bool
	hasReturn() {
		ASSERT(m_llvmBlock);
		llvm::Instruction* inst = m_llvmBlock->getTerminator();
		return inst && inst->getOpcode() == llvm::Instruction::Ret;
	}

	const sl::String&
	getName() {
		return m_name;
	}

	Function*
	getFunction() {
		return m_function;
	}

	Scope*
	getLandingPadScope() {
		return m_landingPadScope;
	}

	llvm::BasicBlock*
	getLlvmBlock() {
		ASSERT(m_llvmBlock);
		return m_llvmBlock;
	}

	Value
	getBlockAddressValue();

	void
	markEntry() {
		m_flags |= (BasicBlockFlag_Entry | BasicBlockFlag_Reachable);
	}

	void
	markReachable() {
		m_flags |= BasicBlockFlag_Reachable;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
