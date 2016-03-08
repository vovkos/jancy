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
	m_tmpGcRootScope = NULL;
}

void
GcShadowStackMgr::clear ()
{
	m_gcRootTypeArray.clear ();
	m_frameMapList.clear ();
	m_restoreFramePointList.clear ();
	m_gcRootArrayValue.clear ();
	m_frameMapFieldValue.clear ();

	m_currentFrameMap = NULL;
	m_tmpGcRootScope = NULL;
}

void
GcShadowStackMgr::finalizeFunction ()
{
	m_restoreFramePointList.clear ();

	if (m_gcRootTypeArray.isEmpty ())
		return;
	
	finalizeFrame ();

	m_gcRootArrayValue.clear ();
	m_gcRootTypeArray.clear ();
	m_frameMapFieldValue.clear ();

	m_currentFrameMap = NULL;
	m_tmpGcRootScope = NULL;
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
	{
		Value frameMapValue (&frameMap, m_module->m_typeMgr.getStdType (StdType_BytePtr));
		m_module->m_llvmIrBuilder.createStore (frameMapValue, m_frameMapFieldValue);
	}
}

void
GcShadowStackMgr::createTmpGcRoot (const Value& value)
{
	Type* type = value.getType ();
	ASSERT (type->getFlags () & TypeFlag_GcRoot);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createAlloca (type, "tmpGcRoot", NULL, &ptrValue);
	m_module->m_llvmIrBuilder.createStore (value, ptrValue);	
	markGcRoot (ptrValue, type, StackGcRootKind_Temporary);
}

void
GcShadowStackMgr::releaseTmpGcRoots ()
{
	if (!m_tmpGcRootScope)
		return;

	ASSERT (m_tmpGcRootScope == m_module->m_namespaceMgr.getCurrentScope ());
	
	m_module->m_namespaceMgr.closeScope ();
	m_tmpGcRootScope = NULL;
}

void
GcShadowStackMgr::markGcRoot (
	const Value& ptrValue,
	Type* type,
	StackGcRootKind kind,
	Scope* scope
	)
{
	if (!m_gcRootArrayValue)
		preCreateFrame ();

	switch (kind)
	{
	case StackGcRootKind_Temporary:
		if (!m_tmpGcRootScope)
			m_tmpGcRootScope = m_module->m_namespaceMgr.openInternalScope ();

		openFrameMap (m_tmpGcRootScope);
		break;

	case StackGcRootKind_Scope:
		if (!scope)
		{
			scope = m_module->m_namespaceMgr.getCurrentScope ();
			ASSERT (scope);
		}

		openFrameMap (scope);
		break;

	case StackGcRootKind_Function:
		openFrameMap (m_module->m_functionMgr.getCurrentFunction ()->getScope ());
		break;

	default:
		ASSERT (false);
	}

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
	ASSERT (m_frameMapFieldValue);

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

	Value frameMapValue (&frameMap, m_module->m_typeMgr.getStdType (StdType_BytePtr));

	if (!scope->m_firstStackVariable)
	{
		m_module->m_llvmIrBuilder.createStore (frameMapValue, m_frameMapFieldValue);
		return;
	}

	// set the frame map before the very first variable init -- it could be lifted later (if not, no big deal)

	LlvmIrInsertPoint prevInsertPoint;
	bool isInsertPointChanged = m_module->m_llvmIrBuilder.restoreInsertPoint (
		scope->m_firstStackVariable->m_liftInsertPoint, 
		&prevInsertPoint
		);

	llvm::Instruction* llvmStore = m_module->m_llvmIrBuilder.createStore (frameMapValue, m_frameMapFieldValue);

	// update lift insert point -- lift AFTER we've set the frame map
	m_module->m_llvmIrBuilder.saveInsertPoint (&scope->m_firstStackVariable->m_liftInsertPoint);
	ASSERT (scope->m_firstStackVariable->m_liftInsertPoint.m_llvmInstruction == llvmStore); 
	
	if (isInsertPointChanged)
		m_module->m_llvmIrBuilder.restoreInsertPoint (prevInsertPoint);
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
	ASSERT (!m_gcRootArrayValue && !m_frameMapFieldValue);

	Type* type = m_module->m_typeMgr.getStdType (StdType_BytePtr); 
	m_module->m_llvmIrBuilder.createAlloca (type, "gcRootArray_tmp", NULL, &m_gcRootArrayValue);
	m_module->m_llvmIrBuilder.createAlloca (type, "gcShadowStackFrameMapField_tmp", NULL, &m_frameMapFieldValue);

	// both m_gcRootArrayValue/m_frameMapFieldValue will be replaced later
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

	// create gc root array stack variable

	Value gcRootArrayValue;
	m_module->m_llvmIrBuilder.createAlloca (type, "gcRootArray", type->getDataPtrType_c (), &gcRootArrayValue);
	m_module->m_operatorMgr.zeroInitialize (gcRootArrayValue);
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

	// create & initialize frame and set it as the new stack top

	type = m_module->m_typeMgr.getStdType (StdType_GcShadowStackFrame);
	Variable* frameVariable = m_module->m_variableMgr.createSimpleStackVariable ("gcShadowStackFrame", type);

	Value srcValue;
	Value dstValue;

	// GcShadowStackFrame.m_prev

	m_module->m_llvmIrBuilder.createGep2 (frameVariable, 0, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (prevStackTopValue, dstValue);
	
	// GcShadowStackFrame.m_map

	Value frameMapFieldValue;
	type = m_module->m_typeMgr.getStdType (StdType_BytePtr);
	m_module->m_llvmIrBuilder.createGep2 (frameVariable, 1, NULL, &frameMapFieldValue);
	m_module->m_llvmIrBuilder.createStore (type->getZeroValue (), frameMapFieldValue);

	// fixup all uses of frame map field

	ASSERT (llvm::isa <llvm::AllocaInst> (m_frameMapFieldValue.getLlvmValue ()));
	llvmAlloca = (llvm::AllocaInst*) m_frameMapFieldValue.getLlvmValue ();
	llvmAlloca->replaceAllUsesWith (frameMapFieldValue.getLlvmValue ());
	llvmAlloca->eraseFromParent ();

	// GcShadowStackFrame.m_gcRootArray

	m_module->m_llvmIrBuilder.createGep2 (frameVariable, 2, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (gcRootArrayValue, dstValue);

	// GcShadowStackFrame.m_gcRootTypeArray

	type = m_module->m_typeMgr.getStdType (StdType_BytePtr);
	m_module->m_llvmIrBuilder.createBitCast (gcRootTypeArrayVariable, type, &srcValue);
	m_module->m_llvmIrBuilder.createGep2 (frameVariable, 3, NULL, &dstValue);
	m_module->m_llvmIrBuilder.createStore (srcValue, dstValue);

	m_module->m_llvmIrBuilder.createStore (frameVariable, stackTopVariable);

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
		m_module->m_llvmIrBuilder.createStore (frameVariable, stackTopVariable);

		Value frameMapValue (&it->m_frameMap, type);
		m_module->m_llvmIrBuilder.createStore (frameMapValue, frameMapFieldValue);
	}

	// done 

	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
}

//.............................................................................

} // namespace ct
} // namespace jnc
