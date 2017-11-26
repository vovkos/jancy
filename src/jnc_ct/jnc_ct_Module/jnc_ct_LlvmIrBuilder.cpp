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
#include "jnc_ct_LlvmIrBuilder.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

LlvmIrBuilder::LlvmIrBuilder ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_llvmIrBuilder = NULL;
	m_llvmAllocaIrBuilder = NULL;
}

void
LlvmIrBuilder::create ()
{
	ASSERT (!m_llvmIrBuilder);

	m_llvmIrBuilder = new llvm::IRBuilder <> (*m_module->getLlvmContext ());
	m_llvmAllocaIrBuilder = new llvm::IRBuilder <> (*m_module->getLlvmContext ());
}

void
LlvmIrBuilder::clear ()
{
	if (!m_llvmIrBuilder)
		return;

	delete m_llvmIrBuilder;
	delete m_llvmAllocaIrBuilder;

	m_llvmIrBuilder = NULL;
	m_llvmAllocaIrBuilder = NULL;
}

void
LlvmIrBuilder::setAllocaBlock (BasicBlock* block)
{
	llvm::TerminatorInst* llvmJmp = block->getLlvmBlock ()->getTerminator ();
	ASSERT (llvm::isa <llvm::BranchInst> (llvmJmp));

	m_llvmAllocaIrBuilder->SetInsertPoint (llvmJmp);
}

llvm::SwitchInst*
LlvmIrBuilder::createSwitch (
	const Value& value,
	BasicBlock* defaultBlock,
	sl::HashTableIterator <intptr_t, BasicBlock*> firstCase,
	size_t caseCount
	)
{
	Type* type = value.getType ();
	ASSERT (type->getTypeKindFlags () & TypeKindFlag_Integer);

	llvm::SwitchInst* inst = m_llvmIrBuilder->CreateSwitch (
		value.getLlvmValue (),
		defaultBlock->getLlvmBlock (),
		caseCount
		);

	sl::HashTableIterator <intptr_t, BasicBlock*> caseIt = firstCase;
	for (; caseIt; caseIt++)
	{
		Value constValue (caseIt->m_key, type);
		BasicBlock* block = caseIt->m_value;

		inst->addCase ((llvm::ConstantInt*) constValue.getLlvmValue (), block->getLlvmBlock ());
	}

	return inst;
}

void
LlvmIrBuilder::setInsertPoint (BasicBlock* block)
{
	if (!(block->getFlags () & BasicBlockFlag_Entry) || !block->hasTerminator ())
		m_llvmIrBuilder->SetInsertPoint (block->getLlvmBlock ());
	else
		m_llvmIrBuilder->SetInsertPoint (block->getLlvmBlock ()->getTerminator ());
}

void
LlvmIrBuilder::saveInsertPoint (LlvmIrInsertPoint* insertPoint)
{
	insertPoint->m_llvmBlock = m_llvmIrBuilder->GetInsertBlock ();

	if (insertPoint->m_llvmBlock->empty ())
	{
		insertPoint->m_llvmInstruction = NULL;
	}
	else
	{
		llvm::BasicBlock::iterator llvmInstIt = m_llvmIrBuilder->GetInsertPoint ();
		ASSERT (&*llvmInstIt);

		insertPoint->m_llvmInstruction =
			llvmInstIt == insertPoint->m_llvmBlock->begin () ?
				NULL :
			llvmInstIt == insertPoint->m_llvmBlock->end () ?
				&insertPoint->m_llvmBlock->back () :
				&*--llvmInstIt;
	}
}

bool
LlvmIrBuilder::restoreInsertPoint (
	const LlvmIrInsertPoint& insertPoint,
	LlvmIrInsertPoint* prevInsertPoint
	)
{
	saveInsertPoint (prevInsertPoint);
	if (insertPoint == *prevInsertPoint)
		return false;

	restoreInsertPoint (insertPoint);
	return true;
}

void
LlvmIrBuilder::restoreInsertPoint (const LlvmIrInsertPoint& insertPoint)
{
	ASSERT (insertPoint);

	if (!insertPoint.m_llvmInstruction)
	{
		if (insertPoint.m_llvmBlock->empty ())
			m_llvmIrBuilder->SetInsertPoint (insertPoint.m_llvmBlock);
		else
			m_llvmIrBuilder->SetInsertPoint (&insertPoint.m_llvmBlock->front ());
	}
	else
	{
		if (insertPoint.m_llvmInstruction == &insertPoint.m_llvmBlock->back ())
			m_llvmIrBuilder->SetInsertPoint (insertPoint.m_llvmBlock);
		else
			m_llvmIrBuilder->SetInsertPoint (&*++llvm::BasicBlock::iterator (insertPoint.m_llvmInstruction));
	}
}

