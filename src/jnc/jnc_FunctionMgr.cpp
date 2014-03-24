#include "pch.h"
#include "jnc_FunctionMgr.h"
#include "jnc_GcShadowStack.h"
#include "jnc_Module.h"

// #define _JNC_NO_JIT

namespace jnc {

//.............................................................................

CFunctionMgr::CFunctionMgr ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	m_pCurrentFunction = NULL;
	memset (m_StdFunctionArray, 0, sizeof (m_StdFunctionArray));
	memset (m_LazyStdFunctionArray, 0, sizeof (m_LazyStdFunctionArray));

	mt::CallOnce (RegisterGcShadowStack, 0);
}

void
CFunctionMgr::Clear ()
{
	m_LlvmFunctionMap.Clear ();
	m_FunctionList.Clear ();
	m_PropertyList.Clear ();
	m_PropertyTemplateList.Clear ();
	m_ScheduleLauncherFunctionList.Clear ();
	m_ThunkFunctionList.Clear ();
	m_ThunkPropertyList.Clear ();
	m_DataThunkPropertyList.Clear ();
	m_ThunkFunctionMap.Clear ();
	m_ThunkPropertyMap.Clear ();
	m_LazyStdFunctionList.Clear ();
	m_ScheduleLauncherFunctionMap.Clear ();
	m_EmissionContextStack.Clear ();
	m_ThisValue.Clear ();
	m_ScopeLevelValue.Clear ();

	m_pCurrentFunction = NULL;
	memset (m_StdFunctionArray, 0, sizeof (m_StdFunctionArray));
	memset (m_LazyStdFunctionArray, 0, sizeof (m_LazyStdFunctionArray));
}

CValue
CFunctionMgr::OverrideThisValue (const CValue& Value)
{
	CValue PrevThisValue = m_ThisValue;
	m_ThisValue = Value;
	return PrevThisValue;
}

CFunction*
CFunctionMgr::CreateFunction (
	EFunction FunctionKind,
	const rtl::CString& Name,
	const rtl::CString& QualifiedName,
	const rtl::CString& Tag,
	CFunctionType* pType
	)
{
	CFunction* pFunction;
	switch (FunctionKind)
	{
	case EFunction_Thunk:
		pFunction = AXL_MEM_NEW (CThunkFunction);
		m_ThunkFunctionList.InsertTail ((CThunkFunction*) pFunction);
		break;

	case EFunction_ScheduleLauncher:
		pFunction = AXL_MEM_NEW (CScheduleLauncherFunction);
		m_ScheduleLauncherFunctionList.InsertTail ((CScheduleLauncherFunction*) pFunction);
		break;

	default:
		pFunction = AXL_MEM_NEW (CFunction);
		m_FunctionList.InsertTail (pFunction);
	}

	pFunction->m_pModule = m_pModule;
	pFunction->m_FunctionKind = FunctionKind;
	pFunction->m_Name = Name;
	pFunction->m_QualifiedName = QualifiedName;
	pFunction->m_Tag = Tag;
	pFunction->m_pType = pType;
	pFunction->m_TypeOverload.AddOverload (pType);
	return pFunction;
}

CProperty*
CFunctionMgr::CreateProperty (
	EProperty PropertyKind,
	const rtl::CString& Name,
	const rtl::CString& QualifiedName,
	const rtl::CString& Tag
	)
{
	CProperty* pProperty;

	switch (PropertyKind)
	{
	case EProperty_Thunk:
		pProperty = AXL_MEM_NEW (CThunkProperty);
		m_ThunkPropertyList.InsertTail ((CThunkProperty*) pProperty);
		break;

	case EProperty_DataThunk:
		pProperty = AXL_MEM_NEW (CDataThunkProperty);
		m_DataThunkPropertyList.InsertTail ((CDataThunkProperty*) pProperty);
		break;

	default:
		pProperty = AXL_MEM_NEW (CProperty);
		m_PropertyList.InsertTail (pProperty);
	}

	pProperty->m_pModule = m_pModule;
	pProperty->m_PropertyKind = PropertyKind;
	pProperty->m_Name = Name;
	pProperty->m_QualifiedName = QualifiedName;
	pProperty->m_Tag = Tag;
	m_pModule->MarkForLayout (pProperty, true);
	return pProperty;
}

CPropertyTemplate*
CFunctionMgr::CreatePropertyTemplate ()
{
	CPropertyTemplate* pPropertyTemplate = AXL_MEM_NEW (CPropertyTemplate);
	pPropertyTemplate->m_pModule = m_pModule;
	m_PropertyTemplateList.InsertTail (pPropertyTemplate);
	return pPropertyTemplate;
}

#pragma AXL_TODO ("get rid of emission context stack: postpone compilation of nested function")

void
CFunctionMgr::PushEmissionContext ()
{
	if (!m_pCurrentFunction)
		return;

	TEmissionContext* pContext = AXL_MEM_NEW (TEmissionContext);
	pContext->m_pCurrentFunction = m_pCurrentFunction;
	pContext->m_ThisValue = m_ThisValue;
	pContext->m_ScopeLevelValue = m_ScopeLevelValue;
	pContext->m_TmpStackGcRootList.TakeOver (&m_pModule->m_OperatorMgr.m_TmpStackGcRootList);

	pContext->m_pCurrentNamespace = m_pModule->m_NamespaceMgr.m_pCurrentNamespace;
	pContext->m_pCurrentScope = m_pModule->m_NamespaceMgr.m_pCurrentScope;
	pContext->m_ScopeLevelStack.TakeOver (&m_pModule->m_NamespaceMgr.m_ScopeLevelStack);

	pContext->m_ReturnBlockArray = m_pModule->m_ControlFlowMgr.m_ReturnBlockArray;
	pContext->m_pCurrentBlock = m_pModule->m_ControlFlowMgr.m_pCurrentBlock;
	pContext->m_pUnreachableBlock = m_pModule->m_ControlFlowMgr.m_pUnreachableBlock;
	pContext->m_ControlFlowMgrFlags = m_pModule->m_ControlFlowMgr.m_Flags;
	pContext->m_LlvmDebugLoc = m_pModule->m_LlvmIrBuilder.GetCurrentDebugLoc ();

	m_EmissionContextStack.InsertTail (pContext);

	m_pModule->m_NamespaceMgr.m_ScopeLevelStack.Clear ();
	m_pModule->m_NamespaceMgr.m_pCurrentNamespace = &m_pModule->m_NamespaceMgr.m_GlobalNamespace;
	m_pModule->m_NamespaceMgr.m_pCurrentScope = NULL;

	m_pModule->m_ControlFlowMgr.m_ReturnBlockArray.Clear ();
	m_pModule->m_ControlFlowMgr.SetCurrentBlock (NULL);
	m_pModule->m_ControlFlowMgr.m_pUnreachableBlock = NULL;
	m_pModule->m_ControlFlowMgr.m_Flags = 0;
	m_pModule->m_LlvmIrBuilder.SetCurrentDebugLoc (llvm::DebugLoc ());
	// m_pModule->m_LlvmIrBuilder.SetCurrentDebugLoc (m_pModule->m_LlvmDiBuilder.GetEmptyDebugLoc ());

	m_pModule->m_VariableMgr.DeallocateTlsVariableArray (m_pCurrentFunction->m_TlsVariableArray);

	m_pCurrentFunction = NULL;
	m_ThisValue.Clear ();
	m_ScopeLevelValue.Clear ();
}

