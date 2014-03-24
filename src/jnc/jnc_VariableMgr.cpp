#include "pch.h"
#include "jnc_VariableMgr.h"
#include "jnc_Parser.llk.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CVariableMgr::CVariableMgr ()
{
	m_pModule = GetCurrentThreadModule ();
	ASSERT (m_pModule);

	memset (m_StdVariableArray, 0, sizeof (m_StdVariableArray));
	CreateStdVariables ();
}

void
CVariableMgr::Clear ()
{
	m_VariableList.Clear ();
	m_AliasList.Clear ();

	m_StaticVariableArray.Clear ();
	m_StaticGcRootArray.Clear ();
	m_GlobalStaticVariableArray.Clear ();
	m_StaticDestructList.Clear ();

	m_TlsVariableArray.Clear ();
	m_TlsGcRootArray.Clear ();
	m_pTlsStructType = NULL;

	memset (m_StdVariableArray, 0, sizeof (m_StdVariableArray));
	CreateStdVariables ();
}

void
CVariableMgr::CreateStdVariables ()
{
	for (size_t i = 0; i < EStdVariable__Count; i++)
		GetStdVariable ((EStdVariable) i);
}

CVariable*
CVariableMgr::GetStdVariable (EStdVariable Variable)
{
	ASSERT ((size_t) Variable < EStdFunc__Count);

	if (m_StdVariableArray [Variable])
		return m_StdVariableArray [Variable];

	CVariable* pVariable;

	switch (Variable)
	{
	case EStdVariable_ScopeLevel:
		pVariable = CreateVariable (
			EStorage_Thread,
			"g_scopeLevel",
			"jnc.g_scopeLevel",
			m_pModule->m_TypeMgr.GetPrimitiveType (EType_SizeT)
			);
		break;

	case EStdVariable_GcShadowStackTop:
		pVariable = CreateVariable (
			EStorage_Thread,
			"g_gcShadowStackTop",
			"jnc.g_gcShadowStackTop",
			m_pModule->m_TypeMgr.GetStdType (EStdType_BytePtr)
			);
		break;

	default:
		ASSERT (false);
		pVariable = NULL;
	}

	m_StdVariableArray [Variable] = pVariable;
	return pVariable;
}

CVariable*
CVariableMgr::CreateVariable (
	EStorage StorageKind,
	const rtl::CString& Name,
	const rtl::CString& QualifiedName,
	CType* pType,
	uint_t PtrTypeFlags,
	rtl::CBoxListT <CToken>* pConstructor,
	rtl::CBoxListT <CToken>* pInitializer
	)
{
	CVariable* pVariable = AXL_MEM_NEW (CVariable);
	pVariable->m_pModule = m_pModule;
	pVariable->m_Name = Name;
	pVariable->m_QualifiedName = QualifiedName;
	pVariable->m_Tag = QualifiedName;
	pVariable->m_pType = pType;
	pVariable->m_StorageKind = StorageKind;
	pVariable->m_PtrTypeFlags = PtrTypeFlags;

	if (StorageKind == EStorage_Stack)
	{
		CScope* pScope = m_pModule->m_NamespaceMgr.GetCurrentScope ();
		ASSERT (pScope);

		pVariable->m_pScope = pScope;
	}

	if (pConstructor)
		pVariable->m_Constructor.TakeOver (pConstructor);

	if (pInitializer)
		pVariable->m_Initializer.TakeOver (pInitializer);

	m_VariableList.InsertTail (pVariable);

	switch (StorageKind)
	{
	case EStorage_Static:
		m_StaticVariableArray.Append (pVariable);

		if (m_pModule->m_NamespaceMgr.GetCurrentNamespace ()->GetNamespaceKind () == ENamespace_Global)
			m_GlobalStaticVariableArray.Append (pVariable);

		break;

	case EStorage_Thread:
		m_TlsVariableArray.Append (pVariable);
		break;

	case EStorage_Stack:
	case EStorage_Heap:
		break;

	default:
		ASSERT (false);
	}

	if (pType->GetTypeKindFlags () & ETypeKindFlag_Import)
	{
		pVariable->m_pType_i = (CImportType*) pType;
		m_pModule->MarkForLayout (pVariable);
	}

	return pVariable;
}

CVariable*
CVariableMgr::CreateOnceFlagVariable (EStorage StorageKind)
{
	return CreateVariable (
		StorageKind,
		"once_flag",
		"once_flag",
		m_pModule->m_TypeMgr.GetPrimitiveType (EType_Int32),
		StorageKind == EStorage_Static ? EPtrTypeFlag_Volatile : 0
		);
}