llvm::IndirectBrInst*
LlvmIrBuilder::createIndirectBr (
	const Value& value,
	BasicBlock** blockArray,
	size_t blockCount
	)
{
	llvm::IndirectBrInst* inst = m_llvmIrBuilder->CreateIndirectBr (value.getLlvmValue (), blockCount);

	for (size_t i = 0; i < blockCount; i++)
		inst->addDestination (blockArray [i]->getLlvmBlock ());

	return inst;
}

llvm::SwitchInst*
LlvmIrBuilder::createSwitch (
	const Value& value,
	BasicBlock* defaultBlock,
	intptr_t* constArray,
	BasicBlock** blockArray,
	size_t caseCount
	)
{
	Type* type = value.getType ();
	ASSERT (type->getTypeKindFlags () & TypeKindFlag_Integer);

	llvm::SwitchInst* inst = m_llvmIrBuilder->CreateSwitch (
		value.getLlvmValue (),
		defaultBlock->getLlvmBlock (),
		caseCount
		);

	for (size_t i = 0; i < caseCount; i++)
	{
		Value constValue (constArray [i], type);
		BasicBlock* block = blockArray [i];

		inst->addCase ((llvm::ConstantInt*) constValue.getLlvmValue (), block->getLlvmBlock ());
	}

	return inst;
}

llvm::PHINode*
LlvmIrBuilder::createPhi (
	const Value* valueArray,
	BasicBlock** blockArray,
	size_t count,
	Value* resultValue
	)
{
	if (valueArray->isEmpty ())
	{
		resultValue->setVoid (m_module);
		return NULL;
	}

	llvm::PHINode* phiNode = m_llvmIrBuilder->CreatePHI (valueArray->getType ()->getLlvmType (), count, "phi");

	for (size_t i = 0; i < count; i++)
		phiNode->addIncoming (valueArray [i].getLlvmValue (), blockArray [i]->getLlvmBlock ());

	resultValue->setLlvmValue (phiNode, valueArray->getType ());
	return phiNode;
}

llvm::PHINode*
LlvmIrBuilder::createPhi (
	const Value& value1,
	BasicBlock* block1,
	const Value& value2,
	BasicBlock* block2,
	Value* resultValue
	)
{
	if (value1.isEmpty ())
	{
		resultValue->setVoid (m_module);
		return NULL;
	}

	llvm::PHINode* phiNode = m_llvmIrBuilder->CreatePHI (value1.getLlvmValue ()->getType (), 2,  "phi");
	phiNode->addIncoming (value1.getLlvmValue (), block1->getLlvmBlock ());
	phiNode->addIncoming (value2.getLlvmValue (), block2->getLlvmBlock ());
	resultValue->setLlvmValue (phiNode, value1.getType ());
	return phiNode;
}

llvm::AllocaInst*
LlvmIrBuilder::createAlloca (
	Type* type,
	const sl::StringRef& name,
	Type* resultType,
	Value* resultValue
	)
{
	llvm::AllocaInst* inst = m_llvmAllocaIrBuilder->CreateAlloca (type->getLlvmType (), 0, name >> toLlvm);
	resultValue->setLlvmValue (inst, resultType);
	return inst;
}

llvm::Value*
LlvmIrBuilder::createGep (
	const Value& value,
	const Value* indexArray,
	size_t indexCount,
	Type* resultType,
	Value* resultValue
	)
{
	char buffer [256];
	sl::Array <llvm::Value*> llvmIndexArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	llvmIndexArray.setCount (indexCount);

	for (size_t i = 0; i < indexCount; i++)
		llvmIndexArray [i] = indexArray [i].getLlvmValue ();

	llvm::Value* inst = m_llvmIrBuilder->CreateGEP (
			value.getLlvmValue (),
			llvm::ArrayRef <llvm::Value*> (llvmIndexArray, indexCount),
			"gep"
			);

	resultValue->setLlvmValue (inst, resultType);
	return inst;
}