void
CFunctionMgr::PopEmissionContext ()
{
	ASSERT (m_pCurrentFunction);
	m_pModule->m_VariableMgr.DeallocateTlsVariableArray (m_pCurrentFunction->m_TlsVariableArray);

	if (m_EmissionContextStack.IsEmpty ())
	{
		m_pCurrentFunction = NULL;
		m_ThisValue.Clear ();
		m_ScopeLevelValue.Clear ();
		m_pModule->m_NamespaceMgr.m_ScopeLevelStack.Clear ();
		m_pModule->m_OperatorMgr.m_TmpStackGcRootList.Clear ();
		return;
	}

	TEmissionContext* pContext = m_EmissionContextStack.RemoveTail ();
	m_pCurrentFunction = pContext->m_pCurrentFunction;
	m_ThisValue = pContext->m_ThisValue;
	m_ScopeLevelValue = pContext->m_ScopeLevelValue;
	m_pModule->m_OperatorMgr.m_TmpStackGcRootList.TakeOver (&pContext->m_TmpStackGcRootList);

	m_pModule->m_NamespaceMgr.m_pCurrentNamespace = pContext->m_pCurrentNamespace;
	m_pModule->m_NamespaceMgr.m_pCurrentScope = pContext->m_pCurrentScope;
	m_pModule->m_NamespaceMgr.m_ScopeLevelStack.TakeOver (&pContext->m_ScopeLevelStack);

	m_pModule->m_ControlFlowMgr.m_ReturnBlockArray = pContext->m_ReturnBlockArray;
	m_pModule->m_ControlFlowMgr.SetCurrentBlock (pContext->m_pCurrentBlock);
	m_pModule->m_ControlFlowMgr.m_pUnreachableBlock = pContext->m_pUnreachableBlock;
	m_pModule->m_ControlFlowMgr.m_Flags = pContext->m_ControlFlowMgrFlags;
	m_pModule->m_LlvmIrBuilder.SetCurrentDebugLoc (pContext->m_LlvmDebugLoc);

	AXL_MEM_DELETE (pContext);

	m_pModule->m_VariableMgr.RestoreTlsVariableArray (m_pCurrentFunction->m_TlsVariableArray);
}

bool
CFunctionMgr::FireOnChanged ()
{
	CFunction* pFunction = m_pCurrentFunction;

	ASSERT (
		pFunction->m_FunctionKind == EFunction_Setter &&
		pFunction->m_pProperty &&
		pFunction->m_pProperty->GetType ()->GetFlags () & EPropertyTypeFlag_Bindable
		);

	CValue PropertyValue = pFunction->m_pProperty;

	if (pFunction->m_pThisType)
	{
		ASSERT (m_ThisValue);
		PropertyValue.InsertToClosureHead (m_ThisValue);
	}

	CValue OnChanged;

	return
		m_pModule->m_OperatorMgr.GetPropertyOnChanged (PropertyValue, &OnChanged) &&
		m_pModule->m_OperatorMgr.MemberOperator (&OnChanged, "call") &&
		m_pModule->m_OperatorMgr.CallOperator (OnChanged);
}

CFunction*
CFunctionMgr::SetCurrentFunction (CFunction* pFunction)
{
	CFunction* pPrevFunction = m_pCurrentFunction;
	m_pCurrentFunction = pFunction;
	return pPrevFunction;
}

bool
CFunctionMgr::Prologue (
	CFunction* pFunction,
	const CToken::CPos& Pos
	)
{
	bool Result;

	PushEmissionContext ();

	m_pCurrentFunction = pFunction;

	// create scope

	m_pModule->m_NamespaceMgr.OpenNamespace (pFunction->m_pParentNamespace);

	pFunction->m_pScope = m_pModule->m_NamespaceMgr.OpenScope (Pos);

	// create entry block (gc roots come here)

	CBasicBlock* pEntryBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("function_entry");
	CBasicBlock* pBodyBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("function_body");

	pFunction->m_pEntryBlock = pEntryBlock;
	pEntryBlock->MarkEntry ();

	m_pModule->m_ControlFlowMgr.SetCurrentBlock (pEntryBlock);

	if (pFunction->m_FunctionKind == EFunction_ModuleConstructor)
	{
		bool Result = m_pModule->m_VariableMgr.AllocatePrimeStaticVariables ();
		if (!Result)
			return false;
	}
	else // do not save / restore scope level in module constructor
	{
		CVariable* pVariable = m_pModule->m_VariableMgr.GetStdVariable (EStdVariable_ScopeLevel);
		m_pModule->m_LlvmIrBuilder.CreateLoad (pVariable, NULL, &m_ScopeLevelValue);
	}

	m_pModule->m_ControlFlowMgr.Jump (pBodyBlock, pBodyBlock);
	m_pModule->m_ControlFlowMgr.m_pUnreachableBlock = NULL;
	m_pModule->m_ControlFlowMgr.m_Flags = 0; // clear jump flag

	// save scope level

	if (pFunction->m_FunctionKind == EFunction_ModuleConstructor)
	{
		Result = m_pModule->m_VariableMgr.InitializeGlobalStaticVariables ();
		if (!Result)
			return false;
	}

	pFunction->GetType ()->GetCallConv ()->CreateArgVariables (pFunction);

	// 'this' arg

	if (pFunction->IsMember ())
		CreateThisValue ();

	return true;
}

