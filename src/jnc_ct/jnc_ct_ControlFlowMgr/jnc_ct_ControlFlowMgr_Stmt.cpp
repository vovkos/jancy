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
#include "jnc_rtl_Regex.h"

namespace jnc {
namespace ct {

//..............................................................................

void
ControlFlowMgr::setRegexFlags(
	RegexCondStmt* stmt,
	AttributeBlock* attributeBlock,
	uint_t defaultAnchorFlags
) {
	if (attributeBlock) {
		PragmaConfig pragmaConfig;

		const sl::Array<Attribute*>& attributeArray = attributeBlock->getAttributeArray();
		size_t count = attributeArray.getCount();
		for (size_t i = 0; i < count; i++) {
			Attribute* attr = attributeArray[i];
			Pragma pragmaKind = PragmaMap::findValue(attr->getName(), Pragma_Undefined);
			if (pragmaKind)
				if (!attr->getValue())
					pragmaConfig.setPragma(pragmaKind, PragmaState_NoValue);
				else
					pragmaConfig.setPragma(
						pragmaKind,
						PragmaState_CustomValue,
						*(uint8_t*)attr->getValue().getConstData()
					);
		}

		if (pragmaConfig.m_regexFlagMask) {
			stmt->m_attrRegexFlagMask = pragmaConfig.m_regexFlagMask;
			stmt->m_prevPragmaRegexFlags = m_module->m_pragmaMgr->m_regexFlags;
			stmt->m_prevPragmaRegexFlagMask = m_module->m_pragmaMgr->m_regexFlagMask;

			m_module->m_pragmaMgr->m_regexFlags &= ~pragmaConfig.m_regexFlagMask;
			m_module->m_pragmaMgr->m_regexFlags |= pragmaConfig.m_regexFlags;
			m_module->m_pragmaMgr->m_regexFlagMask |= pragmaConfig.m_regexFlagMask;
		}
	}

	stmt->m_regexFlags = m_module->m_pragmaMgr->m_regexFlags;
	if (!(m_module->m_pragmaMgr->m_regexFlagMask & (re2::ExecFlag_Anchored | re2::ExecFlag_FullMatch)))
		stmt->m_regexFlags |= defaultAnchorFlags;
}

inline
void
ControlFlowMgr::restoreRegexFlags(RegexCondStmt* stmt) {
	if (!stmt->m_attrRegexFlagMask)
		return;

	m_module->m_pragmaMgr->m_regexFlags &= ~stmt->m_attrRegexFlagMask;
	m_module->m_pragmaMgr->m_regexFlags |= stmt->m_prevPragmaRegexFlags;
	m_module->m_pragmaMgr->m_regexFlagMask &= ~stmt->m_attrRegexFlagMask;
	m_module->m_pragmaMgr->m_regexFlagMask |= stmt->m_prevPragmaRegexFlagMask;
}

void
ControlFlowMgr::ifStmt_Create(
	IfStmt* stmt,
	AttributeBlock* attributeBlock
) {
	setRegexFlags(stmt, attributeBlock);
	stmt->m_thenBlock = createBlock("if_then");
	stmt->m_elseBlock = createBlock("if_else");
	stmt->m_followBlock = stmt->m_elseBlock;
	m_regexCondStmt = stmt;
}

bool
ControlFlowMgr::ifStmt_Condition(
	IfStmt* stmt,
	const Value& value,
	const lex::LineCol& pos
) {
	m_regexCondStmt = NULL;

	bool result = conditionalJump(value, stmt->m_thenBlock, stmt->m_elseBlock);
	if (!result)
		return false;

	m_module->m_namespaceMgr.openScope(pos);
	return true;
}

void
ControlFlowMgr::ifStmt_Else(
	IfStmt* stmt,
	const lex::LineCol& pos
) {
	m_module->m_namespaceMgr.closeScope();
	stmt->m_followBlock = createBlock("if_follow");
	jump(stmt->m_followBlock, stmt->m_elseBlock);
	m_module->m_namespaceMgr.openScope(pos, ScopeFlag_Else);
}

void
ControlFlowMgr::ifStmt_Follow(IfStmt* stmt) {
	m_module->m_namespaceMgr.closeScope();
	follow(stmt->m_followBlock);
	restoreRegexFlags(stmt);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
ControlFlowMgr::switchStmt_Create(SwitchStmt* stmt) {
	stmt->m_switchBlock = NULL;
	stmt->m_defaultBlock = NULL;
	stmt->m_followBlock = createBlock("switch_follow");
}

bool
ControlFlowMgr::switchStmt_Condition(
	SwitchStmt* stmt,
	const Value& value,
	const lex::LineCol& pos
) {
	bool result = m_module->m_operatorMgr.castOperator(value, TypeKind_Int, &stmt->m_value);
	if (!result)
		return false;

	stmt->m_switchBlock = getCurrentBlock();

	BasicBlock* bodyBlock = createBlock("switch_body");
	setCurrentBlock(bodyBlock);
	markUnreachable(bodyBlock);

	Scope* scope = m_module->m_namespaceMgr.openScope(pos);
	scope->m_breakBlock = stmt->m_followBlock;

	m_module->m_namespaceMgr.openScope(pos);
	return true;
}

bool
ControlFlowMgr::switchStmt_Case(
	SwitchStmt* stmt,
	int64_t value,
	const lex::LineCol& pos,
	uint_t scopeFlags
) {
	sl::HashTableIterator<int64_t, BasicBlock*> it = stmt->m_caseMap.visit(value);
	if (it->m_value) {
		err::setFormatStringError("redefinition of label (%lld) of 'switch' statement", value);
		return false;
	}

	m_module->m_namespaceMgr.closeScope();

	BasicBlock* block = createBlock("switch_case");
	block->m_flags |= (stmt->m_switchBlock->m_flags & BasicBlockFlag_Reachable);
	follow(block);
	it->m_value = block;

	m_module->m_namespaceMgr.openScope(pos);
	return true;
}

bool
ControlFlowMgr::switchStmt_Default(
	SwitchStmt* stmt,
	const lex::LineCol& pos,
	uint_t scopeFlags
) {
	if (stmt->m_defaultBlock) {
		err::setFormatStringError("redefinition of 'default' label of 'switch' statement");
		return false;
	}

	m_module->m_namespaceMgr.closeScope();

	BasicBlock* block = createBlock("switch_default");
	block->m_flags |= (stmt->m_switchBlock->m_flags & BasicBlockFlag_Reachable);
	follow(block);
	stmt->m_defaultBlock = block;

	m_module->m_namespaceMgr.openScope(pos);
	return true;
}

void
ControlFlowMgr::switchStmt_Follow(SwitchStmt* stmt) {
	m_module->m_namespaceMgr.closeScope();
	m_module->m_namespaceMgr.closeScope();
	follow(stmt->m_followBlock);

	setCurrentBlock(stmt->m_switchBlock);

	BasicBlock* defaultBlock = stmt->m_defaultBlock ? stmt->m_defaultBlock : stmt->m_followBlock;
	defaultBlock->m_flags |= (stmt->m_switchBlock->m_flags & BasicBlockFlag_Reachable);

	if (m_module->hasCodeGen())
		m_module->m_llvmIrBuilder.createSwitch(
			stmt->m_value,
			defaultBlock,
			stmt->m_caseMap.getHead(),
			stmt->m_caseMap.getCount()
		);

	setCurrentBlock(stmt->m_followBlock);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
ControlFlowMgr::regexSwitchStmt_Create(
	RegexSwitchStmt* stmt,
	AttributeBlock* attributeBlock
) {
	setRegexFlags(stmt, attributeBlock, re2::ExecFlag_FullMatch);
	stmt->m_switchBlock = NULL;
	stmt->m_defaultBlock = NULL;
	stmt->m_followBlock = createBlock("regex_switch_follow");
	stmt->m_regex.createSwitch(stmt->m_regexFlags);
	m_module->m_variableMgr.getRegexMatchVariable(); // ensure the regex match variable in the parent scope
}

bool
ControlFlowMgr::regexSwitchStmt_Condition(
	RegexSwitchStmt* stmt,
	const Value& value1,
	const Value& value2,
	const lex::LineCol& pos
) {
	ClassType* regexStateType = (ClassType*)m_module->m_typeMgr.getStdType(StdType_RegexState);

	bool result;

	if (!value2) { // simple-mode -- one parameter (text); create a fresh-new regex-state
		stmt->m_execMethodName = "execEof";

		Type* flagsType = (ClassType*)m_module->m_typeMgr.getStdType(StdType_RegexExecFlags);
		sl::BoxList<Value> argValueList;
		argValueList.insertTail(Value(stmt->m_regexFlags, flagsType));

		Variable* regexVariable = m_module->m_variableMgr.createStaticRegexVariable(stmt->m_regex);
		if (!regexVariable)
			return false;

		result =
			m_module->m_operatorMgr.newOperator(
				regexStateType,
				&argValueList,
				&stmt->m_regexStateValue
			) &&
			m_module->m_operatorMgr.castOperator(
				value1,
				m_module->m_typeMgr.getPrimitiveType(TypeKind_String),
				&stmt->m_textValue
			);
	} else { // streaming mode -- two parameters (state, text)
		stmt->m_execMethodName = "exec";

		result = m_module->m_operatorMgr.castOperator(
			value1,
			regexStateType->getClassPtrType(ClassPtrTypeKind_Normal, PtrTypeFlag_Safe),
			&stmt->m_regexStateValue
		) &&
		m_module->m_operatorMgr.castOperator(
			value2,
			m_module->m_typeMgr.getPrimitiveType(TypeKind_String),
			&stmt->m_textValue
		);
	}

	if (!result)
		return false;

	stmt->m_switchBlock = getCurrentBlock();

	BasicBlock* bodyBlock = createBlock("regex_switch_body");
	setCurrentBlock(bodyBlock);
	markUnreachable(bodyBlock);

	Scope* scope = m_module->m_namespaceMgr.openScope(pos);
	scope->m_breakBlock = stmt->m_followBlock;

	m_module->m_namespaceMgr.openScope(pos);
	return true;
}

bool
ControlFlowMgr::regexSwitchStmt_Case(
	RegexSwitchStmt* stmt,
	const sl::StringRef& regexSource,
	const lex::LineCol& pos,
	uint_t scopeFlags
) {
	m_module->m_namespaceMgr.closeScope();

	BasicBlock* block = createBlock("regex_switch_case");
	block->m_flags |= (stmt->m_switchBlock->m_flags & BasicBlockFlag_Reachable);
	follow(block);

	uint_t caseId = stmt->m_regex.compileSwitchCase(regexSource);
	if (caseId == -1)
		return false;

	stmt->m_caseMap.getCount();
	stmt->m_caseMap[caseId] = block;

	m_module->m_namespaceMgr.openScope(pos);
	return true;
}

bool
ControlFlowMgr::regexSwitchStmt_Default(
	RegexSwitchStmt* stmt,
	const lex::LineCol& pos,
	uint_t scopeFlags
) {
	if (stmt->m_defaultBlock) {
		err::setFormatStringError("redefinition of 'default' label of 'regex switch' statement");
		return false;
	}

	m_module->m_namespaceMgr.closeScope();

	BasicBlock* block = createBlock("regex_switch_default");
	block->m_flags |= (stmt->m_switchBlock->m_flags & BasicBlockFlag_Reachable);
	follow(block);
	stmt->m_defaultBlock = block;

	m_module->m_namespaceMgr.openScope(pos);
	return true;
}

bool
ControlFlowMgr::regexSwitchStmt_Finalize(RegexSwitchStmt* stmt) {
	bool result;

	m_module->m_namespaceMgr.closeScope();
	m_module->m_namespaceMgr.closeScope();
	follow(stmt->m_followBlock);

	if (!stmt->m_regex.getSwitchCaseCount()) {
		err::setError("empty regex switch");
		return false;
	}

	setCurrentBlock(stmt->m_switchBlock);

	BasicBlock* defaultBlock = stmt->m_defaultBlock ? stmt->m_defaultBlock : stmt->m_followBlock;
	defaultBlock->m_flags |= (stmt->m_switchBlock->m_flags & BasicBlockFlag_Reachable);

	// serialize regex and save the storage value

	stmt->m_regex.finalizeSwitch();

	Variable* regexVariable = m_module->m_variableMgr.createStaticRegexVariable(stmt->m_regex);
	if (!regexVariable)
		return false;

	// execute regex with the supplied state and data;
	// on match, jump to the corresponding case-id

	Value execValue;
	Value execResultValue;
	Value cmpResultValue;
	Value matchValue;
	Value caseIdValue;
	Value matchConstValue((int64_t)re2::ExecResult_Match, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int));

	BasicBlock* matchBlock = createBlock("regex_match");

	result =
		m_module->m_operatorMgr.memberOperator(regexVariable, stmt->m_execMethodName, &execValue) &&
		m_module->m_operatorMgr.callOperator(execValue, stmt->m_regexStateValue, stmt->m_textValue, &execResultValue) &&
		m_module->m_operatorMgr.binaryOperator(BinOpKind_Eq, execResultValue, matchConstValue, &cmpResultValue) &&
		conditionalJump(cmpResultValue, matchBlock, defaultBlock, matchBlock) &&
		m_module->m_operatorMgr.memberOperator(stmt->m_regexStateValue, "m_match", &matchValue) &&
		m_module->m_operatorMgr.storeDataRef(m_module->m_variableMgr.getRegexMatchVariable(), matchValue) &&
		m_module->m_operatorMgr.memberOperator(matchValue, "m_id", &caseIdValue) &&
		m_module->m_operatorMgr.prepareOperand(&caseIdValue);

	if (!result)
		return false;

	if (m_module->hasCodeGen())
		m_module->m_llvmIrBuilder.createSwitch(
			caseIdValue,
			defaultBlock,
			stmt->m_caseMap.getHead(),
			stmt->m_caseMap.getCount()
		);

	setCurrentBlock(stmt->m_followBlock);
	restoreRegexFlags(stmt);
	return true;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
ControlFlowMgr::whileStmt_Create(
	WhileStmt* stmt,
	AttributeBlock* attributeBlock
) {
	setRegexFlags(stmt, attributeBlock);
	stmt->m_conditionBlock = createBlock("while_condition");
	stmt->m_bodyBlock = createBlock("while_body");
	stmt->m_followBlock = createBlock("while_follow");
	follow(stmt->m_conditionBlock);
	m_regexCondStmt = stmt;
}

bool
ControlFlowMgr::whileStmt_Condition(
	WhileStmt* stmt,
	const Value& value,
	const lex::LineCol& pos
) {
	m_regexCondStmt = NULL;
	m_module->m_operatorMgr.gcSafePoint();

	Scope* scope = m_module->m_namespaceMgr.openScope(pos);
	scope->m_breakBlock = stmt->m_followBlock;
	scope->m_continueBlock = stmt->m_conditionBlock;
	return conditionalJump(value, stmt->m_bodyBlock, stmt->m_followBlock);
}

void
ControlFlowMgr::whileStmt_Follow(WhileStmt* stmt) {
	m_module->m_namespaceMgr.closeScope();
	jump(stmt->m_conditionBlock, stmt->m_followBlock);
	restoreRegexFlags(stmt);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
ControlFlowMgr::doStmt_Create(DoStmt* stmt) {
	stmt->m_conditionBlock = createBlock("do_condition");
	stmt->m_bodyBlock = createBlock("do_body");
	stmt->m_followBlock = createBlock("do_follow");
	follow(stmt->m_bodyBlock);
}

void
ControlFlowMgr::doStmt_PreBody(
	DoStmt* stmt,
	const lex::LineCol& pos
) {
	m_module->m_operatorMgr.gcSafePoint();

	Scope* scope = m_module->m_namespaceMgr.openScope(pos);
	scope->m_breakBlock = stmt->m_followBlock;
	scope->m_continueBlock = stmt->m_conditionBlock;
}

void
ControlFlowMgr::doStmt_PostBody(DoStmt* stmt) {
	m_module->m_namespaceMgr.closeScope();
	follow(stmt->m_conditionBlock);
}

bool
ControlFlowMgr::doStmt_Condition(
	DoStmt* stmt,
	const Value& value
) {
	return conditionalJump(value, stmt->m_bodyBlock, stmt->m_followBlock, stmt->m_followBlock);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
ControlFlowMgr::forStmt_Create(
	ForStmt* stmt,
	AttributeBlock* attributeBlock
) {
	setRegexFlags(stmt, attributeBlock);
	stmt->m_bodyBlock = createBlock("for_body");
	stmt->m_followBlock = createBlock("for_follow");
	stmt->m_conditionBlock = stmt->m_bodyBlock;
	stmt->m_loopBlock = stmt->m_bodyBlock;
}

void
ControlFlowMgr::forStmt_PreInit(
	ForStmt* stmt,
	const lex::LineCol& pos
) {
	stmt->m_scope = m_module->m_namespaceMgr.openScope(pos);
}

void
ControlFlowMgr::forStmt_NoCondition(ForStmt* stmt) {
	follow(stmt->m_bodyBlock);
}

void
ControlFlowMgr::forStmt_PreCondition(ForStmt* stmt) {
	stmt->m_conditionBlock = createBlock("for_condition");
	stmt->m_loopBlock = stmt->m_conditionBlock;
	follow(stmt->m_conditionBlock);
	m_regexCondStmt = stmt;
}

bool
ControlFlowMgr::forStmt_PostCondition(
	ForStmt* stmt,
	const Value& value
) {
	m_regexCondStmt = NULL;
	return conditionalJump(value, stmt->m_bodyBlock, stmt->m_followBlock);
}

void
ControlFlowMgr::forStmt_PreLoop(ForStmt* stmt) {
	stmt->m_loopBlock = createBlock("for_loop", m_currentBlock->m_flags & BasicBlockFlag_Reachable);
	setCurrentBlock(stmt->m_loopBlock);
}

void
ControlFlowMgr::forStmt_PostLoop(ForStmt* stmt) {
	jump(stmt->m_conditionBlock, stmt->m_bodyBlock);
}

void
ControlFlowMgr::forStmt_PreBody(ForStmt* stmt) {
	stmt->m_scope->m_breakBlock = stmt->m_followBlock;
	stmt->m_scope->m_continueBlock = stmt->m_loopBlock;

	m_module->m_operatorMgr.gcSafePoint();
}

void
ControlFlowMgr::forStmt_PostBody(ForStmt* stmt) {
	jump(stmt->m_loopBlock, stmt->m_followBlock);
	m_module->m_namespaceMgr.closeScope();
	restoreRegexFlags(stmt);

	if (!(stmt->m_followBlock->getFlags() & BasicBlockFlag_Jumped))
		markUnreachable(stmt->m_followBlock);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
ControlFlowMgr::onceStmt_Create(
	OnceStmt* stmt,
	const lex::LineCol& pos,
	StorageKind storageKind
) {
	Variable* flagVariable;

	if (storageKind != StorageKind_Static && storageKind != StorageKind_Tls) {
		err::setFormatStringError("'%s once' is illegal (only 'static' or 'threadlocal' is allowed)", getStorageKindString(storageKind));
		return false;
	}

	flagVariable = m_module->m_variableMgr.createOnceFlagVariable(storageKind);
	flagVariable->m_pos = pos;

	onceStmt_Create(stmt, flagVariable);
	return true;
}

void
ControlFlowMgr::onceStmt_Create(
	OnceStmt* stmt,
	Variable* flagVariable
) {
	stmt->m_flagVariable = flagVariable;
	stmt->m_followBlock = createBlock("once_follow");
}

bool
ControlFlowMgr::onceStmt_PreBody(
	OnceStmt* stmt,
	const lex::LineCol& pos
) {
	bool result;

	if (!m_module->hasCodeGen())
		return true;

	StorageKind storageKind = stmt->m_flagVariable->getStorageKind();
	ASSERT(storageKind == StorageKind_Static || storageKind == StorageKind_Tls);

	m_module->m_namespaceMgr.setSourcePos(pos);

	Type* type = stmt->m_flagVariable->getType();

	Value value;

	if (storageKind == StorageKind_Tls) {
		BasicBlock* bodyBlock = createBlock("once_body");

		result =
			m_module->m_operatorMgr.binaryOperator(BinOpKind_Eq, stmt->m_flagVariable, Value((int64_t) 0, type), &value) &&
			conditionalJump(value, bodyBlock, stmt->m_followBlock);

		if (!result)
			return false;
	} else {
		result = m_module->m_operatorMgr.loadDataRef(stmt->m_flagVariable, &value);
		if (!result)
			return false;

		uint_t flags = BasicBlockFlag_Jumped | (m_currentBlock->m_flags & BasicBlockFlag_Reachable);

		BasicBlock* preBodyBlock = createBlock("once_prebody");
		BasicBlock* bodyBlock = createBlock("once_body");
		BasicBlock* loopBlock = createBlock("once_loop");

		preBodyBlock->m_flags |= flags;
		bodyBlock->m_flags |= flags;
		loopBlock->m_flags |= flags;

		int64_t constArray[2] = { 0, 1 };
		BasicBlock* blockArray[2] = { preBodyBlock, loopBlock };

		m_module->m_llvmIrBuilder.createSwitch(value, stmt->m_followBlock, constArray, blockArray, 2);

		// loop

		setCurrentBlock(loopBlock);

		result =
			m_module->m_operatorMgr.binaryOperator(BinOpKind_Eq, stmt->m_flagVariable, Value(2, type), &value) &&
			conditionalJump(value, stmt->m_followBlock, loopBlock, preBodyBlock);

		if (!result)
			return false;

		// pre body

		m_module->m_llvmIrBuilder.createCmpXchg(
			stmt->m_flagVariable,
			Value((int64_t) 0, type),
			Value(1, type),
#if (LLVM_VERSION < 0x030900)
			llvm::Acquire,
#else
			llvm::AtomicOrdering::Acquire,
#endif
			llvm::DefaultSynchronizationScope_vn,
			&value
		);

#if (LLVM_VERSION < 0x030500)
		result =
			m_module->m_operatorMgr.binaryOperator(BinOpKind_Eq, value, Value((int64_t) 0, type), &value) &&
			conditionalJump(value, bodyBlock, loopBlock);
#else
		m_module->m_llvmIrBuilder.createExtractValue(value, 1, m_module->m_typeMgr.getPrimitiveType(TypeKind_Bool), &value);
		result = conditionalJump(value, bodyBlock, loopBlock);
#endif

		if (!result)
			return false;
	}

	m_module->m_namespaceMgr.openScope(pos);
	return true;
}

void
ControlFlowMgr::onceStmt_PostBody(OnceStmt* stmt) {
	if (!m_module->hasCodeGen())
		return;

	StorageKind storageKind = stmt->m_flagVariable->getStorageKind();
	ASSERT(storageKind == StorageKind_Static || storageKind == StorageKind_Tls);

	Type* type = stmt->m_flagVariable->getType();

	if (storageKind == StorageKind_Tls) {
		m_module->m_llvmIrBuilder.createStore(
			Value((int64_t) 2, type),
			stmt->m_flagVariable
		);
	} else {
		Value tmpValue;
		m_module->m_llvmIrBuilder.createRmw(
			llvm::AtomicRMWInst::Xchg,
			stmt->m_flagVariable,
			Value((int64_t) 2, type),
#if (LLVM_VERSION < 0x030900)
			llvm::Release,
#else
			llvm::AtomicOrdering::Release,
#endif
			llvm::DefaultSynchronizationScope_vn,
			&tmpValue
		);
	}

	m_module->m_namespaceMgr.closeScope();
	follow(stmt->m_followBlock);
}

bool
ControlFlowMgr::dynamicLayoutStmt_PreBody(
	DynamicLayoutStmt* stmt,
	const Value& layoutValue
) {
	ClassType* layoutType = (ClassType*)m_module->m_typeMgr.getStdType(StdType_DynamicLayout);

	bool result = m_module->m_operatorMgr.castOperator(
		layoutValue,
		layoutType->getClassPtrType(ClassPtrTypeKind_Normal, PtrTypeFlag_Safe),
		&stmt->m_layoutValue
	);

	if (!result)
		return false;

	stmt->m_structType = NULL;
	return true;
}

bool
ControlFlowMgr::dynamicLayoutStmt_PostBody(DynamicLayoutStmt* stmt) {
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
