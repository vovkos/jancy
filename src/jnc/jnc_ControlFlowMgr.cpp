#include "pch.h"
#include "jnc_ControlFlowMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

ControlFlowMgr::ControlFlowMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_flags = 0;
	m_throwLockCount = 0;
	m_currentBlock = NULL;
	m_unreachableBlock = NULL;
}

void
ControlFlowMgr::clear ()
{
	m_flags = 0;
	m_throwLockCount = 0;
	m_blockList.clear ();
	m_returnBlockArray.clear ();
	m_currentBlock = NULL;
	m_unreachableBlock = NULL;
}

BasicBlock*
ControlFlowMgr::createBlock (const rtl::String& name)
{
	BasicBlock* block = AXL_MEM_NEW (BasicBlock);
	block->m_module = m_module;
	block->m_name = name;
	block->m_llvmBlock = llvm::BasicBlock::Create (
		*m_module->getLlvmContext (),
		(const char*) name,
		NULL
		);

	m_blockList.insertTail (block);
	return block;
}

BasicBlock*
ControlFlowMgr::setCurrentBlock (BasicBlock* block)
{
	if (m_currentBlock == block)
		return block;

	BasicBlock* prevCurrentBlock = m_currentBlock;
	if (prevCurrentBlock)
		prevCurrentBlock->m_llvmDebugLoc = m_module->m_llvmIrBuilder.getCurrentDebugLoc ();

	m_currentBlock = block;
	if (!block)
		return prevCurrentBlock;

	if (!block->m_function)
		addBlock (block);

	m_module->m_llvmIrBuilder.setInsertPoint (block);

	if (!block->m_llvmDebugLoc.isUnknown ())
		m_module->m_llvmIrBuilder.setCurrentDebugLoc (block->m_llvmDebugLoc);

	return prevCurrentBlock;
}

void
ControlFlowMgr::addBlock (BasicBlock* block)
{
	ASSERT (!block->m_function);

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	function->getLlvmFunction ()->getBasicBlockList ().push_back (block->m_llvmBlock);
	block->m_function = function;
}

bool
ControlFlowMgr::deleteUnreachableBlocks ()
{
	// llvm blocks might have references even when they are unreachable

	rtl::StdList <BasicBlock> pendingDeleteList;

	rtl::Iterator <BasicBlock> it = m_blockList.getHead ();
	while (it)
	{
		BasicBlock* block = *it;
		it++;

		if (!(block->m_flags & BasicBlockFlag_Reachable))
		{
			m_blockList.remove (block);

			if (block->m_llvmBlock->hasOneUse ())
			{
				pendingDeleteList.insertTail (block);
			}
			else
			{
				if (block->m_function)
					block->m_llvmBlock->eraseFromParent ();
				else
					delete block->m_llvmBlock;

				AXL_MEM_DELETE (block);
			}
		}
	}

	m_unreachableBlock = NULL;
	m_currentBlock = NULL;

	while (!pendingDeleteList.isEmpty ())
	{
		bool isFixedPoint = true;

		it = pendingDeleteList.getHead ();
		while (it)
		{
			BasicBlock* block = *it;
			it++;

			if (block->m_llvmBlock->hasOneUse ())
				continue;				

			if (block->m_function)
				block->m_llvmBlock->eraseFromParent ();
			else
				delete block->m_llvmBlock;

			pendingDeleteList.erase (block);
			isFixedPoint = false;
		}

		if (isFixedPoint && !pendingDeleteList.isEmpty ())
		{
			err::setFormatStringError (
				"invalid control flow graph: %s is unreachable but has uses", 
				pendingDeleteList.getHead ()->m_llvmBlock->getName ().begin ()
				);
			return false;
		}
	}

	return true;
}

BasicBlock*
ControlFlowMgr::getUnreachableBlock ()
{
	if (m_unreachableBlock && m_unreachableBlock->getInstructionCount () == 1)
		return m_unreachableBlock;

	m_unreachableBlock = createBlock ("unreachable_block");
	markUnreachable (m_unreachableBlock);
	return m_unreachableBlock;
}

void
ControlFlowMgr::markUnreachable (BasicBlock* block)
{
	ASSERT (block->isEmpty () && block->m_flags == 0);

	BasicBlock* prevCurrentBlock = setCurrentBlock (block);
	m_module->m_llvmIrBuilder.createUnreachable ();
	setCurrentBlock (prevCurrentBlock);
}