void
CFunctionMgr::CreateThisValue ()
{
	CFunction* pFunction = m_pCurrentFunction;
	ASSERT (pFunction && pFunction->IsMember ());

	CValue ThisArgValue = pFunction->GetType ()->GetCallConv ()->GetThisArgValue (pFunction);

	if (pFunction->m_pThisArgType->Cmp (pFunction->m_pThisType) == 0)
	{
		if (pFunction->m_pThisType->GetTypeKind () != EType_DataPtr)
		{
			m_ThisValue = ThisArgValue;
		}
		else // make it lean
		{
			ASSERT (
				ThisArgValue.GetType ()->GetTypeKind () == EType_DataPtr &&
				((CDataPtrType*) ThisArgValue.GetType ())->GetPtrTypeKind () == EDataPtrType_Normal);

			CDataPtrType* pPtrType = ((CDataPtrType*) ThisArgValue.GetType ());

			CValue PtrValue;
			m_pModule->m_LlvmIrBuilder.CreateExtractValue (ThisArgValue, 0, NULL, &PtrValue);

			pPtrType = pPtrType->GetTargetType ()->GetDataPtrType (EDataPtrType_Lean, pPtrType->GetFlags ());
			m_ThisValue.SetLeanDataPtr (PtrValue.GetLlvmValue (), pPtrType, ThisArgValue);
		}
	}
	else
	{
		ASSERT (pFunction->m_StorageKind == EStorage_Override && pFunction->m_ThisArgDelta < 0);

		CValue PtrValue;
		m_pModule->m_LlvmIrBuilder.CreateBitCast (ThisArgValue, m_pModule->GetSimpleType (EStdType_BytePtr), &PtrValue);
		m_pModule->m_LlvmIrBuilder.CreateGep (PtrValue, (int32_t) pFunction->m_ThisArgDelta, NULL, &PtrValue);
		m_pModule->m_LlvmIrBuilder.CreateBitCast (PtrValue, pFunction->m_pThisType, &m_ThisValue);
	}
}

bool
CFunctionMgr::Epilogue ()
{
	bool Result;

	CFunction* pFunction = m_pCurrentFunction;
	CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();

	ASSERT (m_pCurrentFunction && pScope);

	if (pFunction->m_FunctionKind == EFunction_Destructor)
	{
		ASSERT (pFunction->GetParentType ()->GetTypeKind () == EType_Class && m_ThisValue);

		CClassType* pClassType = (CClassType*) pFunction->GetParentType ();

		Result =
			pClassType->CallMemberPropertyDestructors (m_ThisValue) &&
			pClassType->CallMemberFieldDestructors (m_ThisValue) &&
			pClassType->CallBaseTypeDestructors (m_ThisValue);

		if (!Result)
			return false;
	}

	if (pFunction->m_FunctionKind == EFunction_ModuleDestructor)
		m_pModule->m_VariableMgr.m_StaticDestructList.RunDestructors ();

	Result = m_pModule->m_ControlFlowMgr.CheckReturn ();
	if (!Result)
		return false;

	try
	{
		llvm::verifyFunction (*pFunction->GetLlvmFunction (), llvm::ReturnStatusAction);
	}
	catch (err::CError Error)
	{
		err::SetFormatStringError (
			"LLVM verification fail for '%s': %s",
			pFunction->m_Tag.cc (),
			Error->GetDescription ().cc ()
			);

		return false;
	}

	m_pModule->m_NamespaceMgr.CloseScope ();
	m_pModule->m_NamespaceMgr.CloseNamespace ();

	PopEmissionContext ();
	return true;
}

void
CFunctionMgr::InternalPrologue (
	CFunction* pFunction,
	CValue* pArgValueArray,
	size_t ArgCount
	)
{
	PushEmissionContext ();

	m_pCurrentFunction = pFunction;

	m_pModule->m_NamespaceMgr.OpenInternalScope ();
	m_pModule->m_LlvmIrBuilder.SetCurrentDebugLoc (llvm::DebugLoc ());

	CBasicBlock* pEntryBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("function_entry");
	CBasicBlock* pBodyBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("function_body");

	pFunction->m_pEntryBlock = pEntryBlock;
	pEntryBlock->MarkEntry ();

	m_pModule->m_ControlFlowMgr.SetCurrentBlock (pEntryBlock);

	if (pFunction->m_FunctionKind != EFunction_ModuleConstructor) // do not save / restore scope level in module constructor
	{
		CVariable* pVariable = m_pModule->m_VariableMgr.GetStdVariable (EStdVariable_ScopeLevel);
		m_pModule->m_LlvmIrBuilder.CreateLoad (pVariable, NULL, &m_ScopeLevelValue);
	}

	m_pModule->m_ControlFlowMgr.Jump (pBodyBlock, pBodyBlock);
	m_pModule->m_ControlFlowMgr.m_pUnreachableBlock = NULL;
	m_pModule->m_ControlFlowMgr.m_Flags = 0;

	if (pFunction->IsMember ())
		CreateThisValue ();

	if (ArgCount)
	{
		llvm::Function::arg_iterator LlvmArg = pFunction->GetLlvmFunction ()->arg_begin ();
		rtl::CArrayT <CFunctionArg*> ArgArray = pFunction->GetType ()->GetArgArray ();

		for (size_t i = 0; i < ArgCount; i++, LlvmArg++)
			pArgValueArray [i] = CValue (LlvmArg, ArgArray [i]->GetType ());
	}
}

void
CFunctionMgr::InternalEpilogue ()
{
	CFunction* pFunction = m_pCurrentFunction;

	CBasicBlock* pCurrentBlock = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	if (!pCurrentBlock->HasTerminator ())
	{
		CType* pReturnType = pFunction->GetType ()->GetReturnType ();

		CValue ReturnValue;
		if (pReturnType->GetTypeKind () != EType_Void)
			ReturnValue = pReturnType->GetZeroValue ();

		m_pModule->m_ControlFlowMgr.Return (ReturnValue);
	}

	m_pModule->m_NamespaceMgr.CloseScope ();

	PopEmissionContext ();
}

CFunction*
CFunctionMgr::GetDirectThunkFunction (
	CFunction* pTargetFunction,
	CFunctionType* pThunkFunctionType,
	bool HasUnusedClosure
	)
{
	if (!HasUnusedClosure && pTargetFunction->m_pType->Cmp (pThunkFunctionType) == 0)
		return pTargetFunction;

	char SignatureChar = 'D';

	if (HasUnusedClosure)
	{
		SignatureChar = 'U';
		pThunkFunctionType = pThunkFunctionType->GetStdObjectMemberMethodType ();
	}

	rtl::CString Signature;
	Signature.Format (
		"%c%x.%s",
		SignatureChar,
		pTargetFunction,
		pThunkFunctionType->GetSignature ().cc ()
		);

	rtl::CStringHashTableMapIteratorT <CFunction*> Thunk = m_ThunkFunctionMap.Goto (Signature);
	if (Thunk->m_Value)
		return Thunk->m_Value;

	CThunkFunction* pThunkFunction = (CThunkFunction*) CreateFunction (EFunction_Thunk, pThunkFunctionType);
	pThunkFunction->m_StorageKind = EStorage_Static;
	pThunkFunction->m_Tag = "directThunkFunction";
	pThunkFunction->m_Signature = Signature;
	pThunkFunction->m_pTargetFunction = pTargetFunction;

	Thunk->m_Value = pThunkFunction;

	m_pModule->MarkForCompile (pThunkFunction);
	return pThunkFunction;
}

