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

#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

//..............................................................................

class AsyncSequencerFunction: public CompilableFunction
{
	friend class AsyncLauncherFunction;

protected:
	ClassType* m_promiseType;
	BasicBlock* m_catchBlock;

public:
	AsyncSequencerFunction();

	Type*
	getAsyncReturnType()
	{
		return m_asyncLauncher->getType()->getAsyncReturnType();
	}

	ClassType*
	getPromiseType()
	{
		return m_promiseType;
	}

	BasicBlock*
	getCatchBlock()
	{
		return m_catchBlock;
	}

	virtual
	bool
	compile();

	void
	replaceAllocas();
};

//..............................................................................

class AsyncRegionCalc
{
protected:
	struct Region: sl::ListLink
	{
		llvm::BasicBlock* m_llvmEntryBlock;
		Region* m_prevRegion;
#if (_JNC_DEBUG)
		size_t m_id;
#endif
	};

protected:
	sl::List<Region> m_regionList;
	sl::SimpleHashTable<llvm::BasicBlock*, Region*> m_regionMap;

public:
	void
	calcAsyncRegions(const sl::ArrayRef<BasicBlock*>& asyncBlockArray);

	void
	saveCrossAsyncRegionValues();

protected:
	Region*
	addRegion(
		llvm::BasicBlock* llvmEntryBlock,
		Region* prevRegion = NULL
		);

	bool
	isAsyncRegionEntry(llvm::BasicBlock*);

	bool
	markAsyncRegionEntry(llvm::BasicBlock*);
};

//..............................................................................

} // namespace ct
} // namespace jnc
