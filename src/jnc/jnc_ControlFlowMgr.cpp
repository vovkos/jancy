#include "pch.h"
#include "jnc_ControlFlowMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CControlFlowMgr::CControlFlowMgr ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	m_Flags = 0;
	m_ThrowLockCount = 0;
	m_pCurrentBlock = NULL;
	m_pUnreachableBlock = NULL;
}

void
CControlFlowMgr::Clear ()
{
	m_Flags = 0;
	m_ThrowLockCount = 0;
	m_BlockList.Clear ();
	m_ReturnBlockArray.Clear ();
	m_pCurrentBlock = NULL;
	m_pUnreachableBlock = NULL;
}

CBasicBlock*
CControlFlowMgr::CreateBlock (const rtl::CString& Name)
{
	CBasicBlock* pBlock = AXL_MEM_NEW (CBasicBlock);
	pBlock->m_pModule = m_pModule;
	pBlock->m_Name = Name;
	pBlock->m_pLlvmBlock = llvm::BasicBlock::Create (
		*m_pModule->GetLlvmContext (),
		(const char*) Name,
		NULL
		);

	m_BlockList.InsertTail (pBlock);
	return pBlock;
}

CBasicBlock*
CControlFlowMgr::SetCurrentBlock (CBasicBlock* pBlock)
{
	if (m_pCurrentBlock == pBlock)
		return pBlock;

	CBasicBlock* pPrevCurrentBlock = m_pCurrentBlock;
	if (pPrevCurrentBlock)
		pPrevCurrentBlock->m_LlvmDebugLoc = m_pModule->m_LlvmIrBuilder.GetCurrentDebugLoc ();

	m_pCurrentBlock = pBlock;
	if (!pBlock)
		return pPrevCurrentBlock;

	if (!pBlock->m_pFunction)
		AddBlock (pBlock);

	m_pModule->m_LlvmIrBuilder.SetInsertPoint (pBlock);

	if (!pBlock->m_LlvmDebugLoc.isUnknown ())
		m_pModule->m_LlvmIrBuilder.SetCurrentDebugLoc (pBlock->m_LlvmDebugLoc);

	return pPrevCurrentBlock;
}

void
CControlFlowMgr::AddBlock (CBasicBlock* pBlock)
{
	ASSERT (!pBlock->m_pFunction);

	CFunction* pFunction = m_pModule->m_FunctionMgr.GetCurrentFunction ();
	ASSERT (pFunction);

	pFunction->GetLlvmFunction ()->getBasicBlockList ().push_back (pBlock->m_pLlvmBlock);
	pBlock->m_pFunction = pFunction;
}

void
CControlFlowMgr::DeleteUnreachableBlocks ()
{
	rtl::CIteratorT <CBasicBlock> It = m_BlockList.GetHead ();
	while (It)
	{
		if (It->m_Flags & EBasicBlockFlag_Reachable)
		{
			It++;
			continue;
		}

		CBasicBlock* pBlock = *It;
		It++;

		// check if block was never added

		if (!pBlock->m_pFunction)
			delete pBlock->m_pLlvmBlock;

		m_BlockList.Delete (pBlock);
	}

	m_pUnreachableBlock = NULL;
	m_pCurrentBlock = NULL;
}

CBasicBlock*
CControlFlowMgr::GetUnreachableBlock ()
{
	if (m_pUnreachableBlock && m_pUnreachableBlock->GetInstructionCount () == 1)
		return m_pUnreachableBlock;

	m_pUnreachableBlock = CreateBlock ("unreachable_block");
	MarkUnreachable (m_pUnreachableBlock);
	return m_pUnreachableBlock;
}

void
CControlFlowMgr::MarkUnreachable (CBasicBlock* pBlock)
{
	ASSERT (pBlock->IsEmpty () && pBlock->m_Flags == 0);

	CBasicBlock* pPrevCurrentBlock = SetCurrentBlock (pBlock);
	m_pModule->m_LlvmIrBuilder.CreateUnreachable ();
	SetCurrentBlock (pPrevCurrentBlock);
}

void
CControlFlowMgr::Jump (
	CBasicBlock* pBlock,
	CBasicBlock* pFollowBlock
	)
{
	m_Flags |= EControlFlowFlag_HasJump;
	pBlock->m_Flags |= EBasicBlockFlag_Jumped | (m_pCurrentBlock->m_Flags & EBasicBlockFlag_Reachable);

	m_pModule->m_LlvmIrBuilder.CreateBr (pBlock);

	if (!pFollowBlock)
		pFollowBlock = GetUnreachableBlock ();

	SetCurrentBlock (pFollowBlock);
}