void
ControlFlowMgr::jump (
	BasicBlock* block,
	BasicBlock* followBlock
	)
{
	m_flags |= ControlFlowFlag_HasJump;
	block->m_flags |= BasicBlockFlag_Jumped | (m_currentBlock->m_flags & BasicBlockFlag_Reachable);

	m_module->m_llvmIrBuilder.createBr (block);

	if (!followBlock)
		followBlock = getUnreachableBlock ();

	setCurrentBlock (followBlock);
}

void
ControlFlowMgr::follow (BasicBlock* block)
{
	if (!m_currentBlock->hasTerminator ())
	{
		m_module->m_llvmIrBuilder.createBr (block);
		block->m_flags |= BasicBlockFlag_Jumped | (m_currentBlock->m_flags & BasicBlockFlag_Reachable);
	}

	setCurrentBlock (block);
}

bool
ControlFlowMgr::conditionalJump (
	const Value& value,
	BasicBlock* thenBlock,
	BasicBlock* elseBlock,
	BasicBlock* followBlock
	)
{
	Value boolValue;
	bool result = m_module->m_operatorMgr.castOperator (value, TypeKind_Bool, &boolValue);
	if (!result)
		return false;

	uint_t reachableFlag = (m_currentBlock->m_flags & BasicBlockFlag_Reachable);

	m_flags |= ControlFlowFlag_HasJump;
	thenBlock->m_flags |= BasicBlockFlag_Jumped | reachableFlag;
	elseBlock->m_flags |= BasicBlockFlag_Jumped | reachableFlag;

	m_module->m_llvmIrBuilder.createCondBr (boolValue, thenBlock, elseBlock);

	if (!followBlock)
		followBlock = thenBlock;

	setCurrentBlock (followBlock);
	return true;
}

bool
ControlFlowMgr::breakJump (size_t level)
{
	Scope* targetScope = m_module->m_namespaceMgr.findBreakScope (level);
	if (!targetScope)
	{
		err::setFormatStringError ("illegal break");
		return false;
	}

	onLeaveScope (targetScope);
	jump (targetScope->m_breakBlock);
	return true;
}

bool
ControlFlowMgr::continueJump (size_t level)
{
	Scope* targetScope = m_module->m_namespaceMgr.findContinueScope (level);
	if (!targetScope)
	{
		err::setFormatStringError ("illegal continue");
		return false;
	}

	onLeaveScope (targetScope);
	jump (targetScope->m_continueBlock);
	return true;
}

void
ControlFlowMgr::jumpToFinally (Scope* scope)
{
	ASSERT (scope->m_finallyBlock);

	BasicBlock* returnBlock = createBlock ("return_from_finally");
	addBlock (returnBlock);

	size_t returnBlockId = scope->m_finallyReturnBlockArray.getCount ();
	Value finallyReturnValue;
	finallyReturnValue.setConstInt32 ((uint32_t) returnBlockId, m_module);
	m_module->m_operatorMgr.storeDataRef (scope->m_finallyReturnAddress, finallyReturnValue);
	scope->m_finallyReturnBlockArray.append (returnBlock);
	jump (scope->m_finallyBlock, returnBlock);

	// return block will be jumped from 'finally', but we can mark it now:
	// this way we can avoid extra loop in 'finally'

	returnBlock->m_flags |= BasicBlockFlag_Jumped | BasicBlockFlag_Reachable;
}

void
ControlFlowMgr::onLeaveScope (Scope* targetScope)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	while (scope && scope != targetScope && scope->getFunction () == function)
	{
		scope->m_destructList.runDestructors ();
		m_module->m_operatorMgr.nullifyGcRootList (scope->getGcRootList ());

		if (scope->m_finallyBlock && !(scope->m_flags & ScopeFlag_FinallyDefined))
			jumpToFinally (scope);

		scope = scope->getParentScope ();
	}
}

void
ControlFlowMgr::restoreScopeLevel ()
{
	Value scopeLevelValue = m_module->m_functionMgr.getScopeLevel ();
	if (!scopeLevelValue)
		return;

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "restore scope level before return");
	Variable* variable = m_module->m_variableMgr.getStdVariable (StdVariable_ScopeLevel);
	m_module->m_llvmIrBuilder.createStore (scopeLevelValue, variable);
}

