#include "pch.h"
#include "jnc_LlvmIrBuilder.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CLlvmIrBuilder::CLlvmIrBuilder ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	m_pLlvmIrBuilder = NULL;
}

void
CLlvmIrBuilder::Create ()
{
	Clear ();

	m_pLlvmIrBuilder = new llvm::IRBuilder <> (*m_pModule->GetLlvmContext ());
	m_CommentMdKind = m_pModule->GetLlvmContext ()->getMDKindID ("jnc.comment");
}

void
CLlvmIrBuilder::Clear ()
{
	if (!m_pLlvmIrBuilder)
		return;

	delete m_pLlvmIrBuilder;
	m_pLlvmIrBuilder = NULL;
}

bool
CLlvmIrBuilder::CreateComment_va (
	const char* pFormat,
	axl_va_list va
	)
{
	if (!(m_pModule->GetFlags () & EModuleFlag_IrComments))
		return false;

	char Buffer [256];
	rtl::CString String (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	String.Format_va (pFormat, va);

	return CreateComment_0 (String);
}

bool
CLlvmIrBuilder::CreateComment_0 (const char* pText)
{
	if (!(m_pModule->GetFlags () & EModuleFlag_IrComments))
		return false;

	CBasicBlock* pBlock = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	llvm::BasicBlock* pLlvmBlock = pBlock->GetLlvmBlock ();

	if (pLlvmBlock->getInstList ().empty ())
	{
		pBlock->m_LeadingComment = pText;
		return true;
	}

	llvm::Instruction* pInst = &pLlvmBlock->getInstList ().back ();
	llvm::MDString* pMdString = llvm::MDString::get (*m_pModule->GetLlvmContext (), pText);
	llvm::MDNode* pMdNode = llvm::MDNode::get (*m_pModule->GetLlvmContext (), llvm::ArrayRef <llvm::Value*> ((llvm::Value**) &pMdString, 1));
	pInst->setMetadata (m_CommentMdKind, pMdNode);
	return true;
}

llvm::SwitchInst*
CLlvmIrBuilder::CreateSwitch (
	const CValue& Value,
	CBasicBlock* pDefaultBlock,
	rtl::CHashTableMapIteratorT <intptr_t, CBasicBlock*> FirstCase,
	size_t CaseCount
	)
{
	CType* pType = Value.GetType ();
	ASSERT (pType->GetTypeKindFlags () & ETypeKindFlag_Integer);

	llvm::SwitchInst* pInst = m_pLlvmIrBuilder->CreateSwitch (
		Value.GetLlvmValue (),
		pDefaultBlock->GetLlvmBlock (),
		CaseCount
		);

	rtl::CHashTableMapIteratorT <intptr_t, CBasicBlock*> Case = FirstCase;
	for (; Case; Case++)
	{
		CValue ConstValue (Case->m_Key, pType);
		CBasicBlock* pBlock = Case->m_Value;

		pInst->addCase ((llvm::ConstantInt*) ConstValue.GetLlvmValue (), pBlock->GetLlvmBlock ());
	}

	return pInst;
}

void
CLlvmIrBuilder::SetInsertPoint (CBasicBlock* pBlock)
{
	if (!(pBlock->GetFlags () & EBasicBlockFlag_Entry) || !pBlock->HasTerminator ())
		m_pLlvmIrBuilder->SetInsertPoint (pBlock->GetLlvmBlock ());
	else
		m_pLlvmIrBuilder->SetInsertPoint (pBlock->GetLlvmBlock ()->getTerminator ());
}

llvm::IndirectBrInst*
CLlvmIrBuilder::CreateIndirectBr (
	const CValue& Value,
	CBasicBlock** ppBlockArray,
	size_t BlockCount
	)
{
	llvm::IndirectBrInst* pInst = m_pLlvmIrBuilder->CreateIndirectBr (Value.GetLlvmValue (), BlockCount);

	for (size_t i = 0; i < BlockCount; i++)
		pInst->addDestination (ppBlockArray [i]->GetLlvmBlock ());

	return pInst;
}

llvm::SwitchInst*
CLlvmIrBuilder::CreateSwitch (
	const CValue& Value,
	CBasicBlock* pDefaultBlock,
	intptr_t* pConstArray,
	CBasicBlock** pBlockArray,
	size_t CaseCount
	)
{
	CType* pType = Value.GetType ();
	ASSERT (pType->GetTypeKindFlags () & ETypeKindFlag_Integer);

	llvm::SwitchInst* pInst = m_pLlvmIrBuilder->CreateSwitch (
		Value.GetLlvmValue (),
		pDefaultBlock->GetLlvmBlock (),
		CaseCount
		);

	for (size_t i = 0; i < CaseCount; i++)
	{
		CValue ConstValue (pConstArray [i], pType);
		CBasicBlock* pBlock = pBlockArray [i];

		pInst->addCase ((llvm::ConstantInt*) ConstValue.GetLlvmValue (), pBlock->GetLlvmBlock ());
	}

	return pInst;
}


llvm::PHINode*
CLlvmIrBuilder::CreatePhi (
	const CValue* pValueArray,
	CBasicBlock** pBlockArray,
	size_t Count,
	CValue* pResultValue
	)
{
	if (pValueArray->IsEmpty ())
	{
		pResultValue->SetVoid ();
		return NULL;
	}

	llvm::PHINode* pPhiNode = m_pLlvmIrBuilder->CreatePHI (pValueArray->GetType ()->GetLlvmType (), Count, "phi");

	for (size_t i = 0; i < Count; i++)
		pPhiNode->addIncoming (pValueArray [i].GetLlvmValue (), pBlockArray [i]->GetLlvmBlock ());

	pResultValue->SetLlvmValue (pPhiNode, pValueArray->GetType ());
	return pPhiNode;
}

llvm::PHINode*
CLlvmIrBuilder::CreatePhi (
	const CValue& Value1,
	CBasicBlock* pBlock1,
	const CValue& Value2,
	CBasicBlock* pBlock2,
	CValue* pResultValue
	)
{
	if (Value1.IsEmpty ())
	{
		pResultValue->SetVoid ();
		return NULL;
	}

	llvm::PHINode* pPhiNode = m_pLlvmIrBuilder->CreatePHI (Value1.GetLlvmValue ()->getType (), 2,  "phi");
	pPhiNode->addIncoming (Value1.GetLlvmValue (), pBlock1->GetLlvmBlock ());
	pPhiNode->addIncoming (Value2.GetLlvmValue (), pBlock2->GetLlvmBlock ());
	pResultValue->SetLlvmValue (pPhiNode, Value1.GetType ());
	return pPhiNode;
}

llvm::Value*
CLlvmIrBuilder::CreateGep (
	const CValue& Value,
	const CValue* pIndexArray,
	size_t IndexCount,
	CType* pResultType,
	CValue* pResultValue
	)
{
	char Buffer [256];
	rtl::CArrayT <llvm::Value*> LlvmIndexArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	LlvmIndexArray.SetCount (IndexCount);

	for (size_t i = 0; i < IndexCount; i++)
		LlvmIndexArray [i] = pIndexArray [i].GetLlvmValue ();

	llvm::Value* pInst;
	pInst = m_pLlvmIrBuilder->CreateGEP (
			Value.GetLlvmValue (),
			llvm::ArrayRef <llvm::Value*> (LlvmIndexArray, IndexCount),
			"gep"
			);

	pResultValue->SetLlvmValue (pInst, pResultType);
	return pInst;
}

llvm::Value*
CLlvmIrBuilder::CreateGep (
	const CValue& Value,
	const int32_t* pIndexArray,
	size_t IndexCount,
	CType* pResultType,
	CValue* pResultValue
	)
{
	char Buffer [256];
	rtl::CArrayT <llvm::Value*> LlvmIndexArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	LlvmIndexArray.SetCount (IndexCount);

	for (size_t i = 0; i < IndexCount; i++)
	{
		CValue IndexValue;
		IndexValue.SetConstInt32 (pIndexArray [i], EType_Int32_u);
		LlvmIndexArray [i] = IndexValue.GetLlvmValue ();
	}

	llvm::Value* pInst;
	pInst = m_pLlvmIrBuilder->CreateGEP (
			Value.GetLlvmValue (),
			llvm::ArrayRef <llvm::Value*> (LlvmIndexArray, IndexCount),
			"gep"
			);

	pResultValue->SetLlvmValue (pInst, pResultType);
	return pInst;
}

llvm::CallInst*
CLlvmIrBuilder::CreateCall (
	const CValue& CalleeValue,
	CCallConv* pCallConv,
	llvm::Value* const* pLlvmArgValueArray,
	size_t ArgCount,
	CType* pResultType,
	CValue* pResultValue
	)
{
	llvm::CallInst* pInst;

	if (pResultType->GetTypeKind () != EType_Void)
	{
		pInst = m_pLlvmIrBuilder->CreateCall (
			CalleeValue.GetLlvmValue (),
			llvm::ArrayRef <llvm::Value*> (pLlvmArgValueArray, ArgCount),
			"call"
			);

		ASSERT (pResultValue);
		pResultValue->SetLlvmValue (pInst, pResultType);
	}
	else
	{
		pInst = m_pLlvmIrBuilder->CreateCall (
			CalleeValue.GetLlvmValue (),
			llvm::ArrayRef <llvm::Value*> (pLlvmArgValueArray, ArgCount)
			);

		if (pResultValue)
			pResultValue->SetVoid ();
	}

	llvm::CallingConv::ID LlvmCallConv = pCallConv->GetLlvmCallConv ();
	if (LlvmCallConv)
		pInst->setCallingConv (LlvmCallConv);

	return pInst;
}

llvm::CallInst*
CLlvmIrBuilder::CreateCall (
	const CValue& CalleeValue,
	CCallConv* pCallConv,
	const rtl::CBoxListT <CValue>& ArgValueList,
	CType* pResultType,
	CValue* pResultValue
	)
{
	size_t ArgCount = ArgValueList.GetCount ();

	char Buffer [256];
	rtl::CArrayT <llvm::Value*> LlvmArgValueArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	LlvmArgValueArray.SetCount (ArgCount);

	rtl::CBoxIteratorT <CValue> It = ArgValueList.GetHead ();
	for (size_t i = 0; i < ArgCount; i++, It++)
	{
		ASSERT (It);
		LlvmArgValueArray [i] = It->GetLlvmValue ();
	}

	return CreateCall (CalleeValue, pCallConv, LlvmArgValueArray, ArgCount, pResultType, pResultValue);
}

llvm::CallInst*
CLlvmIrBuilder::CreateCall (
	const CValue& CalleeValue,
	CCallConv* pCallConv,
	const CValue* pArgArray,
	size_t ArgCount,
	CType* pResultType,
	CValue* pResultValue
	)
{
	char Buffer [256];
	rtl::CArrayT <llvm::Value*> LlvmArgValueArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	LlvmArgValueArray.SetCount (ArgCount);

	for (size_t i = 0; i < ArgCount; i++)
		LlvmArgValueArray [i] = pArgArray [i].GetLlvmValue ();

	return CreateCall (CalleeValue, pCallConv, LlvmArgValueArray, ArgCount, pResultType, pResultValue);
}

bool
CLlvmIrBuilder::CreateClosureFunctionPtr (
	const CValue& RawPfnValue,
	const CValue& RawIfaceValue,
	CFunctionPtrType* pResultType,
	CValue* pResultValue
	)
{
	CLlvmScopeComment Comment (this, "create closure function pointer");

	CValue PfnValue;
	CValue IfaceValue;
	CValue FunctionPtrValue = pResultType->GetUndefValue ();

	CFunctionType* pStdObjectMemberMethodType = pResultType->GetTargetType ()->GetStdObjectMemberMethodType ();

	CreateBitCast (RawPfnValue, pStdObjectMemberMethodType->GetFunctionPtrType (EFunctionPtrType_Thin), &PfnValue);
	CreateBitCast (RawIfaceValue, m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr), &IfaceValue);

	CreateInsertValue (FunctionPtrValue, PfnValue, 0, NULL, &FunctionPtrValue);
	CreateInsertValue (FunctionPtrValue, IfaceValue, 1, pResultType, pResultValue);
	return true;
}

