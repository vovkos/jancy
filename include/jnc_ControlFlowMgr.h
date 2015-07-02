// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_BasicBlock.h"
#include "jnc_Value.h"

namespace jnc {

class FunctionType;

//.............................................................................

struct IfStmt
{
	BasicBlock* m_thenBlock;
	BasicBlock* m_elseBlock;
	BasicBlock* m_followBlock;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SwitchStmt
{
	Value m_value;
	BasicBlock* m_switchBlock;
	BasicBlock* m_defaultBlock;
	BasicBlock* m_followBlock;
	rtl::HashTableMap <intptr_t, BasicBlock*, axl::rtl::HashId <intptr_t> > m_caseMap;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct WhileStmt
{
	BasicBlock* m_conditionBlock;
	BasicBlock* m_bodyBlock;
	BasicBlock* m_followBlock;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DoStmt
{
	BasicBlock* m_conditionBlock;
	BasicBlock* m_bodyBlock;
	BasicBlock* m_followBlock;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct ForStmt
{
	Scope* m_scope;
	BasicBlock* m_conditionBlock;
	BasicBlock* m_bodyBlock;
	BasicBlock* m_loopBlock;
	BasicBlock* m_followBlock;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct OnceStmt
{
	Variable* m_flagVariable;
	BasicBlock* m_followBlock;
};

//.............................................................................

enum ControlFlowFlag
{
	ControlFlowFlag_HasReturn = 1,
	ControlFlowFlag_HasJump   = 2,
};

//.............................................................................

class ControlFlowMgr
{
	friend class Module;
	friend class FunctionMgr;

protected:
	Module* m_module;

	rtl::StdList <BasicBlock> m_blockList;
	rtl::Array <BasicBlock*> m_returnBlockArray;
	BasicBlock* m_currentBlock;
	BasicBlock* m_unreachableBlock;

	uint_t m_flags;
	intptr_t m_throwLockCount;

public:
	ControlFlowMgr ();

	Module*
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	int
	getFlags ()
	{
		return m_flags;
	}

	void
	lockThrow ()
	{
		m_throwLockCount++;
	}

	void
	unlockThrow ()
	{
		m_throwLockCount--;
	}

	bool
	isThrowLocked ()
	{
		return m_throwLockCount > 0;
	}

	void
	resetJumpFlag ()
	{
		m_flags &= ~ControlFlowFlag_HasJump;
	}

	BasicBlock*
	createBlock (const rtl::String& name);

	BasicBlock*
	getCurrentBlock ()
	{
		return m_currentBlock;
	}

	BasicBlock*
	setCurrentBlock (BasicBlock* block); // returns prev

	void
	markUnreachable (BasicBlock* block);

	bool
	deleteUnreachableBlocks ();

	// jumps

	void
	jump (
		BasicBlock* block,
		BasicBlock* followBlock = NULL
		);

	void
	follow (BasicBlock* block);

	bool
	conditionalJump (
		const Value& value,
		BasicBlock* thenBlock,
		BasicBlock* elseBlock,
		BasicBlock* followBlock = NULL // if NULL then follow with pThenBlock
		);

	bool
	breakJump (size_t level);

	bool
	continueJump (size_t level);

	bool
	ret (const Value& value);

	bool
	ret ()
	{
		return ret (Value ());
	}

	bool
	throwIf (
		const Value& returnValue,
		FunctionType* type
		);

	bool
	catchLabel ();

	bool
	finallyLabel ();

	void
	endCatch ();

	void
	endFinally ();

	void
	endTry ();

	bool
	checkReturn ();

	// if stmt

	void
	ifStmt_Create (IfStmt* stmt);

	bool
	ifStmt_Condition (
		IfStmt* stmt,
		const Value& value,
		const Token::Pos& pos
		);

	void
	ifStmt_Else (
		IfStmt* stmt,
		const Token::Pos& pos
		);

	void
	ifStmt_Follow (IfStmt* stmt);

	// switch stmt

	void
	switchStmt_Create (SwitchStmt* stmt);

	bool
	switchStmt_Condition (
		SwitchStmt* stmt,
		const Value& value,
		const Token::Pos& pos
		);

	bool
	switchStmt_Case (
		SwitchStmt* stmt,
		intptr_t value,
		const Token::Pos& pos
		);

	bool
	switchStmt_Default (
		SwitchStmt* stmt,
		const Token::Pos& pos
		);

	void
	switchStmt_Follow (SwitchStmt* stmt);

	// while stmt

	void
	whileStmt_Create (WhileStmt* stmt);

	bool
	whileStmt_Condition (
		WhileStmt* stmt,
		const Value& value,
		const Token::Pos& pos
		);

	void
	whileStmt_Follow (WhileStmt* stmt);

	// do stmt

	void
	doStmt_Create (DoStmt* stmt);

	void
	doStmt_PreBody (
		DoStmt* stmt,
		const Token::Pos& pos
		);

	void
	doStmt_PostBody (DoStmt* stmt);

	bool
	doStmt_Condition (
		DoStmt* stmt,
		const Value& value
		);

	// for stmt

	void
	forStmt_Create (ForStmt* stmt);

	void
	forStmt_PreInit (
		ForStmt* stmt,
		const Token::Pos& pos
		);

	void
	forStmt_NoCondition (ForStmt* stmt);

	void
	forStmt_PreCondition (ForStmt* stmt);

	bool
	forStmt_PostCondition (
		ForStmt* stmt,
		const Value& value
		);

	void
	forStmt_PreLoop (ForStmt* stmt);

	void
	forStmt_PostLoop (ForStmt* stmt);

	void
	forStmt_PreBody (ForStmt* stmt);

	void
	forStmt_PostBody (ForStmt* stmt);

	// once stmt

	bool
	onceStmt_Create (
		OnceStmt* stmt,
		const Token::Pos& pos,
		StorageKind storageKind = StorageKind_Static
		);

	void
	onceStmt_Create (
		OnceStmt* stmt,
		Variable* flagVariable
		);

	bool
	onceStmt_PreBody (
		OnceStmt* stmt,
		const Token::Pos& pos
		);

	void
	onceStmt_PostBody (
		OnceStmt* stmt,
		const Token::Pos& pos
		);

protected:
	void
	addBlock (BasicBlock* block);

	void
	onLeaveScope (Scope* targetScope = NULL);

	void
	jumpToFinally (Scope* scope);

	BasicBlock*
	getUnreachableBlock ();
};

//.............................................................................

} // namespace jnc {
