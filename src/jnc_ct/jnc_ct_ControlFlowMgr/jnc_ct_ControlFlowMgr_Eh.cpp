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
#include "jnc_ct_ArrayType.h"

namespace jnc {
namespace ct {

//..............................................................................

BasicBlock*
ControlFlowMgr::getDynamicThrowBlock()
{
	Function* function = m_module->m_functionMgr.getCurrentFunction();
	ASSERT(function->getFunctionKind() != FunctionKind_AsyncSequencer); // async functions always can static-throw

	if (m_dynamicThrowBlock)
		return m_dynamicThrowBlock;

	m_dynamicThrowBlock = createBlock("dynamic_throw_block", BasicBlockFlag_Reachable);
	BasicBlock* prevBlock = setCurrentBlock(m_dynamicThrowBlock);

	Function* throwFunc = m_module->m_functionMgr.getStdFunction(StdFunc_DynamicThrow);
	m_module->m_llvmIrBuilder.createCall(throwFunc, throwFunc->getType(), NULL);
	m_module->m_llvmIrBuilder.createUnreachable();

	setCurrentBlock(prevBlock);
	return m_dynamicThrowBlock;
}

Variable*
ControlFlowMgr::getFinallyRouteIdxVariable()
{
	if (m_finallyRouteIdxVariable)
		return m_finallyRouteIdxVariable;

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	BasicBlock* prevBlock = setCurrentBlock(function->getPrologueBlock());
	m_finallyRouteIdxVariable = m_module->m_variableMgr.createSimpleStackVariable("finallyRouteIdx", m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr));
	setCurrentBlock(prevBlock);
	return m_finallyRouteIdxVariable;
}

void
ControlFlowMgr::markLandingPad(
	BasicBlock* block,
	Scope* scope,
	uint_t flags
	)
{
	ASSERT(flags >= (block->m_flags & BasicBlockFlag_LandingPadMask));

	if (!(block->m_flags & BasicBlockFlag_LandingPadMask))
		m_landingPadBlockArray.append(block);

	block->m_flags |= flags;
	block->m_landingPadScope = scope;
}

void
ControlFlowMgr::throwException()
{
	if (!m_module->hasCodeGen())
		return;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	if (!scope->canStaticThrow())
	{
		jump(getDynamicThrowBlock());
	}
	else
	{
		Scope* catchScope = m_module->m_namespaceMgr.findCatchScope();
		if (catchScope)
		{
			escapeScope(
				catchScope,
				catchScope->m_tryExpr ?
					catchScope->m_tryExpr->m_catchBlock :
					catchScope->m_catchBlock
				);
		}
		else
		{
			FunctionType* currentFunctionType = m_module->m_functionMgr.getCurrentFunction()->getType();
			ASSERT(currentFunctionType->getFlags() & FunctionTypeFlag_ErrorCode);

			Value throwValue = currentFunctionType->getReturnType()->getErrorCodeValue();
			ret(throwValue);
		}
	}
}

void
ControlFlowMgr::setJmp(
	BasicBlock* catchBlock,
	size_t sjljFrameIdx
	)
{
	if (!m_module->hasCodeGen())
		return;

	if (!m_sjljFrameArrayValue)
		preCreateSjljFrameArray();

	Variable* sjljFrameVariable = m_module->m_variableMgr.getStdVariable(StdVariable_SjljFrame);
	Function* setJmpFunc = m_module->m_functionMgr.getStdFunction(StdFunc_SetJmp);

	Value sjljFrameValue;
	Value returnValue;
	m_module->m_llvmIrBuilder.createGep(m_sjljFrameArrayValue, sjljFrameIdx, NULL, &sjljFrameValue);
	m_module->m_llvmIrBuilder.createStore(sjljFrameValue, sjljFrameVariable);

#if (_JNC_OS_POSIX)
	Value signalValue;
	Value zeroValue((int64_t)0, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int));
	m_module->m_llvmIrBuilder.createGep2(sjljFrameValue, 1,  NULL, &signalValue);
	m_module->m_llvmIrBuilder.createStore(zeroValue, signalValue);
#endif

	m_module->m_llvmIrBuilder.createCall(setJmpFunc, setJmpFunc->getType(), sjljFrameValue, &returnValue);

