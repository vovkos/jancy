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

namespace jnc {
namespace ct {

class BasicBlock;

//..............................................................................

struct LlvmIrInsertPoint {
	llvm::BasicBlock* m_llvmBlock;
	llvm::Instruction* m_llvmInstruction;

	LlvmIrInsertPoint() {
		clear();
	}

	LlvmIrInsertPoint(
		llvm::BasicBlock* llvmBlock,
		llvm::Instruction* llvmInstruction = NULL
	) {
		setup(llvmBlock, llvmInstruction);
	}

	operator bool() const {
		return !isEmpty();
	}

	bool
	operator == (const LlvmIrInsertPoint& insertPoint) const {
		return isEqual(insertPoint);
	}

	bool
	operator != (const LlvmIrInsertPoint& insertPoint) const {
		return !isEqual(insertPoint);
	}

	bool
	isEmpty() const {
		return m_llvmBlock == NULL;
	}

	bool
	isEqual(const LlvmIrInsertPoint& insertPoint) const {
		return
			m_llvmBlock == insertPoint.m_llvmBlock &&
			m_llvmInstruction == insertPoint.m_llvmInstruction;
	}

	void
	clear() {
		setup(NULL, NULL);
	}

	void
	setup(
		llvm::BasicBlock* llvmBlock,
		llvm::Instruction* llvmInstruction
	) {
		m_llvmBlock = llvmBlock;
		m_llvmInstruction = llvmInstruction;
	}
};

//..............................................................................

} // namespace ct
} // namespace jnc
