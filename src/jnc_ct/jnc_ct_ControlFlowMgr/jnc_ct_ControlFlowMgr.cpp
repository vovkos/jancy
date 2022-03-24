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

#include "pch.h"
#include "jnc_ct_ControlFlowMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_AsyncSequencerFunction.h"

namespace jnc {
namespace ct {

//..............................................................................

ControlFlowMgr::ControlFlowMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_currentBlock = NULL;
	m_unreachableBlock = NULL;
	m_returnBlock = NULL;
	m_dynamicThrowBlock = NULL;
	m_catchFinallyFollowBlock = NULL;
	m_emissionLockBlock = NULL;
	m_finallyRouteIdxVariable = NULL;
	m_returnValueVariable = NULL;
	m_emissionLockCount = 0;
	m_finallyRouteIdx = -1;
	m_sjljFrameCount = 0;
}

void
ControlFlowMgr::clear() {
	m_blockList.clear();
	m_asyncBlockArray.clear();
	m_returnBlockArray.clear();
	m_landingPadBlockArray.clear();
	m_currentBlock = NULL;
	m_unreachableBlock = NULL;
	m_catchFinallyFollowBlock = NULL;
	m_returnBlock = NULL;
	m_dynamicThrowBlock = NULL;
	m_emissionLockBlock = NULL;
	m_finallyRouteIdxVariable = NULL;
	m_returnValueVariable = NULL;
	m_emissionLockCount = 0;
	m_finallyRouteIdx = -1;
	m_sjljFrameCount = 0;
	m_sjljFrameArrayValue.clear();
	m_prevSjljFrameValue.clear();
}

void
ControlFlowMgr::lockEmission() {
	if (++m_emissionLockCount != 1)
		return;

	ASSERT(!m_emissionLockBlock);
	m_emissionLockBlock = setCurrentBlock(getUnreachableBlock());
}

void
ControlFlowMgr::unlockEmission() {
	if (--m_emissionLockCount)
		return;

	ASSERT(m_emissionLockBlock);
	setCurrentBlock(m_emissionLockBlock);
	m_emissionLockBlock = NULL;
}

void
ControlFlowMgr::finalizeFunction() {
	if (m_sjljFrameArrayValue && m_module->hasCodeGen()) {
		// the EH in Jancy is hand-rolled SJLJ (with no use of the LLVM EH facilities)
		// LLVM optimizer respects LandingPad instructions; otherwise, generates broken code
		// fix: disable optimizations in functions with landing pads (until redesign of the Jancy EH)

		llvm::Function* llvmFunction = m_module->m_functionMgr.getCurrentFunction()->getLlvmFunction();
		llvmFunction->addFnAttr(llvm::Attribute::NoInline);
		llvmFunction->addFnAttr(llvm::Attribute::OptimizeNone);

		finalizeSjljFrameArray();
	}

	m_asyncBlockArray.clear();
	m_returnBlockArray.clear();
	m_landingPadBlockArray.clear();
	m_currentBlock = NULL;
	m_unreachableBlock = NULL;
	m_catchFinallyFollowBlock = NULL;
	m_returnBlock = NULL;
	m_dynamicThrowBlock = NULL;
	m_finallyRouteIdxVariable = NULL;
	m_returnValueVariable = NULL;
	m_finallyRouteIdx = -1;
	m_sjljFrameArrayValue.clear();
	m_prevSjljFrameValue.clear();
}

BasicBlock*
ControlFlowMgr::createBlock(
	const sl::StringRef& name,
	uint_t flags
) {
	BasicBlock* block = AXL_MEM_NEW(BasicBlock);
	block->m_module = m_module;
	block->m_name = name;
	block->m_flags = flags;

	if (m_module->hasCodeGen())
		block->m_llvmBlock = llvm::BasicBlock::Create(
			*m_module->getLlvmContext(),
			name >> toLlvm,
			NULL
		);

	m_blockList.insertTail(block);
	return block;
}

BasicBlock*
ControlFlowMgr::createAsyncBlock(Scope* scope) {
	BasicBlock* block = createBlock("async_block");
	block->m_flags |= BasicBlockFlag_Jumped | BasicBlockFlag_Reachable | BasicBlockFlag_AsyncLandingPad;
	block->m_landingPadScope = scope;
	m_landingPadBlockArray.append(block);
	m_asyncBlockArray.append(block);
	return block;
}

BasicBlock*
ControlFlowMgr::setCurrentBlock(BasicBlock* block) {
	if (m_currentBlock == block)
		return block;

	BasicBlock* prevCurrentBlock = m_currentBlock;
	m_currentBlock = block;

	if (!m_module->hasCodeGen())
		return prevCurrentBlock;

	if (prevCurrentBlock)
		prevCurrentBlock->m_llvmDebugLoc = m_module->m_llvmIrBuilder.getCurrentDebugLoc();

	if (!block)
		return prevCurrentBlock;

	if (!block->m_function) {
		Function* function = m_module->m_functionMgr.getCurrentFunction();
		ASSERT(function);

		function->getLlvmFunction()->getBasicBlockList().push_back(block->m_llvmBlock);
		block->m_function = function;
	}

	m_module->m_llvmIrBuilder.setInsertPoint(block);

#if (LLVM_VERSION < 0x030700)
	bool hasDebugLoc = !block->m_llvmDebugLoc.isUnknown();
#else
	bool hasDebugLoc = (bool)block->m_llvmDebugLoc;
#endif

	if (hasDebugLoc)
		m_module->m_llvmIrBuilder.setCurrentDebugLoc(block->m_llvmDebugLoc);

	return prevCurrentBlock;
}

bool
ControlFlowMgr::deleteUnreachableBlocks() {
	// llvm blocks may have references even when they are unreachable

	sl::List<BasicBlock> pendingDeleteList;

	sl::Iterator<BasicBlock> it = m_blockList.getHead();
	while (it) {
		BasicBlock* block = *it;
		it++;

		if (!(block->m_flags & BasicBlockFlag_Reachable)) {
			m_blockList.remove(block);

			if (!block->m_llvmBlock->use_empty()) {
				pendingDeleteList.insertTail(block);
			} else {
				if (block->m_function)
					block->m_llvmBlock->eraseFromParent();
				else
					delete block->m_llvmBlock;

				AXL_MEM_DELETE(block);
			}
		}
	}

	m_unreachableBlock = NULL;
	m_currentBlock = NULL;

	while (!pendingDeleteList.isEmpty()) {
		bool isFixedPoint = true;

		it = pendingDeleteList.getHead();
		while (it) {
			BasicBlock* block = *it;
			it++;

			if (!block->m_llvmBlock->use_empty())
				continue;

			if (block->m_function)
				block->m_llvmBlock->eraseFromParent();
			else
				delete block->m_llvmBlock;

			pendingDeleteList.erase(block);
			isFixedPoint = false;
		}

		if (isFixedPoint && !pendingDeleteList.isEmpty()) {
			err::setFormatStringError(
				"invalid control flow graph: %s is unreachable but has uses",
				pendingDeleteList.getHead()->m_llvmBlock->getName().begin()
			);
			return false;
		}
	}

	return true;
}

#if (_JNC_DEBUG)
void
ControlFlowMgr::traceAllBlocks() {
	sl::Iterator<BasicBlock> it = m_blockList.getHead();
	for (; it; it++)
		m_module->m_operatorMgr.traceBlock(*it);
}
#endif

BasicBlock*
ControlFlowMgr::getUnreachableBlock() {
	if (m_unreachableBlock)
		return m_unreachableBlock;

	m_unreachableBlock = createBlock("unreachable_block");
	markUnreachable(m_unreachableBlock);
	return m_unreachableBlock;
}

BasicBlock*
ControlFlowMgr::getReturnBlock() {
	if (m_returnBlock)
		return m_returnBlock;

	m_returnBlock = createBlock("return_block");
	BasicBlock* prevBlock = setCurrentBlock(m_returnBlock);

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	FunctionType* functionType = function->getType();

	if (function->getFunctionKind() == FunctionKind_AsyncSequencer) {
		Type* returnType = function->getAsyncLauncher()->getType()->getAsyncReturnType();
		Value returnValue = returnType->getTypeKind() == TypeKind_Void ?
			m_module->m_typeMgr.getPrimitiveType(TypeKind_Variant)->getZeroValue() :
			getReturnValueVariable();

		Function* retFunc = m_module->m_functionMgr.getStdFunction(StdFunc_AsyncRet);
		Value promiseValue = m_module->m_functionMgr.getPromiseValue();
		bool result = m_module->m_operatorMgr.callOperator(retFunc, promiseValue, returnValue);
		ASSERT(result);

		m_module->m_llvmIrBuilder.createRet();
	} else if (functionType->getReturnType()->getTypeKind() == TypeKind_Void) {
		m_module->m_llvmIrBuilder.createRet();
	} else {
		Value returnValue;
		m_module->m_llvmIrBuilder.createLoad(getReturnValueVariable(), NULL, &returnValue);
		functionType->getCallConv()->ret(function, returnValue);
	}

	m_currentBlock->m_flags |= BasicBlockFlag_Return;
	m_returnBlockArray.append(m_currentBlock);

	setCurrentBlock(prevBlock);
	return m_returnBlock;
}

Variable*
ControlFlowMgr::getReturnValueVariable() {
	if (m_returnValueVariable)
		return m_returnValueVariable;

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	Type* returnType = function->getFunctionKind() == FunctionKind_AsyncSequencer ?
		function->getAsyncLauncher()->getType()->getAsyncReturnType() :
		function->getType()->getReturnType();

	ASSERT(returnType->getTypeKind() != TypeKind_Void);

	BasicBlock* prevBlock = setCurrentBlock(function->getPrologueBlock());
	m_returnValueVariable = m_module->m_variableMgr.createSimpleStackVariable("savedReturnValue", returnType);
	setCurrentBlock(prevBlock);
	return m_returnValueVariable;
}

void
ControlFlowMgr::markUnreachable(BasicBlock* block) {
	if (!m_module->hasCodeGen())
		return;

	ASSERT(block->isEmpty() && block->m_flags == 0);
	BasicBlock* prevCurrentBlock = setCurrentBlock(block);
	m_module->m_llvmIrBuilder.createUnreachable();
	setCurrentBlock(prevCurrentBlock);
}

void
ControlFlowMgr::jump(
	BasicBlock* block,
	BasicBlock* followBlock
) {
	block->m_flags |= BasicBlockFlag_Jumped | (m_currentBlock->m_flags & BasicBlockFlag_Reachable);

	if (m_module->hasCodeGen())
		m_module->m_llvmIrBuilder.createBr(block);

	if (!followBlock)
		followBlock = getUnreachableBlock();

	setCurrentBlock(followBlock);
}

void
ControlFlowMgr::follow(BasicBlock* block) {
	if (m_module->hasCodeGen() && !m_currentBlock->hasTerminator()) {
		m_module->m_llvmIrBuilder.createBr(block);
		block->m_flags |= BasicBlockFlag_Jumped | (m_currentBlock->m_flags & BasicBlockFlag_Reachable);
	}

	setCurrentBlock(block);
}

bool
ControlFlowMgr::conditionalJump(
	const Value& value,
	BasicBlock* thenBlock,
	BasicBlock* elseBlock,
	BasicBlock* followBlock
) {
	Value boolValue;
	bool result = m_module->m_operatorMgr.castOperator(value, TypeKind_Bool, &boolValue);
	if (!result)
		return false;

	uint_t reachableFlag = (m_currentBlock->m_flags & BasicBlockFlag_Reachable);

	thenBlock->m_flags |= BasicBlockFlag_Jumped | reachableFlag;
	elseBlock->m_flags |= BasicBlockFlag_Jumped | reachableFlag;

	if (m_module->hasCodeGen())
		m_module->m_llvmIrBuilder.createCondBr(boolValue, thenBlock, elseBlock);

	if (!followBlock)
		followBlock = thenBlock;

	setCurrentBlock(followBlock);
	return true;
}

bool
ControlFlowMgr::breakJump(size_t level) {
	Scope* targetScope = m_module->m_namespaceMgr.findBreakScope(level);
	if (!targetScope) {
		err::setFormatStringError("illegal break");
		return false;
	}

	escapeScope(targetScope, targetScope->m_breakBlock);
	return true;
}

bool
ControlFlowMgr::continueJump(size_t level) {
	Scope* targetScope = m_module->m_namespaceMgr.findContinueScope(level);
	if (!targetScope) {
		err::setFormatStringError("illegal continue");
		return false;
	}

	escapeScope(targetScope, targetScope->m_continueBlock);
	return true;
}

void
ControlFlowMgr::escapeScope(
	Scope* targetScope,
	BasicBlock* targetBlock
) {
	if (!m_module->hasCodeGen())
		return;

	size_t routeIdx = ++m_finallyRouteIdx;

	BasicBlock* firstFinallyBlock = NULL;
	BasicBlock* prevFinallyBlock = NULL;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	while (scope && scope != targetScope) {
		if (scope->m_flags & ScopeFlag_FinallyAhead) {
			ASSERT(scope->m_finallyBlock);

			if (!firstFinallyBlock) {
				firstFinallyBlock = scope->m_finallyBlock;
			} else {
				ASSERT(prevFinallyBlock);
				prevFinallyBlock->m_finallyRouteMap[routeIdx] = scope->m_finallyBlock;
			}

			prevFinallyBlock = scope->m_finallyBlock;
		}

		scope = scope->getParentScope();
	}

	if (!targetBlock) { // escape before normal return
		ASSERT(!firstFinallyBlock); // if we have finally then we return via return-block

		scope = m_module->m_namespaceMgr.getCurrentScope();
		if (scope->m_sjljFrameIdx != -1)
			setSjljFrame(-1);

		return;
	}

	markLandingPad(targetBlock, targetScope, BasicBlockFlag_EscapeScopeLandingPad);

	if (!firstFinallyBlock) {
		jump(targetBlock);
		return;
	}

	prevFinallyBlock->m_finallyRouteMap[routeIdx] = targetBlock;

	Variable* routeIdxVariable = getFinallyRouteIdxVariable();
	Value routeIdxValue(routeIdx, m_module->m_typeMgr.getPrimitiveType(TypeKind_IntPtr));
	m_module->m_llvmIrBuilder.createStore(routeIdxValue, routeIdxVariable);

	jump(firstFinallyBlock);
}

void
ControlFlowMgr::asyncRet(BasicBlock* nextBlock) {
	if (nextBlock) {
		ASSERT(m_module->m_namespaceMgr.getCurrentScope()->m_sjljFrameIdx != -1);
		setSjljFrame(-1);
	}

	m_module->m_llvmIrBuilder.createRet();

	ASSERT(!(m_currentBlock->m_flags & BasicBlockFlag_Return));
	m_currentBlock->m_flags |= BasicBlockFlag_Return;
	m_returnBlockArray.append(m_currentBlock);
	setCurrentBlock(nextBlock ? nextBlock : getUnreachableBlock());
}

bool
ControlFlowMgr::ret(const Value& value) {
	Function* function = m_module->m_functionMgr.getCurrentFunction();
	ASSERT(function);

	bool isAsync = function->getFunctionKind() == FunctionKind_AsyncSequencer;
	FunctionType* functionType = function->getType();

	Type* returnType = isAsync ?
		function->getAsyncLauncher()->getType()->getAsyncReturnType() :
		functionType->getReturnType();

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	ASSERT(scope);

	if (!value) {
		if (returnType->getTypeKind() != TypeKind_Void) {
			err::setFormatStringError(
				"function '%s' must return '%s' value",
				function->getQualifiedName().sz(),
				returnType->getTypeString().sz()
			);
			return false;
		}

		if ((scope->m_flags & ScopeFlag_Finalizable) || isAsync) {
			escapeScope(function->getScope(), getReturnBlock());
			return true;
		}

		escapeScope(NULL, NULL);

		if (m_module->hasCodeGen())
			m_module->m_llvmIrBuilder.createRet();
	} else {
		if (returnType->getTypeKind() == TypeKind_Void) {
			err::setFormatStringError(
				"void function '%s' returning '%s' value",
				function->getQualifiedName().sz(),
				value.getType()->getTypeString().sz()
			);
			return false;
		}

		Value returnValue;
		bool result = m_module->m_operatorMgr.castOperator(value, returnType, &returnValue);
		if (!result)
			return false;

		if (isAsync && (functionType->getFlags() & FunctionTypeFlag_AsyncErrorCode))
			checkErrorCode(
				returnValue,
				returnType,
				((AsyncSequencerFunction*) function)->getCatchBlock()
			);

		if ((scope->getFlags() & ScopeFlag_Finalizable) || isAsync) {
			if (!m_module->hasCodeGen())
				return true;

			m_module->m_llvmIrBuilder.createStore(returnValue, getReturnValueVariable());
			escapeScope(function->getScope(), getReturnBlock());
			return true;
		}

		escapeScope(NULL, NULL);

		if (m_module->hasCodeGen())
			functionType->getCallConv()->ret(function, returnValue);
	}

	ASSERT(!(m_currentBlock->m_flags & BasicBlockFlag_Return));
	m_currentBlock->m_flags |= BasicBlockFlag_Return;
	m_returnBlockArray.append(m_currentBlock);
	setCurrentBlock(getUnreachableBlock());
	return true;
}

bool
ControlFlowMgr::checkReturn() {
	if (!m_module->hasCodeGen() || m_currentBlock->hasTerminator())
		return true;

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	Type* returnType;

	if (function->getFunctionKind() == FunctionKind_AsyncSequencer) {
		function = function->getAsyncLauncher();
		returnType = function->getType()->getAsyncReturnType();
	} else {
		returnType = function->getType()->getReturnType();
	}

	if (!(m_currentBlock->m_flags & BasicBlockFlag_Reachable)) {
		m_module->m_llvmIrBuilder.createUnreachable(); // just to make LLVM happy
	} else if (returnType->getTypeKind() == TypeKind_Void) {
		ret();
	} else if (m_returnBlockArray.isEmpty()) {
		err::setFormatStringError(
			"function '%s' must return '%s' value",
			function->getQualifiedName().sz(),
			returnType->getTypeString().sz()
		);
		return false;
	} else {
		err::setFormatStringError(
			"not all control paths in function '%s' return a value",
			function->getQualifiedName().sz()
		);
		return false;
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
