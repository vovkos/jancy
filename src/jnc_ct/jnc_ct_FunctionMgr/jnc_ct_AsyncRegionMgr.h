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

#include "jnc_ct_Pch.h"
#include "jnc_Def.h"

namespace jnc {
namespace ct {

class Module;
class BasicBlock;

//..............................................................................

class AsyncRegionMgr
{
protected:
	struct Region: sl::ListLink
	{
		llvm::BasicBlock* m_llvmEntryBlock;
		Region* m_parentRegion;
#if (_JNC_DEBUG)
		size_t m_id;
#endif
	};

protected:
	Module* m_module;
	sl::List<Region> m_regionList;
	sl::SimpleHashTable<llvm::BasicBlock*, Region*> m_basicBlockMap;
	sl::SimpleHashTable<llvm::Instruction*, llvm::AllocaInst*> m_crossRegionValueMap;

public:
	AsyncRegionMgr(Module* module)
	{
		m_module = module;
	}

	void
	calcRegions(const sl::ArrayRef<BasicBlock*>& asyncBlockArray);

	void
	saveCrossRegionValues();

protected:
	Region*
	createRegion(
		llvm::BasicBlock* llvmEntryBlock,
		Region* parentRegion = NULL
		);

	void
	saveCrossRegionValue(
		llvm::Instruction* llvmOpInst,
		llvm::Instruction* llvmTargetInst,
		size_t opIdx
		);
};


//..............................................................................

} // namespace ct
} // namespace jnc
