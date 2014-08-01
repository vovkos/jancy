#include "pch.h"
#include "jnc_CdeclCallConv_msc64.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

llvm::FunctionType*
CCdeclCallConv_msc64::GetLlvmFunctionType (CFunctionType* pFunctionType)
{
	CType* pReturnType = pFunctionType->GetReturnType ();
	rtl::CArrayT <CFunctionArg*> ArgArray = pFunctionType->GetArgArray ();
	size_t ArgCount = ArgArray.GetCount ();

	char Buffer [256];
	rtl::CArrayT <llvm::Type*> LlvmArgTypeArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));
	LlvmArgTypeArray.SetCount (ArgCount);

	size_t j = 0;

	if (pReturnType->GetFlags () & ETypeFlag_StructRet)
	{
		if (pReturnType->GetSize () <= sizeof (uint64_t))
		{
			pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64);
		}
		else
		{
			pReturnType = pReturnType->GetDataPtrType_c ();

			ArgCount++;
			LlvmArgTypeArray.SetCount (ArgCount);
			LlvmArgTypeArray [0] = pReturnType->GetLlvmType ();
			j = 1;
		}
	}

	bool HasStructRetArgs = false;

	for (size_t i = 0; j < ArgCount; i++, j++)
	{
		CType* pType = ArgArray [i]->GetType ();
		if (!(pType->GetFlags () & ETypeFlag_StructRet))
		{
			LlvmArgTypeArray [j] = pType->GetLlvmType ();
		}
		else if (pType->GetSize () <= sizeof (uint64_t))
		{
			LlvmArgTypeArray [j] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64)->GetLlvmType ();
			HasStructRetArgs = true;
		}
		else
		{
			LlvmArgTypeArray [j] = pType->GetDataPtrType_c ()->GetLlvmType ();
			HasStructRetArgs = true;
		}
	}

	if (HasStructRetArgs)
		pFunctionType->m_Flags |= EFunctionTypeFlag_StructRetArgs;

	return llvm::FunctionType::get (
		pReturnType->GetLlvmType (),
		llvm::ArrayRef <llvm::Type*> (LlvmArgTypeArray, ArgCount),
		(pFunctionType->GetFlags () & EFunctionTypeFlag_VarArg) != 0
		);
}

llvm::Function*
CCdeclCallConv_msc64::CreateLlvmFunction (
	CFunctionType* pFunctionType,
	const char* pTag
	)
{
	llvm::Function* pLlvmFunction = CCallConv::CreateLlvmFunction (pFunctionType, pTag);

	CType* pReturnType = pFunctionType->GetReturnType ();
	if (!(pReturnType->GetFlags () & ETypeFlag_StructRet) ||
		pReturnType->GetSize () <= sizeof (uint64_t))
		return pLlvmFunction;

	pLlvmFunction->addAttribute (1, llvm::Attribute::StructRet);
	return pLlvmFunction;
}

void
CCdeclCallConv_msc64::Call (
	const CValue& CalleeValue,
	CFunctionType* pFunctionType,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	CType* pReturnType = pFunctionType->GetReturnType ();

	if (!(pFunctionType->GetFlags () & EFunctionTypeFlag_StructRetArgs) &&
		!(pReturnType->GetFlags () & ETypeFlag_StructRet))
	{
		CCallConv::Call (CalleeValue, pFunctionType, pArgValueList, pResultValue);
		return;
	}

	CValue TmpReturnValue;

	if (pReturnType->GetFlags () & ETypeFlag_StructRet)
	{
		m_pModule->m_LlvmIrBuilder.CreateAlloca (pReturnType, "tmpRetVal", NULL, &TmpReturnValue);

		if (pReturnType->GetSize () > sizeof (uint64_t))
			pArgValueList->InsertHead (TmpReturnValue);
	}

	if (pFunctionType->GetFlags () & EFunctionTypeFlag_StructRetArgs)
	{
		rtl::CBoxIteratorT <CValue> It = pArgValueList->GetHead ();
		for (; It; It++)
		{
			CType* pType = It->GetType ();
			if (!(pType->GetFlags () & ETypeFlag_StructRet))
				continue;

			CValue TmpValue;
			m_pModule->m_LlvmIrBuilder.CreateAlloca (pType, "tmpArg", NULL, &TmpValue);

			rtl::CString s1 = TmpValue.GetLlvmTypeString ();
			rtl::CString s2 = It->GetLlvmTypeString ();

			m_pModule->m_LlvmIrBuilder.CreateStore (*It, TmpValue);
		
			if (pType->GetSize () > sizeof (uint64_t))
			{
				*It = TmpValue;
			}
			else
			{
				pType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64)->GetDataPtrType_c ();
				m_pModule->m_LlvmIrBuilder.CreateBitCast (TmpValue, pType, &TmpValue);
				m_pModule->m_LlvmIrBuilder.CreateLoad (TmpValue, NULL, &TmpValue);
				*It = TmpValue;
			}		
		}
	}

	m_pModule->m_LlvmIrBuilder.CreateCall (
		CalleeValue,
		pFunctionType,
		*pArgValueList,
		pResultValue
		);

	if (pReturnType->GetFlags () & ETypeFlag_StructRet)
	{
		if (pReturnType->GetSize () <= sizeof (uint64_t))
		{
			CValue TmpValue;
			CType* pType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64)->GetDataPtrType_c ();
			m_pModule->m_LlvmIrBuilder.CreateBitCast (TmpReturnValue, pType, &TmpValue);
			m_pModule->m_LlvmIrBuilder.CreateStore (*pResultValue, TmpValue);
		}

		m_pModule->m_LlvmIrBuilder.CreateLoad (TmpReturnValue, pReturnType, pResultValue);
	}
}