void
CControlFlowMgr::Follow (CBasicBlock* pBlock)
{
	if (!m_pCurrentBlock->HasTerminator ())
	{
		m_pModule->m_LlvmIrBuilder.CreateBr (pBlock);
		pBlock->m_Flags |= EBasicBlockFlag_Jumped | (m_pCurrentBlock->m_Flags & EBasicBlockFlag_Reachable);
	}

	SetCurrentBlock (pBlock);
}

bool
CControlFlowMgr::ConditionalJump (
	const CValue& Value,
	CBasicBlock* pThenBlock,
	CBasicBlock* pElseBlock,
	CBasicBlock* pFollowBlock
	)
{
	CValue BoolValue;
	bool Result = m_pModule->m_OperatorMgr.CastOperator (Value, EType_Bool, &BoolValue);
	if (!Result)
		return false;

	uint_t ReachableFlag = (m_pCurrentBlock->m_Flags & EBasicBlockFlag_Reachable);

	m_Flags |= EControlFlowFlag_HasJump;
	pThenBlock->m_Flags |= EBasicBlockFlag_Jumped | ReachableFlag;
	pElseBlock->m_Flags |= EBasicBlockFlag_Jumped | ReachableFlag;

	m_pModule->m_LlvmIrBuilder.CreateCondBr (BoolValue, pThenBlock, pElseBlock);

	if (!pFollowBlock)
		pFollowBlock = pThenBlock;

	SetCurrentBlock (pFollowBlock);
	return true;
}

bool
CControlFlowMgr::Break (size_t Level)
{
	CScope* pTargetScope = m_pModule->m_NamespaceMgr.FindBreakScope (Level);
	if (!pTargetScope)
	{
		err::SetFormatStringError ("illegal break");
		return false;
	}

	OnLeaveScope (pTargetScope);
	Jump (pTargetScope->m_pBreakBlock);
	return true;
}

bool
CControlFlowMgr::Continue (size_t Level)
{
	CScope* pTargetScope = m_pModule->m_NamespaceMgr.FindBreakScope (Level);
	if (!pTargetScope)
	{
		err::SetFormatStringError ("illegal continue");
		return false;
	}

	OnLeaveScope (pTargetScope);
	Jump (pTargetScope->m_pContinueBlock);
	return true;
}

void
CControlFlowMgr::OnLeaveScope (CScope* pTargetScope)
{
	CFunction* pFunction = m_pModule->m_FunctionMgr.GetCurrentFunction ();
	ASSERT (pFunction);

	CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();
	while (pScope && pScope != pTargetScope && pScope->GetFunction () == pFunction)
	{
		pScope->m_DestructList.RunDestructors ();
		m_pModule->m_OperatorMgr.NullifyGcRootList (pScope->GetGcRootList ());

		if (pScope->m_pFinallyBlock && !(pScope->m_Flags & EScopeFlag_FinallyDefined))
		{
			CBasicBlock* pReturnBlock = CreateBlock ("finally_return_block");
			AddBlock (pReturnBlock);

			size_t ReturnBlockId = pScope->m_FinallyReturnBlockArray.GetCount ();
			CValue FinallyReturnValue;
			FinallyReturnValue.SetConstInt32 ((uint32_t) ReturnBlockId);
			m_pModule->m_OperatorMgr.StoreDataRef (pScope->m_pFinallyReturnAddress, FinallyReturnValue);
			pScope->m_FinallyReturnBlockArray.Append (pReturnBlock);
			Jump (pScope->m_pFinallyBlock, pReturnBlock);

			// return block will be jumped from 'finally', but we can mark it now:
			// this way we can avoid extra loop in 'finally'

			pReturnBlock->m_Flags |= EBasicBlockFlag_Jumped | EBasicBlockFlag_Reachable;
		}

		pScope = pScope->GetParentScope ();
	}
}

void
CControlFlowMgr::RestoreScopeLevel ()
{
	CValue ScopeLevelValue = m_pModule->m_FunctionMgr.GetScopeLevel ();
	if (!ScopeLevelValue)
		return;

	CLlvmScopeComment Comment (&m_pModule->m_LlvmIrBuilder, "restore scope level before return");
	CVariable* pVariable = m_pModule->m_VariableMgr.GetStdVariable (EStdVariable_ScopeLevel);
	m_pModule->m_LlvmIrBuilder.CreateStore (ScopeLevelValue, pVariable);
}