CVariable*
CVariableMgr::CreateArgVariable (
	CFunctionArg* pArg,
	llvm::Value* pLlvmArgValue
	)
{
	bool Result;

	CVariable* pVariable = CreateStackVariable (
		pArg->GetName (),
		pArg->GetType (),
		pArg->GetPtrTypeFlags ()
		);

	pVariable->m_pParentUnit = pArg->GetParentUnit ();
	pVariable->m_Pos = *pArg->GetPos ();
	pVariable->m_Flags |= EModuleItemFlag_User;

	CValue PtrValue;
	Result = m_pModule->m_OperatorMgr.Allocate (
		EStorage_Stack,
		pArg->GetType (),
		pArg->GetName (),
		&PtrValue
		);

	if (!Result)
		return NULL;

	pVariable->m_pLlvmAllocValue = PtrValue.GetLlvmValue ();
	pVariable->m_pLlvmValue = PtrValue.GetLlvmValue ();

	if ((m_pModule->GetFlags () & EModuleFlag_DebugInfo) &&
		(pVariable->GetFlags () & EModuleItemFlag_User))
	{
		pVariable->m_LlvmDiDescriptor = m_pModule->m_LlvmDiBuilder.CreateLocalVariable (
			pVariable,
			llvm::dwarf::DW_TAG_arg_variable
			);

		m_pModule->m_LlvmDiBuilder.CreateDeclare (pVariable);
	}

	return pVariable;
}

llvm::GlobalVariable*
CVariableMgr::CreateLlvmGlobalVariable (
	CType* pType,
	const char* pTag
	)
{
	llvm::GlobalVariable* pLlvmValue = new llvm::GlobalVariable (
		*m_pModule->GetLlvmModule (),
		pType->GetLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		(llvm::Constant*) pType->GetZeroValue ().GetLlvmValue (),
		pTag
		);

	m_LlvmGlobalVariableArray.Append (pLlvmValue);
	return pLlvmValue;
}

CAlias*
CVariableMgr::CreateAlias (
	const rtl::CString& Name,
	const rtl::CString& QualifiedName,
	CType* pType,
	rtl::CBoxListT <CToken>* pInitializer
	)
{
	ASSERT (pInitializer);

	CAlias* pAlias = AXL_MEM_NEW (CAlias);
	pAlias->m_Name = Name;
	pAlias->m_QualifiedName = QualifiedName;
	pAlias->m_Tag = QualifiedName;
	pAlias->m_pType = pType;
	pAlias->m_Initializer.TakeOver (pInitializer);

	m_AliasList.InsertTail (pAlias);

	if (pType->GetTypeKindFlags () & ETypeKindFlag_Import)
	{
		pAlias->m_pType_i = (CImportType*) pType;
		m_pModule->MarkForLayout (pAlias);
	}

	return pAlias;
}

bool
CVariableMgr::CreateTlsStructType ()
{
	bool Result;

	CStructType* pType = m_pModule->m_TypeMgr.CreateStructType ("Tls", "jnc.Tls");

	size_t Count = m_TlsVariableArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CVariable* pVariable = m_TlsVariableArray [i];

		if (pVariable->m_pType->GetTypeKindFlags () & ETypeKindFlag_Aggregate)
		{
			err::SetFormatStringError ("'thread' variables cannot have aggregate type '%s'",  pVariable->m_pType->GetTypeString ().cc ());
			return false;
		}

		pVariable->m_pTlsField = pType->CreateField (pVariable->m_pType);
	}

	Result = pType->EnsureLayout ();
	if (!Result)
		return false;

	m_pTlsStructType = pType;
	return true;
}

bool
CVariableMgr::AllocatePrimeStaticVariables ()
{
	bool Result;

	size_t Count = m_StaticVariableArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CVariable* pVariable = m_StaticVariableArray [i];

		Result = AllocatePrimeStaticVariable (pVariable);
		if (!Result)
			return false;
	}

	return true;
}

bool
CVariableMgr::AllocatePrimeStaticVariable (CVariable* pVariable)
{
	ASSERT (pVariable->m_StorageKind == EStorage_Static);
	ASSERT (m_pModule->m_ControlFlowMgr.GetCurrentBlock () == m_pModule->GetConstructor ()->GetEntryBlock ());

	CType* pType = pVariable->GetType ();

	pVariable->m_pLlvmAllocValue = CreateLlvmGlobalVariable (pType, pVariable->GetQualifiedName ());

	CValue PtrValue (pVariable->m_pLlvmAllocValue, pType->GetDataPtrType_c ());
	bool Result = m_pModule->m_OperatorMgr.Prime (EStorage_Static, PtrValue, pType, &PtrValue);
	if (!Result)
		return false;

	pVariable->m_pLlvmValue = PtrValue.GetLlvmValue ();

	if (pVariable->m_pType->GetFlags () & ETypeFlag_GcRoot)
		m_StaticGcRootArray.Append (pVariable);

	if (m_pModule->GetFlags () & EModuleFlag_DebugInfo)
		pVariable->m_LlvmDiDescriptor = m_pModule->m_LlvmDiBuilder.CreateGlobalVariable (pVariable);

	return true;
}

