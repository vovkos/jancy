#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

void
COperatorMgr::CallTraceFunction (
	const char* pFunctionName,
	const char* pString
	)
{
	CModuleItem* pItem = m_pModule->m_NamespaceMgr.GetGlobalNamespace ()->FindItem (pFunctionName);
	if (pItem && pItem->GetItemKind () == EModuleItem_Function)
	{
		CValue LiteralValue;
		LiteralValue.SetCharArray (pString);
		m_pModule->m_OperatorMgr.CallOperator ((CFunction*) pItem, LiteralValue);
	}
}

CType*
COperatorMgr::GetFunctionType (
	const CValue& OpValue,
	CFunctionType* pFunctionType
	)
{
	CFunctionPtrType* pFunctionPtrType = pFunctionType->GetFunctionPtrType (
		EType_FunctionRef,
		EFunctionPtrType_Thin
		);

	CClosure* pClosure = OpValue.GetClosure ();
	if (!pClosure)
		return pFunctionPtrType;

	return GetClosureOperatorResultType (pFunctionPtrType, pClosure->GetArgValueList ());
}

CType*
COperatorMgr::GetClosureOperatorResultType (
	const CValue& RawOpValue,
	rtl::CBoxListT <CValue>* pArgValueList
	)
{
	CValue OpValue;
	bool Result = PrepareOperand (RawOpValue, &OpValue);
	if (!Result)
		return NULL;

	EType TypeKind = OpValue.GetType ()->GetTypeKind ();
	if (TypeKind != EType_FunctionRef && TypeKind != EType_FunctionPtr)
	{
		err::SetFormatStringError (
			"closure operator cannot be applied to '%s'",
			OpValue.GetType ()->GetTypeString ().cc () // thanks a lot gcc
			);
		return NULL;
	}

	ref::CPtrT <CClosure> Closure = AXL_REF_NEW (CClosure);
	Closure->Append (*pArgValueList);
	return Closure->GetClosureType (OpValue.GetType ());
}

bool
COperatorMgr::GetClosureOperatorResultType (
	const CValue& RawOpValue,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	CType* pResultType = GetClosureOperatorResultType (RawOpValue, pArgValueList);
	if (!pResultType)
		return false;

	pResultValue->SetType (pResultType);
	return true;
}

bool
COperatorMgr::ClosureOperator (
	const CValue& RawOpValue,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	CValue OpValue;
	bool Result = PrepareOperand (RawOpValue, &OpValue);
	if (!Result)
		return false;

	EType TypeKind = OpValue.GetType ()->GetTypeKind ();
	if (TypeKind != EType_FunctionRef && TypeKind != EType_FunctionPtr)
	{
		err::SetFormatStringError (
			"closure operator cannot be applied to '%s'",
			OpValue.GetType ()->GetTypeString ().cc ()
			);
		return false;
	}

	*pResultValue = OpValue;

	CClosure* pClosure = pResultValue->GetClosure ();
	if (!pClosure)
		pClosure = pResultValue->CreateClosure ();

	pClosure->Append (*pArgValueList);
	return true;
}

CType*
COperatorMgr::GetUnsafeVarArgType (CType* pType)
{
	for (;;)
	{
		CType* pPrevType = pType;

		EType TypeKind = pType->GetTypeKind ();
		switch (TypeKind)
		{
		case EType_PropertyRef:
			pType = ((CPropertyPtrType*) pType)->GetTargetType ()->GetReturnType ();
			break;

		case EType_DataRef:
			pType = ((CDataPtrType*) pType)->GetTargetType ();
			break;

		case EType_BitField:
			pType = ((CBitFieldType*) pType)->GetBaseType ();
			break;

		case EType_Enum:
			pType = ((CEnumType*) pType)->GetBaseType ();
			break;

		case EType_Float:
			pType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Double);
			break;

		case EType_Array:
			pType = ((CArrayType*) pType)->GetElementType ()->GetDataPtrType_c (EType_DataPtr, EPtrTypeFlag_Const);
			break;

		case EType_DataPtr:
			pType = ((CDataPtrType*) pType)->GetTargetType ()->GetDataPtrType_c (EType_DataPtr, EPtrTypeFlag_Const);
			break;

		default:
			if (pType->GetTypeKindFlags () & ETypeKindFlag_Integer)
			{
				pType = pType->GetSize () > 4 ?
					m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64) :
					m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int32);
			}
		}

		if (pType == pPrevType)
			return pType;
	}
}

