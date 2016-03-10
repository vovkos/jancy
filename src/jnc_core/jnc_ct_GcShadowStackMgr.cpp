#include "pch.h"
#include "jnc_ct_GcShadowStackMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

GcShadowStackMgr::GcShadowStackMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_currentFrameMap = NULL;
}

void
GcShadowStackMgr::clear ()
{
	m_gcRootTypeArray.clear ();
	m_frameMapList.clear ();
	m_restoreFramePointList.clear ();
	m_gcRootArrayValue.clear ();	
	m_frameVariable = NULL;
	m_currentFrameMap = NULL;
}

void
GcShadowStackMgr::finalizeFunction ()
{
	m_restoreFramePointList.clear (); // this list is built even when there are no gc-roots

	if (!m_frameVariable)
		return;

	finalizeFrame ();

	m_gcRootArrayValue.clear ();
	m_gcRootTypeArray.clear ();	
	m_frameVariable = NULL;
	m_currentFrameMap = NULL;
}

void
GcShadowStackMgr::finalizeScope (Scope* scope)
{
	Scope* parentScope = scope->getParentScope ();
	GcShadowStackFrameMap* frameMap = parentScope ? parentScope->m_gcShadowStackFrameMap : NULL; 

	if (frameMap == m_currentFrameMap) // stays the same
		return;

	m_currentFrameMap = frameMap;

	if (m_module->m_controlFlowMgr.getCurrentBlock ()->getFlags () & BasicBlockFlag_Reachable)
		setFrameMap (frameMap, false);
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

	openFrameMap (scope);

	size_t index = m_gcRootTypeArray.getCount ();

	Value bytePtrValue;
	Value gcRootValue;
	Type* bytePtrType = m_module->m_typeMgr.getStdType (StdType_BytePtr);

	m_module->m_llvmIrBuilder.createGep (m_gcRootArrayValue, index, NULL, &gcRootValue);
	m_module->m_llvmIrBuilder.createBitCast (ptrValue, bytePtrType, &bytePtrValue);
	m_module->m_llvmIrBuilder.createStore (bytePtrValue, gcRootValue);

	m_currentFrameMap->m_gcRootIndexArray.append (index);
	m_gcRootTypeArray.append (type);
}

