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

namespace jnc {
namespace ct {

//..............................................................................

void
ControlFlowMgr::enterReactor(
	ReactorClassType* reactorType,
	const Value& reactionIdxValue
) {
	m_reactorBody = new ReactorBody;
	m_reactorBody->m_reactorType = reactorType;
	m_reactorBody->m_reactionIdxValue = reactionIdxValue;
	m_reactorBody->m_reactorSwitchBlock = m_currentBlock;
	m_reactorBody->m_reactorBodyBlock = createBlock("reactor_body");
	m_reactorBody->m_reactorFollowBlock = createBlock("reactor_follow");
	m_reactorBody->m_reactionBlock = NULL;
	m_reactorBody->m_reactionBindingCount = 0;

	setCurrentBlock(m_reactorBody->m_reactorBodyBlock);
	m_reactorBody->m_reactorBodyBlock->m_flags |= BasicBlockFlag_Jumped | BasicBlockFlag_Reachable;
}

void
ControlFlowMgr::leaveReactor() {
	ASSERT(m_reactorBody);
	follow(m_reactorBody->m_reactorFollowBlock);
	setCurrentBlock(m_reactorBody->m_reactorSwitchBlock);

	size_t reactionCount = m_reactorBody->m_reactionBlockArray.getCount();

	if (m_module->hasCodeGen()) {
		char buffer[256];
		char buffer2[256];
		sl::Array<BasicBlock*> blockArray(rc::BufKind_Stack, buffer, sizeof(buffer));
		sl::Array<int64_t> caseIdArray(rc::BufKind_Stack, buffer2, sizeof(buffer2));
		blockArray.setCount(reactionCount);
		caseIdArray.setCount(reactionCount);

		sl::Array<BasicBlock*>::Rwi blockRwi = blockArray;
		sl::Array<int64_t>::Rwi caseRwi = caseIdArray;

		size_t caseCount = 0;
		for (size_t i = 0; i < reactionCount; i++) {
			BasicBlock* block = m_reactorBody->m_reactionBlockArray[i];
			if (!block)
				continue;

			blockRwi[caseCount] = block;
			caseRwi[caseCount] = i;
			caseCount++;
		}

		if (!caseCount)
			m_module->m_llvmIrBuilder.createBr(m_reactorBody->m_reactorBodyBlock);
		else
			m_module->m_llvmIrBuilder.createSwitch(
				m_reactorBody->m_reactionIdxValue,
				m_reactorBody->m_reactorBodyBlock,
				caseIdArray,
				blockArray,
				caseCount
			);
	}

	setCurrentBlock(m_reactorBody->m_reactorFollowBlock);
	m_reactorBody->m_reactorType->m_reactionCount = reactionCount;
	delete m_reactorBody;
	m_reactorBody = NULL;
}

bool
ControlFlowMgr::addOnEventHandler(
	const sl::BoxList<Value>& valueList,
	const sl::Array<FunctionArg*>& argArray,
	const PragmaConfig* pragmaConfig,
	sl::List<Token>* tokenList
) {
	ASSERT(m_reactorBody);

	FunctionType* functionType = argArray.isEmpty() ?
		m_module->m_typeMgr.getFunctionType(argArray) :
		m_module->m_typeMgr.createUserFunctionType(argArray);

	Function* handler = m_reactorBody->m_reactorType->createUnnamedMethod(FunctionKind_Internal, functionType);
	handler->m_parentUnit = m_module->m_unitMgr.getCurrentUnit();
    handler->m_flags |= ModuleItemFlag_User;
    handler->setBody(pragmaConfig, tokenList);

	size_t onEventIdx = m_reactorBody->m_reactionBlockArray.getCount();
    m_reactorBody->m_reactorType->addOnEventHandler(onEventIdx, handler);
	m_reactorBody->m_reactionBlockArray.append(NULL); // onevent occupies one reaction slot

	Function* addBindingFunc = getReactorMethod(m_module, ReactorMethod_AddOnEventBinding);
	Value thisValue = m_module->m_functionMgr.getThisValue();
	Value onEventIdxValue(onEventIdx, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT));

	sl::ConstBoxIterator<Value> it = valueList.getHead();
	for (; it; it++) {
		bool result = m_module->m_operatorMgr.callOperator(addBindingFunc, thisValue, onEventIdxValue, *it);
		if (!result)
			return false;
	}

	return true;
}

size_t
ControlFlowMgr::finalizeReactiveExpressionImpl() {
	ASSERT(m_reactorBody && m_reactorBody->m_reactionBlock);

	if (!m_module->hasCodeGen() || !m_reactorBody->m_reactionBindingCount) {
		m_reactorBody->m_reactionBlock = NULL;
		return -1; // no bindings, not reactive
	}

	BasicBlock* reactionBlock;

	if (!m_reactorBody->m_llvmReactionIt)
		reactionBlock = m_reactorBody->m_reactionBlock;
	else {
		llvm::BasicBlock* llvmBlock = m_reactorBody->m_reactionBlock->getLlvmBlock();
		if (!llvmBlock->getTerminator()) {
			// LLVM can't split incomplete blocks (but why?!) -- so we need to do a dummy follow
			BasicBlock* followBlock = createBlock("follow_block");
			follow(followBlock);
		}

		reactionBlock = new BasicBlock(m_module, "reaction_block");
		reactionBlock->m_function = m_reactorBody->m_reactionBlock->m_function;
		reactionBlock->m_llvmBlock = llvmBlock->splitBasicBlock(
			++m_reactorBody->m_llvmReactionIt,
			reactionBlock->m_name >> toLlvm
		);

		reactionBlock->m_flags |= BasicBlockFlag_Jumped | BasicBlockFlag_Reachable;
		m_blockList.insertTail(reactionBlock);
	}

	size_t reactionIdx = m_reactorBody->m_reactionBlockArray.getCount();
	m_reactorBody->m_reactionBlockArray.append(reactionBlock);
	m_reactorBody->m_reactionBlock = NULL;
	m_reactorBody->m_llvmReactionIt = NULL;
	return reactionIdx;
}

void
ControlFlowMgr::finalizeReactiveStmt(size_t reactionIdx) {
	ASSERT(m_reactorBody);

	Function* enterReactiveStmtFunc = getReactorMethod(m_module, ReactorMethod_EnterReactiveStmt);
	Type* sizeType = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
	BasicBlock* reactionBlock = m_reactorBody->m_reactionBlockArray[reactionIdx];
	ASSERT(reactionBlock);

	BasicBlock* prevBlock = setCurrentBlock(reactionBlock);
	m_module->m_llvmIrBuilder.setInsertPoint(&*reactionBlock->getLlvmBlock()->begin());
	m_module->m_operatorMgr.callOperator(
		enterReactiveStmtFunc,
		m_module->m_functionMgr.getThisValue(),
		Value(reactionIdx, sizeType),
		Value(getReactionIdx(), sizeType)
	);

	setCurrentBlock(prevBlock);
	finalizeReaction(reactionIdx);
}

void
ControlFlowMgr::finalizeReaction(size_t reactionIdx) {
	Value cmpValue;

	bool result = m_module->m_operatorMgr.binaryOperator(
		BinOpKind_Eq,
		m_reactorBody->m_reactionIdxValue,
		Value(reactionIdx, m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT)),
		&cmpValue
	);

	ASSERT(result);

	BasicBlock* followBlock = createBlock("follow_block");

	result = conditionalJump(
		cmpValue,
		m_reactorBody->m_reactorFollowBlock,
		followBlock,
		followBlock
	);

	ASSERT(result);
}

//..............................................................................

} // namespace ct
} // namespace jnc