bool
CVariableMgr::InitializeGlobalStaticVariables ()
{
	bool Result;

	size_t Count = m_GlobalStaticVariableArray.GetCount ();
	for (size_t i = 0; i < Count; i++)
	{
		CVariable* pVariable = m_GlobalStaticVariableArray [i];

		Result = m_pModule->m_OperatorMgr.ParseInitializer (
			pVariable,
			pVariable->m_pItemDecl->GetParentUnit (),
			pVariable->m_Constructor,
			pVariable->m_Initializer
			);

		if (pVariable->m_pType->GetTypeKind () == EType_Class)
		{
			CFunction* pDestructor = ((CClassType*) pVariable->m_pType)->GetDestructor ();
			if (pDestructor)
				m_StaticDestructList.AddDestructor (pDestructor, pVariable);
		}

		if (!Result)
			return false;
	}

	return true;
}

bool
CVariableMgr::AllocatePrimeInitializeVariable (CVariable* pVariable)
{
	CType* pType = pVariable->GetType ();

	if ((pType->GetTypeKindFlags () & ETypeKindFlag_Ptr) && 
		(pType->GetFlags () & EPtrTypeFlag_Safe) &&
		pVariable->GetInitializer ().IsEmpty ())
	{
		err::SetFormatStringError (
			"missing initalizer for '%s' variable '%s'", 
			pType->GetTypeString ().cc (),
			pVariable->GetQualifiedName ().cc ()
			);

		return false;
	}

	EStorage StorageKind = pVariable->m_StorageKind;
	switch (StorageKind)
	{
	case EStorage_Static:
		return AllocatePrimeInitializeStaticVariable (pVariable);

	case EStorage_Thread:
		return AllocatePrimeInitializeTlsVariable (pVariable);

	default:
		return AllocatePrimeInitializeNonStaticVariable (pVariable);
	}
}

bool
CVariableMgr::AllocatePrimeInitializeStaticVariable (CVariable* pVariable)
{
	bool Result;

	// allocate and prime in module constructor

	CBasicBlock* pBlock = m_pModule->m_ControlFlowMgr.GetCurrentBlock ();
	m_pModule->m_ControlFlowMgr.SetCurrentBlock (m_pModule->GetConstructor ()->GetEntryBlock ());

	AllocatePrimeStaticVariable (pVariable);

	m_pModule->m_ControlFlowMgr.SetCurrentBlock (pBlock);

	// initialize within 'once' block

	CToken::CPos Pos = *pVariable->GetItemDecl ()->GetPos ();

	TOnceStmt Stmt;
	m_pModule->m_ControlFlowMgr.OnceStmt_Create (&Stmt, Pos);

	Result =
		m_pModule->m_ControlFlowMgr.OnceStmt_PreBody (&Stmt, Pos) &&
		m_pModule->m_OperatorMgr.ParseInitializer (
			pVariable,
			pVariable->m_pItemDecl->GetParentUnit (),
			pVariable->m_Constructor,
			pVariable->m_Initializer
			);

	if (!Result)
		return false;

	if (pVariable->m_pType->GetTypeKind () == EType_Class)
	{
		CFunction* pDestructor = ((CClassType*) pVariable->m_pType)->GetDestructor ();
		if (pDestructor)
			m_StaticDestructList.AddDestructor (pDestructor, pVariable, Stmt.m_pFlagVariable);
	}

	if (!pVariable->m_Initializer.IsEmpty ())
		Pos = pVariable->m_Initializer.GetTail ()->m_Pos;
	else if (!pVariable->m_Constructor.IsEmpty ())
		Pos = pVariable->m_Constructor.GetTail ()->m_Pos;

	m_pModule->m_ControlFlowMgr.OnceStmt_PostBody (&Stmt, Pos);

	return true;
}

bool
CVariableMgr::AllocatePrimeInitializeTlsVariable (CVariable* pVariable)
{
	bool Result;

	AllocateTlsVariable (pVariable);

	// initialize within 'once' block

	CToken::CPos Pos = *pVariable->GetItemDecl ()->GetPos ();

	TOnceStmt Stmt;
	m_pModule->m_ControlFlowMgr.OnceStmt_Create (&Stmt, Pos, EStorage_Thread);

	Result =
		m_pModule->m_ControlFlowMgr.OnceStmt_PreBody (&Stmt, Pos) &&
		m_pModule->m_OperatorMgr.ParseInitializer (
			pVariable,
			pVariable->m_pItemDecl->GetParentUnit (),
			pVariable->m_Constructor,
			pVariable->m_Initializer
			);

	if (!Result)
		return false;

	if (!pVariable->m_Initializer.IsEmpty ())
		Pos = pVariable->m_Initializer.GetTail ()->m_Pos;
	else if (!pVariable->m_Constructor.IsEmpty ())
		Pos = pVariable->m_Constructor.GetTail ()->m_Pos;

	m_pModule->m_ControlFlowMgr.OnceStmt_PostBody (&Stmt, Pos);

	return true;
}

