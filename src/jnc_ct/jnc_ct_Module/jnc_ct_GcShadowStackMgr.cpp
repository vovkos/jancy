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
#include "jnc_ct_GcShadowStackMgr.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ArrayType.h"

namespace jnc {
namespace ct {

//..............................................................................

GcShadowStackFrameMap::~GcShadowStackFrameMap() {
	if (m_mapKind != GcShadowStackFrameMapKind_Dynamic)
		return;

	// invalidate all call-site-local boxes

	size_t count = m_gcRootArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Box* box = (Box*)m_gcRootArray[i];
		if (box->m_flags & BoxFlag_CallSiteLocal)
			box->m_flags |= BoxFlag_Invalid;
	}
}

//..............................................................................

GcShadowStackMgr::GcShadowStackMgr() {
	m_module = Module::getCurrentConstructedModule();
	ASSERT(m_module);

	m_frameVariable = NULL;
	m_gcRootCount = 0;
}

void
GcShadowStackMgr::clear() {
	m_frameMapList.clear();
	m_functionFrameMapArray.clear();
	m_frameVariable = NULL;
	m_gcRootArrayValue.clear();
	m_gcRootCount = 0;
}

void
GcShadowStackMgr::finalizeFunction() {
	if (!m_frameVariable)
		return;

	if (m_module->hasCodeGen())
		finalizeFrame();

	m_gcRootArrayValue.clear();
	m_functionFrameMapArray.clear();
	m_frameVariable = NULL;
	m_gcRootCount = 0;
}

void
GcShadowStackMgr::finalizeScope(Scope* scope) {
	if (!scope->m_gcShadowStackFrameMap ||
		!(m_module->m_controlFlowMgr.getCurrentBlock()->getFlags() & BasicBlockFlag_Reachable))
		return;

	setFrameMap(scope->m_gcShadowStackFrameMap, GcShadowStackFrameMapOp_Close);
}

void
GcShadowStackMgr::createTmpGcRoot(const Value& value) {
	Type* type = value.getType();
	ASSERT(type->getFlags() & TypeFlag_GcRoot);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createAlloca(type, NULL, &ptrValue);
	m_module->m_llvmIrBuilder.createStore(value, ptrValue);
	markGcRoot(ptrValue, type);
}

void
GcShadowStackMgr::markGcRoot(
	const Value& ptrValue,
	Type* type
) {
	// type is not necessarily a gc root (e.g., validator thin ptr)

	if (!m_frameVariable)
		preCreateFrame();

	Variable* variableBeingLifted = m_module->m_variableMgr.getCurrentLiftedStackVariable();
	Scope* scope = variableBeingLifted ?
		variableBeingLifted->getScope() :
		m_module->m_namespaceMgr.getCurrentScope();

	GcShadowStackFrameMap* frameMap = openFrameMap(scope);

	size_t index = m_gcRootCount++;

	Value bytePtrValue;
	Value gcRootValue;
	Type* bytePtrType = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);

	m_module->m_llvmIrBuilder.createGep(m_gcRootArrayValue, bytePtrType, index, NULL, &gcRootValue);
	m_module->m_llvmIrBuilder.createBitCast(ptrValue, bytePtrType, &bytePtrValue);
	m_module->m_llvmIrBuilder.createStore(bytePtrValue, gcRootValue);

	frameMap->m_gcRootArray.append(index);
	frameMap->m_gcRootTypeArray.append(type);
}