	BasicBlock* followBlock = createBlock("follow_block");

#if (!_JNC_OS_POSIX)
	bool result = conditionalJump(returnValue, catchBlock, followBlock, followBlock);
	ASSERT(result);
#else
	BasicBlock* preCatchBlock = createBlock("pre_catch_block");
	Function* saveSignalInfoFunc = m_module->m_functionMgr.getStdFunction(StdFunc_SaveSignalInfo);

	bool result = conditionalJump(returnValue, preCatchBlock, followBlock, followBlock);
	ASSERT(result);

	setCurrentBlock(preCatchBlock);
	m_module->m_llvmIrBuilder.createCall(saveSignalInfoFunc, saveSignalInfoFunc->getType(), sjljFrameValue, NULL);
	jump(catchBlock, followBlock);
#endif

	if (sjljFrameIdx >= m_sjljFrameCount)
	{
		ASSERT(m_sjljFrameCount == sjljFrameIdx);
		m_sjljFrameCount = sjljFrameIdx + 1;
	}
}

void
ControlFlowMgr::setJmpFinally(
	BasicBlock* finallyBlock,
	size_t sjljFrameIdx
	)
{
	if (!m_module->hasCodeGen())
		return;

	BasicBlock* catchBlock = createBlock("finally_sjlj_block");
	setJmp(catchBlock, sjljFrameIdx);

	BasicBlock* prevBlock = setCurrentBlock(catchBlock);
	Variable* finallyRouteIdxVariable = getFinallyRouteIdxVariable();
	uint64_t minusOne = -1;
	Value minusOneValue(&minusOne, finallyRouteIdxVariable->getType());
	m_module->m_llvmIrBuilder.createStore(minusOneValue, finallyRouteIdxVariable);
	jump(finallyBlock);
	setCurrentBlock(prevBlock);
}

void
ControlFlowMgr::beginTryOperator(TryExpr* tryExpr)
{
	if (!m_module->hasCodeGen())
		return;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	tryExpr->m_prev = scope->m_tryExpr;
	tryExpr->m_catchBlock = createBlock("try_catch_block");
	tryExpr->m_sjljFrameIdx = tryExpr->m_prev ?
		tryExpr->m_prev->m_sjljFrameIdx + 1 :
		scope->m_sjljFrameIdx + 1;

	setJmp(tryExpr->m_catchBlock, tryExpr->m_sjljFrameIdx);
	scope->m_tryExpr = tryExpr;
}

bool
ControlFlowMgr::endTryOperator(
	TryExpr* tryExpr,
	Value* value
	)
{
	Value errorValue;
	Type* type = value->getType();
	if (type->getTypeKind() == TypeKind_Void)
	{
		value->setConstBool(true, m_module);
		errorValue.setConstBool(false, m_module);
	}
	else if (type->getTypeKindFlags() & TypeKindFlag_ErrorCode)
	{
		errorValue = type->getErrorCodeValue();
	}
	else
	{
		err::setFormatStringError("'%s' cannot be used as error code", type->getTypeString().sz());
		return false;
	}

	if (!m_module->hasCodeGen())
		return true;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();

	BasicBlock* prevBlock = m_currentBlock;
	BasicBlock* phiBlock = createBlock("try_phi_block");

	ASSERT(tryExpr->m_sjljFrameIdx != -1);
	setSjljFrame(tryExpr->m_sjljFrameIdx - 1); // restore prev sjlj frame on normal flow
	jump(phiBlock, tryExpr->m_catchBlock);

	markLandingPad(tryExpr->m_catchBlock, scope, BasicBlockFlag_ExceptionLandingPad);
	jump(phiBlock, phiBlock);

	m_module->m_llvmIrBuilder.createPhi(*value, prevBlock, errorValue, tryExpr->m_catchBlock, value);

	scope->m_tryExpr = tryExpr->m_prev;
	return true;
}

