#include "pch.h"
#include "jnc_CdeclCallConv_gcc64.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CType*
CCdeclCallConv_gcc64::GetArgCoerceType (CType* pType)
{
	#pragma AXL_TODO ("implement proper coercion for structures with floating point fields")

	return pType->GetSize () > sizeof (uint64_t) ?
		m_pModule->m_TypeMgr.GetStdType (EStdType_Int64Int64) :
		m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64);
}

llvm::FunctionType*
CCdeclCallConv_gcc64::GetLlvmFunctionType (CFunctionType* pFunctionType)
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
		if (pReturnType->GetSize () > sizeof (uint64_t) * 2) // return in memory
		{
			ArgCount++;
			LlvmArgTypeArray.SetCount (ArgCount);
			LlvmArgTypeArray [0] = pReturnType->GetDataPtrType_c ()->GetLlvmType ();
			j = 1;

			pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
		}
		else // coerce
		{
			pReturnType = GetArgCoerceType (pReturnType);
		}
	}

	bool HasCoercedArgs = false;

	for (size_t i = 0; j < ArgCount; i++, j++)
	{
		CType* pType = ArgArray [i]->GetType ();
		if (!(pType->GetFlags () & ETypeFlag_StructRet))
		{
			LlvmArgTypeArray [j] = pType->GetLlvmType ();
		}
		else if (pType->GetSize () > sizeof (uint64_t) * 2) // pass in memory
		{
			LlvmArgTypeArray [j] = pType->GetDataPtrType_c ()->GetLlvmType ();
			HasCoercedArgs = true;
		}
		else // coerce
		{
			LlvmArgTypeArray [j] = GetArgCoerceType (pType)->GetLlvmType ();
			HasCoercedArgs = true;
		}
	}

	if (HasCoercedArgs)
		pFunctionType->m_Flags |= EFunctionTypeFlag_CoercedArgs;

	return llvm::FunctionType::get (
		pReturnType->GetLlvmType (),
		llvm::ArrayRef <llvm::Type*> (LlvmArgTypeArray, ArgCount),
		(pFunctionType->GetFlags () & EFunctionTypeFlag_VarArg) != 0
		);
}

llvm::Function*
CCdeclCallConv_gcc64::CreateLlvmFunction (
	CFunctionType* pFunctionType,
	const char* pTag
	)
{
	llvm::Function* pLlvmFunction = CCallConv::CreateLlvmFunction (pFunctionType, pTag);

	size_t j = 1;

	CType* pReturnType = pFunctionType->GetReturnType ();
	if ((pReturnType->GetFlags () & ETypeFlag_StructRet) &&
		pReturnType->GetSize () > sizeof (uint64_t) * 2) // return in memory
	{
		pLlvmFunction->addAttribute (1, llvm::Attribute::StructRet);
		j = 2;
	}

	if (pFunctionType->GetFlags () & EFunctionTypeFlag_CoercedArgs)
	{
		rtl::CArrayT <CFunctionArg*> ArgArray = pFunctionType->GetArgArray ();
		size_t ArgCount = ArgArray.GetCount ();

		for (size_t i = 0; i < ArgCount; i++, j++)
		{
			CType* pType = ArgArray [i]->GetType ();
			if ((pType->GetFlags () & ETypeFlag_StructRet) &&
				pType->GetSize () > sizeof (uint64_t) * 2) // pass in memory
			{
				pLlvmFunction->addAttribute (j, llvm::Attribute::ByVal);
			}
		}
	}

	return pLlvmFunction;
}