void
GcShadowStackMgr::openFrameMap (Scope* scope)
{
	Scope* parentScope = scope->getParentScope ();
	GcShadowStackFrameMap* prevFrameMap = parentScope ? parentScope->m_gcShadowStackFrameMap : NULL;

	if (scope->m_gcShadowStackFrameMap != prevFrameMap) // this scope already has its own frame map
	{
		m_currentFrameMap = scope->m_gcShadowStackFrameMap;
		return;
	}

	GcShadowStackFrameMap* frameMap = AXL_MEM_NEW (GcShadowStackFrameMap);
	frameMap->m_prev = prevFrameMap;
	m_frameMapList.insertTail (frameMap);
	scope->m_gcShadowStackFrameMap = frameMap;
	m_currentFrameMap = frameMap;

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
GcShadowStackMgr::addRestoreFramePoint (
	BasicBlock* block,
	GcShadowStackFrameMap* frameMap
	)
{
	RestoreFramePoint* restorePoint = AXL_MEM_NEW (RestoreFramePoint);
	restorePoint->m_block = block;
	restorePoint->m_frameMap = frameMap;
	m_restoreFramePointList.insertTail (restorePoint);
}

void
GcShadowStackMgr::preCreateFrame ()
{
	ASSERT (!m_frameVariable && !m_gcRootArrayValue);

	Type* type = m_module->m_typeMgr.getStdType (StdType_GcShadowStackFrame); 
	m_frameVariable = m_module->m_variableMgr.createSimpleStackVariable ("gcShadowStackFrame", type);

	type = m_module->m_typeMgr.getStdType (StdType_BytePtr); 
	m_module->m_llvmIrBuilder.createAlloca (type, "gcRootArray_tmp", NULL, &m_gcRootArrayValue);

	// m_gcRootArrayValue will be replaced later
}

void
GcShadowStackMgr::finalizeFrame ()
{
	ASSERT (!m_currentFrameMap); // finalizeScope must have been called

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	// create shadow stack frame in the beginning of entry block (which dominates the whole body)

	BasicBlock* entryBlock = function->getEntryBlock ();
	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (entryBlock);

	m_module->m_controlFlowMgr.setCurrentBlock (entryBlock);
	m_module->m_llvmIrBuilder.setInsertPoint (entryBlock->getLlvmBlock ()->begin ());

	size_t count = m_gcRootTypeArray.getCount ();

	// create gc root type array static variable

	Type* type = m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr_u)->getArrayType (count);

	Variable* gcRootTypeArrayVariable = m_module->m_variableMgr.createSimpleStaticVariable (
		"gcShadowStackTypeArray",
		function->m_tag + ".gcShadowStackTypeArray",
		type,
		Value (m_gcRootTypeArray.ca (), type)
		);

	// create gc root array stack variable (no need to zero-init now, GcHeap::openFrameMap will do that)

	Value gcRootArrayValue;
	m_module->m_llvmIrBuilder.createAlloca (type, "gcRootArray", type->getDataPtrType_c (), &gcRootArrayValue);
	type = m_module->m_typeMgr.getStdType (StdType_BytePtr)->getDataPtrType_c ();
	m_module->m_llvmIrBuilder.createBitCast (gcRootArrayValue, type, &gcRootArrayValue);

	// fixup all uses of gc root array

	ASSERT (llvm::isa <llvm::AllocaInst> (m_gcRootArrayValue.getLlvmValue ()));
	llvm::AllocaInst* llvmAlloca = (llvm::AllocaInst*) m_gcRootArrayValue.getLlvmValue ();
	llvmAlloca->replaceAllUsesWith (gcRootArrayValue.getLlvmValue ());
	llvmAlloca->eraseFromParent ();

	// get prev shadow stack top

	Value prevStackTopValue;
	Variable* stackTopVariable = m_module->m_variableMgr.getStdVariable (StdVariable_GcShadowStackTop);
	m_module->m_llvmIrBuilder.createLoad (stackTopVariable , NULL, &prevStackTopValue);

	// initialize frame and set it as the new stack top

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

	// GcShadowStackFrame.m_gcRootTypeArray

	type = m_module->m_typeMgr.getStdType (StdType_BytePtr);
	m_module->m_llvmIrBuilder.createBitCast (gcRootTypeArrayVariable, type, &srcValue);
	m_module->m_llvmIrBuilder.createGep2 (m_frameVariable, 3, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (srcValue, dstValue);

	m_module->m_llvmIrBuilder.createStore (m_frameVariable, stackTopVariable);

	// restore previous stack top before every ret

	sl::Array <BasicBlock*> returnBlockArray = m_module->m_controlFlowMgr.getReturnBlockArray ();
	count = returnBlockArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BasicBlock* block = returnBlockArray [i];
		llvm::TerminatorInst* llvmRet = block->getLlvmBlock ()->getTerminator ();
		ASSERT (llvm::isa <llvm::ReturnInst> (llvmRet));

		m_module->m_llvmIrBuilder.setInsertPoint (llvmRet);
		m_module->m_llvmIrBuilder.createStore (prevStackTopValue, stackTopVariable);
	}

	// restore current stack top at every restore point

	sl::Iterator <RestoreFramePoint> it = m_restoreFramePointList.getHead ();
	for (; it; it++)
	{
		ASSERT (!it->m_block->getLlvmBlock ()->empty ());
		m_module->m_llvmIrBuilder.setInsertPoint (it->m_block->getLlvmBlock ()->begin ());
		m_module->m_llvmIrBuilder.createStore (m_frameVariable, stackTopVariable);

		Value frameMapValue (&it->m_frameMap, type);
		m_module->m_llvmIrBuilder.createStore (frameMapValue, frameMapFieldValue);
	}

	// done 

	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
}

//.............................................................................

} // namespace ct
} // namespace jnc