void
ControlFlowMgr::checkErrorCode(
	const Value& returnValue,
	Type* returnType,
	BasicBlock* throwBlock
	)
{
	bool result;

	ASSERT(returnType->getTypeKindFlags() & TypeKindFlag_ErrorCode);

	Value indicatorValue;
	if (returnType->getTypeKind() == TypeKind_Bool || !(returnType->getTypeKindFlags() & TypeKindFlag_Integer))
	{
		indicatorValue = returnValue;
	}
	else
	{
		uint64_t minusOne = -1;
		Value minusOneValue;
		minusOneValue.createConst(&minusOne, returnType);

		result = m_module->m_operatorMgr.binaryOperator(BinOpKind_Ne, returnValue, minusOneValue, &indicatorValue);
		ASSERT(result);
	}

	BasicBlock* followBlock = createBlock("follow_block");

	if (throwBlock)
	{
		result = conditionalJump(indicatorValue, followBlock, throwBlock, followBlock);
		ASSERT(result);
		return;
	}

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	if (!scope->canStaticThrow())
	{
		throwBlock = getDynamicThrowBlock();

		result = conditionalJump(indicatorValue, followBlock, throwBlock, followBlock);
		ASSERT(result);
	}
	else
	{
		throwBlock = createBlock("static_throw_block");

		result = conditionalJump(indicatorValue, followBlock, throwBlock, throwBlock);
		ASSERT(result);

		throwException();
		setCurrentBlock(followBlock);
	}
}

void
ControlFlowMgr::finalizeTryScope(Scope* scope)
{
	scope->m_flags |= ScopeFlag_CatchAhead;

	bool result = catchLabel(Token::Pos());
	ASSERT(result);

	finalizeCatchScope(scope);
}

bool
ControlFlowMgr::catchLabel(const lex::LineCol& pos)
{
	bool result;

	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	if ((scope->m_flags & ScopeFlag_Function) && !(scope->m_flags & ScopeFlag_FinallyAhead))
	{
		result = checkReturn();
		if (!result)
			return false;
	}

	if (scope->m_flags & ScopeFlag_Disposable)
	{
		m_module->m_namespaceMgr.closeScope();
		scope = m_module->m_namespaceMgr.getCurrentScope();
	}

	if (!(scope->m_flags & ScopeFlag_CatchAhead))
	{
		err::setFormatStringError("'catch' is already defined");
		return false;
	}

	ASSERT(!(scope->m_flags & ScopeFlag_Finally));

	m_module->m_namespaceMgr.closeScope();

	if (m_currentBlock->m_flags & BasicBlockFlag_Reachable)
	{
		if (scope->m_flags & ScopeFlag_FinallyAhead)
		{
			ASSERT(scope->m_finallyBlock);
			normalFinallyFlow(scope->m_finallyBlock);
		}
		else
		{
			m_catchFinallyFollowBlock = createBlock("catch_follow");

			ASSERT(scope->m_sjljFrameIdx != -1);
			setSjljFrame(scope->m_sjljFrameIdx - 1); // restore prev sjlj frame on normal flow
			jump(m_catchFinallyFollowBlock);
		}
	}

	ASSERT(scope->m_catchBlock);
	setCurrentBlock(scope->m_catchBlock);

	Scope* catchScope = m_module->m_namespaceMgr.openScope(pos, ScopeFlag_Catch);
	catchScope->m_flags |= scope->m_flags & (ScopeFlag_Nested | ScopeFlag_FinallyAhead | ScopeFlag_Finalizable); // propagate
	markLandingPad(scope->m_catchBlock, catchScope, BasicBlockFlag_ExceptionLandingPad);

	if (scope->m_flags & ScopeFlag_FinallyAhead)
	{
		catchScope->m_finallyBlock = scope->m_finallyBlock;
		catchScope->m_sjljFrameIdx++;
		setJmpFinally(catchScope->m_finallyBlock, catchScope->m_sjljFrameIdx);
	}

	return true;
}

void
ControlFlowMgr::finalizeCatchScope(Scope* scope)
{
	if (m_catchFinallyFollowBlock)
	{
		follow(m_catchFinallyFollowBlock);
		m_catchFinallyFollowBlock = NULL; // used already
	}
}