CType*
COperatorMgr::GetCallOperatorResultType (
	const CValue& RawOpValue,
	rtl::CBoxListT <CValue>* pArgValueList
	)
{
	bool Result;

	CValue OpValue;
	PrepareOperandType (RawOpValue, &OpValue);

	if (OpValue.GetType ()->GetTypeKind () == EType_ClassPtr)
	{
		CFunction* pCallOperator = ((CClassPtrType*) OpValue.GetType ())->GetTargetType ()->GetCallOperator ();
		if (!pCallOperator)
		{
			err::SetFormatStringError ("cannot call '%s'", OpValue.GetType ()->GetTypeString ().cc ());
			return NULL;
		}

		CValue ObjValue = OpValue;

		OpValue.SetFunctionTypeOverload (pCallOperator->GetTypeOverload ());
		OpValue.InsertToClosureTail (ObjValue);
	}

	ref::CPtrT <CClosure> Closure = OpValue.GetClosure ();
	if (Closure)
	{
		Result = Closure->Apply (pArgValueList);
		if (!Result)
			return NULL;
	}

	if (RawOpValue.GetValueKind () == EValue_FunctionTypeOverload)
	{
		size_t i = RawOpValue.GetFunctionTypeOverload ()->ChooseOverload (*pArgValueList);
		if (i == -1)
			return NULL;

		CFunctionType* pFunctionType = RawOpValue.GetFunctionTypeOverload ()->GetOverload (i);
		return pFunctionType->GetReturnType ();
	}

	CFunctionType* pFunctionType;

	CType* pOpType = OpValue.GetType ();
	EType TypeKind = pOpType->GetTypeKind ();

	switch (TypeKind)
	{
	case EType_Function:
		pFunctionType = (CFunctionType*) pOpType;
		break;

	case EType_FunctionRef:
	case EType_FunctionPtr:
		pFunctionType = ((CFunctionPtrType*) pOpType)->GetTargetType ();
		break;

	default:
		err::SetFormatStringError ("cannot call '%s'", pOpType->GetTypeString ().cc ());
		return NULL;
	}

	return pFunctionType->GetReturnType ();
}

bool
COperatorMgr::GetCallOperatorResultType (
	const CValue& RawOpValue,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	CType* pResultType = GetCallOperatorResultType (RawOpValue, pArgValueList);
	if (!pResultType)
		return false;

	pResultValue->SetType (pResultType);
	return true;
}

bool
COperatorMgr::CallOperator (
	const CValue& RawOpValue,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	bool Result;

	CValue OpValue;
	CValue UnusedReturnValue;
	rtl::CBoxListT <CValue> EmptyArgValueList;

	if (!pResultValue)
		pResultValue = &UnusedReturnValue;

	if (!pArgValueList)
		pArgValueList = &EmptyArgValueList;

	Result = PrepareOperand (RawOpValue, &OpValue, 0);
	if (!Result)
		return false;

	if (OpValue.GetType ()->GetTypeKind () == EType_ClassPtr)
	{
		CClassPtrType* pPtrType = (CClassPtrType*) OpValue.GetType ();

		CFunction* pCallOperator = pPtrType->GetTargetType ()->GetCallOperator ();
		if (!pCallOperator)
		{
			err::SetFormatStringError ("cannot call '%s'", pPtrType->GetTypeString ().cc ());
			return false;
		}

		if ((pCallOperator->GetFlags () & EMulticastMethodFlag_InaccessibleViaEventPtr) && pPtrType->IsEventPtrType ())
		{
			err::SetFormatStringError ("'Call' is inaccessible via 'event' pointer");
			return false;
		}

		CValue ObjValue = OpValue;

		OpValue.SetFunction (pCallOperator);
		OpValue.InsertToClosureTail (ObjValue);
	}

	ref::CPtrT <CClosure> Closure = OpValue.GetClosure ();
	if (Closure)
	{
		Result = Closure->Apply (pArgValueList);
		if (!Result)
			return false;
	}

	if (OpValue.GetValueKind () == EValue_Function && OpValue.GetFunction ()->IsOverloaded ())
	{
		CFunction* pFunction = OpValue.GetFunction ()->ChooseOverload (*pArgValueList);
		if (!pFunction)
			return false;

		OpValue.SetFunction (pFunction);
		OpValue.SetClosure (Closure);
	}

	if (OpValue.GetValueKind () == EValue_Function)
	{
		CFunction* pFunction = OpValue.GetFunction ();

		if (pFunction->IsVirtual ())
		{
			Result = GetVirtualMethod (pFunction, Closure, &OpValue);
			if (!Result)
				return false;
		}

		return CallImpl (OpValue, pFunction->GetType (), pArgValueList, pResultValue);
	}

	CType* pOpType = OpValue.GetType ();
	if (!(pOpType->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr) ||
		((CFunctionPtrType*) pOpType)->GetPtrTypeKind () == EFunctionPtrType_Weak)
	{
		err::SetFormatStringError ("cannot call '%s'", pOpType->GetTypeString ().cc ());
		return false;
	}

	CFunctionPtrType* pFunctionPtrType = ((CFunctionPtrType*) pOpType);
	return pFunctionPtrType->HasClosure () ?
		CallClosureFunctionPtr (OpValue, pArgValueList, pResultValue) :
		CallImpl (OpValue, pFunctionPtrType->GetTargetType (), pArgValueList, pResultValue);
}