bool
CVariableMgr::AllocatePrimeInitializeNonStaticVariable (CVariable* pVariable)
{
	bool Result;

	CValue PtrValue;
	Result = m_pModule->m_OperatorMgr.Allocate (
		pVariable->m_StorageKind,
		pVariable->m_pType,
		pVariable->m_Tag,
		&PtrValue
		);

	if (!Result)
		return false;
	
	if (pVariable->m_StorageKind == EStorage_Heap) // local heap variable
		m_pModule->m_OperatorMgr.MarkStackGcRoot (
			PtrValue, 
			pVariable->m_pType->GetDataPtrType_c ()
			);

	pVariable->m_pLlvmAllocValue = PtrValue.GetLlvmValue ();

	if (pVariable->m_pType->GetTypeKind () == EType_Class)
	{
		Result = m_pModule->m_OperatorMgr.Prime (pVariable->m_StorageKind, PtrValue, pVariable->m_pType, &PtrValue);
		if (!Result)
			return false;

		pVariable->m_pLlvmValue = PtrValue.GetLlvmValue ();
	}
	else
	{
		pVariable->m_pLlvmValue = pVariable->m_pLlvmAllocValue;

		if (pVariable->m_Initializer.IsEmpty () ||
			pVariable->m_Initializer.GetHead ()->m_Token == '{')
		{
			m_pModule->m_LlvmIrBuilder.CreateStore (pVariable->m_pType->GetZeroValue (), PtrValue);
		}
	}

	if ((m_pModule->GetFlags () & EModuleFlag_DebugInfo) &&
		(pVariable->GetFlags () & EModuleItemFlag_User))
	{
		pVariable->m_LlvmDiDescriptor = m_pModule->m_LlvmDiBuilder.CreateLocalVariable (pVariable);
		m_pModule->m_LlvmDiBuilder.CreateDeclare (pVariable);
	}

	Result = m_pModule->m_OperatorMgr.ParseInitializer (
		pVariable,
		pVariable->m_pItemDecl->GetParentUnit (),
		pVariable->m_Constructor,
		pVariable->m_Initializer
		);

	if (!Result)
		return false;

	return true;
}

void
CVariableMgr::AllocateTlsVariable (CVariable* pVariable)
{
	ASSERT (m_pModule->m_FunctionMgr.GetCurrentFunction ());

	// create alloca in function entry block

	CFunction* pFunction = m_pModule->m_FunctionMgr.GetCurrentFunction ();
	CBasicBlock* pBlock = m_pModule->m_ControlFlowMgr.SetCurrentBlock (pFunction->GetEntryBlock ());

	CValue PtrValue;
	llvm::AllocaInst* pLlvmAlloca = m_pModule->m_LlvmIrBuilder.CreateAlloca (
		pVariable->m_pType,
		pVariable->m_QualifiedName,
		NULL,
		&PtrValue
		);

	m_pModule->m_ControlFlowMgr.SetCurrentBlock (pBlock);

	pVariable->m_pLlvmAllocValue = pLlvmAlloca;
	pVariable->m_pLlvmValue = pLlvmAlloca;

	pFunction->AddTlsVariable (pVariable);
}

void
CVariableMgr::DeallocateTlsVariableArray (
	const TTlsVariable* ppArray,
	size_t Count
	)
{
	for (size_t i = 0; i < Count; i++)
	{
		CVariable* pVariable = ppArray [i].m_pVariable;
		ASSERT (pVariable->m_pLlvmValue == ppArray [i].m_pLlvmAlloca);

		pVariable->m_pLlvmValue = NULL;
		pVariable->m_pLlvmAllocValue = NULL;
	}
}

void
CVariableMgr::RestoreTlsVariableArray (
	const TTlsVariable* ppArray,
	size_t Count
	)
{
	for (size_t i = 0; i < Count; i++)
	{
		CVariable* pVariable = ppArray [i].m_pVariable;
		llvm::AllocaInst* pLlvmAlloca = ppArray [i].m_pLlvmAlloca;

		pVariable->m_pLlvmValue = pLlvmAlloca;
		pVariable->m_pLlvmAllocValue = pLlvmAlloca;
	}
}

//.............................................................................

} // namespace jnc {
