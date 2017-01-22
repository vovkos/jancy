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

namespace jnc {
namespace ct {

//..............................................................................

GcShadowStackMgr::GcShadowStackMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_frameVariable = NULL;
	m_gcRootCount = 0;
}

void
GcShadowStackMgr::clear ()
{
	m_frameMapList.clear ();
	m_frameVariable = NULL;
	m_gcRootArrayValue.clear ();
	m_gcRootCount = 0;
}

void
GcShadowStackMgr::finalizeFunction ()
{
	if (!m_frameVariable)
		return;

	finalizeFrame ();

	m_gcRootArrayValue.clear ();
	m_frameVariable = NULL;
	m_gcRootCount = 0;
}

void
GcShadowStackMgr::finalizeScope (Scope* scope)
{
	Scope* parentScope = scope->getParentScope ();
	GcShadowStackFrameMap* parentFrameMap = parentScope ? parentScope->m_gcShadowStackFrameMap : NULL;

	if (parentFrameMap == scope->m_gcShadowStackFrameMap) // stays the same
		return;

	if (m_module->m_controlFlowMgr.getCurrentBlock ()->getFlags () & BasicBlockFlag_Reachable)
		setFrameMap (parentFrameMap, false);
}

void
GcShadowStackMgr::createTmpGcRoot (const Value& value)
{
	Type* type = value.getType ();
	ASSERT (type->getFlags () & TypeFlag_GcRoot);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createAlloca (type, "tmpGcRoot", NULL, &ptrValue);
	m_module->m_llvmIrBuilder.createStore (value, ptrValue);
	markGcRoot (ptrValue, type);
}

void
GcShadowStackMgr::markGcRoot (
	const Value& ptrValue,
	Type* type,
	Scope* scope
	)
{
	if (!m_frameVariable)
		preCreateFrame ();

	if (!scope)
		scope = m_module->m_namespaceMgr.getCurrentScope ();

	GcShadowStackFrameMap* frameMap = openFrameMap (scope);

	size_t index = m_gcRootCount++;

	Value bytePtrValue;
	Value gcRootValue;
	Type* bytePtrType = m_module->m_typeMgr.getStdType (StdType_BytePtr);

	m_module->m_llvmIrBuilder.createGep (m_gcRootArrayValue, index, NULL, &gcRootValue);
	m_module->m_llvmIrBuilder.createBitCast (ptrValue, bytePtrType, &bytePtrValue);
	m_module->m_llvmIrBuilder.createStore (bytePtrValue, gcRootValue);

	frameMap->m_gcRootArray.append (index);
	frameMap->m_gcRootTypeArray.append (type);
}

GcShadowStackFrameMap*
GcShadowStackMgr::openFrameMap (Scope* scope)
{
	Scope* parentScope = scope->getParentScope ();
	GcShadowStackFrameMap* prevFrameMap = parentScope ? parentScope->m_gcShadowStackFrameMap : NULL;

	if (scope->m_gcShadowStackFrameMap != prevFrameMap) // this scope already has its own frame map
	{
		ASSERT (scope->m_gcShadowStackFrameMap);
		return scope->m_gcShadowStackFrameMap;
	}

	GcShadowStackFrameMap* frameMap = AXL_MEM_NEW (GcShadowStackFrameMap);
	frameMap->m_prev = prevFrameMap;
	m_frameMapList.insertTail (frameMap);
	scope->m_gcShadowStackFrameMap = frameMap;

	// also update all the nested scopes in the scope stack

	Scope* childScope = m_module->m_namespaceMgr.getCurrentScope ();
	while (childScope != scope)
	{
		if (childScope->m_gcShadowStackFrameMap == prevFrameMap)
			childScope->m_gcShadowStackFrameMap = frameMap;

		childScope = childScope->getParentScope ();
		ASSERT (childScope);
	}

	// we need to set the map in the beginning of scope

	ASSERT (scope->m_gcShadowStackFrameMapInsertPoint);

	LlvmIrInsertPoint prevInsertPoint;
	bool isInsertPointChanged = m_module->m_llvmIrBuilder.restoreInsertPoint (
		scope->m_gcShadowStackFrameMapInsertPoint,
		&prevInsertPoint
		);

	setFrameMap (frameMap, true);

	// if first stack variable lift point is the same, update it -- must lift AFTER frame map is set

	if (scope->m_firstStackVariable && scope->m_firstStackVariable->m_liftInsertPoint == scope->m_gcShadowStackFrameMapInsertPoint)
		m_module->m_llvmIrBuilder.saveInsertPoint (&scope->m_firstStackVariable->m_liftInsertPoint);

	if (isInsertPointChanged)
		m_module->m_llvmIrBuilder.restoreInsertPoint (prevInsertPoint);

	return frameMap;
}

void
GcShadowStackMgr::setFrameMap (
	GcShadowStackFrameMap* frameMap,
	bool isOpen
	)
{
	ASSERT (m_frameVariable);

	Function* function = m_module->m_functionMgr.getStdFunction (StdFunc_SetGcShadowStackFrameMap);

	m_module->m_llvmIrBuilder.createCall3 (
		function,
		function->getType (),
		m_frameVariable,
		Value (&frameMap, m_module->m_typeMgr.getStdType (StdType_BytePtr)),
		Value (isOpen, m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool)),
		NULL
		);
}