bool
CLlvmIrBuilder::CreateClosurePropertyPtr (
	const CValue& RawPfnValue,
	const CValue& RawIfaceValue,
	CPropertyPtrType* pResultType,
	CValue* pResultValue
	)
{
	CLlvmScopeComment Comment (this, "create closure property pointer");

	CValue PfnValue;
	CValue IfaceValue;
	CValue PropertyPtrValue = pResultType->GetUndefValue ();

	CPropertyType* pStdObjectMemberPropertyType = pResultType->GetTargetType ()->GetStdObjectMemberPropertyType ();

	CreateBitCast (RawPfnValue, pStdObjectMemberPropertyType->GetPropertyPtrType (EPropertyPtrType_Thin), &PfnValue);
	CreateBitCast (RawIfaceValue, m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr), &IfaceValue);

	CreateInsertValue (PropertyPtrValue, PfnValue, 0, NULL, &PropertyPtrValue);
	CreateInsertValue (PropertyPtrValue, IfaceValue, 1, pResultType, pResultValue);
	return true;
}

bool
CLlvmIrBuilder::RuntimeError (const CValue& ErrorValue)
{
	CFunction* pRuntimeError = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_RuntimeError);

	// TODO: calc real code address

	CValue CodeAddrValue = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr)->GetZeroValue ();

	CreateCall2 (
		pRuntimeError,
		pRuntimeError->GetType (),
		ErrorValue,
		CodeAddrValue,
		NULL
		);

	return true;
}

//.............................................................................

} // namespace jnc {