CProperty*
CFunctionMgr::GetDirectThunkProperty (
	CProperty* pTargetProperty,
	CPropertyType* pThunkPropertyType,
	bool HasUnusedClosure
	)
{
	if (!HasUnusedClosure && pTargetProperty->m_pType->Cmp (pThunkPropertyType) == 0)
		return pTargetProperty;

	rtl::CString Signature;
	Signature.Format (
		"%c%x.%s",
		HasUnusedClosure ? 'U' : 'D',
		pTargetProperty,
		pThunkPropertyType->GetSignature ().cc ()
		);

	rtl::CStringHashTableMapIteratorT <CProperty*> Thunk = m_ThunkPropertyMap.Goto (Signature);
	if (Thunk->m_Value)
		return Thunk->m_Value;

	CThunkProperty* pThunkProperty = (CThunkProperty*) CreateProperty (EProperty_Thunk);
	pThunkProperty->m_StorageKind = EStorage_Static;
	pThunkProperty->m_Signature = Signature;
	pThunkProperty->m_Tag = "g_directThunkProperty";

	bool Result = pThunkProperty->Create (pTargetProperty, pThunkPropertyType, HasUnusedClosure);
	if (!Result)
		return NULL;

	Thunk->m_Value = pThunkProperty;

	pThunkProperty->EnsureLayout ();
	return pThunkProperty;
}

CProperty*
CFunctionMgr::GetDirectDataThunkProperty (
	CVariable* pTargetVariable,
	CPropertyType* pThunkPropertyType,
	bool HasUnusedClosure
	)
{
	bool Result;

	rtl::CString Signature;
	Signature.Format (
		"%c%x.%s",
		HasUnusedClosure ? 'U' : 'D',
		pTargetVariable,
		pThunkPropertyType->GetSignature ().cc ()
		);

	rtl::CStringHashTableMapIteratorT <CProperty*> Thunk = m_ThunkPropertyMap.Goto (Signature);
	if (Thunk->m_Value)
		return Thunk->m_Value;

	CDataThunkProperty* pThunkProperty = (CDataThunkProperty*) CreateProperty (EProperty_DataThunk);
	pThunkProperty->m_StorageKind = EStorage_Static;
	pThunkProperty->m_Signature = Signature;
	pThunkProperty->m_pTargetVariable = pTargetVariable;
	pThunkProperty->m_Tag = "g_directDataThunkProperty";

	if (HasUnusedClosure)
		pThunkPropertyType = pThunkPropertyType->GetStdObjectMemberPropertyType ();

	Result = pThunkProperty->Create (pThunkPropertyType);
	if (!Result)
		return NULL;

	Thunk->m_Value = pThunkProperty;

	pThunkProperty->EnsureLayout ();
	m_pModule->MarkForCompile (pThunkProperty);
	return pThunkProperty;
}

CFunction*
CFunctionMgr::GetScheduleLauncherFunction (
	CFunctionPtrType* pTargetFunctionPtrType,
	EClassPtrType SchedulerPtrTypeKind
	)
{
	rtl::CString Signature = pTargetFunctionPtrType->GetSignature ();
	if (SchedulerPtrTypeKind == EClassPtrType_Weak)
		Signature += ".w";

	rtl::CStringHashTableMapIteratorT <CFunction*> Thunk = m_ScheduleLauncherFunctionMap.Goto (Signature);
	if (Thunk->m_Value)
		return Thunk->m_Value;

	CClassPtrType* pSchedulerPtrType = ((CClassType*) m_pModule->GetSimpleType (EStdType_Scheduler))->GetClassPtrType (SchedulerPtrTypeKind);

	rtl::CArrayT <CFunctionArg*> ArgArray  = pTargetFunctionPtrType->GetTargetType ()->GetArgArray ();
	ArgArray.Insert (0, pTargetFunctionPtrType->GetSimpleFunctionArg ());
	ArgArray.Insert (1, pSchedulerPtrType->GetSimpleFunctionArg ());

	CFunctionType* pLauncherType = m_pModule->m_TypeMgr.GetFunctionType (ArgArray);
	CScheduleLauncherFunction* pLauncherFunction = (CScheduleLauncherFunction*) CreateFunction (EFunction_ScheduleLauncher, pLauncherType);
	pLauncherFunction->m_StorageKind = EStorage_Static;
	pLauncherFunction->m_Tag = "scheduleLauncherFunction";
	pLauncherFunction->m_Signature = Signature;

	Thunk->m_Value = pLauncherFunction;

	m_pModule->MarkForCompile (pLauncherFunction);
	return pLauncherFunction;
}

bool
CFunctionMgr::InjectTlsPrologues ()
{
	rtl::CIteratorT <CFunction> Function = m_FunctionList.GetHead ();
	for (; Function; Function++)
	{
		CFunction* pFunction = *Function;
		if (pFunction->GetEntryBlock () && !pFunction->GetTlsVariableArray ().IsEmpty ())
			InjectTlsPrologue (pFunction);
	}

	rtl::CIteratorT <CThunkFunction> ThunkFunction = m_ThunkFunctionList.GetHead ();
	for (; ThunkFunction; ThunkFunction++)
	{
		CFunction* pFunction = *ThunkFunction;
		if (!pFunction->GetTlsVariableArray ().IsEmpty ())
			InjectTlsPrologue (pFunction);
	}

	rtl::CIteratorT <CScheduleLauncherFunction> ScheduleLauncherFunction = m_ScheduleLauncherFunctionList.GetHead ();
	for (; ScheduleLauncherFunction; ScheduleLauncherFunction++)
	{
		CFunction* pFunction = *ScheduleLauncherFunction;
		if (!pFunction->GetTlsVariableArray ().IsEmpty ())
			InjectTlsPrologue (pFunction);
	}

	return true;
}

void
CFunctionMgr::InjectTlsPrologue (CFunction* pFunction)
{
	CBasicBlock* pBlock = pFunction->GetEntryBlock ();
	ASSERT (pBlock);

	rtl::CArrayT <TTlsVariable> TlsVariableArray = pFunction->GetTlsVariableArray ();
	ASSERT (!TlsVariableArray.IsEmpty ());

	m_pModule->m_ControlFlowMgr.SetCurrentBlock (pBlock);

	llvm::BasicBlock::iterator LlvmAnchor = pBlock->GetLlvmBlock ()->begin ();

	if (llvm::isa <llvm::CallInst> (LlvmAnchor)) // skip gc-enter
		LlvmAnchor++;

	m_pModule->m_LlvmIrBuilder.SetInsertPoint (LlvmAnchor);

	CFunction* pGetTls = GetStdFunction (EStdFunc_GetTls);
	CValue TlsValue;
	LlvmAnchor = m_pModule->m_LlvmIrBuilder.CreateCall (pGetTls, pGetTls->GetType (), &TlsValue);

	// tls variables used in this function

	size_t Count = TlsVariableArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CStructField* pField = TlsVariableArray [i].m_pVariable->GetTlsField ();
		ASSERT (pField);

		CValue PtrValue;
		m_pModule->m_LlvmIrBuilder.CreateGep2 (TlsValue, pField->GetLlvmIndex (), NULL, &PtrValue);
		TlsVariableArray [i].m_pLlvmAlloca->replaceAllUsesWith (PtrValue.GetLlvmValue ());
	}

	// unfortunately, erasing could not be safely done inside the above loop (cause of InsertPoint)
	// so just have a dedicated loop for erasing alloca's

	Count = TlsVariableArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
		TlsVariableArray [i].m_pLlvmAlloca->eraseFromParent ();

	// skip all the gep's to get past tls prologue

	LlvmAnchor++;
	while (llvm::isa <llvm::GetElementPtrInst> (LlvmAnchor))
		LlvmAnchor++;

	pFunction->m_pLlvmPostTlsPrologueInst = LlvmAnchor;
}