void
CCdeclCallConv_msc64::Return (
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

	if (pReturnType->GetSize () > sizeof (uint64_t))
	{
		CValue ReturnPtrValue (pFunction->GetLlvmFunction ()->arg_begin());

		m_pModule->m_LlvmIrBuilder.CreateStore (Value, ReturnPtrValue);
		m_pModule->m_LlvmIrBuilder.CreateRet (ReturnPtrValue);
	}
	else
	{
		CValue TmpValue;
		m_pModule->m_LlvmIrBuilder.CreateAlloca (pReturnType, "tmpRetVal", NULL, &TmpValue);
		m_pModule->m_LlvmIrBuilder.CreateStore (Value, TmpValue);

		CType* pType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64)->GetDataPtrType_c ();
		m_pModule->m_LlvmIrBuilder.CreateBitCast (TmpValue, pType, &TmpValue);
		m_pModule->m_LlvmIrBuilder.CreateLoad (TmpValue, NULL, &TmpValue);
		m_pModule->m_LlvmIrBuilder.CreateRet (TmpValue);
	}
}

CValue
CCdeclCallConv_msc64::GetThisArgValue (CFunction* pFunction)
{
	ASSERT (pFunction->IsMember ());

	CType* pReturnType = pFunction->GetType ()->GetReturnType ();
	if (!(pReturnType->GetFlags () & ETypeFlag_StructRet) ||
		pReturnType->GetSize () <= sizeof (uint64_t))
		return CCallConv::GetThisArgValue (pFunction);

	llvm::Function::arg_iterator LlvmArg = pFunction->GetLlvmFunction ()->arg_begin();
	LlvmArg++;
	return CValue (LlvmArg, pFunction->GetThisArgType ());
}

CValue
CCdeclCallConv_msc64::GetArgValue (
	CFunctionArg* pArg,
	llvm::Value* pLlvmValue
	)
{
	CType* pType = pArg->GetType ();

	if (!(pType->GetFlags () & ETypeFlag_StructRet))
		return CValue (pLlvmValue, pType);

	if (pType->GetSize () > sizeof (uint64_t))
	{
		CValue Value;
		m_pModule->m_LlvmIrBuilder.CreateLoad (pLlvmValue, pType, &Value);
		return Value;
	}

	CType* pInt64Type = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64);

	CValue Value;
	m_pModule->m_LlvmIrBuilder.CreateAlloca (pInt64Type, "tmpArg", NULL, &Value);
	m_pModule->m_LlvmIrBuilder.CreateStore (pLlvmValue, Value);
	m_pModule->m_LlvmIrBuilder.CreateBitCast (Value, pType->GetDataPtrType_c (), &Value);
	m_pModule->m_LlvmIrBuilder.CreateLoad (Value, pType, &Value);

	return Value;
}

void
CCdeclCallConv_msc64::CreateArgVariables (CFunction* pFunction)
{
	CType* pReturnType = pFunction->GetType ()->GetReturnType ();

	llvm::Function::arg_iterator LlvmArg = pFunction->GetLlvmFunction ()->arg_begin ();
	if ((pReturnType->GetFlags () & ETypeFlag_StructRet) && 
		pReturnType->GetSize () > sizeof (uint64_t))
		LlvmArg++;

	size_t i = 0;
	if (pFunction->IsMember ()) // skip this
	{
		i++;
		LlvmArg++;
	}

	rtl::CArrayT <CFunctionArg*> ArgArray = pFunction->GetType ()->GetArgArray ();
	size_t ArgCount = ArgArray.GetCount ();
	for (; i < ArgCount; i++, LlvmArg++)
	{
		CFunctionArg* pArg = ArgArray [i];
		if (!pArg->IsNamed ())
			continue;

		CType* pType = pArg->GetType ();
		llvm::Value* pLlvmArgValue = LlvmArg;

		if (!(pType->GetFlags () & ETypeFlag_StructRet))
		{
			CVariable* pArgVariable = m_pModule->m_VariableMgr.CreateArgVariable (pArg, pLlvmArgValue);
			pFunction->GetScope ()->AddItem (pArgVariable);

			m_pModule->m_LlvmIrBuilder.CreateStore (pLlvmArgValue, pArgVariable);
		}
		else
		{
			CVariable* pArgVariable = m_pModule->m_VariableMgr.CreateStackVariable (pArg->GetName (), pType);
			pFunction->GetScope ()->AddItem (pArgVariable);

			m_pModule->m_VariableMgr.AllocatePrimeInitializeVariable (pArgVariable);

			if (pType->GetSize () > sizeof (uint64_t))
			{
				CValue TmpValue;
				m_pModule->m_LlvmIrBuilder.CreateLoad (pLlvmArgValue, NULL, &TmpValue);
				m_pModule->m_LlvmIrBuilder.CreateStore (TmpValue, pArgVariable);
			}
			else
			{
				CType* pInt64Type = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64);

				CValue TmpValue;
				m_pModule->m_LlvmIrBuilder.CreateAlloca (pInt64Type, "tmpArg", NULL, &TmpValue);
				m_pModule->m_LlvmIrBuilder.CreateStore (pLlvmArgValue, TmpValue);

				m_pModule->m_LlvmIrBuilder.CreateBitCast (TmpValue, pType->GetDataPtrType_c (), &TmpValue);
				m_pModule->m_LlvmIrBuilder.CreateLoad (TmpValue, NULL, &TmpValue);
				m_pModule->m_LlvmIrBuilder.CreateStore (TmpValue, pArgVariable);
			}		
		}
	}
}

//.............................................................................

} // namespace jnc {