llvm::Value*
LlvmIrBuilder::createGep (
	const Value& value,
	const int32_t* indexArray,
	size_t indexCount,
	Type* resultType,
	Value* resultValue
	)
{
	char buffer [256];
	sl::Array <llvm::Value*> llvmIndexArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	llvmIndexArray.setCount (indexCount);

	for (size_t i = 0; i < indexCount; i++)
	{
		Value indexValue;
		indexValue.setConstInt32 (indexArray [i], m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32_u));
		llvmIndexArray [i] = indexValue.getLlvmValue ();
	}

	llvm::Value* inst = m_llvmIrBuilder->CreateGEP (
			value.getLlvmValue (),
			llvm::ArrayRef <llvm::Value*> (llvmIndexArray, indexCount),
			"gep"
			);

	resultValue->setLlvmValue (inst, resultType);
	return inst;
}

llvm::CallInst*
LlvmIrBuilder::createCall (
	const Value& calleeValue,
	CallConv* callConv,
	llvm::Value* const* llvmArgValueArray,
	size_t argCount,
	Type* resultType,
	Value* resultValue
	)
{
	llvm::CallInst* inst;

	if (resultType->getTypeKind () != TypeKind_Void)
	{
		inst = m_llvmIrBuilder->CreateCall (
			calleeValue.getLlvmValue (),
			llvm::ArrayRef <llvm::Value*> (llvmArgValueArray, argCount),
			"call"
			);

		ASSERT (resultValue);
		resultValue->setLlvmValue (inst, resultType);
	}
	else
	{
		inst = m_llvmIrBuilder->CreateCall (
			calleeValue.getLlvmValue (),
			llvm::ArrayRef <llvm::Value*> (llvmArgValueArray, argCount)
			);

		if (resultValue)
			resultValue->setVoid (m_module);
	}

	llvm::CallingConv::ID llvmCallConv = callConv->getLlvmCallConv ();
	if (llvmCallConv)
		inst->setCallingConv (llvmCallConv);

	return inst;
}

llvm::CallInst*
LlvmIrBuilder::createCall (
	const Value& calleeValue,
	CallConv* callConv,
	const sl::BoxList <Value>& argValueList,
	Type* resultType,
	Value* resultValue
	)
{
	size_t argCount = argValueList.getCount ();

	char buffer [256];
	sl::Array <llvm::Value*> llvmArgValueArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	llvmArgValueArray.setCount (argCount);

	sl::BoxIterator <Value> it = argValueList.getHead ();
	for (size_t i = 0; i < argCount; i++, it++)
	{
		ASSERT (it);
		llvmArgValueArray [i] = it->getLlvmValue ();
	}

	return createCall (calleeValue, callConv, llvmArgValueArray, argCount, resultType, resultValue);
}

llvm::CallInst*
LlvmIrBuilder::createCall (
	const Value& calleeValue,
	CallConv* callConv,
	const Value* argArray,
	size_t argCount,
	Type* resultType,
	Value* resultValue
	)
{
	char buffer [256];
	sl::Array <llvm::Value*> llvmArgValueArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	llvmArgValueArray.setCount (argCount);

	for (size_t i = 0; i < argCount; i++)
		llvmArgValueArray [i] = argArray [i].getLlvmValue ();

	return createCall (calleeValue, callConv, llvmArgValueArray, argCount, resultType, resultValue);
}

bool
LlvmIrBuilder::createClosureFunctionPtr (
	const Value& rawPtrValue,
	const Value& rawClosureValue,
	FunctionPtrType* resultType,
	Value* resultValue
	)
{
	Value ptrValue;
	Value closureValue;

	createBitCast (rawPtrValue, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);
	createBitCast (rawClosureValue, m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr), &closureValue);

	Value functionPtrValue = resultType->getUndefValue ();
	createInsertValue (functionPtrValue, ptrValue, 0, NULL, &functionPtrValue);
	createInsertValue (functionPtrValue, closureValue, 1, resultType, resultValue);
	return true;
}

bool
LlvmIrBuilder::createClosurePropertyPtr (
	const Value& rawPtrValue,
	const Value& rawClosureValue,
	PropertyPtrType* resultType,
	Value* resultValue
	)
{
	Value ptrValue;
	Value closureValue;

	createBitCast (rawPtrValue, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);
	createBitCast (rawClosureValue, m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr), &closureValue);

	Value functionPtrValue = resultType->getUndefValue ();
	createInsertValue (functionPtrValue, ptrValue, 0, NULL, &functionPtrValue);
	createInsertValue (functionPtrValue, closureValue, 1, resultType, resultValue);
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
