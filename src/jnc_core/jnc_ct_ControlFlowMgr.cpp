#include "pch.h"
#include "jnc_ct_ControlFlowMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

ControlFlowMgr::ControlFlowMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_currentBlock = NULL;
	m_unreachableBlock = NULL;
	m_returnBlock = NULL;
	m_catchFinallyFollowBlock = NULL;
	m_returnBlock = NULL;
	m_finallyRouteIdxVariable = NULL;
	m_returnValueVariable = NULL;
	m_throwLockCount = 0;
	m_finallyRouteCount = 0;
}

void
ControlFlowMgr::clear ()
{
	m_blockList.clear ();
	m_returnBlockArray.clear ();
	m_currentBlock = NULL;
	m_unreachableBlock = NULL;
	m_catchFinallyFollowBlock = NULL;
	m_returnBlock = NULL;
	m_finallyRouteIdxVariable = NULL;
	m_returnValueVariable = NULL;
	m_throwLockCount = 0;
	m_finallyRouteCount = 0;
}

void 
ControlFlowMgr::finalizeFunction ()
{
	m_returnBlockArray.clear ();
	m_currentBlock = NULL;
	m_unreachableBlock = NULL;
	m_catchFinallyFollowBlock = NULL;
	m_returnBlock = NULL;
	m_finallyRouteIdxVariable = NULL;
	m_returnValueVariable = NULL;
	m_throwLockCount = 0;
	m_finallyRouteCount = 0;
}

