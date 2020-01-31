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
#include "jnc_ct_AsyncSequencerFunction.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

AsyncSequencerFunction::AsyncSequencerFunction()
{
	m_functionKind = FunctionKind_AsyncSequencer;
	m_promiseType = NULL;
	m_catchBlock = NULL;
}

bool
AsyncSequencerFunction::compile()
{
	ASSERT(m_parentUnit && m_parentNamespace);

	bool result;

	m_module->m_unitMgr.setCurrentUnit(m_parentUnit);
	m_module->m_namespaceMgr.openNamespace(m_parentNamespace);

	Value promiseValue;
	m_module->m_functionMgr.internalPrologue(this, &promiseValue, 1, &m_bodyPos);
	m_module->m_functionMgr.m_promiseValue = promiseValue;

	Scope* scope = m_module->m_namespaceMgr.openScope(
		m_bodyPos,
		ScopeFlag_CatchAhead | ScopeFlag_HasCatch
		);

	m_catchBlock = scope->m_catchBlock;

	ASSERT(!m_promiseType);
	m_promiseType = ((ClassPtrType*)promiseValue.getType())->getTargetType();

	const sl::Array<Field*>& promiseFieldArray = m_promiseType->getFieldArray();
	size_t argCount = promiseFieldArray.getCount();

	size_t i = 0;

	if (isMember())
	{
		// add this arg

		Field* argField = promiseFieldArray[0];
		Value argFieldValue;

		result = m_module->m_operatorMgr.getField(promiseValue, argField, &argFieldValue);
		ASSERT(result);

		m_module->m_llvmIrBuilder.createLoad(argFieldValue, m_thisType, &m_module->m_functionMgr.m_thisValue);

		if (m_thisType->getTypeKind() == TypeKind_DataPtr)
			m_module->m_operatorMgr.makeLeanDataPtr(&m_module->m_functionMgr.m_thisValue);

		i = 1;
	}

	// add arg fields to the scope

	scope->createConst("thisPromise", promiseValue);

	for (; i < argCount; i++)
	{
		Field* argField = promiseFieldArray[i];

		Value argFieldValue;
		result = m_module->m_operatorMgr.getField(promiseValue, argField, &argFieldValue);
		ASSERT(result);

		Variable* argVar = m_module->m_variableMgr.createAsyncArgVariable(
			argField->getName(),
			argField->getType(),
			argFieldValue
			);

		scope->addItem(argVar);
	}

	BasicBlock* asyncBlock0 = m_module->m_controlFlowMgr.createAsyncBlock(scope);
	BasicBlock* asyncBlock1 = m_module->m_controlFlowMgr.createAsyncBlock(scope);
	BasicBlock* switchBlock = m_module->m_controlFlowMgr.setCurrentBlock(asyncBlock0);

	// if we have a scheduler, re-schedule this async sequencer

	Value schedulerValue;
	Value resumeFuncValue;
	Value stateIdValue;
	Value stateValue;

	stateIdValue.setConstSizeT(1, m_module);

	BasicBlock* schedulerBlock = m_module->m_controlFlowMgr.createBlock("scheduler_block");

	result =
		m_module->m_operatorMgr.getPromiseField(promiseValue, "m_state", &stateValue) &&
		m_module->m_operatorMgr.storeDataRef(stateValue, stateIdValue);
		m_module->m_operatorMgr.getPromiseField(promiseValue, "m_scheduler", &schedulerValue) &&
		m_module->m_operatorMgr.loadDataRef(&schedulerValue) &&
		m_module->m_controlFlowMgr.conditionalJump(schedulerValue, schedulerBlock, asyncBlock1, schedulerBlock) &&
		m_module->m_operatorMgr.binaryOperator(BinOpKind_At, this, schedulerValue, &resumeFuncValue) &&
		m_module->m_operatorMgr.closureOperator(resumeFuncValue, promiseValue, &resumeFuncValue) &&
		m_module->m_operatorMgr.callOperator(resumeFuncValue);

	ASSERT(result);

	m_module->m_controlFlowMgr.asyncRet(asyncBlock1);

	// parse body

	Parser parser(m_module, Parser::Mode_Compile);

	result =
		parser.parseBody(SymbolKind_compound_stmt, m_bodyPos, m_body) &&
		m_module->m_controlFlowMgr.checkReturn();

	if (!result)
		return false;

	// extract state and switch-jump accordingly

	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock(switchBlock);

	result =
		m_module->m_operatorMgr.getPromiseField(promiseValue, "m_state", &stateValue) &&
		m_module->m_operatorMgr.loadDataRef(&stateValue);

	ASSERT(result);

	sl::Array<BasicBlock*> asyncBlockArray = m_module->m_controlFlowMgr.getAsyncBlockArray();
	size_t count = asyncBlockArray.getCount();

	char buffer[256];
	sl::Array<intptr_t> stateIdArray(ref::BufKind_Stack, buffer, sizeof(buffer));
	stateIdArray.setCount(count);

	for (size_t i = 0; i < count; i++)
		stateIdArray[i] = i;

	m_module->m_llvmIrBuilder.createSwitch(
		stateValue,
		m_catchBlock,
		stateIdArray,
		asyncBlockArray,
		count
		);

	// sync-catch and async-throw

	m_module->m_controlFlowMgr.setCurrentBlock(prevBlock);
	result = m_module->m_controlFlowMgr.catchLabel(m_module->m_namespaceMgr.getSourcePos());
	ASSERT(result);

	Function* throwFunc = m_module->m_functionMgr.getStdFunction(StdFunc_AsyncThrow);
	m_module->m_llvmIrBuilder.createGep2(promiseValue, 0, NULL, &promiseValue);
	m_module->m_llvmIrBuilder.createCall(throwFunc, throwFunc->getType(), promiseValue, NULL);
	m_module->m_controlFlowMgr.asyncRet(NULL);
	m_module->m_namespaceMgr.closeScope();
	m_module->m_functionMgr.internalEpilogue();
	m_module->m_namespaceMgr.closeNamespace();

	AsyncRegionCalc calc;
	calc.calcAsyncRegions(asyncBlockArray);
	calc.saveCrossAsyncRegionValues();
	return true;
}

