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
	m_dynamicThrowBlock = NULL;
	m_catchFinallyFollowBlock = NULL;
	m_finallyRouteIdxVariable = NULL;
	m_returnValueVariable = NULL;
	m_finallyRouteIdx = -1;
	m_sjljFrameCount = 0;
}

void
ControlFlowMgr::clear ()
{
	m_blockList.clear ();
	m_returnBlockArray.clear ();
	m_landingPadBlockArray.clear ();
	m_currentBlock = NULL;
	m_unreachableBlock = NULL;
	m_catchFinallyFollowBlock = NULL;
	m_returnBlock = NULL;	
	m_dynamicThrowBlock = NULL;
	m_finallyRouteIdxVariable = NULL;
	m_returnValueVariable = NULL;
	m_finallyRouteIdx = -1;
	m_sjljFrameCount = 0;
	m_sjljFrameArrayValue.clear ();
	m_prevSjljFrameValue.clear ();
}

void 
ControlFlowMgr::finalizeFunction ()
{
	if (m_sjljFrameArrayValue)
		finalizeSjljFrameArray ();

	m_returnBlockArray.clear ();
	m_landingPadBlockArray.clear ();
	m_currentBlock = NULL;
	m_unreachableBlock = NULL;
	m_catchFinallyFollowBlock = NULL;
	m_returnBlock = NULL;	
	m_dynamicThrowBlock = NULL;
	m_finallyRouteIdxVariable = NULL;
	m_returnValueVariable = NULL;
	m_finallyRouteIdx = -1;
	m_sjljFrameArrayValue.clear ();
	m_prevSjljFrameValue.clear ();
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
	block->m_flags = flags;
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
	// llvm blocks may have references even when they are unreachable

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
ControlFlowMgr::markLandingPad (
	BasicBlock* block,
	Scope* scope,
	LandingPadKind landingPadKind
	)
{
	ASSERT (!block->m_landingPadKind || block->m_landingPadKind == landingPadKind);

	if (!block->m_landingPadKind)
	{
		block->m_landingPadKind = landingPadKind;
		m_landingPadBlockArray.append (block);
	}

	block->m_landingPadScope = scope; // override scope anyway
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
	BasicBlock* targetBlock
	)
{
	size_t routeIdx = ++m_finallyRouteIdx;
	
	BasicBlock* firstFinallyBlock = NULL;
	BasicBlock* prevFinallyBlock = NULL;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();
	while (scope && scope != targetScope)
	{
		if (scope->m_flags & ScopeFlag_FinallyAhead)
		{
			ASSERT (scope->m_finallyBlock);

			if (!firstFinallyBlock)
			{
				firstFinallyBlock = scope->m_finallyBlock;
			}
			else 
			{
				ASSERT (prevFinallyBlock);
				prevFinallyBlock->m_finallyRouteMap [routeIdx] = scope->m_finallyBlock;
			}

			prevFinallyBlock = scope->m_finallyBlock;
		}

		scope = scope->getParentScope ();
	}

	if (!targetBlock) // escape before normal return
	{
		ASSERT (!firstFinallyBlock); // if we have finally then we return via return-block
		
		scope = m_module->m_namespaceMgr.getCurrentScope ();
		if (scope->m_sjljFrameIdx != -1)
			setSjljFrame (-1);
		
		return;
	}

	markLandingPad (targetBlock, targetScope, LandingPadKind_EscapeScope);

	if (!firstFinallyBlock)
	{
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
			escapeScope (function->getScope (), getReturnBlock ());
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
			escapeScope (function->getScope (), getReturnBlock ());
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
		ret ();
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
