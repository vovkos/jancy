#include "pch.h"
#include "jnc_Function.h"
#include "jnc_Module.h"
#include "jnc_ClassType.h"
#include "jnc_Parser.llk.h"

namespace jnc {

//.............................................................................

const char*
GetFunctionKindString (EFunction FunctionKind)
{
	static const char* StringTable [EFunction__Count] =
	{
		"undefined-function-kind",  // EFunction_Undefined,
		"named-function",           // EFunction_Named,
		"get",                      // EFunction_Getter,
		"set",                      // EFunction_Setter,
		"bind",                     // EFunction_Binder,
		"prime",                    // EFunction_Primer,
		"preconstruct",             // EFunction_PreConstructor,
		"construct",                // EFunction_Constructor,
		"destruct",                 // EFunction_Destructor,
		"static construct",         // EFunction_StaticConstructor,
		"static destruct",          // EFunction_StaticDestructor,
		"module construct",         // EFunction_ModuleConstructor,
		"module destruct",          // EFunction_ModuleDestructor,
		"call-operator",            // EFunction_CallOperator,
		"cast-operator",            // EFunction_CastOperator,
		"unary-operator",           // EFunction_UnaryOperator,
		"binary-operator",          // EFunction_BinaryOperator,
		"operator_new",             // EFunction_OperatorNew,
		"internal",                 // EFunction_Internal,
		"thunk",                    // EFunction_Thunk,
		"reaction",                 // EFunction_Reaction,
		"schedule-launcher",        // EFunction_ScheduleLauncher,
	};

	return (size_t) FunctionKind < EFunction__Count ?
		StringTable [FunctionKind] :
		StringTable [EFunction_Undefined];
}

//.............................................................................

int
GetFunctionKindFlags (EFunction FunctionKind)
{
	static int FlagTable [EFunction__Count] =
	{
		0,                              // EFunction_Undefined,
		0,                              // EFunction_Named,
		EFunctionKindFlag_NoOverloads,  // EFunction_Getter,
		0,                              // EFunction_Setter,
		0,                              // EFunction_Binder,
		0,                              // EFunction_Primer,
		EFunctionKindFlag_NoStorage   | // EFunction_PreConstructor,
		EFunctionKindFlag_NoOverloads |
		EFunctionKindFlag_NoArgs,
		EFunctionKindFlag_NoStorage,    // EFunction_Constructor,
		EFunctionKindFlag_NoStorage   | // EFunction_Destructor,
		EFunctionKindFlag_NoOverloads |
		EFunctionKindFlag_NoArgs,
		EFunctionKindFlag_NoStorage   | // EFunction_StaticConstructor,
		EFunctionKindFlag_NoOverloads |
		EFunctionKindFlag_NoArgs,
		EFunctionKindFlag_NoStorage   | // EFunction_StaticDestructor,
		EFunctionKindFlag_NoOverloads |
		EFunctionKindFlag_NoArgs,
		EFunctionKindFlag_NoStorage   | // EFunction_ModuleConstructor,
		EFunctionKindFlag_NoOverloads |
		EFunctionKindFlag_NoArgs,
		EFunctionKindFlag_NoStorage   | // EFunction_ModuleDestructor,
		EFunctionKindFlag_NoOverloads |
		EFunctionKindFlag_NoArgs,
		0,                              // EFunction_CallOperator,
		EFunctionKindFlag_NoOverloads | // EFunction_CastOperator,
		EFunctionKindFlag_NoArgs,
		EFunctionKindFlag_NoOverloads | // EFunction_UnaryOperator,
		EFunctionKindFlag_NoArgs,
		0,                              // EFunction_BinaryOperator,
		0,                              // EFunction_OperatorNew,
		0,                              // EFunction_Internal,
		0,                              // EFunction_Thunk,
		0,                              // EFunction_Reaction,
		0,                              // EFunction_ScheduleLauncher,
	};

	return FunctionKind >= 0 && FunctionKind < EFunction__Count ? FlagTable [FunctionKind] : 0;
}

//.............................................................................

CFunction::CFunction ()
{
	m_ItemKind = EModuleItem_Function;
	m_FunctionKind = EFunction_Undefined;
	m_pType = NULL;
	m_pCastOpType = NULL;
	m_pThisArgType = NULL;
	m_pThisType = NULL;
	m_ThisArgDelta = 0;
	m_ThisArgTypeFlags = 0;
	m_pVirtualOriginClassType = NULL;
	m_pProperty = NULL;
	m_ClassVTableIndex = -1;
	m_PropertyVTableIndex = -1;
	m_pEntryBlock = NULL;
	m_pScope = NULL;
	m_pLlvmFunction = NULL;
	m_pLlvmPostTlsPrologueInst = NULL;
	m_pfMachineCode = NULL;
	m_MachineCodeSize = 0;
}

void
CFunction::MarkGc ()
{
	llvm::Function* pLlvmFunction = GetLlvmFunction ();
	if (!pLlvmFunction->hasGC ())
		pLlvmFunction->setGC ("jnc-shadow-stack");
}

bool
CFunction::SetBody (rtl::CBoxListT <CToken>* pTokenList)
{
	if (!m_Body.IsEmpty ())
	{
		err::SetFormatStringError ("'%s' already has a body", m_Tag.cc ());
		return false;
	}

	if (m_StorageKind == EStorage_Abstract)
	{
		err::SetFormatStringError ("'%s' is abstract and hence cannot have a body", m_Tag.cc ());
		return false;
	}

	m_Body.TakeOver (pTokenList);
	m_pModule->MarkForCompile (this);
	return true;
}

llvm::Function*
CFunction::GetLlvmFunction ()
{
	if (m_pLlvmFunction)
		return m_pLlvmFunction;

	m_pLlvmFunction = m_pType->GetCallConv ()->CreateLlvmFunction (m_pType, m_Tag);
	m_pModule->m_FunctionMgr.m_LlvmFunctionMap [m_pLlvmFunction] = this;
	return m_pLlvmFunction;
}

llvm::DISubprogram
CFunction::GetLlvmDiSubprogram ()
{
	if (m_LlvmDiSubprogram)
		return m_LlvmDiSubprogram;

	m_LlvmDiSubprogram = m_pModule->m_LlvmDiBuilder.CreateFunction (this);
	return m_LlvmDiSubprogram;
}

void
CFunction::ConvertToMemberMethod (CNamedType* pParentType)
{
	ASSERT (m_TypeOverload.GetOverloadCount () == 1);

	m_pParentNamespace = pParentType;
	m_pType = pParentType->GetMemberMethodType (m_pType, m_ThisArgTypeFlags);
	m_TypeOverload = m_pType;

	ASSERT (!m_pType->GetArgArray ().IsEmpty ());
	m_pThisArgType = m_pType->GetArgArray () [0]->GetType ();
	m_pThisType = m_pThisArgType;
}

bool
CFunction::AddOverload (CFunction* pFunction)
{
	bool Result = m_TypeOverload.AddOverload (pFunction->m_pType);
	if (!Result)
		return false;

	m_OverloadArray.Append (pFunction);
	return true;
}

void
CFunction::AddTlsVariable (CVariable* pVariable)
{
	llvm::AllocaInst* pLlvmAlloca = (llvm::AllocaInst*) pVariable->GetLlvmValue ();
	ASSERT (pLlvmAlloca && llvm::isa <llvm::AllocaInst> (*pLlvmAlloca));

	TTlsVariable TlsVariable;
	TlsVariable.m_pVariable = pVariable;
	TlsVariable.m_pLlvmAlloca = pLlvmAlloca;
	m_TlsVariableArray.Append (TlsVariable);
}

bool
CFunction::Compile ()
{
	ASSERT (!m_Body.IsEmpty ()); // otherwise what are we doing here?
	ASSERT (!m_pEntryBlock || m_FunctionKind == EFunction_ModuleConstructor);

	bool Result;

	if (m_pEntryBlock) // already compiled
		return true;

	m_pModule->m_UnitMgr.SetCurrentUnit (m_pItemDecl->GetParentUnit ());

	// prologue

	CToken::CPos BeginPos = m_Body.GetHead ()->m_Pos;
	CToken::CPos EndPos = m_Body.GetTail ()->m_Pos;

	Result = m_pModule->m_FunctionMgr.Prologue (this, BeginPos);
	if (!Result)
		return false;

	TOnceStmt Stmt; // for static constructors

	// parse body

	CParser Parser;
	Parser.m_pModule = m_pModule;
	Parser.m_Stage = CParser::EStage_Pass2;

	ESymbol StartSymbol = ESymbol_compound_stmt;

	if (m_FunctionKind == EFunction_StaticConstructor)
	{
		CDerivableType* pParentType = GetParentType ();
		if (!pParentType)
		{
			err::SetFormatStringError ("static constructors for properties are not yet supported");
			return false;
		}

		m_pModule->m_ControlFlowMgr.OnceStmt_Create (&Stmt, pParentType->GetStaticOnceFlagVariable ());

		Result = m_pModule->m_ControlFlowMgr.OnceStmt_PreBody (&Stmt, BeginPos);
		if (!Result)
			return false;
	}
	else if (m_FunctionKind == EFunction_PreConstructor)
	{
		CDerivableType* pParentType = GetParentType ();
		if (!pParentType)
		{
			err::SetFormatStringError ("preconstructors for properties are not yet supported");
			return false;
		}

		CFunction* pStaticConstructor = pParentType->GetStaticConstructor ();
		if (pStaticConstructor)
			m_pModule->m_OperatorMgr.CallOperator (pStaticConstructor);

		CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
		ASSERT (ThisValue);

		EType TypeKind = pParentType->GetTypeKind ();
		switch (TypeKind)
		{
		case EType_Struct:
			Result = ((CStructType*) pParentType)->InitializeFields (ThisValue);
			break;

		case EType_Union:
			Result = ((CUnionType*) pParentType)->InitializeField (ThisValue);
			break;

		case EType_Class:
			Result = ((CClassType*) pParentType)->GetIfaceStructType ()->InitializeFields (ThisValue);
			break;
		}
	}
	else if (m_FunctionKind == EFunction_Constructor)
	{
		StartSymbol = ESymbol_constructor_compound_stmt;

		ENamespace Namespace = m_pParentNamespace->GetNamespaceKind ();
		ASSERT (Namespace == ENamespace_Type || Namespace == ENamespace_Property);

		if (Namespace == ENamespace_Type)
			Parser.m_pConstructorType = (CDerivableType*) m_pParentNamespace;
		else
			Parser.m_pConstructorProperty = (CProperty*) m_pParentNamespace;
	}

	Result = Parser.ParseTokenList (StartSymbol, m_Body, true);
	if (!Result)
		return false;

	if (m_FunctionKind == EFunction_StaticConstructor)
		m_pModule->m_ControlFlowMgr.OnceStmt_PostBody (&Stmt, EndPos);

	// epilogue

	m_pModule->m_NamespaceMgr.SetSourcePos (EndPos);

	Result = m_pModule->m_FunctionMgr.Epilogue ();
	if (!Result)
		return false;

	return true;
}

//.............................................................................


CModuleItem*
CLazyStdFunction::GetActualItem ()
{
	return m_pModule->m_FunctionMgr.GetStdFunction (m_Func);
}

//.............................................................................

} // namespace jnc {
