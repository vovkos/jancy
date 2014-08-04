#include "pch.h"
#include "jnc_CallConv_gcc32.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

llvm::FunctionType*
CCallConv_gcc32::GetLlvmFunctionType (CFunctionType* pFunctionType)
{
	CType* pReturnType = pFunctionType->GetReturnType ();
	if (!(pReturnType->GetFlags () & ETypeFlag_StructRet))
		return CCallConv::GetLlvmFunctionType (pFunctionType);

	rtl::CArrayT <CFunctionArg*> ArgArray = pFunctionType->GetArgArray ();
	size_t ArgCount = ArgArray.GetCount () + 1;

	char Buffer [256];
	rtl::CArrayT <llvm::Type*> LlvmArgTypeArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	LlvmArgTypeArray.SetCount (ArgCount);

	LlvmArgTypeArray [0] = pReturnType->GetDataPtrType_c ()->GetLlvmType ();

	for (size_t i = 0, j = 1; j < ArgCount; i++, j++)
		LlvmArgTypeArray [j] = ArgArray [i]->GetType ()->GetLlvmType ();

	return llvm::FunctionType::get (
		m_pModule->GetSimpleType (EType_Void)->GetLlvmType (),
		llvm::ArrayRef <llvm::Type*> (LlvmArgTypeArray, ArgCount),
		(pFunctionType->GetFlags () & EFunctionTypeFlag_VarArg) != 0
		);
}

llvm::Function*
CCallConv_gcc32::CreateLlvmFunction (
	CFunctionType* pFunctionType,
	const char* pTag
	)
{
	llvm::Function* pLlvmFunction = CCallConv::CreateLlvmFunction (pFunctionType, pTag);

	CType* pReturnType = pFunctionType->GetReturnType ();
	if (pReturnType->GetFlags () & ETypeFlag_StructRet)
		pLlvmFunction->addAttribute (1, llvm::Attribute::StructRet);

	return pLlvmFunction;
}

void
CCallConv_gcc32::Call (
	const CValue& CalleeValue,
	CFunctionType* pFunctionType,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	CType* pReturnType = pFunctionType->GetReturnType ();
	if (!(pReturnType->GetFlags () & ETypeFlag_StructRet))
	{
		CCallConv::Call (CalleeValue, pFunctionType, pArgValueList, pResultValue);
		return;
	}

	CVariable* pReturnVariable = m_pModule->m_VariableMgr.CreateStackVariable ("returnValue", pReturnType);
	m_pModule->m_VariableMgr.AllocatePrimeInitializeVariable (pReturnVariable);

	CValue ReturnPtrValue;
	ReturnPtrValue.SetLlvmValue (
		pReturnVariable->GetLlvmValue (),
		pReturnType->GetDataPtrType_c (),
		EValue_Variable
		);

	pArgValueList->InsertHead (ReturnPtrValue);

	llvm::CallInst* pInst = m_pModule->m_LlvmIrBuilder.CreateCall (
		CalleeValue,
		pFunctionType->GetCallConv (),
		*pArgValueList,
		m_pModule->GetSimpleType (EType_Void),
		NULL
		);

	pInst->addAttribute (1, llvm::Attribute::StructRet);

	*pResultValue = pReturnVariable;
}

void
CCallConv_gcc32::Return (
	CFunction* pFunction,
	const CValue& Value
	)
{
	CType* pReturnType = pFunction->GetType ()->GetReturnType ();
	if (!(pReturnType->GetFlags () & ETypeFlag_StructRet))
	{
		CCallConv::Return (pFunction, Value);
		return;
	}

	llvm::Function::arg_iterator LlvmArg = pFunction->GetLlvmFunction ()->arg_begin();

	CValue ReturnPtrValue;
	ReturnPtrValue.SetLlvmValue (LlvmArg, pReturnType->GetDataPtrType_c ());
	m_pModule->m_LlvmIrBuilder.CreateStore (Value, ReturnPtrValue);
	m_pModule->m_LlvmIrBuilder.CreateRet ();
}

CValue
CCallConv_gcc32::GetThisArgValue (CFunction* pFunction)
{
	ASSERT (pFunction->IsMember ());

	CType* pReturnType = pFunction->GetType ()->GetReturnType ();
	if (!(pReturnType->GetFlags () & ETypeFlag_StructRet))
		return CCallConv::GetThisArgValue (pFunction);

	llvm::Function::arg_iterator LlvmArg = pFunction->GetLlvmFunction ()->arg_begin();
	LlvmArg++;
	return CValue (LlvmArg, pFunction->GetThisArgType ());
}

void
CCallConv_gcc32::CreateArgVariables (CFunction* pFunction)
{
	CType* pReturnType = pFunction->GetType ()->GetReturnType ();
	CCallConv::CreateArgVariablesImpl (
		pFunction,
		(pReturnType->GetFlags () & ETypeFlag_StructRet) ? 1 : 0
		);
}

//.............................................................................

} // namespace jnc {