void
AsyncSequencerFunction::replaceAllocas()
{
	// replace all alloca's with GEPs

	llvm::Function::arg_iterator llvmArg = m_llvmFunction->arg_begin();
	llvm::Value* llvmPromiseValue = &*llvmArg;
	llvm::BasicBlock* llvmAllocaBlock = m_allocaBlock->getLlvmBlock();
	llvm::BasicBlock::iterator it = llvmAllocaBlock->begin();
	llvm::DataLayout llvmDataLayout(m_module->getLlvmModule());

	Value bufferValue;
	m_module->m_llvmIrBuilder.setInsertPoint(&*it);
	m_module->m_llvmIrBuilder.createBitCast(llvmPromiseValue, m_module->m_typeMgr.getStdType(StdType_BytePtr), &bufferValue);

	size_t offset = m_promiseType->m_ifaceStructType->m_size;

	while (it != llvmAllocaBlock->end())
	{
		if (!llvm::isa<llvm::AllocaInst> (it))
		{
			it++;
			continue;
		}

		llvm::AllocaInst* llvmAlloca = (llvm::AllocaInst*) &*it++;
		llvm::Type* llvmPtrType = llvmAlloca->getType();
		llvm::Type* llvmType = llvmAlloca->getAllocatedType();
		size_t size = (size_t) llvmDataLayout.getTypeAllocSize(llvmType);
		size_t typeAlign = llvmDataLayout.getPrefTypeAlignment(llvmType);
		size_t allocaAlign = llvmAlloca->getAlignment();

		offset = sl::align(offset, AXL_MAX(typeAlign, allocaAlign));

		Value gepValue;
		m_module->m_llvmIrBuilder.setInsertPoint(llvmAlloca);
		m_module->m_llvmIrBuilder.createGep(bufferValue, offset, NULL, &gepValue);
		m_module->m_llvmIrBuilder.createBitCast(gepValue.getLlvmValue(), llvmPtrType, &gepValue);
		llvmAlloca->replaceAllUsesWith(gepValue.getLlvmValue());
		llvmAlloca->eraseFromParent();

		offset += size;
	}

	size_t delta = offset - m_promiseType->m_ifaceStructType->m_size;

	m_promiseType->m_ifaceStructType->m_size = offset;
	m_promiseType->m_classStructType->m_size += delta;
	m_promiseType->m_size += delta;
}