bool
ControlFlowMgr::ret (const Value& value)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	FunctionType* functionType = function->getType ();
	Type* returnType = functionType->getReturnType ();

	if (!value)
	{
		if (function->getType ()->getReturnType ()->getTypeKind () != TypeKind_Void)
		{
			err::setFormatStringError (
				"function '%s' must return a '%s' value",
				function->m_tag.cc (),  // thanks a lot gcc
				returnType->getTypeString ().cc ()
				);
			return false;
		}

		onLeaveScope ();
		restoreScopeLevel ();
		m_module->m_llvmIrBuilder.createRet ();
	}
	else
	{
		Value returnValue;
		bool result = m_module->m_operatorMgr.castOperator (value, returnType, &returnValue);
		if (!result)
			return false;

		Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
		if (!(scope->getFlags () & ScopeFlag_HasFinally))
		{
			onLeaveScope ();
		}
		else
		{
			Variable* variable = m_module->m_variableMgr.createStackVariable ("savedReturnValue", returnType);
			m_module->m_variableMgr.allocatePrimeInitializeVariable (variable);

			m_module->m_operatorMgr.storeDataRef (variable, returnValue);
			onLeaveScope ();
			m_module->m_operatorMgr.loadDataRef (variable, &returnValue);
		}

		restoreScopeLevel ();
		functionType->getCallConv ()->ret (function, returnValue);
	}

	ASSERT (!(m_currentBlock->m_flags & BasicBlockFlag_Return));
	m_currentBlock->m_flags |= BasicBlockFlag_Return;
	m_returnBlockArray.append (m_currentBlock);

	m_flags |= ControlFlowFlag_HasReturn;

	setCurrentBlock (getUnreachableBlock ());
	return true;
}

bool
ControlFlowMgr::throwIf (
	const Value& returnValue,
	FunctionType* functionType
	)
{
	ASSERT (functionType->getFlags () & FunctionTypeFlag_Throws);

	bool result;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	if (!(scope->getFlags () & ScopeFlag_CanThrow))
	{
		err::setFormatStringError (
			"cannot call throwing function from here ('%s' does not throw and there is no 'try' or 'catch')",
			m_module->m_functionMgr.getCurrentFunction ()->m_tag.cc ()
			);
		return false;
	}

	BasicBlock* throwBlock = createBlock ("throw_block");
	BasicBlock* followBlock = createBlock ("follow_block");

	Type* returnType = functionType->getReturnType ();

	Value indicatorValue;
	if (!(returnType->getTypeKindFlags () & TypeKindFlag_Integer))
	{
		result = m_module->m_operatorMgr.unaryOperator (UnOpKind_LogNot, returnValue, &indicatorValue);
		if (!result)
			return false;
	}
	else if (!(returnType->getTypeKindFlags () & TypeKindFlag_Unsigned))
	{
		Value zeroValue = returnType->getZeroValue ();

		result = m_module->m_operatorMgr.binaryOperator (BinOpKind_Lt, returnValue, zeroValue, &indicatorValue);
		if (!result)
			return false;
	}
	else
	{
		uint64_t minusOne = -1;

		Value minusOneValue;
		minusOneValue.createConst (&minusOne, returnType);

		result = m_module->m_operatorMgr.binaryOperator (BinOpKind_Eq, returnValue, minusOneValue, &indicatorValue);
		if (!result)
			return false;
	}

	result = conditionalJump (indicatorValue, throwBlock, followBlock);
	if (!result)
		return false;

	Scope* catchScope = m_module->m_namespaceMgr.findCatchScope ();

	if (catchScope)
	{
		onLeaveScope (catchScope);
		catchScope->m_destructList.runDestructors (); // but don't nullify gc-root list
		jump (catchScope->m_catchBlock);
	}
	else
	{
		FunctionType* currentFunctionType = m_module->m_functionMgr.getCurrentFunction ()->getType ();
		Type* currentReturnType = currentFunctionType->getReturnType ();

		Value throwValue;
		if (currentReturnType->cmp (returnValue.getType ()) == 0)
		{
			throwValue = returnValue; // re-throw
		}
		else if (currentReturnType->getTypeKindFlags () & TypeKindFlag_Integer)
		{
			uint64_t minusOne = -1;
			throwValue.createConst (&minusOne, currentReturnType);
		}
		else
		{
			throwValue = currentReturnType->getZeroValue ();
		}

		ret (throwValue);
	}

	follow (followBlock);
	return true;
}