GcShadowStackFrameMap*
GcShadowStackMgr::openFrameMap(Scope* scope) {
	Scope* parentScope = scope->getParentScope();

	if (scope->m_gcShadowStackFrameMap) // this scope already has its own frame map
		return scope->m_gcShadowStackFrameMap;

	GcShadowStackFrameMap* frameMap = new GcShadowStackFrameMap;
	frameMap->m_scope = scope;
	m_frameMapList.insertTail(frameMap);
	m_functionFrameMapArray.append(frameMap);
	scope->m_gcShadowStackFrameMap = frameMap;

	// we need to set the map in the beginning of scope

	ASSERT(scope->m_gcShadowStackFrameMapInsertPoint);

	LlvmIrInsertPoint prevInsertPoint;
	bool isInsertPointChanged = m_module->m_llvmIrBuilder.restoreInsertPoint(
		scope->m_gcShadowStackFrameMapInsertPoint,
		&prevInsertPoint
	);

	setFrameMap(frameMap, GcShadowStackFrameMapOp_Open);

	// if first stack variable lift point is the same, update it -- must lift AFTER frame map is set

	if (scope->m_firstStackVariable && scope->m_firstStackVariable->m_liftInsertPoint == scope->m_gcShadowStackFrameMapInsertPoint)
		m_module->m_llvmIrBuilder.saveInsertPoint(&scope->m_firstStackVariable->m_liftInsertPoint);

	if (isInsertPointChanged)
		m_module->m_llvmIrBuilder.restoreInsertPoint(prevInsertPoint);

	return frameMap;
}

void
GcShadowStackMgr::setFrameMap(
	GcShadowStackFrameMap* frameMap,
	GcShadowStackFrameMapOp op
) {
	ASSERT(m_frameVariable);

	Function* function = m_module->m_functionMgr.getStdFunction(StdFunc_SetGcShadowStackFrameMap);

	m_module->m_llvmIrBuilder.createCall3(
		function,
		function->getType(),
		m_frameVariable,
		Value(&frameMap, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr)),
		Value(op, m_module->m_typeMgr.getPrimitiveType(TypeKind_Int)),
		NULL
	);
}

void
GcShadowStackMgr::preCreateFrame() {
	ASSERT(!m_frameVariable && !m_gcRootArrayValue);

	Type* type = m_module->m_typeMgr.getStdType(StdType_GcShadowStackFrame);
	m_frameVariable = m_module->m_variableMgr.createSimpleStackVariable("gcShadowStackFrame", type);

	type = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
	m_module->m_llvmIrBuilder.createAlloca(type, type->getDataPtrType_c(), &m_gcRootArrayValue);

	// m_gcRootArrayValue will be replaced later
}