bool
CControlFlowMgr::Return (const CValue& Value)
{
	CFunction* pFunction = m_pModule->m_FunctionMgr.GetCurrentFunction ();
	ASSERT (pFunction);

	CFunctionType* pFunctionType = pFunction->GetType ();
	CType* pReturnType = pFunctionType->GetReturnType ();

	if (!Value)
	{
		if (pFunction->GetType ()->GetReturnType ()->GetTypeKind () != EType_Void)
		{
			err::SetFormatStringError (
				"function '%s' must return a '%s' value",
				pFunction->m_Tag.cc (),  // thanks a lot gcc
				pReturnType->GetTypeString ().cc ()
				);
			return false;
		}

		OnLeaveScope ();
		RestoreScopeLevel ();
		m_pModule->m_LlvmIrBuilder.CreateRet ();
	}
	else
	{
		CValue ReturnValue;
		bool Result = m_pModule->m_OperatorMgr.CastOperator (Value, pReturnType, &ReturnValue);
		if (!Result)
			return false;

		CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();
		if (!(pScope->GetFlags () & EScopeFlag_HasFinally))
		{
			OnLeaveScope ();
		}
		else
		{
			CVariable* pVariable = m_pModule->m_VariableMgr.CreateStackVariable ("savedReturnValue", pReturnType);
			m_pModule->m_VariableMgr.AllocatePrimeInitializeVariable (pVariable);

			m_pModule->m_OperatorMgr.StoreDataRef (pVariable, ReturnValue);
			OnLeaveScope ();
			m_pModule->m_OperatorMgr.LoadDataRef (pVariable, &ReturnValue);
		}

		RestoreScopeLevel ();
		pFunctionType->GetCallConv ()->Return (pFunction, ReturnValue);
	}

	ASSERT (!(m_pCurrentBlock->m_Flags & EBasicBlockFlag_Return));
	m_pCurrentBlock->m_Flags |= EBasicBlockFlag_Return;
	m_ReturnBlockArray.Append (m_pCurrentBlock);

	m_Flags |= EControlFlowFlag_HasReturn;

	SetCurrentBlock (GetUnreachableBlock ());
	return true;
}

bool
CControlFlowMgr::Throw (
	const CValue& ReturnValue,
	CFunctionType* pFunctionType
	)
{
	ASSERT (pFunctionType->GetFlags () & EFunctionTypeFlag_Throws);

	bool Result;

	CBasicBlock* pThrowBlock = CreateBlock ("throw_block");
	CBasicBlock* pFollowBlock = CreateBlock ("follow_block");

	CType* pReturnType = pFunctionType->GetReturnType ();
	rtl::CConstBoxListT <CToken> ThrowCondition = pFunctionType->GetThrowCondition ();

	CValue IndicatorValue;
	if (!ThrowCondition.IsEmpty ())
	{
		Result = m_pModule->m_OperatorMgr.ParseThrowCondition (
			NULL, // TODO: fix
			ThrowCondition,
			ReturnValue,
			&IndicatorValue
			);
		if (!Result)
			return false;
	}
	else if (!(pReturnType->GetTypeKindFlags () & ETypeKindFlag_Integer))
	{
		Result = m_pModule->m_OperatorMgr.UnaryOperator (EUnOp_LogNot, ReturnValue, &IndicatorValue);
		if (!Result)
			return false;
	}
	else if (!(pReturnType->GetTypeKindFlags () & ETypeKindFlag_Unsigned))
	{
		CValue ZeroValue = pReturnType->GetZeroValue ();

		Result = m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Lt, ReturnValue, ZeroValue, &IndicatorValue);
		if (!Result)
			return false;
	}
	else
	{
		uint64_t MinusOne = -1;

		CValue MinusOneValue;
		MinusOneValue.CreateConst (&MinusOne, pReturnType);

		Result = m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Eq, ReturnValue, MinusOneValue, &IndicatorValue);
		if (!Result)
			return false;
	}

	Result = ConditionalJump (IndicatorValue, pThrowBlock, pFollowBlock);
	if (!Result)
		return false;

	CScope* pCatchScope = m_pModule->m_NamespaceMgr.FindCatchScope ();

	if (pCatchScope)
	{
		OnLeaveScope (pCatchScope);
		pCatchScope->m_DestructList.RunDestructors (); // but don't nullify gc-root list
		Jump (pCatchScope->m_pCatchBlock);
	}
	else
	{
		CFunctionType* pCurrentFunctionType = m_pModule->m_FunctionMgr.GetCurrentFunction ()->GetType ();
		CType* pCurrentReturnType = pCurrentFunctionType->GetReturnType ();

		CValue ThrowValue;
		if (!pCurrentFunctionType->GetThrowCondition ().IsEmpty ())
		{
			if (!pCurrentFunctionType->IsThrowConditionMatch (pFunctionType))
			{
				err::SetFormatStringError ("functions with throw conditions need to 'catch' and 'return' manually");
				return false;
			}

			ThrowValue = ReturnValue; // re-throw
		}
		else if (pCurrentReturnType->GetTypeKindFlags () & ETypeKindFlag_Integer)
		{
			uint64_t MinusOne = -1;
			ThrowValue.CreateConst (&MinusOne, pCurrentReturnType);
		}
		else
		{
			ThrowValue = pCurrentReturnType->GetZeroValue ();
		}

		Return (ThrowValue);
	}

	Follow (pFollowBlock);
	return true;
}