BasicBlock*
ControlFlowMgr::createBlock (
	const sl::String& name,
	uint_t flags
	)
{
	BasicBlock* block = AXL_MEM_NEW (BasicBlock);
	block->m_module = m_module;
	block->m_name = name;
	block->m_flags = 0;
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

	sl::StdList <BasicBlock> pendingDeleteList;

	sl::Iterator <BasicBlock> it = m_blockList.getHead ();
	while (it)
	{
		BasicBlock* block = *it;
		it++;

		if (!(block->m_flags & BasicBlockFlag_Reachable))
		{
			m_blockList.remove (block);

			if (!block->m_llvmBlock->use_empty ())
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

			if (!block->m_llvmBlock->use_empty ())
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

BasicBlock*
ControlFlowMgr::getReturnBlock ()
{
	if (m_returnBlock)
		return m_returnBlock;

	m_returnBlock = createBlock ("return_block");
	BasicBlock* prevBlock = setCurrentBlock (m_returnBlock);

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	FunctionType* functionType = function->getType ();
	if (functionType->getReturnType ()->getTypeKind () == TypeKind_Void)
	{
		m_module->m_llvmIrBuilder.createRet ();
	}
	else
	{
		Value returnValue;
		m_module->m_llvmIrBuilder.createLoad (getReturnValueVariable (), NULL, &returnValue);
		functionType->getCallConv ()->ret (function, returnValue);
	}

	m_currentBlock->m_flags |= BasicBlockFlag_Return;
	m_returnBlockArray.append (m_currentBlock);
	
	setCurrentBlock (prevBlock);
	return m_returnBlock;
}

Variable* 
ControlFlowMgr::getFinallyRouteIdxVariable ()
{
	if (m_finallyRouteIdxVariable)
		return m_finallyRouteIdxVariable;

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	BasicBlock* prevBlock = setCurrentBlock (function->getEntryBlock ());	
	m_finallyRouteIdxVariable = m_module->m_variableMgr.createSimpleStackVariable ("finallyRouteIdx", m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr));
	setCurrentBlock (prevBlock);
	return m_finallyRouteIdxVariable;
}

Variable* 
ControlFlowMgr::getReturnValueVariable ()
{
	if (m_returnValueVariable)
		return m_returnValueVariable;

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	Type* returnType = function->getType ()->getReturnType ();
	ASSERT (returnType->getTypeKind () != TypeKind_Void);

	BasicBlock* prevBlock = setCurrentBlock (function->getEntryBlock ());	
	m_returnValueVariable = m_module->m_variableMgr.createSimpleStackVariable ("savedReturnValue", returnType);
	setCurrentBlock (prevBlock);
	return m_returnValueVariable;
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

	escapeScope (targetScope, targetScope->m_breakBlock);
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

	escapeScope (targetScope, targetScope->m_continueBlock);
	return true;
}

void
ControlFlowMgr::escapeScope (
	Scope* targetScope,
	BasicBlock* targetBlock,
	bool isThrow // during throw we have to nullify gc stack roots of target scope
	)
{
	size_t routeIdx = m_finallyRouteCount;
	
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	BasicBlock* firstFinallyBlock = NULL;
	BasicBlock* prevFinallyBlock = NULL;
	while (scope && scope != targetScope)
	{
		m_module->m_operatorMgr.disposeDisposableVariableList (scope->m_disposableVariableList);
		m_module->m_operatorMgr.nullifyGcRootList (scope->m_gcStackRootList);

		if (scope->m_flags & ScopeFlag_FinallyAhead)
		{
			BasicBlock* finallyBlock = scope->getOrCreateFinallyBlock ();
			if (!firstFinallyBlock)
			{
				firstFinallyBlock = finallyBlock;
			}
			else 
			{
				ASSERT (prevFinallyBlock);
				prevFinallyBlock->m_finallyRouteMap [routeIdx] = finallyBlock;
			}

			prevFinallyBlock = finallyBlock;			
		}

		scope = scope->getParentScope ();
	}

	if (isThrow)
	{
		m_module->m_operatorMgr.disposeDisposableVariableList (scope->m_disposableVariableList);
		m_module->m_operatorMgr.nullifyGcRootList (scope->m_gcStackRootList);
	}

	if (!firstFinallyBlock)
	{
		if (targetBlock)
			jump (targetBlock);

		return;
	}

	BasicBlock* followBlock = NULL;

	if (!targetBlock)
	{
		followBlock = createBlock ("finally_follow");
		targetBlock = followBlock;
	}

	prevFinallyBlock->m_finallyRouteMap [routeIdx] = targetBlock;

	Variable* routeIdxVariable = getFinallyRouteIdxVariable ();
	Value routeIdxValue (routeIdx, m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr));
	m_module->m_llvmIrBuilder.createStore (routeIdxValue, routeIdxVariable);
	
	jump (firstFinallyBlock, followBlock);

	m_finallyRouteCount++;
}

bool
ControlFlowMgr::ret (const Value& value)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	FunctionType* functionType = function->getType ();
	Type* returnType = functionType->getReturnType ();

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();

	if (!value)
	{
		if (functionType->getReturnType ()->getTypeKind () != TypeKind_Void)
		{
			err::setFormatStringError (
				"function '%s' must return a '%s' value",
				function->m_tag.cc (), 
				returnType->getTypeString ().cc ()
				);
			return false;
		}

		if (scope->m_flags & ScopeFlag_Finalizable)
		{
			escapeScope (NULL, getReturnBlock ());
			return true;
		}

		escapeScope (NULL, NULL);
		m_module->m_llvmIrBuilder.createRet ();
	}
	else
	{
		Value returnValue;
		bool result = m_module->m_operatorMgr.castOperator (value, returnType, &returnValue);
		if (!result)
			return false;

		if (scope->getFlags () & ScopeFlag_Finalizable)
		{
			m_module->m_llvmIrBuilder.createStore (returnValue, getReturnValueVariable ());
			escapeScope (NULL, getReturnBlock ());
			return true;
		}

		escapeScope (NULL, NULL);
		functionType->getCallConv ()->ret (function, returnValue);
	}

	ASSERT (!(m_currentBlock->m_flags & BasicBlockFlag_Return));
	m_currentBlock->m_flags |= BasicBlockFlag_Return;
	m_returnBlockArray.append (m_currentBlock);
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
		result = m_module->m_operatorMgr.castOperator (returnValue, m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool), &indicatorValue);
		if (!result)
			return false;
	}
	else if (!(returnType->getTypeKindFlags () & TypeKindFlag_Unsigned))
	{
		Value zeroValue = returnType->getZeroValue ();

		result = m_module->m_operatorMgr.binaryOperator (BinOpKind_Ge, returnValue, zeroValue, &indicatorValue);
		if (!result)
			return false;
	}
	else
	{
		uint64_t minusOne = -1;

		Value minusOneValue;
		minusOneValue.createConst (&minusOne, returnType);

		result = m_module->m_operatorMgr.binaryOperator (BinOpKind_Ne, returnValue, minusOneValue, &indicatorValue);
		if (!result)
			return false;
	}

	result = conditionalJump (indicatorValue, followBlock, throwBlock, throwBlock);
	if (!result)
		return false;

	Scope* catchScope = m_module->m_namespaceMgr.findCatchScope ();
	if (catchScope)
	{
		escapeScope (catchScope, catchScope->m_catchBlock, true);
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

	setCurrentBlock (followBlock);
	return true;
}

