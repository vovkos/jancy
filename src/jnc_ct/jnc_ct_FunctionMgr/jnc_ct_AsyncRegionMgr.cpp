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
#include "jnc_ct_AsyncRegionMgr.h"
#include "jnc_ct_Module.h"

#if (_JNC_DEBUG)
//#	define _JNC_ADD_ASYNC_REGION_MD      1
//#	define _JNC_TRACE_ASYNC_REGION_VALUE 1
#endif

#if (_JNC_TRACE_ASYNC_REGION_VALUE)
#	define JNC_TRACE_ASYNC_REGION_VALUE TRACE
#else
#	define JNC_TRACE_ASYNC_REGION_VALUE (void)
#endif

namespace jnc {
namespace ct {

//..............................................................................

void
AsyncRegionMgr::calcRegions(const sl::ArrayRef<BasicBlock*>& asyncBlockArray) {
	ASSERT(!asyncBlockArray.isEmpty() && m_module == asyncBlockArray[0]->getModule());

	size_t count = asyncBlockArray.getCount();
	for (size_t i = 0; i < count; i++)
		createRegion(asyncBlockArray[i]->getLlvmBlock());

	sl::Array<llvm::BasicBlock*> frontArray[2];

	sl::Iterator<Region> regionIt = m_regionList.getHead();
	for (; regionIt; regionIt++) {
		frontArray[0].copy(regionIt->m_llvmEntryBlock);
		size_t srcIdx = 0;

		do {
			size_t dstIdx = !srcIdx;
			frontArray[dstIdx].clear();

			size_t count = frontArray[srcIdx].getCount();
			for (size_t i = 0; i < count; i++) {
				llvm::BasicBlock* llvmBlock = frontArray[srcIdx][i];
				llvm::TerminatorInst* llvmTermInst = llvmBlock->getTerminator();

				size_t count = llvmTermInst->getNumSuccessors();
				for (size_t i = 0; i < count; i++) {
					llvm::BasicBlock* llvmNextBlock = llvmTermInst->getSuccessor(i);
					sl::HashTableIterator<llvm::BasicBlock*, Region*> mapIt = m_basicBlockMap.visit(llvmNextBlock);
					if (mapIt->m_value) {
						if (mapIt->m_value == *regionIt || // jump to the same region -- done
							mapIt->m_value->m_llvmEntryBlock == llvmNextBlock) // jump to another region entry -- done
							continue;

						if (mapIt->m_value != regionIt->m_parentRegion) { // jump to another region interior -- create a new subregion
							mapIt->m_value = createRegion(llvmNextBlock, mapIt->m_value);
							continue;
						}
					}

					// else, keep flood-filling the region

					mapIt->m_value = *regionIt;
					frontArray[dstIdx].append(llvmNextBlock);
				}
			}

			srcIdx = dstIdx;
		} while (!frontArray[srcIdx].isEmpty());
	}

#if (_JNC_ADD_ASYNC_REGION_MD)
	Type* mdType = m_module->m_typeMgr.getPrimitiveType(TypeKind_IntPtr);
	llvm::LLVMContext* llvmContext = m_module->getLlvmContext();
	uint_t mdKindId = llvmContext->getMDKindID("jnc.async-region");
	sl::HashTableIterator<llvm::BasicBlock*, Region*> blockIt = m_basicBlockMap.getHead();
	for (; blockIt; blockIt++) {
		Value regionIdValue(blockIt->m_value->m_id, mdType);
		llvm::MDNode* llvmMd = llvm::MDNode::get(*llvmContext, llvm::ArrayRef<llvm::Value*>(regionIdValue.getLlvmValue()));
		llvm::Instruction* llvmInst = blockIt->getKey()->begin();
		llvmInst->setMetadata(mdKindId, llvmMd);
	}
#endif
}

void
AsyncRegionMgr::preserveCrossRegionValues() {
	sl::HashTableIterator<llvm::BasicBlock*, Region*> blockIt = m_basicBlockMap.getHead();
	for (; blockIt; blockIt++) {
		llvm::BasicBlock* llvmBlock = blockIt->getKey();
		llvm::BasicBlock::iterator instIt = llvmBlock->begin();
		for (; instIt != llvmBlock->end(); instIt++) {
			llvm::Instruction* llvmInst = &*instIt;
			size_t opCount = llvmInst->getNumOperands();
			for (size_t i = 0; i < opCount; i++) {
				llvm::Value* llvmOp = llvmInst->getOperand(i);
				if (!llvm::isa<llvm::Instruction>(llvmOp))
					continue;

				llvm::Instruction* llvmOpInst = (llvm::Instruction*)llvmOp;
				llvm::BasicBlock* llvmOpBlock = llvmOpInst->getParent();
				Region* opRegion = m_basicBlockMap.findValue(llvmOpBlock, NULL);
				if (opRegion && opRegion != blockIt->m_value)
					preserveCrossRegionValue(llvmOpInst, llvmInst, i);
			}
		}
	}
}

AsyncRegionMgr::Region*
AsyncRegionMgr::createRegion(
	llvm::BasicBlock* llvmEntryBlock,
	Region* parentRegion
) {
	Region* region = AXL_MEM_NEW(Region);
	region->m_llvmEntryBlock = llvmEntryBlock;
	region->m_parentRegion = parentRegion;
#if (_JNC_ADD_ASYNC_REGION_MD)
	region->m_id = m_regionList.getCount();
#endif

	m_regionList.insertTail(region);
	m_basicBlockMap[llvmEntryBlock] = region;
	return region;
}

void
AsyncRegionMgr::preserveCrossRegionValue(
	llvm::Instruction* llvmOpInst,
	llvm::Instruction* llvmTargetInst,
	size_t opIdx
) {
	LlvmIrBuilderImpl* llvmIrBuilder = m_module->m_llvmIrBuilder.getLlvmIrBuilder();
	LlvmIrBuilderImpl* llvmAllocaIrBuilder = m_module->m_llvmIrBuilder.getLlvmAllocaIrBuilder();

	llvm::AllocaInst* llvmAlloca;

	sl::HashTableIterator<llvm::Instruction*, llvm::AllocaInst*> it = m_crossRegionValueMap.visit(llvmOpInst);
	if (it->m_value) {
		llvmAlloca = it->m_value;

		JNC_TRACE_ASYNC_REGION_VALUE(
			"Re-using preserved cross-async-region value: %s\n",
			getLlvmInstructionString(llvmAlloca).sz()
		);
	} else {
		llvmAlloca = llvmAllocaIrBuilder->CreateAlloca(llvmOpInst->getType());

		JNC_TRACE_ASYNC_REGION_VALUE(
			"Preseving cross-async-region value: %s\n  into: %s\n",
			getLlvmInstructionString(llvmOpInst).sz(),
			getLlvmInstructionString(llvmAlloca).sz()
		);

		llvm::Instruction* llvmNextInst = &*++llvm::BasicBlock::iterator(llvmOpInst);
		ASSERT(llvmNextInst); // each block must have a terminator, which is never an operand
		llvmIrBuilder->SetInsertPoint(llvmNextInst);
		llvmIrBuilder->CreateStore(llvmOpInst, llvmAlloca);
		it->m_value = llvmAlloca;

		// note, that there is no need to add extra GC roots: all
		// necessary roots should already be there -- regardless of await-s
	}

	llvmIrBuilder->SetInsertPoint(llvmTargetInst);
	llvm::LoadInst* llvmLoad = llvmIrBuilder->CreateLoad(llvmAlloca);
	llvmTargetInst->setOperand(opIdx, llvmLoad);
}

//..............................................................................

} // namespace ct
} // namespace jnc
