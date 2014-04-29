#include "pch.h"
#include "jnc_ControlFlowMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
CControlFlowMgr::IfStmt_Create (TIfStmt* pStmt)
{
	pStmt->m_pThenBlock = CreateBlock ("if_then");
	pStmt->m_pElseBlock = CreateBlock ("if_else");
	pStmt->m_pFollowBlock = pStmt->m_pElseBlock;
}

bool
CControlFlowMgr::IfStmt_Condition (
	TIfStmt* pStmt,
	const CValue& Value,
	const CToken::CPos& Pos
	)
{
	bool Result = ConditionalJump (Value, pStmt->m_pThenBlock, pStmt->m_pElseBlock);
	if (!Result)
		return false;

	m_pModule->m_NamespaceMgr.OpenScope (Pos);
	return true;
}

void
CControlFlowMgr::IfStmt_Else (
	TIfStmt* pStmt,
	const CToken::CPos& Pos
	)
{
	m_pModule->m_NamespaceMgr.CloseScope ();
	pStmt->m_pFollowBlock = CreateBlock ("if_follow");
	Jump (pStmt->m_pFollowBlock, pStmt->m_pElseBlock);
	m_pModule->m_NamespaceMgr.OpenScope (Pos);
}

void
CControlFlowMgr::IfStmt_Follow (TIfStmt* pStmt)
{
	m_pModule->m_NamespaceMgr.CloseScope ();
	Follow (pStmt->m_pFollowBlock);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
CControlFlowMgr::SwitchStmt_Create (TSwitchStmt* pStmt)
{
	pStmt->m_pSwitchBlock = NULL;
	pStmt->m_pDefaultBlock = NULL;
	pStmt->m_pFollowBlock = CreateBlock ("switch_follow");
}

bool
CControlFlowMgr::SwitchStmt_Condition (
	TSwitchStmt* pStmt,
	const CValue& Value,
	const CToken::CPos& Pos
	)
{
	bool Result = m_pModule->m_OperatorMgr.CastOperator (Value, EType_Int, &pStmt->m_Value);
	if (!Result)
		return false;

	pStmt->m_pSwitchBlock = GetCurrentBlock ();

	CBasicBlock* pBodyBlock = CreateBlock ("switch_body");
	SetCurrentBlock (pBodyBlock);
	MarkUnreachable (pBodyBlock);

	CScope* pScope = m_pModule->m_NamespaceMgr.OpenScope (Pos);
	pScope->m_pBreakBlock = pStmt->m_pFollowBlock;
	return true;
}

bool
CControlFlowMgr::SwitchStmt_Case (
	TSwitchStmt* pStmt,
	intptr_t Value,
	const CToken::CPos& Pos
	)
{
	rtl::CHashTableMapIteratorT <intptr_t, CBasicBlock*> It = pStmt->m_CaseMap.Goto (Value);
	if (It->m_Value)
	{
		err::SetFormatStringError ("redefinition of label (%d) of switch statement", Value);
		return false;
	}

	m_pModule->m_NamespaceMgr.CloseScope ();

	CBasicBlock* pBlock = CreateBlock ("switch_case");
	pBlock->m_Flags |= (pStmt->m_pSwitchBlock->m_Flags & EBasicBlockFlag_Reachable);
	Follow (pBlock);
	It->m_Value = pBlock;

	CScope* pScope = m_pModule->m_NamespaceMgr.OpenScope (Pos);
	pScope->m_pBreakBlock = pStmt->m_pFollowBlock;
	return true;
}

bool
CControlFlowMgr::SwitchStmt_Default (
	TSwitchStmt* pStmt,
	const CToken::CPos& Pos
	)
{
	if (pStmt->m_pDefaultBlock)
	{
		err::SetFormatStringError ("redefinition of 'default' label of switch statement");
		return false;
	}

	CBasicBlock* pBlock = CreateBlock ("switch_default");
	pBlock->m_Flags |= (pStmt->m_pSwitchBlock->m_Flags & EBasicBlockFlag_Reachable);
	Follow (pBlock);
	pStmt->m_pDefaultBlock = pBlock;

	CScope* pScope = m_pModule->m_NamespaceMgr.OpenScope (Pos);
	pScope->m_pBreakBlock = pStmt->m_pFollowBlock;
	return true;
}

void
CControlFlowMgr::SwitchStmt_Follow (TSwitchStmt* pStmt)
{
	m_pModule->m_NamespaceMgr.CloseScope ();
	Follow (pStmt->m_pFollowBlock);

	SetCurrentBlock (pStmt->m_pSwitchBlock);

	CBasicBlock* pDefaultBlock = pStmt->m_pDefaultBlock ? pStmt->m_pDefaultBlock : pStmt->m_pFollowBlock;

	m_pModule->m_LlvmIrBuilder.CreateSwitch (
		pStmt->m_Value,
		pDefaultBlock,
		pStmt->m_CaseMap.GetHead (),
		pStmt->m_CaseMap.GetCount ()
		);

	SetCurrentBlock (pStmt->m_pFollowBlock);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
CControlFlowMgr::WhileStmt_Create (TWhileStmt* pStmt)
{
	pStmt->m_pConditionBlock = CreateBlock ("while_condition");
	pStmt->m_pBodyBlock = CreateBlock ("while_body");
	pStmt->m_pFollowBlock = CreateBlock ("while_follow");
	Follow (pStmt->m_pConditionBlock);
}

bool
CControlFlowMgr::WhileStmt_Condition (
	TWhileStmt* pStmt,
	const CValue& Value,
	const CToken::CPos& Pos
	)
{
	m_pModule->m_OperatorMgr.GcPulse ();

	CScope* pScope = m_pModule->m_NamespaceMgr.OpenScope (Pos);
	pScope->m_pBreakBlock = pStmt->m_pFollowBlock;
	pScope->m_pContinueBlock = pStmt->m_pConditionBlock;
	return ConditionalJump (Value, pStmt->m_pBodyBlock, pStmt->m_pFollowBlock);
}

void
CControlFlowMgr::WhileStmt_Follow (TWhileStmt* pStmt)
{
	m_pModule->m_NamespaceMgr.CloseScope ();
	Jump (pStmt->m_pConditionBlock, pStmt->m_pFollowBlock);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
CControlFlowMgr::DoStmt_Create (TDoStmt* pStmt)
{
	pStmt->m_pConditionBlock = CreateBlock ("do_condition");
	pStmt->m_pBodyBlock = CreateBlock ("do_body");
	pStmt->m_pFollowBlock = CreateBlock ("do_follow");
	Follow (pStmt->m_pBodyBlock);
}

void
CControlFlowMgr::DoStmt_PreBody (
	TDoStmt* pStmt,
	const CToken::CPos& Pos
	)
{
	m_pModule->m_OperatorMgr.GcPulse ();

	CScope* pScope = m_pModule->m_NamespaceMgr.OpenScope (Pos);
	pScope->m_pBreakBlock = pStmt->m_pFollowBlock;
	pScope->m_pContinueBlock = pStmt->m_pConditionBlock;
}

void
CControlFlowMgr::DoStmt_PostBody (TDoStmt* pStmt)
{
	m_pModule->m_NamespaceMgr.CloseScope ();
	Follow (pStmt->m_pConditionBlock);
}

bool
CControlFlowMgr::DoStmt_Condition (
	TDoStmt* pStmt,
	const CValue& Value
	)
{
	return ConditionalJump (Value, pStmt->m_pBodyBlock, pStmt->m_pFollowBlock, pStmt->m_pFollowBlock);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
CControlFlowMgr::ForStmt_Create (TForStmt* pStmt)
{
	pStmt->m_pBodyBlock = CreateBlock ("for_body");
	pStmt->m_pFollowBlock = CreateBlock ("for_follow");
	pStmt->m_pConditionBlock = pStmt->m_pBodyBlock;
	pStmt->m_pLoopBlock = pStmt->m_pBodyBlock;
}

void
CControlFlowMgr::ForStmt_PreInit (
	TForStmt* pStmt,
	const CToken::CPos& Pos
	)
{
	pStmt->m_pScope = m_pModule->m_NamespaceMgr.OpenScope (Pos);
}

void
CControlFlowMgr::ForStmt_NoCondition (TForStmt* pStmt)
{
	Follow (pStmt->m_pBodyBlock);
}

void
CControlFlowMgr::ForStmt_PreCondition (TForStmt* pStmt)
{
	pStmt->m_pConditionBlock = CreateBlock ("for_condition");
	pStmt->m_pLoopBlock = pStmt->m_pConditionBlock;
	Follow (pStmt->m_pConditionBlock);
}

bool
CControlFlowMgr::ForStmt_PostCondition (
	TForStmt* pStmt,
	const CValue& Value
	)
{
	return ConditionalJump (Value, pStmt->m_pBodyBlock, pStmt->m_pFollowBlock);
}

void
CControlFlowMgr::ForStmt_PreLoop (TForStmt* pStmt)
{
	pStmt->m_pLoopBlock = CreateBlock ("for_loop");
	SetCurrentBlock (pStmt->m_pLoopBlock);
}

void
CControlFlowMgr::ForStmt_PostLoop (TForStmt* pStmt)
{
	Jump (pStmt->m_pConditionBlock, pStmt->m_pBodyBlock);
}

void
CControlFlowMgr::ForStmt_PreBody (TForStmt* pStmt)
{
	pStmt->m_pScope->m_pBreakBlock = pStmt->m_pFollowBlock;
	pStmt->m_pScope->m_pContinueBlock = pStmt->m_pConditionBlock;

	m_pModule->m_OperatorMgr.GcPulse ();
}

void
CControlFlowMgr::ForStmt_PostBody (TForStmt* pStmt)
{
	m_pModule->m_NamespaceMgr.CloseScope ();
	Jump (pStmt->m_pLoopBlock, pStmt->m_pFollowBlock);

	if (!(pStmt->m_pFollowBlock->GetFlags () & EBasicBlockFlag_Jumped))
		MarkUnreachable (pStmt->m_pFollowBlock);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
CControlFlowMgr::OnceStmt_Create (
	TOnceStmt* pStmt,
	const CToken::CPos& Pos,
	EStorage StorageKind
	)
{
	CVariable* pFlagVariable;

	if (StorageKind != EStorage_Static && StorageKind != EStorage_Thread)
	{
		err::SetFormatStringError ("'%s once' is illegal (only 'static' or 'thread' is allowed)", GetStorageKindString (StorageKind));
		return false;
	}

	pFlagVariable = m_pModule->m_VariableMgr.CreateOnceFlagVariable (StorageKind);
	pFlagVariable->GetItemDecl ()->m_Pos = Pos;

	if (StorageKind == EStorage_Static)
	{
		CBasicBlock* pBlock = m_pModule->m_ControlFlowMgr.SetCurrentBlock (m_pModule->GetConstructor ()->GetEntryBlock ());
		m_pModule->m_VariableMgr.AllocatePrimeStaticVariable (pFlagVariable);
		m_pModule->m_ControlFlowMgr.SetCurrentBlock (pBlock);
	}

	OnceStmt_Create (pStmt, pFlagVariable);
	return true;
}

void
CControlFlowMgr::OnceStmt_Create (
	TOnceStmt* pStmt,
	CVariable* pFlagVariable
	)
{
	pStmt->m_pFlagVariable = pFlagVariable;
	pStmt->m_pFollowBlock = CreateBlock ("once_follow");
}

bool
CControlFlowMgr::OnceStmt_PreBody (
	TOnceStmt* pStmt,
	const CToken::CPos& Pos
	)
{
	bool Result;

	EStorage StorageKind = pStmt->m_pFlagVariable->GetStorageKind ();
	ASSERT (StorageKind == EStorage_Static || StorageKind == EStorage_Thread);

	m_pModule->m_NamespaceMgr.SetSourcePos (Pos);

	CType* pType = pStmt->m_pFlagVariable->GetType ();

	CValue Value;

	if (StorageKind == EStorage_Thread)
	{
		CBasicBlock* pBodyBlock = CreateBlock ("once_body");

		Result =
			m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Eq, pStmt->m_pFlagVariable, CValue ((int64_t) 0, pType), &Value) &&
			ConditionalJump (Value, pBodyBlock, pStmt->m_pFollowBlock);

		if (!Result)
			return false;
	}
	else
	{
		Result = m_pModule->m_OperatorMgr.LoadDataRef (pStmt->m_pFlagVariable, &Value);
		if (!Result)
			return false;

		uint_t Flags = EBasicBlockFlag_Jumped | (m_pCurrentBlock->m_Flags & EBasicBlockFlag_Reachable);

		CBasicBlock* pPreBodyBlock = CreateBlock ("once_prebody");
		CBasicBlock* pBodyBlock = CreateBlock ("once_body");
		CBasicBlock* pLoopBlock = CreateBlock ("once_loop");

		pPreBodyBlock->m_Flags |= Flags;
		pBodyBlock->m_Flags |= Flags;
		pLoopBlock->m_Flags |= Flags;

		intptr_t ConstArray [2] = { 0, 1 };
		CBasicBlock* BlockArray [2] = { pPreBodyBlock, pLoopBlock };

		m_pModule->m_LlvmIrBuilder.CreateSwitch (Value, pStmt->m_pFollowBlock, ConstArray, BlockArray, 2);

		// loop

		SetCurrentBlock (pLoopBlock);

		Result =
			m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Eq, pStmt->m_pFlagVariable, CValue (2, pType), &Value) &&
			ConditionalJump (Value, pStmt->m_pFollowBlock, pLoopBlock, pPreBodyBlock);

		if (!Result)
			return false;

		// pre body

		m_pModule->m_LlvmIrBuilder.CreateCmpXchg (
			pStmt->m_pFlagVariable,
			CValue ((int64_t) 0, pType),
			CValue (1, pType),
			llvm::Acquire,
			llvm::CrossThread,
			&Value
			);

		Result =
			m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Eq, Value, CValue ((int64_t) 0, pType), &Value) &&
			ConditionalJump (Value, pBodyBlock, pLoopBlock);

		if (!Result)
			return false;
	}

	m_pModule->m_NamespaceMgr.OpenScope (Pos);
	return true;
}

void
CControlFlowMgr::OnceStmt_PostBody (
	TOnceStmt* pStmt,
	const CToken::CPos& Pos
	)
{
	EStorage StorageKind = pStmt->m_pFlagVariable->GetStorageKind ();
	ASSERT (StorageKind == EStorage_Static || StorageKind == EStorage_Thread);

	CType* pType = pStmt->m_pFlagVariable->GetType ();

	m_pModule->m_NamespaceMgr.CloseScope ();
	m_pModule->m_NamespaceMgr.SetSourcePos (Pos);

	if (StorageKind == EStorage_Thread)
	{
		m_pModule->m_LlvmIrBuilder.CreateStore (
			CValue ((int64_t) 2, pType),
			pStmt->m_pFlagVariable
			);
	}
	else
	{
		CValue TmpValue;
		m_pModule->m_LlvmIrBuilder.CreateRmw (
			llvm::AtomicRMWInst::Xchg,
			pStmt->m_pFlagVariable,
			CValue ((int64_t) 2, pType),
			llvm::Release,
			llvm::CrossThread,
			&TmpValue
			);
	}

	Follow (pStmt->m_pFollowBlock);
}

//.............................................................................

} // namespace jnc {
