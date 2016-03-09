// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

namespace jnc {
namespace ct {

class BasicBlock;

//.............................................................................

struct LlvmIrInsertPoint
{
	llvm::BasicBlock* m_llvmBlock;
	llvm::Instruction* m_llvmInstruction;

	LlvmIrInsertPoint ()
	{
		clear ();
	}

	LlvmIrInsertPoint (
		llvm::BasicBlock* llvmBlock,
		llvm::Instruction* llvmInstruction = NULL
		)
	{
		setup (llvmBlock, llvmInstruction);
	}

	operator bool () const
	{
		return !isEmpty ();
	}

	bool
	operator == (const LlvmIrInsertPoint& insertPoint) const
	{
		return isEqual (insertPoint);
	}

	bool
	operator != (const LlvmIrInsertPoint& insertPoint) const
	{
		return !isEqual (insertPoint);
	}

	bool
	isEmpty () const
	{
		return m_llvmBlock == NULL;
	}

	bool
	isEqual (const LlvmIrInsertPoint& insertPoint) const
	{
		return 
			m_llvmBlock == insertPoint.m_llvmBlock &&
			m_llvmInstruction == insertPoint.m_llvmInstruction;
	}

	void
	clear ()
	{
		setup (NULL, NULL);
	}

	void
	setup (
		llvm::BasicBlock* llvmBlock,
		llvm::Instruction* llvmInstruction
		)
	{
		m_llvmBlock = llvmBlock;
		m_llvmInstruction = llvmInstruction;
	}
};

//.............................................................................

} // namespace ct
} // namespace jnc
