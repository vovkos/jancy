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
ControlFlowMgr::ifStmt_Create (IfStmt* stmt)
{
	stmt->m_thenBlock = createBlock ("if_then");
	stmt->m_elseBlock = createBlock ("if_else");
	stmt->m_followBlock = stmt->m_elseBlock;
}

bool
ControlFlowMgr::ifStmt_Condition (
	IfStmt* stmt,
	const Value& value,
	const Token::Pos& pos
	)
{
	bool result = conditionalJump (value, stmt->m_thenBlock, stmt->m_elseBlock);
	if (!result)
		return false;

	m_module->m_namespaceMgr.openScope (pos);
	return true;
}

void
ControlFlowMgr::ifStmt_Else (
	IfStmt* stmt,
	const Token::Pos& pos
	)
{
	m_module->m_namespaceMgr.closeScope ();
	stmt->m_followBlock = createBlock ("if_follow");
	jump (stmt->m_followBlock, stmt->m_elseBlock);
	m_module->m_namespaceMgr.openScope (pos);
}

void
ControlFlowMgr::ifStmt_Follow (IfStmt* stmt)
{
	m_module->m_namespaceMgr.closeScope ();
	follow (stmt->m_followBlock);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
ControlFlowMgr::switchStmt_Create (SwitchStmt* stmt)
{
	stmt->m_switchBlock = NULL;
	stmt->m_defaultBlock = NULL;
	stmt->m_followBlock = createBlock ("switch_follow");
}

bool
ControlFlowMgr::switchStmt_Condition (
	SwitchStmt* stmt,
	const Value& value,
	const Token::Pos& pos
	)
{
	bool result = m_module->m_operatorMgr.castOperator (value, TypeKind_Int, &stmt->m_value);
	if (!result)
		return false;

	stmt->m_switchBlock = getCurrentBlock ();

	BasicBlock* bodyBlock = createBlock ("switch_body");
	setCurrentBlock (bodyBlock);
	markUnreachable (bodyBlock);

	Scope* scope = m_module->m_namespaceMgr.openScope (pos);
	scope->m_breakBlock = stmt->m_followBlock;

	m_module->m_namespaceMgr.openScope (pos);
	return true;
}

bool
ControlFlowMgr::switchStmt_Case (
	SwitchStmt* stmt,
	intptr_t value,
	const Token::Pos& pos,
	uint_t scopeFlags
	)
{
	sl::HashTableMapIterator <intptr_t, BasicBlock*> it = stmt->m_caseMap.visit (value);
	if (it->m_value)
	{
		err::setFormatStringError ("redefinition of label (%d) of switch statement", value);
		return false;
	}

	m_module->m_namespaceMgr.closeScope ();

	BasicBlock* block = createBlock ("switch_case");
	block->m_flags |= (stmt->m_switchBlock->m_flags & BasicBlockFlag_Reachable);
	follow (block);
	it->m_value = block;

	m_module->m_namespaceMgr.openScope (pos);
	return true;
}

bool
ControlFlowMgr::switchStmt_Default (
	SwitchStmt* stmt,
	const Token::Pos& pos,
	uint_t scopeFlags
	)
{
	if (stmt->m_defaultBlock)
	{
		err::setFormatStringError ("redefinition of 'default' label of switch statement");
		return false;
	}

	m_module->m_namespaceMgr.closeScope ();

	BasicBlock* block = createBlock ("switch_default");
	block->m_flags |= (stmt->m_switchBlock->m_flags & BasicBlockFlag_Reachable);
	follow (block);
	stmt->m_defaultBlock = block;

	m_module->m_namespaceMgr.openScope (pos);
	return true;
}

void
ControlFlowMgr::switchStmt_Follow (SwitchStmt* stmt)
{
	m_module->m_namespaceMgr.closeScope ();
	m_module->m_namespaceMgr.closeScope ();
	follow (stmt->m_followBlock);

	setCurrentBlock (stmt->m_switchBlock);

	BasicBlock* defaultBlock = stmt->m_defaultBlock ? stmt->m_defaultBlock : stmt->m_followBlock;
	defaultBlock->m_flags |= (stmt->m_switchBlock->m_flags & BasicBlockFlag_Reachable);

	m_module->m_llvmIrBuilder.createSwitch (
		stmt->m_value,
		defaultBlock,
		stmt->m_caseMap.getHead (),
		stmt->m_caseMap.getCount ()
		);

	setCurrentBlock (stmt->m_followBlock);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
ControlFlowMgr::whileStmt_Create (WhileStmt* stmt)
{
	stmt->m_conditionBlock = createBlock ("while_condition");
	stmt->m_bodyBlock = createBlock ("while_body");
	stmt->m_followBlock = createBlock ("while_follow");
	follow (stmt->m_conditionBlock);
}

bool
ControlFlowMgr::whileStmt_Condition (
	WhileStmt* stmt,
	const Value& value,
	const Token::Pos& pos
	)
{
	m_module->m_operatorMgr.gcSafePoint ();

	Scope* scope = m_module->m_namespaceMgr.openScope (pos);
	scope->m_breakBlock = stmt->m_followBlock;
	scope->m_continueBlock = stmt->m_conditionBlock;
	return conditionalJump (value, stmt->m_bodyBlock, stmt->m_followBlock);
}

void
ControlFlowMgr::whileStmt_Follow (WhileStmt* stmt)
{
	m_module->m_namespaceMgr.closeScope ();
	jump (stmt->m_conditionBlock, stmt->m_followBlock);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
ControlFlowMgr::doStmt_Create (DoStmt* stmt)
{
	stmt->m_conditionBlock = createBlock ("do_condition");
	stmt->m_bodyBlock = createBlock ("do_body");
	stmt->m_followBlock = createBlock ("do_follow");
	follow (stmt->m_bodyBlock);
}

void
ControlFlowMgr::doStmt_PreBody (
	DoStmt* stmt,
	const Token::Pos& pos
	)
{
	m_module->m_operatorMgr.gcSafePoint ();

	Scope* scope = m_module->m_namespaceMgr.openScope (pos);
	scope->m_breakBlock = stmt->m_followBlock;
	scope->m_continueBlock = stmt->m_conditionBlock;
}

void
ControlFlowMgr::doStmt_PostBody (DoStmt* stmt)
{
	m_module->m_namespaceMgr.closeScope ();
	follow (stmt->m_conditionBlock);
}

bool
ControlFlowMgr::doStmt_Condition (
	DoStmt* stmt,
	const Value& value
	)
{
	return conditionalJump (value, stmt->m_bodyBlock, stmt->m_followBlock, stmt->m_followBlock);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
ControlFlowMgr::forStmt_Create (ForStmt* stmt)
{
	stmt->m_bodyBlock = createBlock ("for_body");
	stmt->m_followBlock = createBlock ("for_follow");
	stmt->m_conditionBlock = stmt->m_bodyBlock;
	stmt->m_loopBlock = stmt->m_bodyBlock;
}

void
ControlFlowMgr::forStmt_PreInit (
	ForStmt* stmt,
	const Token::Pos& pos
	)
{
	stmt->m_scope = m_module->m_namespaceMgr.openScope (pos);
}

void
ControlFlowMgr::forStmt_NoCondition (ForStmt* stmt)
{
	follow (stmt->m_bodyBlock);
}

void
ControlFlowMgr::forStmt_PreCondition (ForStmt* stmt)
{
	stmt->m_conditionBlock = createBlock ("for_condition");
	stmt->m_loopBlock = stmt->m_conditionBlock;
	follow (stmt->m_conditionBlock);
}

bool
ControlFlowMgr::forStmt_PostCondition (
	ForStmt* stmt,
	const Value& value
	)
{
	return conditionalJump (value, stmt->m_bodyBlock, stmt->m_followBlock);
}

void
ControlFlowMgr::forStmt_PreLoop (ForStmt* stmt)
{
	stmt->m_loopBlock = createBlock ("for_loop", m_currentBlock->m_flags & BasicBlockFlag_Reachable);
	setCurrentBlock (stmt->m_loopBlock);
}

void
ControlFlowMgr::forStmt_PostLoop (ForStmt* stmt)
{
	jump (stmt->m_conditionBlock, stmt->m_bodyBlock);
}

void
ControlFlowMgr::forStmt_PreBody (ForStmt* stmt)
{
	stmt->m_scope->m_breakBlock = stmt->m_followBlock;
	stmt->m_scope->m_continueBlock = stmt->m_loopBlock;

	m_module->m_operatorMgr.gcSafePoint ();
}

void
ControlFlowMgr::forStmt_PostBody (ForStmt* stmt)
{
	m_module->m_namespaceMgr.closeScope ();
	jump (stmt->m_loopBlock, stmt->m_followBlock);

	if (!(stmt->m_followBlock->getFlags () & BasicBlockFlag_Jumped))
		markUnreachable (stmt->m_followBlock);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
ControlFlowMgr::onceStmt_Create (
	OnceStmt* stmt,
	const Token::Pos& pos,
	StorageKind storageKind
	)
{
	Variable* flagVariable;

	if (storageKind != StorageKind_Static && storageKind != StorageKind_Tls)
	{
		err::setFormatStringError ("'%s once' is illegal (only 'static' or 'threadlocal' is allowed)", getStorageKindString (storageKind));
		return false;
	}

	flagVariable = m_module->m_variableMgr.createOnceFlagVariable (storageKind);
	flagVariable->m_pos = pos;

	if (storageKind == StorageKind_Static)
	{
		BasicBlock* block = setCurrentBlock (m_module->getConstructor ()->getEntryBlock ());
		m_module->m_operatorMgr.zeroInitialize (flagVariable);
		setCurrentBlock (block);
	}

	onceStmt_Create (stmt, flagVariable);
	return true;
}

void
ControlFlowMgr::onceStmt_Create (
	OnceStmt* stmt,
	Variable* flagVariable
	)
{
	stmt->m_flagVariable = flagVariable;
	stmt->m_followBlock = createBlock ("once_follow");
}

bool
ControlFlowMgr::onceStmt_PreBody (
	OnceStmt* stmt,
	const Token::Pos& pos
	)
{
	bool result;

	StorageKind storageKind = stmt->m_flagVariable->getStorageKind ();
	ASSERT (storageKind == StorageKind_Static || storageKind == StorageKind_Tls);

	m_module->m_namespaceMgr.setSourcePos (pos);

	Type* type = stmt->m_flagVariable->getType ();

	Value value;

	if (storageKind == StorageKind_Tls)
	{
		BasicBlock* bodyBlock = createBlock ("once_body");

		result =
			m_module->m_operatorMgr.binaryOperator (BinOpKind_Eq, stmt->m_flagVariable, Value ((int64_t) 0, type), &value) &&
			conditionalJump (value, bodyBlock, stmt->m_followBlock);

		if (!result)
			return false;
	}
	else
	{
		result = m_module->m_operatorMgr.loadDataRef (stmt->m_flagVariable, &value);
		if (!result)
			return false;

		uint_t flags = BasicBlockFlag_Jumped | (m_currentBlock->m_flags & BasicBlockFlag_Reachable);

		BasicBlock* preBodyBlock = createBlock ("once_prebody");
		BasicBlock* bodyBlock = createBlock ("once_body");
		BasicBlock* loopBlock = createBlock ("once_loop");

		preBodyBlock->m_flags |= flags;
		bodyBlock->m_flags |= flags;
		loopBlock->m_flags |= flags;

		intptr_t constArray [2] = { 0, 1 };
		BasicBlock* blockArray [2] = { preBodyBlock, loopBlock };

		m_module->m_llvmIrBuilder.createSwitch (value, stmt->m_followBlock, constArray, blockArray, 2);

		// loop

		setCurrentBlock (loopBlock);

		result =
			m_module->m_operatorMgr.binaryOperator (BinOpKind_Eq, stmt->m_flagVariable, Value (2, type), &value) &&
			conditionalJump (value, stmt->m_followBlock, loopBlock, preBodyBlock);

		if (!result)
			return false;

		// pre body

		m_module->m_llvmIrBuilder.createCmpXchg (
			stmt->m_flagVariable,
			Value ((int64_t) 0, type),
			Value (1, type),
			llvm::AtomicOrdering::Acquire,
			llvm::CrossThread,
			&value
			);

		m_module->m_llvmIrBuilder.createExtractValue (value, 0, type, &value);

		result =
			m_module->m_operatorMgr.binaryOperator (BinOpKind_Eq, value, Value ((int64_t) 0, type), &value) &&
			conditionalJump (value, bodyBlock, loopBlock);

		if (!result)
			return false;
	}

	m_module->m_namespaceMgr.openScope (pos);
	return true;
}

void
ControlFlowMgr::onceStmt_PostBody (
	OnceStmt* stmt,
	const Token::Pos& pos
	)
{
	StorageKind storageKind = stmt->m_flagVariable->getStorageKind ();
	ASSERT (storageKind == StorageKind_Static || storageKind == StorageKind_Tls);

	Type* type = stmt->m_flagVariable->getType ();

	m_module->m_namespaceMgr.closeScope ();
	m_module->m_namespaceMgr.setSourcePos (pos);

	if (storageKind == StorageKind_Tls)
	{
		m_module->m_llvmIrBuilder.createStore (
			Value ((int64_t) 2, type),
			stmt->m_flagVariable
			);
	}
	else
	{
		Value tmpValue;
		m_module->m_llvmIrBuilder.createRmw (
			llvm::AtomicRMWInst::Xchg,
			stmt->m_flagVariable,
			Value ((int64_t) 2, type),
			llvm::AtomicOrdering::Release,
			llvm::CrossThread,
			&tmpValue
			);
	}

	follow (stmt->m_followBlock);
}

//..............................................................................

} // namespace ct
} // namespace jnc