bool
ControlFlowMgr::finallyLabel(const lex::LineCol& pos)
{
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope();
	if (scope->m_flags & ScopeFlag_Disposable)
	{
		m_module->m_namespaceMgr.closeScope();
		scope = m_module->m_namespaceMgr.getCurrentScope();
	}

	if (scope->m_flags & ScopeFlag_CatchAhead)
	{
		err::setFormatStringError("'finally' should follow 'catch'");
		return false;
	}

	if (!(scope->m_flags & ScopeFlag_FinallyAhead))
	{
		err::setFormatStringError("'finally' is already defined");
		return false;
	}

	if (scope->m_flags & ScopeFlag_Try)
	{
		scope->m_flags |= ScopeFlag_CatchAhead;
		bool result = catchLabel(pos);
		ASSERT(result);
	}

	m_module->m_namespaceMgr.closeScope();

	ASSERT(scope->m_finallyBlock);

	if (m_currentBlock->m_flags & BasicBlockFlag_Reachable)
		normalFinallyFlow(scope->m_finallyBlock);

	setCurrentBlock(scope->m_finallyBlock);

	Scope* finallyScope = m_module->m_namespaceMgr.openScope(pos, ScopeFlag_Finally);
	finallyScope->m_flags |= scope->m_flags & ScopeFlag_Nested; // propagate
	finallyScope->m_finallyBlock = scope->m_finallyBlock; // to access finally route map

	markLandingPad(scope->m_finallyBlock, finallyScope, BasicBlockFlag_ExceptionLandingPad);

	return true;
}

void ControlFlowMgr::finalizeFinallyScope(Scope* scope)
{
	ASSERT(scope && scope->m_finallyBlock && m_finallyRouteIdxVariable);

	if (!(m_currentBlock->m_flags & BasicBlockFlag_Reachable))
	{
		m_catchFinallyFollowBlock = NULL;
		return;
	}

	Value routeIdxValue;
	m_module->m_operatorMgr.loadDataRef(m_finallyRouteIdxVariable, &routeIdxValue);

	BasicBlock* throwBlock = getDynamicThrowBlock();

	size_t count = scope->m_finallyBlock->m_finallyRouteMap.getCount();
	if (!count)
	{
		ASSERT(false);
		jump(throwBlock);
		return;
	}

	char buffer1[256];
	sl::Array<intptr_t> routeIdxArray(rc::BufKind_Stack, buffer1, sizeof(buffer1));
	routeIdxArray.setCount(count);

	char buffer2[256];
	sl::Array<BasicBlock*> blockArray(rc::BufKind_Stack, buffer2, sizeof(buffer2));
	blockArray.setCount(count);

	sl::HashTableIterator<size_t, BasicBlock*> it = scope->m_finallyBlock->m_finallyRouteMap.getHead();
	for (size_t i = 0; it; it++, i++)
	{
		ASSERT(i < count);

		routeIdxArray[i] = it->getKey();
		blockArray[i] = it->m_value;
		it->m_value->markReachable();
	}

	m_module->m_llvmIrBuilder.createSwitch(
		routeIdxValue,
		throwBlock, // default to throw block
		routeIdxArray,
		blockArray,
		count
		);

	if (m_catchFinallyFollowBlock)
	{
		setCurrentBlock(m_catchFinallyFollowBlock);
		m_catchFinallyFollowBlock = NULL;
	}
	else
	{
		setCurrentBlock(getUnreachableBlock());
	}
}

bool
ControlFlowMgr::disposeVariable(Variable* variable)
{
	bool result;

	Value disposableValue = variable;
	result = m_module->m_operatorMgr.loadDataRef(variable, &disposableValue);
	if (!result)
		return false;

	BasicBlock* disposeBlock = NULL;
	BasicBlock* followBlock = NULL;

	Type* type = variable->getType();
	ASSERT(type->getTypeKind() == TypeKind_ClassPtr || type->getTypeKind() == TypeKind_DataPtr);

	if (type->getTypeKind() == TypeKind_DataPtr &&
		(((DataPtrType*)type)->getTargetType()->getTypeKindFlags() & TypeKindFlag_Ptr))
	{
		disposeBlock = createBlock("dispose_ptr_block");
		followBlock = createBlock("dispose_ptr_follow_block");

		result =
			m_module->m_operatorMgr.unaryOperator(UnOpKind_Indir, &disposableValue) &&
			m_module->m_operatorMgr.loadDataRef(&disposableValue) &&
			conditionalJump(disposableValue, disposeBlock, followBlock, disposeBlock);

		if (!result)
			return false;
	}

	Value disposeValue;
	result =
		m_module->m_operatorMgr.memberOperator(disposableValue, "dispose", &disposeValue) &&
		m_module->m_operatorMgr.callOperator(disposeValue);

	if (!result)
		return false;

	if (followBlock)
		follow(followBlock);

	return true;
}