bool
COperatorMgr::CastArgValueList (
	CFunctionType* pFunctionType,
	rtl::CBoxListT <CValue>* pArgValueList
	)
{
	bool Result;

	rtl::CArrayT <CFunctionArg*> ArgArray = pFunctionType->GetArgArray ();

	size_t FormalArgCount = ArgArray.GetCount ();
	size_t ActualArgCount = pArgValueList->GetCount ();

	bool IsVarArg = (pFunctionType->GetFlags () & EFunctionTypeFlag_VarArg) != 0;
	bool IsUnsafeVarArg = (pFunctionType->GetCallConv ()->GetFlags () & ECallConvFlag_UnsafeVarArg) != 0;

	size_t CommonArgCount;

	if (ActualArgCount <= FormalArgCount)
	{
		CommonArgCount = ActualArgCount;
	}
	else if (IsVarArg)
	{
		CommonArgCount = FormalArgCount;
	}
	else
	{
		err::SetFormatStringError ("too many arguments in a call to '%s'", pFunctionType->GetTypeString ().cc ());
		return false;
	}

	size_t i = 0;
	rtl::CBoxIteratorT <CValue> ArgValueIt = pArgValueList->GetHead ();

	// common for both formal and actual

	for (; i < CommonArgCount; ArgValueIt++, i++)
	{
		CValue ArgValue = *ArgValueIt;

		CFunctionArg* pArg = ArgArray [i];
		if (ArgValue.IsEmpty ())
		{
			rtl::CConstBoxListT <CToken> Initializer = pArg->GetInitializer ();
			if (Initializer.IsEmpty ())
			{
				err::SetFormatStringError (
					"argument (%d) of '%s' has no default value",
					i + 1,
					pFunctionType->GetTypeString ().cc ()
					);
				return false;
			}

			Result = EvaluateAlias (pArg->GetItemDecl ()->GetParentUnit (), Initializer, &ArgValue);
			if (!Result)
				return false;
		}

		CType* pFormalArgType = pArg->GetType ();

		Result =
			CheckCastKind (ArgValue, pFormalArgType) &&
			CastOperator (ArgValue, pFormalArgType, &*ArgValueIt); // store it in the same list entry

		if (!Result)
			return false;
	}

	// default formal arguments

	for (; i < FormalArgCount; i++)
	{
		CValue ArgValue;

		CFunctionArg* pArg = ArgArray [i];
		rtl::CConstBoxListT <CToken> Initializer = pArg->GetInitializer ();
		if (Initializer.IsEmpty ())
		{
			err::SetFormatStringError (
				"argument (%d) of '%s' has no default value",
				i + 1,
				pFunctionType->GetTypeString ().cc ()
				);
			return false;
		}

		Result = EvaluateAlias (pArg->GetItemDecl ()->GetParentUnit (), Initializer, &ArgValue);
		if (!Result)
			return false;

		CType* pFormalArgType = pArg->GetType ();

		Result =
			CheckCastKind (ArgValue, pFormalArgType) &&
			CastOperator (&ArgValue, pFormalArgType);

		if (!Result)
			return false;

		pArgValueList->InsertTail (ArgValue);
	}

	if (!IsVarArg)
		return true;

	// vararg arguments

	if (!IsUnsafeVarArg)
	{
		err::SetFormatStringError ("only 'cdecl' vararg is currently supported");
		return false;
	}

	for (; ArgValueIt; ArgValueIt++)
	{
		CValue ArgValue = *ArgValueIt;

		if (ArgValue.IsEmpty ())
		{
			err::SetFormatStringError ("vararg arguments cannot be skipped");
			return false;
		}

		CType* pFormalArgType = GetUnsafeVarArgType (ArgValue.GetType ());

		Result = CastOperator (ArgValue, pFormalArgType, &*ArgValueIt); // store it in the same list entry
		if (!Result)
			return false;
	}

	return true;
}