void
GcShadowStackMgr::finalizeFrame() {
	ASSERT(m_frameVariable && m_gcRootArrayValue);

	Function* function = m_module->m_functionMgr.getCurrentFunction();
	ASSERT(function);

	BasicBlock* prologueBlock = function->getPrologueBlock();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock(prologueBlock);

	m_module->m_controlFlowMgr.setCurrentBlock(prologueBlock);
	m_module->m_llvmIrBuilder.setInsertPoint(&*prologueBlock->getLlvmBlock()->begin());

	// create gc root array stack variable (no need to zero-init now, GcHeap::openFrameMap will do that)

	Type* type = m_module->m_typeMgr.getPrimitiveType(TypeKind_IntPtr_u)->getArrayType(m_gcRootCount);

	Value gcRootArrayValue;
	m_module->m_llvmIrBuilder.createAlloca(type, type->getDataPtrType_c(), &gcRootArrayValue);
	type = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr)->getDataPtrType_c();
	m_module->m_llvmIrBuilder.createBitCast(gcRootArrayValue, type, &gcRootArrayValue);

	// fixup all uses of gc root array

	ASSERT(llvm::isa<llvm::AllocaInst>(m_gcRootArrayValue.getLlvmValue()));
	llvm::AllocaInst* llvmAlloca = (llvm::AllocaInst*)m_gcRootArrayValue.getLlvmValue();
	llvmAlloca->replaceAllUsesWith(gcRootArrayValue.getLlvmValue());
	llvmAlloca->eraseFromParent();
	m_gcRootArrayValue = gcRootArrayValue;

	// initialize frame

	Type* frameType = m_frameVariable->getType();

	// GcShadowStackFrame.m_map

	Value frameMapFieldValue;
	type = m_module->m_typeMgr.getStdType(StdType_ByteThinPtr);
	m_module->m_llvmIrBuilder.createGep2(m_frameVariable, frameType, 1, NULL, &frameMapFieldValue);
	m_module->m_llvmIrBuilder.createStore(type->getZeroValue(), frameMapFieldValue);

	// GcShadowStackFrame.m_gcRootArray

	Value gcRootArrayFieldValue;
	m_module->m_llvmIrBuilder.createGep2(m_frameVariable, frameType, 2, NULL, &gcRootArrayFieldValue);
	m_module->m_llvmIrBuilder.createStore(gcRootArrayValue, gcRootArrayFieldValue);

	Variable* stackTopVariable;
	bool isAsync = function->getFunctionKind() == FunctionKind_AsyncSequencer;

	if (isAsync) {
		Value promiseValue = m_module->m_functionMgr.getPromiseValue();
		ASSERT(promiseValue);

		Value frameFieldValue;
		bool result = m_module->m_operatorMgr.getPromiseField(promiseValue, "m_gcShadowStackFrame", &frameFieldValue);
		ASSERT(result);

		Value frameValue;
		m_module->m_llvmIrBuilder.createBitCast(m_frameVariable, m_module->m_typeMgr.getStdType(StdType_ByteThinPtr), &frameValue);
		m_module->m_llvmIrBuilder.createStore(frameValue, frameFieldValue);
	} else {
		Value prevStackTopValue;
		stackTopVariable = m_module->m_variableMgr.getStdVariable(StdVariable_GcShadowStackTop);
		m_module->m_llvmIrBuilder.createLoad(stackTopVariable, stackTopVariable->getType(), &prevStackTopValue);

		// GcShadowStackFrame.m_prev

		Value prevFieldValue;
		m_module->m_llvmIrBuilder.createGep2(m_frameVariable, frameType, 0, NULL, &prevFieldValue);
		m_module->m_llvmIrBuilder.createStore(prevStackTopValue, prevFieldValue);

		// set new frame as the new stack top

		m_module->m_llvmIrBuilder.createStore(m_frameVariable, stackTopVariable);

		// restore previous stack top before every ret

		sl::Array<BasicBlock*> returnBlockArray = m_module->m_controlFlowMgr.getReturnBlockArray();
		size_t count = returnBlockArray.getCount();
		for (size_t i = 0; i < count; i++) {
			BasicBlock* block = returnBlockArray[i];
			llvm::Instruction* llvmRet = block->getLlvmBlock()->getTerminator();
			ASSERT(llvm::isa<llvm::ReturnInst>(llvmRet));

			m_module->m_llvmIrBuilder.setInsertPoint(llvmRet);
			m_module->m_llvmIrBuilder.createStore(prevStackTopValue, stackTopVariable);
		}
	}

	// calculate frame map m_prev field

	size_t count = m_functionFrameMapArray.getCount();
	for (size_t i = 0; i < count; i++) {
		GcShadowStackFrameMap* map = m_functionFrameMapArray[i];
		Scope* scope = map->m_scope->getParentScope();
		map->m_prev = scope ? scope->findGcShadowStackFrameMap() : NULL;
	}

	// restore current stack top and frame map at every landing pad

	sl::Array<BasicBlock*> landingPadBlockArray = m_module->m_controlFlowMgr.getLandingPadBlockArray();
	count = landingPadBlockArray.getCount();
	for (size_t i = 0; i < count; i++) {
		BasicBlock* block = landingPadBlockArray[i];
		Scope* scope = block->getLandingPadScope();
		ASSERT(scope && !block->getLlvmBlock()->empty());

		m_module->m_llvmIrBuilder.setInsertPoint(&*block->getLlvmBlock()->begin());

		// on async & exception landing pads we must restore frame pointer

		if (!isAsync && (block->getFlags () & BasicBlockFlag_SjljLandingPadMask))
			m_module->m_llvmIrBuilder.createStore(m_frameVariable, stackTopVariable);

		GcShadowStackFrameMap* map = scope->findGcShadowStackFrameMap();
		setFrameMap(map, GcShadowStackFrameMapOp_Restore);
	}

	// done

	m_module->m_controlFlowMgr.setCurrentBlock(prevBlock);
}

//..............................................................................

} // namespace ct
} // namespace jnc