void
ControlFlowMgr::finalizeDisposableScope(Scope* scope)
{
	size_t count = scope->m_disposableVariableArray.getCount();
	ASSERT(scope && count && scope->m_disposeLevelVariable);

	bool result = finallyLabel(Token::Pos());
	ASSERT(result);

	BasicBlock* switchBlock = m_currentBlock;

	char buffer1[256];
	sl::Array<intptr_t> levelArray(rc::BufKind_Stack, buffer1, sizeof(buffer1));
	levelArray.setCount(count);

	char buffer2[256];
	sl::Array<BasicBlock*> blockArray(rc::BufKind_Stack, buffer2, sizeof(buffer2));
	blockArray.setCount(count + 1);

	for (size_t i = 0, j = count; i < count; i++, j--)
	{
		BasicBlock* block = createBlock("dispose_variable_block", BasicBlockFlag_Reachable);
		levelArray[i] = j;
		blockArray[i] = block;
	}

	BasicBlock* followBlock = createBlock("dispose_finally_follow_block");
	blockArray[count] = followBlock;

	for (intptr_t i = count - 1, j = 0; i >= 0; i--, j++)
	{
		setCurrentBlock(blockArray[j]);

		Variable* variable = scope->m_disposableVariableArray[i];
		result = disposeVariable(variable);
		ASSERT(result);

		follow(blockArray[j + 1]);
	}

	setCurrentBlock(switchBlock);

	Value disposeLevelValue;
	m_module->m_llvmIrBuilder.createLoad(
		scope->m_disposeLevelVariable,
		scope->m_disposeLevelVariable->getType(),
		&disposeLevelValue
		);

	m_module->m_llvmIrBuilder.createSwitch(
		disposeLevelValue,
		followBlock,
		levelArray,
		blockArray,
		count
		);

	setCurrentBlock(followBlock);

	scope = m_module->m_namespaceMgr.getCurrentScope();
	ASSERT(scope->m_flags & ScopeFlag_Finally);

	finalizeFinallyScope(scope);
}

void
ControlFlowMgr::normalFinallyFlow(BasicBlock* finallyBlock)
{
	if (!m_module->hasCodeGen())
		return;

	if (!m_catchFinallyFollowBlock)
		m_catchFinallyFollowBlock = createBlock("finally_follow");

	size_t routeIdx = ++m_finallyRouteIdx;

	finallyBlock->m_finallyRouteMap[routeIdx] = m_catchFinallyFollowBlock;

	Variable* routeIdxVariable = getFinallyRouteIdxVariable();
	Value routeIdxValue(routeIdx, m_module->m_typeMgr.getPrimitiveType(TypeKind_IntPtr));
	m_module->m_llvmIrBuilder.createStore(routeIdxValue, routeIdxVariable);

	jump(finallyBlock);
}

void
ControlFlowMgr::setSjljFrame(size_t index)
{
	if (!m_module->hasCodeGen())
		return;

	ASSERT(m_sjljFrameArrayValue);

	Variable* sjljFrameVariable = m_module->m_variableMgr.getStdVariable(StdVariable_SjljFrame);

	if (index == -1)
	{
		m_module->m_llvmIrBuilder.createStore(m_prevSjljFrameValue, sjljFrameVariable);
	}
	else
	{
		Value sjljFrameValue;
		m_module->m_llvmIrBuilder.createGep(m_sjljFrameArrayValue, index, NULL, &sjljFrameValue);
		m_module->m_llvmIrBuilder.createStore(sjljFrameValue, sjljFrameVariable);
	}
}

