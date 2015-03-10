#include "pch.h"
#include "jnc_LlvmIrBuilder.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

LlvmIrBuilder::LlvmIrBuilder ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_llvmIrBuilder = NULL;

	m_commentMdKind = 0;
}

void
LlvmIrBuilder::create ()
{
	clear ();

	m_llvmIrBuilder = new llvm::IRBuilder <> (*m_module->getLlvmContext ());

	if (m_module->getFlags () & ModuleFlag_IrComments)
		m_commentMdKind = m_module->getLlvmContext ()->getMDKindID ("jnc.comment");
}

void
LlvmIrBuilder::clear ()
{
	if (!m_llvmIrBuilder)
		return;

	delete m_llvmIrBuilder;
	m_llvmIrBuilder = NULL;
}

bool
LlvmIrBuilder::createComment_va (
	const char* format,
	axl_va_list va
	)
{
	if (!(m_module->getFlags () & ModuleFlag_IrComments))
		return false;

	char buffer [256];
	rtl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	string.format_va (format, va);

	return createComment_0 (string);
}

bool
LlvmIrBuilder::createComment_0 (const char* text)
{
	if (!(m_module->getFlags () & ModuleFlag_IrComments))
		return false;

	BasicBlock* block = m_module->m_controlFlowMgr.getCurrentBlock ();
	llvm::BasicBlock* llvmBlock = block->getLlvmBlock ();

	if (llvmBlock->getInstList ().empty ())
	{
		block->m_leadingComment = text;
		return true;
	}

	llvm::Instruction* inst = &llvmBlock->getInstList ().back ();
	llvm::MDString* mdString = llvm::MDString::get (*m_module->getLlvmContext (), text);
	llvm::MDNode* mdNode = llvm::MDNode::get (*m_module->getLlvmContext (), llvm::ArrayRef <llvm::Value*> ((llvm::Value**) &mdString, 1));
	inst->setMetadata (m_commentMdKind, mdNode);
	return true;
}

llvm::SwitchInst*
LlvmIrBuilder::createSwitch (
	const Value& value,
	BasicBlock* defaultBlock,
	rtl::HashTableMapIterator <intptr_t, BasicBlock*> firstCase,
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

	rtl::HashTableMapIterator <intptr_t, BasicBlock*> caseIt = firstCase;
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
	const char* name,
	Type* resultType,
	Value* resultValue
	)
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	// always create alloca in entry block

	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (function->getEntryBlock ());
	llvm::AllocaInst* inst = m_llvmIrBuilder->CreateAlloca (type->getLlvmType (), 0, name);
	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);

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
	rtl::Array <llvm::Value*> llvmIndexArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	llvmIndexArray.setCount (indexCount);

	for (size_t i = 0; i < indexCount; i++)
		llvmIndexArray [i] = indexArray [i].getLlvmValue ();

	llvm::Value* inst;
	inst = m_llvmIrBuilder->CreateGEP (
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
	rtl::Array <llvm::Value*> llvmIndexArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	llvmIndexArray.setCount (indexCount);

	for (size_t i = 0; i < indexCount; i++)
	{
		Value indexValue;
		indexValue.setConstInt32 (indexArray [i], m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32_u));
		llvmIndexArray [i] = indexValue.getLlvmValue ();
	}

	llvm::Value* inst;
	inst = m_llvmIrBuilder->CreateGEP (
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
	const rtl::BoxList <Value>& argValueList,
	Type* resultType,
	Value* resultValue
	)
{
	size_t argCount = argValueList.getCount ();

	char buffer [256];
	rtl::Array <llvm::Value*> llvmArgValueArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	llvmArgValueArray.setCount (argCount);

	rtl::BoxIterator <Value> it = argValueList.getHead ();
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
	rtl::Array <llvm::Value*> llvmArgValueArray (ref::BufKind_Stack, buffer, sizeof (buffer));
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
	createBitCast (rawClosureValue, m_module->m_typeMgr.getStdType (StdType_ObjectPtr), &closureValue);

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
	createBitCast (rawClosureValue, m_module->m_typeMgr.getStdType (StdType_ObjectPtr), &closureValue);

	Value functionPtrValue = resultType->getUndefValue ();
	createInsertValue (functionPtrValue, ptrValue, 0, NULL, &functionPtrValue);
	createInsertValue (functionPtrValue, closureValue, 1, resultType, resultValue);
	return true;
}

bool
LlvmIrBuilder::runtimeError (const Value& errorValue)
{
	Function* runtimeError = m_module->m_functionMgr.getStdFunction (StdFunction_RuntimeError);

	// TODO: calc real code address

	Value codeAddrValue = m_module->m_typeMgr.getStdType (StdType_BytePtr)->getZeroValue ();

	createCall2 (
		runtimeError,
		runtimeError->getType (),
		errorValue,
		codeAddrValue,
		NULL
		);

	return true;
}

//.............................................................................

} // namespace jnc {