class CJitEventListener: public llvm::JITEventListener
{
protected:
	CFunctionMgr* m_pFunctionMgr;

public:
	CJitEventListener (CFunctionMgr* pFunctionMgr)
	{
		m_pFunctionMgr = pFunctionMgr;
	}

	virtual
	void
	NotifyObjectEmitted (const llvm::ObjectImage& LLvmObjectImage)
	{
		// printf ("NotifyObjectEmitted\n");
	}

	virtual
	void
	NotifyFunctionEmitted (
		const llvm::Function& LlvmFunction,
		void* p,
		size_t Size,
		const EmittedFunctionDetails& Details
		)
	{
		CFunction* pFunction = m_pFunctionMgr->FindFunctionByLlvmFunction ((llvm::Function*) &LlvmFunction);
		if (pFunction)
		{
			pFunction->m_pfMachineCode = p;
			pFunction->m_MachineCodeSize = Size;
		}
	}
};

void
LlvmFatalErrorHandler (
	void* pContext,
	const std::string& ErrorString,
	bool ShouldGenerateCrashDump
	)
{
	throw err::CreateStringError (ErrorString.c_str ());
}

bool
CFunctionMgr::JitFunctions (llvm::ExecutionEngine* pExecutionEngine)
{
#ifdef _JNC_NO_JIT
	err::SetFormatStringError ("LLVM jitting is disabled");
	return false;
#endif

	CScopeThreadModule ScopeModule (m_pModule);
	llvm::ScopedFatalErrorHandler ScopeErrorHandler (LlvmFatalErrorHandler);

	CJitEventListener JitEventListener (this);
	pExecutionEngine->RegisterJITEventListener (&JitEventListener);

	try
	{
		rtl::CIteratorT <CFunction> Function = m_FunctionList.GetHead ();
		for (; Function; Function++)
		{
			CFunction* pFunction = *Function;

			if (!pFunction->GetEntryBlock ())
				continue;

				void* pf = pExecutionEngine->getPointerToFunction (pFunction->GetLlvmFunction ());
				pFunction->m_pfMachineCode = pf;

				// ASSERT (pFunction->m_pfMachineCode == pf && pFunction->m_MachineCodeSize != 0);
		}

		// for MC jitter this should do all the job
		pExecutionEngine->finalizeObject ();
	}
	catch (err::CError Error)
	{
		err::SetFormatStringError ("LLVM jitting failed: %s", Error->GetDescription ().cc ());
		pExecutionEngine->UnregisterJITEventListener (&JitEventListener);
		return false;
	}

	pExecutionEngine->UnregisterJITEventListener (&JitEventListener);
	return true;
}