bool
CControlFlowMgr::Catch ()
{
	bool Result;

	CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();
	ASSERT (pScope && pScope->m_pCatchBlock);

	if (pScope->m_Flags & EScopeFlag_FinallyDefined)
	{
		err::SetFormatStringError ("'catch' cannot follow 'finally'");
		return false;
	}

	if (!(pScope->m_pCatchBlock->GetFlags () & EBasicBlockFlag_Jumped))
	{
		err::SetFormatStringError ("useless 'catch'");
		return false;
	}

	pScope->m_Flags |= EScopeFlag_CatchDefined;

	if (pScope->IsFunctionScope ())
	{
		Result = CheckReturn ();
		if (!Result)
			return false;
	}

	pScope->m_DestructList.Clear ();

	Follow (pScope->m_pCatchBlock);
	return true;
}

bool
CControlFlowMgr::Finally ()
{
	bool Result;

	CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();
	ASSERT (pScope && pScope->m_pFinallyBlock);

	if (!(pScope->m_pFinallyBlock->GetFlags () & EBasicBlockFlag_Jumped))
	{
		err::SetFormatStringError ("useless 'finally'");
		return false;
	}

	pScope->m_Flags |= EScopeFlag_FinallyDefined;

	if (pScope->IsFunctionScope ())
	{
		Result = CheckReturn ();
		if (!Result)
			return false;
	}
	else
	{
		pScope->m_DestructList.RunDestructors (); // but don't nullify gc-root list
	}

	pScope->m_DestructList.Clear ();

	Follow (pScope->m_pFinallyBlock);
	return true;
}

bool
CControlFlowMgr::EndTry ()
{
	CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();
	ASSERT (pScope && pScope->m_pCatchBlock);

	return !(pScope->GetFlags () & EScopeFlag_CatchDefined) ? Catch () : true;
}

bool
CControlFlowMgr::EndFinally ()
{
	CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();
	ASSERT (pScope && pScope->m_pFinallyBlock && pScope->m_pFinallyReturnAddress);

	CValue ReturnAddressValue;
	m_pModule->m_OperatorMgr.LoadDataRef (pScope->m_pFinallyReturnAddress, &ReturnAddressValue);

	#pragma AXL_TODO ("switch to indirect-branch as soon as LLVM supports it on Windows")

	size_t BlockCount = pScope->m_FinallyReturnBlockArray.GetCount ();
	ASSERT (BlockCount);

	char Buffer [256];
	rtl::CArrayT <intptr_t> IntArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	IntArray.SetCount (BlockCount);

	for (size_t i = 0; i < BlockCount; i++)
		IntArray [i] = i;

	m_pModule->m_LlvmIrBuilder.CreateSwitch (
		ReturnAddressValue,
		pScope->m_FinallyReturnBlockArray [0], // something needs to be specified as default block
		IntArray,
		pScope->m_FinallyReturnBlockArray,
		BlockCount
		);

	return true;
}

bool
CControlFlowMgr::CheckReturn ()
{
	if (m_pCurrentBlock->HasTerminator ())
		return true;

	CFunction* pFunction = m_pModule->m_FunctionMgr.GetCurrentFunction ();
	CType* pReturnType = pFunction->GetType ()->GetReturnType ();

	if (!(m_pCurrentBlock->GetFlags () & EBasicBlockFlag_Reachable))
	{
		m_pModule->m_LlvmIrBuilder.CreateUnreachable (); // just to make LLVM happy
	}
	else if (pReturnType->GetTypeKind () == EType_Void)
	{
		m_pModule->m_ControlFlowMgr.Return ();
	}
	else if (!(m_pModule->m_ControlFlowMgr.GetFlags () & EControlFlowFlag_HasReturn))
	{
		err::SetFormatStringError (
			"function '%s' must return a '%s' value",
			pFunction->m_Tag.cc (),
			pReturnType->GetTypeString ().cc ()
			);
		return false;
	}
	else
	{
		err::SetFormatStringError (
			"not all control paths in function '%s' return a value",
			pFunction->m_Tag.cc ()
			);
		return false;
	}

	return true;
}

//.............................................................................

} // namespace jnc {