void
ControlFlowMgr::preCreateSjljFrameArray()
{
	Function* function = m_module->m_functionMgr.getCurrentFunction();
	BasicBlock* prologueBlock = function->getPrologueBlock();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock(prologueBlock);

	ASSERT(!m_sjljFrameArrayValue);
	Type* type = m_module->m_typeMgr.getStdType(StdType_SjljFrame);
	m_module->m_llvmIrBuilder.createAlloca(
		type,
		"sjljFrameArray_tmp",
		type->getDataPtrType_c(),
		&m_sjljFrameArrayValue
		);

	Variable* variable = m_module->m_variableMgr.getStdVariable(StdVariable_SjljFrame);
	m_module->m_llvmIrBuilder.createLoad(variable, variable->getType(), &m_prevSjljFrameValue);

	m_module->m_controlFlowMgr.setCurrentBlock(prevBlock);
}

void
ControlFlowMgr::finalizeSjljFrameArray()
{
	ASSERT(m_sjljFrameArrayValue);

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	ASSERT(function);

	BasicBlock* prologueBlock = function->getPrologueBlock();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock(prologueBlock);

	m_module->m_controlFlowMgr.setCurrentBlock(prologueBlock);
	m_module->m_llvmIrBuilder.setInsertPoint(&*prologueBlock->getLlvmBlock()->begin());

	// create sjlj frame array stack variable (no need to zero-init now, GcHeap::openFrameMap will do that)

	Type* type = m_module->m_typeMgr.getStdType(StdType_SjljFrame);
	ArrayType* arrayType = type->getArrayType(m_sjljFrameCount);

	Value sjljFrameArrayValue;
	llvm::AllocaInst* llvmAlloca = m_module->m_llvmIrBuilder.createAlloca(arrayType, "sjljFrameArray", NULL, &sjljFrameArrayValue);
	m_module->m_llvmIrBuilder.createBitCast(sjljFrameArrayValue, type->getDataPtrType_c(), &sjljFrameArrayValue);

#if (_JNC_CPU_AMD64)
#	if (LLVM_VERSION_MAJOR < 10)
	llvmAlloca->setAlignment(16);
#	else
	llvmAlloca->setAlignment(llvm::Align(16));
#	endif
#endif

	// fixup all uses of sjlj frame array

	ASSERT(llvm::isa<llvm::AllocaInst> (m_sjljFrameArrayValue.getLlvmValue()));
	llvmAlloca = (llvm::AllocaInst*)m_sjljFrameArrayValue.getLlvmValue();
	llvmAlloca->replaceAllUsesWith(sjljFrameArrayValue.getLlvmValue());
	llvmAlloca->eraseFromParent();

	m_sjljFrameArrayValue = sjljFrameArrayValue;

	// if this function has no gc shadow stack frame, we must also manually restore
	// previous gc shadow stack frame pointer on exception landing pads

	Variable* gcShadowStackTopVariable;
	Value prevGcShadowStackFrameValue;

	bool hasGcShadowStackFrame = m_module->m_gcShadowStackMgr.hasFrame() && function->getFunctionKind() != FunctionKind_AsyncSequencer;
	if (!hasGcShadowStackFrame)
	{
		gcShadowStackTopVariable = m_module->m_variableMgr.getStdVariable(StdVariable_GcShadowStackTop);
		m_module->m_llvmIrBuilder.createLoad(gcShadowStackTopVariable, NULL, &prevGcShadowStackFrameValue);
	}

	// restore sjlj frame at every landing pad

	size_t count = m_landingPadBlockArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		BasicBlock* block = m_landingPadBlockArray[i];
		ASSERT(block->m_landingPadScope && !block->m_llvmBlock->empty());

		m_module->m_llvmIrBuilder.setInsertPoint(&*block->m_llvmBlock->begin());
		setSjljFrame(block->m_landingPadScope->m_sjljFrameIdx);

		// also restore prev gc shadow stack frame if GcShadowStackFrameMgr will not do it for us

		if (!hasGcShadowStackFrame && (block->m_flags & BasicBlockFlag_SjljLandingPadMask))
			m_module->m_llvmIrBuilder.createStore(prevGcShadowStackFrameValue, gcShadowStackTopVariable);
	}

	// done

	m_module->m_controlFlowMgr.setCurrentBlock(prevBlock);
}

//..............................................................................

} // namespace ct
} // namespace jnc