CFunction*
CFunctionMgr::GetStdFunction (EStdFunc Func)
{
	ASSERT ((size_t) Func < EStdFunc__Count);

	if (m_StdFunctionArray [Func])
		return m_StdFunctionArray [Func];

	CType* ArgTypeArray [8] = { 0 }; // 8 is enough for all the std functions

	CType* pReturnType;
	CFunctionType* pFunctionType;
	CFunction* pFunction;

	switch (Func)
	{
	case EStdFunc_RuntimeError:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int);
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 2);
		pFunction = CreateFunction (EFunction_Internal, "jnc.runtimeError", pFunctionType);
		break;

	case EStdFunc_CheckNullPtr:
		pFunction = CreateCheckNullPtr ();
		break;

	case EStdFunc_CheckScopeLevel:
		pFunction = CreateCheckScopeLevel ();
		break;

	case EStdFunc_CheckClassPtrScopeLevel:
		pFunction = CreateCheckClassPtrScopeLevel ();
		break;

	case EStdFunc_CheckDataPtrRange:
		pFunction = CreateCheckDataPtrRange ();
		break;

	case EStdFunc_DynamicCastClassPtr:
		pReturnType = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr);
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 2);
		pFunction = CreateFunction (EFunction_Internal, "jnc.dynamicCastClassPtr", pFunctionType);
		break;

	case EStdFunc_StrengthenClassPtr:
		pReturnType = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr);
		ArgTypeArray [0] = ((CClassType*) m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectClass))->GetClassPtrType (EClassPtrType_Weak);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 1);
		pFunction = CreateFunction (EFunction_Internal, "jnc.strengthenClassPtr", pFunctionType);
		break;

	case EStdFunc_GetDataPtrSpan:
		pFunction = CreateGetDataPtrSpan ();
		break;

	case EStdFunc_GcAllocate:
		pReturnType = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr);
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 2);
		pFunction = CreateFunction (EFunction_Internal, "jnc.gcAllocate", pFunctionType);
		break;

	case EStdFunc_GcTryAllocate:
		pReturnType = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr);
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 2);
		pFunction = CreateFunction (EFunction_Internal, "jnc.gcTryAllocate", pFunctionType);
		break;

	case EStdFunc_MarkGcRoot:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr)->GetDataPtrType_c ();
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 2);
		pFunction = CreateFunction (EFunction_Internal, "jnc.markGcRoot", pFunctionType);
		pFunction->m_pLlvmFunction = llvm::Intrinsic::getDeclaration (m_pModule->GetLlvmModule (), llvm::Intrinsic::gcroot);
		break;

	case EStdFunc_GetTls:
		pReturnType = m_pModule->m_VariableMgr.GetTlsStructType ()->GetDataPtrType (EDataPtrType_Thin);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, NULL, 0);
		pFunction = CreateFunction (EFunction_Internal, "jnc.getTls", pFunctionType);
		break;

	case EStdFunc_GcEnter:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, NULL, 0);
		pFunction = CreateFunction (EFunction_Internal, "jnc.gcEnter", pFunctionType);
		break;

	case EStdFunc_GcLeave:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, NULL, 0);
		pFunction = CreateFunction (EFunction_Internal, "jnc.gcLeave", pFunctionType);
		break;

	case EStdFunc_GcPulse:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, NULL, 0);
		pFunction = CreateFunction (EFunction_Internal, "jnc.gcPulse", pFunctionType);
		break;

	case EStdFunc_RunGc:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, NULL, 0);
		pFunction = CreateFunction ("runGc", "jnc.runGc", pFunctionType);
		break;

	case EStdFunc_CreateThread:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64_u);
		ArgTypeArray [0] = ((CFunctionType*) m_pModule->m_TypeMgr.GetStdType (EStdType_SimpleFunction))->GetFunctionPtrType (EFunctionPtrType_Normal, EPtrTypeFlag_Safe);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 1);
		pFunction = CreateFunction ("createThread", "jnc.createThread", pFunctionType);
		break;

	case EStdFunc_Sleep:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int32_u);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 1);
		pFunction = CreateFunction ("sleep", "jnc.sleep", pFunctionType);
		break;

	case EStdFunc_GetTimestamp:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64_u);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr);
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, NULL, 0);
		pFunction = CreateFunction ("getTimestamp", "jnc.getTimestamp", pFunctionType);
		break;

	case EStdFunc_GetCurrentThreadId:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64_u);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, NULL, 0);
		pFunction = CreateFunction ("getCurrentThreadId", "jnc.getCurrentThreadId", pFunctionType);
		break;

	case EStdFunc_GetLastError:
		pReturnType = m_pModule->m_TypeMgr.GetStdType (EStdType_Error)->GetDataPtrType (EDataPtrType_Normal, EPtrTypeFlag_Const);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, NULL, 0);
		pFunction = CreateFunction ("getLastError", "jnc.getLastError", pFunctionType);
		break;

	case EStdFunc_StrLen:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType (EDataPtrType_Normal, EPtrTypeFlag_Const);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 1);
		pFunction = CreateFunction ("strlen", "strlen", pFunctionType);
		break;

	case EStdFunc_MemCpy:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void)->GetDataPtrType (EDataPtrType_Normal);
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void)->GetDataPtrType (EDataPtrType_Normal, EPtrTypeFlag_Const);
		ArgTypeArray [2] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 3);
		pFunction = CreateFunction ("memcpy", "memcpy", pFunctionType);
		break;

	case EStdFunc_MemCat:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void)->GetDataPtrType (EDataPtrType_Normal);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void)->GetDataPtrType (EDataPtrType_Normal, EPtrTypeFlag_Const);
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		ArgTypeArray [2] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void)->GetDataPtrType (EDataPtrType_Normal, EPtrTypeFlag_Const);
		ArgTypeArray [3] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 4);
		pFunction = CreateFunction ("memcat", "memcat", pFunctionType);
		break;

	case EStdFunc_Rand:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, NULL, 0);
		pFunction = CreateFunction ("rand", "rand", pFunctionType);
		break;

	case EStdFunc_Printf:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int_p);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType (EType_DataPtr, EDataPtrType_Thin, EPtrTypeFlag_Const);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (
			m_pModule->m_TypeMgr.GetCallConv (ECallConv_Cdecl),
			pReturnType,
			ArgTypeArray, 1,
			EFunctionTypeFlag_VarArg
			);
		pFunction = CreateFunction ("printf", "printf", pFunctionType);
		break;

	case EStdFunc_Atoi:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType (EType_DataPtr, EDataPtrType_Thin, EPtrTypeFlag_Const);
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 1);
		pFunction = CreateFunction ("atoi", "atoi", pFunctionType);
		break;

	case EStdFunc_Format:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType (EType_DataPtr, EDataPtrType_Normal, EPtrTypeFlag_Const);
		ArgTypeArray [0] = pReturnType;
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (
			m_pModule->m_TypeMgr.GetCallConv (ECallConv_Cdecl),
			pReturnType,
			ArgTypeArray, 1,
			EFunctionTypeFlag_VarArg
			);
		pFunction = CreateFunction ("format", "jnc.format", pFunctionType);
		break;

	case EStdFunc_AppendFmtLiteral_a:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_FmtLiteral)->GetDataPtrType_c (),
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType_c (EType_DataPtr, EPtrTypeFlag_Const),
		ArgTypeArray [2] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT),
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 3);
		pFunction = CreateFunction (EFunction_Internal, "jnc.appendFmtLiteral_a", pFunctionType);
		break;

	case EStdFunc_AppendFmtLiteral_p:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_FmtLiteral)->GetDataPtrType_c (),
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType_c (EType_DataPtr, EPtrTypeFlag_Const),
		ArgTypeArray [2] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType (EDataPtrType_Normal, EPtrTypeFlag_Const),
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 3);
		pFunction = CreateFunction (EFunction_Internal, "jnc.appendFmtLiteral_p", pFunctionType);
		break;

	case EStdFunc_AppendFmtLiteral_i32:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_FmtLiteral)->GetDataPtrType_c (),
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType_c (EType_DataPtr, EPtrTypeFlag_Const),
		ArgTypeArray [2] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int32),
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 3);
		pFunction = CreateFunction (EFunction_Internal, "jnc.appendFmtLiteral_i32", pFunctionType);
		break;

	case EStdFunc_AppendFmtLiteral_ui32:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_FmtLiteral)->GetDataPtrType_c (),
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType_c (EType_DataPtr, EPtrTypeFlag_Const),
		ArgTypeArray [2] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int32_u),
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 3);
		pFunction = CreateFunction (EFunction_Internal, "jnc.appendFmtLiteral_ui32", pFunctionType);
		break;

	case EStdFunc_AppendFmtLiteral_i64:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_FmtLiteral)->GetDataPtrType_c (),
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType_c (EType_DataPtr, EPtrTypeFlag_Const),
		ArgTypeArray [2] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64),
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 3);
		pFunction = CreateFunction (EFunction_Internal, "jnc.appendFmtLiteral_i64", pFunctionType);
		break;

	case EStdFunc_AppendFmtLiteral_ui64:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_FmtLiteral)->GetDataPtrType_c (),
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType_c (EType_DataPtr, EPtrTypeFlag_Const),
		ArgTypeArray [2] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int64_u),
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 3);
		pFunction = CreateFunction (EFunction_Internal, "jnc.appendFmtLiteral_ui64", pFunctionType);
		break;

	case EStdFunc_AppendFmtLiteral_f:
		pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT);
		ArgTypeArray [0] = m_pModule->m_TypeMgr.GetStdType (EStdType_FmtLiteral)->GetDataPtrType_c (),
		ArgTypeArray [1] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Char)->GetDataPtrType_c (EType_DataPtr, EPtrTypeFlag_Const),
		ArgTypeArray [2] = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Double),
		pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, 3);
		pFunction = CreateFunction (EFunction_Internal, "jnc.appendFmtLiteral_f", pFunctionType);
		break;

	case EStdFunc_SimpleMulticastCall:
		pFunction = ((CMulticastClassType*) m_pModule->m_TypeMgr.GetStdType (EStdType_SimpleMulticast))->GetMethod (EMulticastMethod_Call);
		break;

	default:
		ASSERT (false);
		pFunction = NULL;
	}

	m_StdFunctionArray [Func] = pFunction;
	return pFunction;
}