bool
COperatorMgr::CallClosureFunctionPtr (
	const CValue& OpValue,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	ASSERT (OpValue.GetType ()->GetTypeKindFlags () & ETypeKindFlag_FunctionPtr);

	CFunctionPtrType* pFunctionPointerType = (CFunctionPtrType*) OpValue.GetType ();
	CFunctionType* pFunctionType = pFunctionPointerType->GetTargetType ();
	CFunctionType* pAbstractMethodType = pFunctionType->GetStdObjectMemberMethodType ();

	CheckFunctionPtrNull (OpValue);

	CValue PfnValue;
	CValue IfaceValue;
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 0, pAbstractMethodType->GetFunctionPtrType (EFunctionPtrType_Thin), &PfnValue);
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (OpValue, 1, m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr), &IfaceValue);
	pArgValueList->InsertHead (IfaceValue);

	return CallImpl (PfnValue, pAbstractMethodType, pArgValueList, pResultValue);
}

bool
COperatorMgr::CallImpl (
	const CValue& PfnValue,
	CFunctionType* pFunctionType,
	rtl::CBoxListT <CValue>* pArgValueList,
	CValue* pResultValue
	)
{
	bool Result = CastArgValueList (pFunctionType, pArgValueList);
	if (!Result)
		return false;

	if (m_pModule->m_FunctionMgr.GetScopeLevel ())
	{
		CValue ScopeLevelValue = m_pModule->m_NamespaceMgr.GetCurrentScopeLevel ();
		CVariable* pVariable = m_pModule->m_VariableMgr.GetStdVariable (EStdVariable_ScopeLevel);
		m_pModule->m_LlvmIrBuilder.CreateStore (ScopeLevelValue, pVariable);
	}

	pFunctionType->GetCallConv ()->Call (
		PfnValue,
		pFunctionType,
		pArgValueList,
		pResultValue
		);

	if (pResultValue->GetType ()->GetFlags () & ETypeFlag_GcRoot)
		CreateTmpStackGcRoot (*pResultValue);

	if ((pFunctionType->GetFlags () & EFunctionTypeFlag_Throws) &&
		!m_pModule->m_ControlFlowMgr.IsThrowLocked ())
	{
		CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();
		if (!(pScope->GetFlags () & EScopeFlag_CanThrow))
		{
			err::SetFormatStringError (
				"cannot call throwing function from here ('%s' does not throw and there is no 'try' or 'catch')",
				m_pModule->m_FunctionMgr.GetCurrentFunction ()->m_Tag.cc ()
				);
			return false;
		}

		Result = m_pModule->m_ControlFlowMgr.Throw (*pResultValue, pFunctionType);
		if (!Result)
			return false;
	}

	return true;
}

void
COperatorMgr::GcPulse ()
{
	CFunction* pFunction = m_pModule->m_FunctionMgr.GetStdFunction (EStdFunc_GcPulse);
	m_pModule->m_LlvmIrBuilder.CreateCall (pFunction, pFunction->GetType (), NULL);
}

//.............................................................................

} // namespace jnc {