bool
ControlFlowMgr::catchLabel ()
{
	bool result;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	if (scope->m_flags & ScopeFlag_CatchDefined)
	{
		err::setFormatStringError ("'catch' is already defined");
		return false;
	}
	
	ASSERT (scope->m_catchBlock && !scope->m_catchFollowBlock);
	if (scope->m_flags & ScopeFlag_FinallyDefined)
	{
		err::setFormatStringError ("'catch' cannot follow 'finally'");
		return false;
	}

	if (!(scope->m_catchBlock->m_flags & BasicBlockFlag_Jumped))
	{
		err::setFormatStringError ("useless 'catch'");
		return false;
	}

	scope->m_flags |= ScopeFlag_CatchDefined;

	if (scope->isFunctionScope ())
	{
		result = checkReturn ();
		if (!result)
			return false;
	}

	scope->m_destructList.clear ();
	
	scope->m_catchFollowBlock = createBlock ("catch_follow");

	if (scope->m_finallyBlock)
		jumpToFinally (scope);

	jump (scope->m_catchFollowBlock, scope->m_catchBlock);
	
	// this scope cannot catch anymore

	scope->m_catchBlock = NULL;

	Scope* parentScope = scope->getParentScope ();
	if (!parentScope || !(parentScope->m_flags & ScopeFlag_CanThrow))
		scope->m_flags &= ~ScopeFlag_CanThrow;

	return true;
}

bool
ControlFlowMgr::finallyLabel ()
{
	bool result;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope && scope->m_finallyBlock);

	if (scope->m_flags & ScopeFlag_FinallyDefined)
	{
		err::setFormatStringError ("'finally' is already defined");
		return false;
	}

	if (scope->m_catchBlock && !(scope->m_flags & ScopeFlag_CatchDefined)) // try { } stmt
	{
		result = catchLabel ();
		if (!result)
			return false;
	}

	if (!(scope->m_finallyBlock->m_flags & BasicBlockFlag_Jumped))
	{
		err::setFormatStringError ("useless 'finally'");
		return false;
	}

	scope->m_flags |= ScopeFlag_FinallyDefined;

	if (scope->isFunctionScope ())
	{
		result = checkReturn ();
		if (!result)
			return false;
	}
	else
	{
		scope->m_destructList.runDestructors (); // but don't nullify gc-root list
	}

	scope->m_destructList.clear ();

	follow (scope->m_finallyBlock);
	return true;
}

void
ControlFlowMgr::endCatch ()
{
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope && scope->m_catchFollowBlock);
	
	follow (scope->m_catchFollowBlock);	
}

void
ControlFlowMgr::endFinally ()
{
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope && scope->m_finallyBlock && scope->m_finallyReturnAddress);

	Value returnAddressValue;
	m_module->m_operatorMgr.loadDataRef (scope->m_finallyReturnAddress, &returnAddressValue);

	#pragma AXL_TODO ("switch to indirect-branch as soon as LLVM supports it on Windows")

	size_t blockCount = scope->m_finallyReturnBlockArray.getCount ();
	ASSERT (blockCount);

	char buffer [256];
	rtl::Array <intptr_t> intArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	intArray.setCount (blockCount);

	for (size_t i = 0; i < blockCount; i++)
		intArray [i] = i;

	m_module->m_llvmIrBuilder.createSwitch (
		returnAddressValue,
		scope->m_finallyReturnBlockArray [0], // something needs to be specified as default block
		intArray,
		scope->m_finallyReturnBlockArray,
		blockCount
		);

	setCurrentBlock (scope->m_catchFollowBlock ? scope->m_catchFollowBlock : getUnreachableBlock ());
}

void
ControlFlowMgr::endTry ()
{
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	
	ASSERT (
		scope && scope->m_catchBlock &&
		(scope->getFlags () & ScopeFlag_Try) && 
		!(scope->getFlags () & (ScopeFlag_CatchDefined | ScopeFlag_FinallyDefined))
		);

	if (!(scope->m_catchBlock->m_flags & BasicBlockFlag_Jumped))
		return; // ignore useless try

	bool result = catchLabel ();
	ASSERT (result);

	endCatch ();
}

bool
ControlFlowMgr::checkReturn ()
{
	if (m_currentBlock->hasTerminator ())
		return true;

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	Type* returnType = function->getType ()->getReturnType ();

	if (!(m_currentBlock->m_flags & BasicBlockFlag_Reachable))
	{
		m_module->m_llvmIrBuilder.createUnreachable (); // just to make LLVM happy
	}
	else if (returnType->getTypeKind () == TypeKind_Void)
	{
		m_module->m_controlFlowMgr.ret ();
	}
	else if (!(m_module->m_controlFlowMgr.getFlags () & ControlFlowFlag_HasReturn))
	{
		err::setFormatStringError (
			"function '%s' must return a '%s' value",
			function->m_tag.cc (),
			returnType->getTypeString ().cc ()
			);
		return false;
	}
	else
	{
		err::setFormatStringError (
			"not all control paths in function '%s' return a value",
			function->m_tag.cc ()
			);
		return false;
	}

	return true;
}

//.............................................................................

} // namespace jnc {