CLazyStdFunction*
CFunctionMgr::GetLazyStdFunction (EStdFunc Func)
{
	ASSERT ((size_t) Func < EStdFunc__Count);

	if (m_LazyStdFunctionArray [Func])
		return m_LazyStdFunctionArray [Func];

	const char* NameTable [EStdFunc__Count] =
	{
		NULL, // EStdFunc_RuntimeError,
		NULL, // EStdFunc_CheckNullPtr,
		NULL, // EStdFunc_CheckScopeLevel,
		NULL, // EStdFunc_CheckClassPtrScopeLevel,
		NULL, // EStdFunc_CheckDataPtrRange,
		NULL, // EStdFunc_DynamicCastClassPtr,
		NULL, // EStdFunc_StrengthenClassPtr,
		"getDataPtrSpan",   // EStdFunc_GetDataPtrSpan,
		NULL, // EStdFunc_GcAllocate,
		NULL, // EStdFunc_GcTryAllocate,
		NULL, // EStdFunc_GcEnter,
		NULL, // EStdFunc_GcLeave,
		NULL, // EStdFunc_GcPulse,
		NULL, // EStdFunc_MarkGcRoot,
		"runGc",              // EStdFunc_RunGc,
		"getCurrentThreadId", // EStdFunc_GetCurrentThreadId,
		"createThread",       // EStdFunc_CreateThread,
		"sleep",              // EStdFunc_Sleep,
		"getTimestamp",       // EStdFunc_GetTimestamp,
		"format",             // EStdFunc_Format,
		"strlen",             // EStdFunc_StrLen,
		"memcpy",             // EStdFunc_MemCpy,
		"memcat",             // EStdFunc_MemCat,
		"rand",               // EStdFunc_Rand,
		"printf",             // EStdFunc_Printf,
		"atoi",               // EStdFunc_Atoi,
		NULL, // EStdFunc_GetTls,
		NULL, // EStdFunc_AppendFmtLiteral_a,
		NULL, // EStdFunc_AppendFmtLiteral_p,
		NULL, // EStdFunc_AppendFmtLiteral_i32,
		NULL, // EStdFunc_AppendFmtLiteral_ui32,
		NULL, // EStdFunc_AppendFmtLiteral_i64,
		NULL, // EStdFunc_AppendFmtLiteral_ui64,
		NULL, // EStdFunc_AppendFmtLiteral_f,
		NULL, // EStdFunc_SimpleMulticastCall,
		"getLastError",       // EStdFunc_GetLastError,
	};

	const char* pName = NameTable [Func];
	ASSERT (pName);

	CLazyStdFunction* pFunction = AXL_MEM_NEW (CLazyStdFunction);
	pFunction->m_pModule = m_pModule;
	pFunction->m_Name = pName;
	pFunction->m_Func = Func;
	m_LazyStdFunctionList.InsertTail (pFunction);
	m_LazyStdFunctionArray [Func] = pFunction;
	return pFunction;
}

CFunction*
CFunctionMgr::CreateCheckNullPtr ()
{
	CType* pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
	CType* ArgTypeArray [] =
	{
		m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr),
		m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int),
	};

	CFunctionType* pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, countof (ArgTypeArray));
	CFunction* pFunction = CreateFunction (EFunction_Internal, "jnc.checkNullPtr", pFunctionType);

	CValue ArgValueArray [2];
	InternalPrologue (pFunction, ArgValueArray, countof (ArgValueArray));

	CValue ArgValue1 = ArgValueArray [0];
	CValue ArgValue2 = ArgValueArray [1];

	CBasicBlock* pFailBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("iface_fail");
	CBasicBlock* pSuccessBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("iface_success");

	CValue NullValue = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr)->GetZeroValue ();

	CValue CmpValue;
	m_pModule->m_LlvmIrBuilder.CreateEq_i (ArgValue1, NullValue, &CmpValue);
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pFailBlock, pSuccessBlock);

	m_pModule->m_LlvmIrBuilder.RuntimeError (ArgValue2);

	m_pModule->m_ControlFlowMgr.Follow (pSuccessBlock);

	InternalEpilogue ();

	return pFunction;
}

CFunction*
CFunctionMgr::CreateCheckScopeLevel ()
{
	CType* pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
	CType* ArgTypeArray [] =
	{
		m_pModule->m_TypeMgr.GetStdType (EStdType_ObjHdrPtr),
		m_pModule->m_TypeMgr.GetStdType (EStdType_ObjHdrPtr),
	};

	CFunctionType* pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, countof (ArgTypeArray));
	CFunction* pFunction = CreateFunction (EFunction_Internal, "jnc.checkScopeLevel", pFunctionType);

	CValue ArgValueArray [2];
	InternalPrologue (pFunction, ArgValueArray, countof (ArgValueArray));

	CValue ArgValue1 = ArgValueArray [0];
	CValue ArgValue2 = ArgValueArray [1];

	CBasicBlock* pNoNullBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("scope_nonull");
	CBasicBlock* pFailBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("scope_fail");
	CBasicBlock* pSuccessBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("scope_success");

	CValue CmpValue;
	CValue NullValue = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjHdrPtr)->GetZeroValue ();

	m_pModule->m_LlvmIrBuilder.CreateEq_i (ArgValue1, NullValue, &CmpValue);
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pSuccessBlock, pNoNullBlock, pNoNullBlock);

	m_pModule->m_LlvmIrBuilder.CreateGep2 (ArgValue1, 0, NULL, &ArgValue1);
	m_pModule->m_LlvmIrBuilder.CreateLoad (ArgValue1, NULL, &ArgValue1);
	m_pModule->m_LlvmIrBuilder.CreateGep2 (ArgValue2, 0, NULL, &ArgValue2);
	m_pModule->m_LlvmIrBuilder.CreateLoad (ArgValue2, NULL, &ArgValue2);
	m_pModule->m_LlvmIrBuilder.CreateGt_u (ArgValue1, ArgValue2, &CmpValue);
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pFailBlock, pSuccessBlock);
	m_pModule->m_LlvmIrBuilder.RuntimeError (ERuntimeError_ScopeMismatch);

	m_pModule->m_ControlFlowMgr.Follow (pSuccessBlock);

	InternalEpilogue ();

	return pFunction;
}

