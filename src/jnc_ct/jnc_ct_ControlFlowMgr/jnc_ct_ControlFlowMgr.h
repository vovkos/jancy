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

#include "jnc_ct_BasicBlock.h"
#include "jnc_ct_Value.h"
#include "jnc_ct_RegexMgr.h"

namespace jnc {
namespace ct {

class FunctionType;

//..............................................................................

struct IfStmt
{
	BasicBlock* m_thenBlock;
	BasicBlock* m_elseBlock;
	BasicBlock* m_followBlock;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SwitchStmt
{
	Value m_value;
	BasicBlock* m_switchBlock;
	BasicBlock* m_defaultBlock;
	BasicBlock* m_followBlock;
	sl::SimpleHashTable <intptr_t, BasicBlock*> m_caseMap;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct ReSwitchStmt
{
	Value m_regexStateValue;
	Value m_dataValue;
	Value m_sizeValue;

	fsm::Regex m_regex;
	BasicBlock* m_switchBlock;
	BasicBlock* m_defaultBlock;
	BasicBlock* m_followBlock;
	sl::StdList <ReSwitchAcceptContext> m_acceptContextList;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct WhileStmt
{
	BasicBlock* m_conditionBlock;
	BasicBlock* m_bodyBlock;
	BasicBlock* m_followBlock;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct DoStmt
{
	BasicBlock* m_conditionBlock;
	BasicBlock* m_bodyBlock;
	BasicBlock* m_followBlock;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct ForStmt
{
	Scope* m_scope;
	BasicBlock* m_conditionBlock;
	BasicBlock* m_bodyBlock;
	BasicBlock* m_loopBlock;
	BasicBlock* m_followBlock;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct OnceStmt
{
	Variable* m_flagVariable;
	BasicBlock* m_followBlock;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct TryExpr
{
	TryExpr* m_prev;
	BasicBlock* m_catchBlock;
	size_t m_sjljFrameIdx;
};

//..............................................................................

class ControlFlowMgr
{
	friend class Module;

protected:
	Module* m_module;

	sl::StdList <BasicBlock> m_blockList;
	sl::Array <BasicBlock*> m_returnBlockArray;
	sl::Array <BasicBlock*> m_landingPadBlockArray;
	BasicBlock* m_currentBlock;
	BasicBlock* m_unreachableBlock;
	BasicBlock* m_catchFinallyFollowBlock;
	BasicBlock* m_returnBlock;
	BasicBlock* m_dynamicThrowBlock;
	Variable* m_returnValueVariable;
	Variable* m_finallyRouteIdxVariable;
	size_t m_finallyRouteIdx;
	size_t m_sjljFrameCount;
	Value m_sjljFrameArrayValue;
	Value m_prevSjljFrameValue;

public:
	ControlFlowMgr ();

	Module*
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	BasicBlock*
	createBlock (
		const sl::StringRef& name,
		uint_t flags = 0
		);

	BasicBlock*
	getCurrentBlock ()
	{
		return m_currentBlock;
	}

	BasicBlock*
	setCurrentBlock (BasicBlock* block); // returns prev

	void
	markUnreachable (BasicBlock* block);

	void
	markLandingPad (
		BasicBlock* block,
		Scope* scope,
		LandingPadKind landingPadKind
		);

	bool
	deleteUnreachableBlocks ();

	sl::Array <BasicBlock*>
	getReturnBlockArray ()
	{
		return m_returnBlockArray;
	}

	sl::Array <BasicBlock*>
	getLandingPadBlockArray ()
	{
		return m_landingPadBlockArray;
	}

	void
	finalizeFunction ();

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
	checkReturn ();

	// exception handling

	void
	beginTryOperator (TryExpr* tryExpr);

	bool
	endTryOperator (
		TryExpr* tryExpr,
		Value* value
		);

	void
	throwException ();

	bool
	throwExceptionIf (
		const Value& returnValue,
		FunctionType* type
		);

	void
	setJmp (
		BasicBlock* catchBlock,
		size_t sjljFrameIdx
		);

	void
	setJmpFinally (
		BasicBlock* finallyBlock,
		size_t sjljFrameIdx
		);

	void
	finalizeTryScope (Scope* scope);

	bool
	catchLabel (const Token::Pos& pos);

	void
	finalizeCatchScope (Scope* scope);

	bool
	finallyLabel (const Token::Pos& pos);

	void
	finalizeFinallyScope (Scope* scope);

	void
	finalizeDisposableScope (Scope* scope);

	bool
	disposeVariable (Variable* variable);

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
		const Token::Pos& pos,
		uint_t scopeFlags
		);

	bool
	switchStmt_Default (
		SwitchStmt* stmt,
		const Token::Pos& pos,
		uint_t scopeFlags
		);

	void
	switchStmt_Follow (SwitchStmt* stmt);

	// regex switch stmt

	void
	reSwitchStmt_Create (ReSwitchStmt* stmt);

	bool
	reSwitchStmt_Condition (
		ReSwitchStmt* stmt,
		const Value& regexStateValue,
		const Value& dataValue,
		const Value& sizeValue,
		const Token::Pos& pos
		);

	bool
	reSwitchStmt_Case (
		ReSwitchStmt* stmt,
		const sl::StringRef& regexSource,
		const Token::Pos& pos,
		uint_t scopeFlags
		);

	bool
	reSwitchStmt_Default (
		ReSwitchStmt* stmt,
		const Token::Pos& pos,
		uint_t scopeFlags
		);

	bool
	reSwitchStmt_Finalize (ReSwitchStmt* stmt);

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

	Variable*
	getFinallyRouteIdxVariable ();

protected:
	void
	addBlock (BasicBlock* block);

	void
	escapeScope (
		Scope* targetScope,
		BasicBlock* targetBlock
		);

	BasicBlock*
	getUnreachableBlock ();

	BasicBlock*
	getReturnBlock ();

	BasicBlock*
	getDynamicThrowBlock ();

	Variable*
	getReturnValueVariable ();

	void
	normalFinallyFlow ();

	void
	preCreateSjljFrameArray ();

	void
	finalizeSjljFrameArray ();

	void
	setSjljFrame (size_t index);
};

//..............................................................................

} // namespace ct
} // namespace jnc