bool
ControlFlowMgr::catchLabel (const Token::Pos& pos)
{
	bool result;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (!(scope->m_flags & ScopeFlag_Finally));

	if (!(scope->m_flags & ScopeFlag_CatchAhead))
	{
		err::setFormatStringError ("'catch' is already defined");
		return false;
	}

	if (!scope->m_catchBlock)
	{
		err::setFormatStringError ("useless 'catch'");
		return false;
	}

	if (scope->m_flags & ScopeFlag_Function)
	{
		result = checkReturn ();
		if (!result)
			return false;
	}

	if (m_currentBlock->m_flags & BasicBlockFlag_Reachable)
	{
		if (scope->m_flags & ScopeFlag_FinallyAhead)
		{
			normalFinallyFlow ();
		}
		else
		{
			m_catchFinallyFollowBlock = createBlock ("catch_follow");
			m_module->m_operatorMgr.nullifyGcRootList (scope->m_gcStackRootList);
			jump (m_catchFinallyFollowBlock);
		}
	}

	m_module->m_namespaceMgr.closeScope ();
	setCurrentBlock (scope->m_catchBlock);

	Scope* catchScope = m_module->m_namespaceMgr.openScope (pos, ScopeFlag_Catch);
	catchScope->m_flags |= scope->m_flags & (ScopeFlag_Nested | ScopeFlag_FinallyAhead | ScopeFlag_Finalizable); // propagate
	catchScope->m_finallyBlock = scope->m_finallyBlock; // propagate
	return true;
}

bool
ControlFlowMgr::closeCatch ()
{
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT ((scope->m_flags & ScopeFlag_Catch) && !(scope->m_flags & ScopeFlag_FinallyAhead));

	if (scope->m_flags & ScopeFlag_Function)
	{
		bool result = checkReturn ();
		if (!result)
			return false;
	}
	
	if (m_currentBlock->m_flags & BasicBlockFlag_Reachable)
	{
		ASSERT (m_catchFinallyFollowBlock);

		m_module->m_operatorMgr.nullifyGcRootList (scope->m_gcStackRootList);
		follow (m_catchFinallyFollowBlock);
		m_catchFinallyFollowBlock = NULL; // used already
	}

	m_module->m_namespaceMgr.closeScope ();
	return true;
}

bool
ControlFlowMgr::finallyLabel (const Token::Pos& pos)
{
	bool result;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	if (scope->m_flags & ScopeFlag_CatchAhead)
	{
		err::setFormatStringError ("'finally' should follow 'catch'");
		return false;
	}

	if (!(scope->m_flags & ScopeFlag_FinallyAhead))
	{
		err::setFormatStringError ("'finally' is already defined");
		return false;
	}

	if (scope->m_flags & ScopeFlag_Function)
	{
		result = checkReturn ();
		if (!result)
			return false;

	}
	else if (scope->m_flags & ScopeFlag_Try) 
	{
		// inject empty catch

		scope->m_flags |= ScopeFlag_CatchAhead;
		result = catchLabel (pos);
		if (!result)
			return false;
	}

	if (!scope->m_finallyBlock)
	{
		err::setFormatStringError ("useless 'finally'");
		return false;
	}

	if (m_currentBlock->m_flags & BasicBlockFlag_Reachable)
		normalFinallyFlow ();

	m_module->m_namespaceMgr.closeScope ();
	setCurrentBlock (scope->m_finallyBlock);

	Scope* finallyScope = m_module->m_namespaceMgr.openScope (pos, ScopeFlag_Finally);
	finallyScope->m_flags |= scope->m_flags & ScopeFlag_Nested; // propagate
	finallyScope->m_finallyBlock = scope->m_finallyBlock; // propagate
	return true;
}