void
GcShadowStackMgr::preCreateFrame ()
{
	ASSERT (!m_frameVariable && !m_gcRootArrayValue);

	Type* type = m_module->m_typeMgr.getStdType (StdType_GcShadowStackFrame);
	m_frameVariable = m_module->m_variableMgr.createSimpleStackVariable ("gcShadowStackFrame", type);

	type = m_module->m_typeMgr.getStdType (StdType_BytePtr);
	m_module->m_llvmIrBuilder.createAlloca (type, "gcRootArray_tmp", type->getDataPtrType_c (), &m_gcRootArrayValue);

	// m_gcRootArrayValue will be replaced later
}

void
GcShadowStackMgr::finalizeFrame ()
{
	ASSERT (m_frameVariable && m_gcRootArrayValue);

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	BasicBlock* entryBlock = function->getEntryBlock ();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (entryBlock);

	m_module->m_controlFlowMgr.setCurrentBlock (entryBlock);
	m_module->m_llvmIrBuilder.setInsertPoint (entryBlock->getLlvmBlock ()->begin ());

	// create gc root array stack variable (no need to zero-init now, GcHeap::openFrameMap will do that)

	Type* type = m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr_u)->getArrayType (m_gcRootCount);

	Value gcRootArrayValue;
	m_module->m_llvmIrBuilder.createAlloca (type, "gcRootArray", type->getDataPtrType_c (), &gcRootArrayValue);
	type = m_module->m_typeMgr.getStdType (StdType_BytePtr)->getDataPtrType_c ();
	m_module->m_llvmIrBuilder.createBitCast (gcRootArrayValue, type, &gcRootArrayValue);

	// fixup all uses of gc root array

	ASSERT (llvm::isa <llvm::AllocaInst> (m_gcRootArrayValue.getLlvmValue ()));
	llvm::AllocaInst* llvmAlloca = (llvm::AllocaInst*) m_gcRootArrayValue.getLlvmValue ();
	llvmAlloca->replaceAllUsesWith (gcRootArrayValue.getLlvmValue ());
	llvmAlloca->eraseFromParent ();
	m_gcRootArrayValue = gcRootArrayValue;

	// get prev shadow stack top

	Value prevStackTopValue;
	Variable* stackTopVariable = m_module->m_variableMgr.getStdVariable (StdVariable_GcShadowStackTop);
	m_module->m_llvmIrBuilder.createLoad (stackTopVariable , NULL, &prevStackTopValue);

	// initialize frame

	Value srcValue;
	Value dstValue;

	// GcShadowStackFrame.m_prev

	m_module->m_llvmIrBuilder.createGep2 (m_frameVariable, 0, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (prevStackTopValue, dstValue);

	// GcShadowStackFrame.m_map

	Value frameMapFieldValue;
	type = m_module->m_typeMgr.getStdType (StdType_BytePtr);
	m_module->m_llvmIrBuilder.createGep2 (m_frameVariable, 1, NULL, &frameMapFieldValue);
	m_module->m_llvmIrBuilder.createStore (type->getZeroValue (), frameMapFieldValue);

	// GcShadowStackFrame.m_gcRootArray

	m_module->m_llvmIrBuilder.createGep2 (m_frameVariable, 2, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (gcRootArrayValue, dstValue);

	// set new frame as the new stack top

	m_module->m_llvmIrBuilder.createStore (m_frameVariable, stackTopVariable);

	// restore previous stack top before every ret

	sl::Array <BasicBlock*> returnBlockArray = m_module->m_controlFlowMgr.getReturnBlockArray ();
	size_t count = returnBlockArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BasicBlock* block = returnBlockArray [i];
		llvm::TerminatorInst* llvmRet = block->getLlvmBlock ()->getTerminator ();
		ASSERT (llvm::isa <llvm::ReturnInst> (llvmRet));

		m_module->m_llvmIrBuilder.setInsertPoint (llvmRet);
		m_module->m_llvmIrBuilder.createStore (prevStackTopValue, stackTopVariable);
	}

	// restore current stack top and frame map at every landing pad

	sl::Array <BasicBlock*> landingPadBlockArray = m_module->m_controlFlowMgr.getLandingPadBlockArray ();
	count = landingPadBlockArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BasicBlock* block = landingPadBlockArray [i];
		Scope* scope = block->getLandingPadScope ();
		ASSERT (scope && !block->getLlvmBlock ()->empty ());

		m_module->m_llvmIrBuilder.setInsertPoint (block->getLlvmBlock ()->begin ());

		// on exception landing pads we must restore frame pointer

		if (block->getLandingPadKind () == LandingPadKind_Exception)
			m_module->m_llvmIrBuilder.createStore (m_frameVariable, stackTopVariable);

#if 0
		Value frameMapValue (&scope->m_gcShadowStackFrameMap, type);
		m_module->m_llvmIrBuilder.createStore (frameMapValue, frameMapFieldValue);
#else
		setFrameMap (scope->m_gcShadowStackFrameMap, false); // easier to see in LLVM IR
#endif
	}

	// done

	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
}

//..............................................................................

} // namespace ct
} // namespace jnc