CFunction*
CFunctionMgr::CreateCheckClassPtrScopeLevel ()
{
	CType* pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
	CType* ArgTypeArray [] =
	{
		m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr),
		m_pModule->m_TypeMgr.GetStdType (EStdType_ObjHdrPtr),
	};

	CFunctionType* pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, countof (ArgTypeArray));
	CFunction* pFunction = CreateFunction (EFunction_Internal, "jnc.checkClassPtrScopeLevel", pFunctionType);

	CValue ArgValueArray [2];
	InternalPrologue (pFunction, ArgValueArray, countof (ArgValueArray));

	CValue ArgValue1 = ArgValueArray [0];
	CValue ArgValue2 = ArgValueArray [1];

	CBasicBlock* pNoNullBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("scope_nonull");
	CBasicBlock* pFailBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("scope_fail");
	CBasicBlock* pSuccessBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("scope_success");

	CValue CmpValue;
	CValue NullValue = m_pModule->m_TypeMgr.GetStdType (EStdType_ObjectPtr)->GetZeroValue ();

	m_pModule->m_LlvmIrBuilder.CreateEq_i (ArgValue1, NullValue, &CmpValue);
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pSuccessBlock, pNoNullBlock, pNoNullBlock);

	static int32_t LlvmIndexArray [] =
	{
		0, // TIfaceHdr**
		0, // TIfaceHdr*
		1, // TObjHdr**
	};

	CValue ObjPtrValue;
	m_pModule->m_LlvmIrBuilder.CreateGep (ArgValue1, LlvmIndexArray, countof (LlvmIndexArray), NULL, &ObjPtrValue); // TObjHdr** ppObject
	m_pModule->m_LlvmIrBuilder.CreateLoad (ObjPtrValue, NULL, &ObjPtrValue);  // TObjHdr* pObject

	CValue SrcScopeLevelValue;
	m_pModule->m_LlvmIrBuilder.CreateGep2 (ObjPtrValue, 0, NULL, &SrcScopeLevelValue);     // size_t* pScopeLevel
	m_pModule->m_LlvmIrBuilder.CreateLoad (SrcScopeLevelValue, NULL, &SrcScopeLevelValue); // size_t ScopeLevel

	m_pModule->m_LlvmIrBuilder.CreateGep2 (ArgValue2, 0, NULL, &ArgValue2);
	m_pModule->m_LlvmIrBuilder.CreateLoad (ArgValue2, NULL, &ArgValue2);

	m_pModule->m_LlvmIrBuilder.CreateGt_u (SrcScopeLevelValue, ArgValue2, &CmpValue); // SrcScopeLevel > DstScopeLevel
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pFailBlock, pSuccessBlock);
	m_pModule->m_LlvmIrBuilder.RuntimeError (ERuntimeError_ScopeMismatch);

	m_pModule->m_ControlFlowMgr.Follow (pSuccessBlock);

	InternalEpilogue ();

	return pFunction;
}

CFunction*
CFunctionMgr::CreateCheckDataPtrRange ()
{
	CType* pReturnType = m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void);
	CType* ArgTypeArray [] =
	{
		m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr),
		m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT),
		m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr),
		m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr),
	};

	CFunctionType* pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pReturnType, ArgTypeArray, countof (ArgTypeArray));
	CFunction* pFunction = CreateFunction (EFunction_Internal, "jnc.checkDataPtrRange", pFunctionType);

	CValue ArgValueArray [4];
	InternalPrologue (pFunction, ArgValueArray, countof (ArgValueArray));

	CValue ArgValue1 = ArgValueArray [0];
	CValue ArgValue2 = ArgValueArray [1];
	CValue ArgValue3 = ArgValueArray [2];
	CValue ArgValue4 = ArgValueArray [3];

	CBasicBlock* pFailBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("sptr_fail");
	CBasicBlock* pSuccessBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("sptr_success");
	CBasicBlock* pCmp2Block = m_pModule->m_ControlFlowMgr.CreateBlock ("sptr_cmp2");
	CBasicBlock* pCmp3Block = m_pModule->m_ControlFlowMgr.CreateBlock ("sptr_cmp3");

	CValue NullValue = m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr)->GetZeroValue ();

	CValue CmpValue;
	m_pModule->m_LlvmIrBuilder.CreateEq_i (ArgValue1, NullValue, &CmpValue);
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pFailBlock, pCmp2Block, pCmp2Block);

	m_pModule->m_LlvmIrBuilder.CreateLt_u (ArgValue1, ArgValue3, &CmpValue);
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pFailBlock, pCmp3Block, pCmp3Block);

	CValue PtrEndValue;
	m_pModule->m_LlvmIrBuilder.CreateGep (ArgValue1, ArgValue2, NULL ,&PtrEndValue);
	m_pModule->m_LlvmIrBuilder.CreateGt_u (PtrEndValue, ArgValue4, &CmpValue);
	m_pModule->m_ControlFlowMgr.ConditionalJump (CmpValue, pFailBlock, pSuccessBlock);

	m_pModule->m_LlvmIrBuilder.RuntimeError (ERuntimeError_DataPtrOutOfRange);

	m_pModule->m_ControlFlowMgr.Follow (pSuccessBlock);

	InternalEpilogue ();

	return pFunction;
}

// size_t
// jnc.GetDataPtrSpan (jnc.DataPtr Ptr);

CFunction*
CFunctionMgr::CreateGetDataPtrSpan ()
{
	CType* pIntPtrType = m_pModule->GetSimpleType (EType_Int_p);
	CType* ArgTypeArray [] =
	{
		m_pModule->m_TypeMgr.GetPrimitiveType (EType_Void)->GetDataPtrType (EDataPtrType_Normal, EPtrTypeFlag_Const),
	};

	CFunctionType* pFunctionType = m_pModule->m_TypeMgr.GetFunctionType (pIntPtrType, ArgTypeArray, countof (ArgTypeArray));
	CFunction* pFunction = CreateFunction ("getDataPtrSpan", "jnc.getDataPtrSpan", pFunctionType);

	CValue ArgValue;
	InternalPrologue (pFunction, &ArgValue, 1);

	CValue PtrValue;
	CValue RangeEndValue;
	CValue SpanValue;
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (ArgValue, 0, NULL, &PtrValue);
	m_pModule->m_LlvmIrBuilder.CreateExtractValue (ArgValue, 2, NULL, &RangeEndValue);
	m_pModule->m_LlvmIrBuilder.CreatePtrToInt (PtrValue, pIntPtrType, &PtrValue);
	m_pModule->m_LlvmIrBuilder.CreatePtrToInt (RangeEndValue, pIntPtrType, &RangeEndValue);
	m_pModule->m_LlvmIrBuilder.CreateSub_i (RangeEndValue, PtrValue, pIntPtrType, &SpanValue);
	m_pModule->m_LlvmIrBuilder.CreateRet (SpanValue);

	InternalEpilogue ();

	return pFunction;
}

//.............................................................................

} // namespace jnc {