bool
ControlFlowMgr::closeFinally ()
{
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope && (scope->m_flags & ScopeFlag_Finally) && scope->m_finallyBlock && m_finallyRouteIdxVariable);

	m_module->m_operatorMgr.nullifyGcRootList (scope->m_gcStackRootList);

	Value routeIdxValue;
	m_module->m_operatorMgr.loadDataRef (m_finallyRouteIdxVariable, &routeIdxValue);

	size_t count = scope->m_finallyBlock->m_finallyRouteMap.getCount ();
	ASSERT (count);

	char buffer1 [256];
	sl::Array <intptr_t> routeIdxArray (ref::BufKind_Stack, buffer1, sizeof (buffer1));
	routeIdxArray.setCount (count);

	char buffer2 [256];
	sl::Array <BasicBlock*> blockArray (ref::BufKind_Stack, buffer2, sizeof (buffer2));
	blockArray.setCount (count);

	sl::HashTableMapIterator <size_t, BasicBlock*> it = scope->m_finallyBlock->m_finallyRouteMap.getHead ();
	for (size_t i = 0; it; it++, i++)
	{
		ASSERT (i < count);

		routeIdxArray [i] = it->m_key;
		blockArray [i] = it->m_value;
		it->m_value->markReachable ();
	}

	m_module->m_llvmIrBuilder.createSwitch (
		routeIdxValue,
		blockArray [0], // something needs to be specified as default block
		routeIdxArray,
		blockArray,
		count
		);

	if (m_catchFinallyFollowBlock)
	{
		setCurrentBlock (m_catchFinallyFollowBlock);
		m_catchFinallyFollowBlock = NULL;
	}
	else
	{
		setCurrentBlock (getUnreachableBlock ());
	}

	m_module->m_namespaceMgr.closeScope ();
	return true;
}

void
ControlFlowMgr::normalFinallyFlow ()
{
	if (!m_catchFinallyFollowBlock)
		m_catchFinallyFollowBlock = createBlock ("finally_follow");

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope->m_flags & ScopeFlag_FinallyAhead);

	m_module->m_operatorMgr.disposeDisposableVariableList (scope->m_disposableVariableList);
	m_module->m_operatorMgr.nullifyGcRootList (scope->m_gcStackRootList);

	ASSERT (m_catchFinallyFollowBlock);

	size_t routeIdx = m_finallyRouteCount;

	BasicBlock* finallyBlock = scope->getOrCreateFinallyBlock ();
	finallyBlock->m_finallyRouteMap [routeIdx] = m_catchFinallyFollowBlock;

	Variable* routeIdxVariable = getFinallyRouteIdxVariable ();
	Value routeIdxValue (routeIdx, m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr));
	m_module->m_llvmIrBuilder.createStore (routeIdxValue, routeIdxVariable);
	jump (finallyBlock);

	m_finallyRouteCount++;
}

bool
ControlFlowMgr::closeTry ()
{
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope && (scope->m_flags & ScopeFlag_Try));

	if (!scope->m_catchBlock)
	{
		err::setFormatStringError ("useless 'try'");
		return false;
	}

	return 
		catchLabel (Token::Pos ()) &&
		closeCatch (); 
}

bool
ControlFlowMgr::nestedScopeLabel (
	const Token::Pos& pos,
	uint_t scopeFlags
	)
{
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	ASSERT (scope);

	if (scope->m_flags & (ScopeFlag_Try | ScopeFlag_Catch | ScopeFlag_Finally | ScopeFlag_Nested))
	{
		err::setFormatStringError ("'nestedscope' can only be used in regular scopes (not 'try', 'catch', 'finally' or 'nestedscope')");
		return false;
	}

	Scope* nestedScope = m_module->m_namespaceMgr.openScope (pos, scopeFlags | ScopeFlag_Nested);
	nestedScope->m_flags |= scope->m_flags & ScopeFlag_Function; // propagate
	return true;
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
	else if (m_returnBlockArray.isEmpty ())
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

} // namespace ct
} // namespace jnc