void
CCdeclCallConv_gcc64::Call (
	const CValue& CalleeValue,
	CFunctionType* pFunctionType,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	CType* pReturnType = pFunctionType->GetReturnType ();

	if (!(pFunctionType->GetFlags () & EFunctionTypeFlag_CoercedArgs) &&
		!(pReturnType->GetFlags () & ETypeFlag_StructRet))
	{
		CCallConv::Call (CalleeValue, pFunctionType, pArgValueList, pResultValue);
		return;
	}


	CValue TmpReturnValue;

	if ((pReturnType->GetFlags () & ETypeFlag_StructRet) &&
		pReturnType->GetSize () > sizeof (uint64_t) * 2) // return in memory
	{
		m_pModule->m_LlvmIrBuilder.CreateAlloca (
			pReturnType,
			"tmpRetVal",
			pReturnType->GetDataPtrType_c (),
			&TmpReturnValue
			);

		pArgValueList->InsertHead (TmpReturnValue);
	}

	unsigned j = 1;
	char Buffer [256];
	rtl::CArrayT <unsigned> ByValArgIdxArray (ref::EBuf_Stack, Buffer, sizeof (Buffer));

	if (pFunctionType->GetFlags () & EFunctionTypeFlag_CoercedArgs)
	{
		rtl::CBoxIteratorT <CValue> It = pArgValueList->GetHead ();
		for (; It; It++, j++)
		{
			CType* pType = It->GetType ();
			if (!(pType->GetFlags () & ETypeFlag_StructRet))
				continue;

			if (pType->GetSize () > sizeof (uint64_t) * 2) // pass in memory
			{
				CValue TmpValue;
				m_pModule->m_LlvmIrBuilder.CreateAlloca (pType, "tmpArg", NULL, &TmpValue);
				m_pModule->m_LlvmIrBuilder.CreateStore (*It, TmpValue);

				*It = TmpValue;
				ByValArgIdxArray.Append (j);
			}
			else // coerce
			{
				CType* pCoerceType = GetArgCoerceType (pType);
				m_pModule->m_OperatorMgr.ForceCast (It.GetObject (), pCoerceType);
			}
		}
	}

	llvm::CallInst* pLlvmInst = m_pModule->m_LlvmIrBuilder.CreateCall (
		CalleeValue,
		this,
		*pArgValueList,
		TmpReturnValue ?
			m_pModule->GetSimpleType (EType_Void) :
			pReturnType,
		pResultValue
		);

	size_t ByValArgCount = ByValArgIdxArray.GetCount ();
	for (size_t i = 0; i < ByValArgCount; i++)
		pLlvmInst->addAttribute (ByValArgIdxArray [i], llvm::Attribute::ByVal);

	if (pReturnType->GetFlags () & ETypeFlag_StructRet)
	{
		if (pReturnType->GetSize () > sizeof (uint64_t) * 2) // return in memory
		{
			pLlvmInst->addAttribute (1, llvm::Attribute::StructRet);
			m_pModule->m_LlvmIrBuilder.CreateLoad (TmpReturnValue, pReturnType, pResultValue);
		}
		else // coerce
		{
			CType* pCoerceType = GetArgCoerceType (pReturnType);
			pResultValue->OverrideType (pCoerceType);
			m_pModule->m_OperatorMgr.ForceCast (pResultValue, pReturnType);
		}
	}
}

void
CCdeclCallConv_gcc64::Return (
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

	if (pReturnType->GetSize () > sizeof (uint64_t) * 2) // return in memory
	{
		CValue ReturnPtrValue (pFunction->GetLlvmFunction ()->arg_begin());

		m_pModule->m_LlvmIrBuilder.CreateStore (Value, ReturnPtrValue);
		m_pModule->m_LlvmIrBuilder.CreateRet ();
	}
	else // coerce
	{
		CType* pCoerceType = GetArgCoerceType (pReturnType);

		CValue TmpValue;
		m_pModule->m_OperatorMgr.ForceCast (Value, pCoerceType, &TmpValue);
		m_pModule->m_LlvmIrBuilder.CreateRet (TmpValue);
	}
}

CValue
CCdeclCallConv_gcc64::GetThisArgValue (CFunction* pFunction)
{
	ASSERT (pFunction->IsMember ());

	CType* pReturnType = pFunction->GetType ()->GetReturnType ();
	if (!(pReturnType->GetFlags () & ETypeFlag_StructRet) ||
		pReturnType->GetSize () <= sizeof (uint64_t) * 2)
		return CCallConv::GetThisArgValue (pFunction);

	llvm::Function::arg_iterator LlvmArg = pFunction->GetLlvmFunction ()->arg_begin();
	LlvmArg++;
	return CValue (LlvmArg, pFunction->GetThisArgType ());
}

CValue
CCdeclCallConv_gcc64::GetArgValue (
	CFunctionArg* pArg,
	llvm::Value* pLlvmValue
	)
{
	CType* pType = pArg->GetType ();

	if (!(pType->GetFlags () & ETypeFlag_StructRet))
		return CValue (pLlvmValue, pType);

	CValue Value;

	if (pType->GetSize () > sizeof (uint64_t) * 2) // passed in memory
	{
		m_pModule->m_LlvmIrBuilder.CreateLoad (pLlvmValue, pType, &Value);
	}
	else // coerce
	{
		CType* pCoerceType = GetArgCoerceType (pType);
		m_pModule->m_OperatorMgr.ForceCast (CValue (pLlvmValue, pCoerceType), pType, &Value);
	}

	return Value;
}

void
CCdeclCallConv_gcc64::CreateArgVariables (CFunction* pFunction)
{
	CType* pReturnType = pFunction->GetType ()->GetReturnType ();

	llvm::Function::arg_iterator LlvmArg = pFunction->GetLlvmFunction ()->arg_begin ();
	if ((pReturnType->GetFlags () & ETypeFlag_StructRet) &&
		pReturnType->GetSize () > sizeof (uint64_t) * 2)
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

		CVariable* pArgVariable = m_pModule->m_VariableMgr.CreateStackVariable (pArg->GetName (), pArg->GetType ());
		pFunction->GetScope ()->AddItem (pArgVariable);
		m_pModule->m_VariableMgr.AllocatePrimeInitializeVariable (pArgVariable);

		CValue ArgValue = CCdeclCallConv_gcc64::GetArgValue (pArg, LlvmArg);
		m_pModule->m_LlvmIrBuilder.CreateStore (ArgValue, pArgVariable);
	}
}

//.............................................................................

} // namespace jnc {