//..............................................................................

void
AsyncRegionCalc::calcAsyncRegions(const sl::ArrayRef<BasicBlock*>& asyncBlockArray)
{
	ASSERT(!asyncBlockArray.isEmpty());

	size_t count = asyncBlockArray.getCount();
	for (size_t i = 0; i < count; i++)
		addRegion(asyncBlockArray[i]->getLlvmBlock());

	sl::Array<llvm::BasicBlock*> frontArray[2];

	sl::Iterator<Region> it = m_regionList.getHead();
	for (; it; it++)
	{
		Region* region = *it;

		frontArray[0].copy(it->m_llvmEntryBlock);
		size_t srcIdx = 0;

		do
		{
			size_t dstIdx = !srcIdx;
			frontArray[dstIdx].clear();

			size_t count = frontArray[srcIdx].getCount();
			for (size_t i = 0; i < count; i++)
			{
				llvm::BasicBlock* llvmBlock = frontArray[srcIdx][i];
				llvm::TerminatorInst* llvmTermInst = llvmBlock->getTerminator();

				size_t count = llvmTermInst->getNumSuccessors();
				for (size_t i = 0; i < count; i++)
				{
					llvm::BasicBlock* llvmNextBlock = llvmTermInst->getSuccessor(i);
					sl::HashTableIterator<llvm::BasicBlock*, Region*> mapIt = m_regionMap.visit(llvmNextBlock);
					if (mapIt->m_value)
					{
						if (mapIt->m_value->m_llvmEntryBlock == llvmNextBlock) // a region entry
							continue;

						if (mapIt->m_value != region->m_prevRegion)
						{
							mapIt->m_value = addRegion(llvmNextBlock, mapIt->m_value);
							continue;
						}
					}

					mapIt->m_value = region;
					frontArray[dstIdx].append(llvmNextBlock);
				}
			}

			srcIdx = dstIdx;
		} while (!frontArray[srcIdx].isEmpty());
	}

#if (_JNC_DEBUG)
	Module* module = asyncBlockArray[0]->getModule();
	Type* mdType = module->m_typeMgr.getPrimitiveType(TypeKind_IntPtr);
	llvm::LLVMContext* llvmContext = module->getLlvmContext();
	uint_t mdKindId = llvmContext->getMDKindID("jnc.async-region");
	sl::HashTableIterator<llvm::BasicBlock*, Region*> it2 = m_regionMap.getHead();
	for (; it2; it2++)
	{
		Value regionIdValue(it2->m_value->m_id, mdType);
		llvm::MDNode* llvmMd = llvm::MDNode::get(*llvmContext, llvm::ArrayRef<llvm::Value*>(regionIdValue.getLlvmValue()));
		llvm::Instruction* llvmInst = it2->getKey()->begin();
		llvmInst->setMetadata(mdKindId, llvmMd);
	}
#endif
}

void
AsyncRegionCalc::saveCrossAsyncRegionValues()
{
}

AsyncRegionCalc::Region*
AsyncRegionCalc::addRegion(
	llvm::BasicBlock* llvmEntryBlock,
	Region* prevRegion
	)
{
	Region* region = AXL_MEM_NEW(Region);
	region->m_llvmEntryBlock = llvmEntryBlock;
	region->m_prevRegion = prevRegion;
#if (_JNC_DEBUG)
	region->m_id = m_regionList.getCount();
#endif

	m_regionList.insertTail(region);
	m_regionMap[llvmEntryBlock] = region;
	return region;
}

//..............................................................................

} // namespace ct
} // namespace jnc
