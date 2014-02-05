#include "pch.h"
#include "jnc_ReactorClassType.h"
#include "jnc_Module.h"
#include "jnc_Parser.llk.h"

namespace jnc {

//.............................................................................

CReactorClassType::CReactorClassType ()
{
	m_ClassTypeKind = EClassType_Reactor;
	m_BindSiteCount = 0;
	memset (m_FieldArray, 0, sizeof (m_FieldArray));
	memset (m_MethodArray, 0, sizeof (m_MethodArray));
}

CFunction*
CReactorClassType::CreateHandler (const rtl::CArrayT <CFunctionArg*>& ArgArray)
{
	CFunctionType* pType = m_pModule->m_TypeMgr.GetFunctionType (ArgArray);
	return CreateUnnamedMethod (EStorage_Member, EFunction_Reaction, pType);
}

bool
CReactorClassType::SetBody (rtl::CBoxListT <CToken>* pTokenList)
{
	if (!m_Body.IsEmpty ())
	{
		err::SetFormatStringError ("'%s' already has a body", m_Tag.cc ());
		return false;
	}

	m_Body.TakeOver (pTokenList);
	m_pModule->MarkForCompile (this);
	return true;
}

bool
CReactorClassType::CalcLayout ()
{
	bool Result;

	if (m_Body.IsEmpty ())
	{
		err::SetFormatStringError ("reactor '%s' has no body", m_Tag.cc ()); // thanks a lot gcc
		return false;
	}

	// scan

	CParser Parser;
	Parser.m_Stage = CParser::EStage_ReactorScan;
	Parser.m_pModule = m_pModule;
	Parser.m_pReactorType = this;

	CFunction* pStart = m_MethodArray [EReactorMethod_Start];
	CFunction* pPrevFunction = m_pModule->m_FunctionMgr.SetCurrentFunction (pStart);

	m_pModule->m_NamespaceMgr.OpenNamespace (this);

	Result = Parser.ParseTokenList (ESymbol_reactor_body_0, m_Body, false);
	if (!Result)
		return false;

	m_pModule->m_NamespaceMgr.CloseNamespace ();
	m_pModule->m_FunctionMgr.SetCurrentFunction (pPrevFunction);

	ASSERT (Parser.m_ReactorBindSiteTotalCount);
	m_BindSiteCount = Parser.m_ReactorBindSiteTotalCount;

	CType* pBindSiteType = m_pModule->m_TypeMgr.GetStdType (EStdType_ReactorBindSite);
	CArrayType* pBindSiteArrayType = pBindSiteType->GetArrayType (m_BindSiteCount);
	m_FieldArray [EReactorField_BindSiteArray] = CreateField (pBindSiteArrayType);

	Result = CClassType::CalcLayout ();
	if (!Result)
		return false;

	return true;
}

bool
CReactorClassType::BindHandlers (const rtl::CConstListT <TReaction>& HandlerList)
{
	bool Result;

	CStructType* pBindSiteType = (CStructType*) m_pModule->m_TypeMgr.GetStdType (EStdType_ReactorBindSite);
	CStructField* pEventPtrField = *pBindSiteType->GetFieldList ().GetHead ();
	CStructField* pCookieField = *pBindSiteType->GetFieldList ().GetTail ();

	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
	ASSERT (ThisValue);

	CValue BindSiteArrayValue;
	Result = m_pModule->m_OperatorMgr.GetField (
		ThisValue,
		m_FieldArray [EReactorField_BindSiteArray],
		NULL,
		&BindSiteArrayValue
		);

	if (!Result)
		return false;

	rtl::CIteratorT <TReaction> Handler = HandlerList.GetHead ();
	size_t i = 0;
	for (; Handler; Handler++)
	{
		CFunction* pFunction = Handler->m_pFunction;

		rtl::CBoxIteratorT <CValue> Value = Handler->m_BindSiteList.GetHead ();
		for (; Value; Value++, i++)
		{
			CValue EventValue = *Value;
			CValue HandlerValue = pFunction;
			HandlerValue.InsertToClosureHead (ThisValue);

			if (EventValue.GetType ()->GetTypeKind () == EType_ClassRef) 
			{
				Result = m_pModule->m_OperatorMgr.UnaryOperator (EUnOp_Addr, &EventValue); // turn into a pointer
				ASSERT (Result);
			}

			CValue IdxValue (i, EType_SizeT);
			CValue AddMethodValue;
			CValue CookieValue;
			CValue BindSiteValue;
			CValue DstEventValue;
			CValue DstCookieValue;

			Result =
				m_pModule->m_OperatorMgr.MemberOperator (EventValue, "add", &AddMethodValue) &&
				m_pModule->m_OperatorMgr.CallOperator (AddMethodValue, HandlerValue, &CookieValue) &&
				m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Idx, BindSiteArrayValue, IdxValue, &BindSiteValue) &&
				m_pModule->m_OperatorMgr.GetStructField (BindSiteValue, pEventPtrField, NULL, &DstEventValue) &&
				m_pModule->m_OperatorMgr.GetStructField (BindSiteValue, pCookieField, NULL, &DstCookieValue) &&
				m_pModule->m_OperatorMgr.StoreDataRef (DstCookieValue, CookieValue);

			if (!Result)
				return false;

			// force-cast event pointers (normal cast would result in dynamic cast)

			CType* pEventType = m_pModule->m_TypeMgr.GetStdType (EStdType_SimpleEventPtr);
			m_pModule->m_LlvmIrBuilder.CreateBitCast (EventValue, pEventType, &EventValue);
			m_pModule->m_LlvmIrBuilder.CreateStore (EventValue, DstEventValue);
		}
	}

	ASSERT (i == m_BindSiteCount);
	return true;
}


bool
CReactorClassType::CallStopMethod ()
{
	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
	ASSERT (ThisValue);

	CValue StopMethodValue = m_MethodArray [EReactorMethod_Stop];
	StopMethodValue.InsertToClosureHead (ThisValue);
	return m_pModule->m_OperatorMgr.CallOperator (StopMethodValue);
}

bool
CReactorClassType::CompileConstructor ()
{
	ASSERT (m_pConstructor);

	bool Result;

	size_t ArgCount = m_pConstructor->GetType ()->GetArgArray ().GetCount ();
	ASSERT (ArgCount == 1 || ArgCount == 2);

	CValue ArgValueArray [2];
	m_pModule->m_FunctionMgr.InternalPrologue (m_pConstructor, ArgValueArray, ArgCount);

	if (ArgCount == 2)
	{
		CStructField* pField = m_FieldArray [EReactorField_Parent];
		ASSERT (pField);

		CValue ParentFieldValue;
		Result =
			m_pModule->m_OperatorMgr.GetClassField (ArgValueArray [0], pField, NULL, &ParentFieldValue) &&
			m_pModule->m_OperatorMgr.StoreDataRef (ParentFieldValue, ArgValueArray [1]);

		if (!Result)
			return false;
	}

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

bool
CReactorClassType::CompileDestructor ()
{
	ASSERT (m_pDestructor);

	bool Result;

	CValue ArgValue;
	m_pModule->m_FunctionMgr.InternalPrologue (m_pDestructor, &ArgValue, 1);

	Result = CallStopMethod ();
	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

bool
CReactorClassType::CompileStartMethod ()
{
	bool Result;

	CFunction* pStartMethod = m_MethodArray [EReactorMethod_Start];
	CFunction* pStopMethod = m_MethodArray [EReactorMethod_Stop];

	Result = m_pModule->m_FunctionMgr.Prologue (pStartMethod, m_Body.GetHead ()->m_Pos);
	if (!Result)
		return false;

	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
	ASSERT (ThisValue);

	// stop

	Result = CallStopMethod ();
	if (!Result)
		return false;

	// save arguments

	rtl::CArrayT <CFunctionArg*> ArgArray = pStartMethod->GetType ()->GetArgArray ();
	size_t ArgCount = ArgArray.GetCount ();
	size_t i = 1;

	rtl::CIteratorT <CStructField> ArgField = m_FirstArgField;
	llvm::Function::arg_iterator LlvmArg = pStartMethod->GetLlvmFunction ()->arg_begin();
	LlvmArg++;

	for (; i < ArgCount; i++, LlvmArg++, ArgField++)
	{
		CFunctionArg* pArg = ArgArray [i];
		llvm::Value* pLlvmArg = LlvmArg;
		CStructField* pArgField = *ArgField;

		if (!pArg->IsNamed ())
			continue;

		CValue ArgValue (pLlvmArg, pArg->GetType ());

		CValue StoreValue;
		Result = m_pModule->m_OperatorMgr.GetField (ThisValue, pArgField, NULL, &StoreValue);
		if (!Result)
			return false;

		m_pModule->m_LlvmIrBuilder.CreateStore (ArgValue, StoreValue);
	}

	// compile start

	CParser Parser;
	Parser.m_Stage = CParser::EStage_Pass2;
	Parser.m_pModule = m_pModule;
	Parser.m_pReactorType = this;

	Result = Parser.ParseTokenList (ESymbol_reactor_body, m_Body, true);
	if (!Result)
		return false;

	// modify state

	CValue StateValue;
	Result =
		m_pModule->m_OperatorMgr.GetField (ThisValue, m_FieldArray [EReactorField_State], NULL, &StateValue) &&
		m_pModule->m_OperatorMgr.StoreDataRef (StateValue, CValue ((int64_t) 1, EType_Int_p));

	if (!Result)
		return false;

	// done

	Result = m_pModule->m_FunctionMgr.Epilogue ();
	if (!Result)
		return false;

	return true;
}
bool
CReactorClassType::CompileStopMethod ()
{
	bool Result;

	CStructType* pBindSiteType = (CStructType*) m_pModule->m_TypeMgr.GetStdType (EStdType_ReactorBindSite);
	CStructField* pEventPtrField = *pBindSiteType->GetFieldList ().GetHead ();
	CStructField* pCookieField = *pBindSiteType->GetFieldList ().GetTail ();

	m_pModule->m_FunctionMgr.InternalPrologue (m_MethodArray [EReactorMethod_Stop]);

	CValue ThisValue = m_pModule->m_FunctionMgr.GetThisValue ();
	ASSERT (ThisValue);

	CBasicBlock* pUnadviseBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("unadvise_block");
	CBasicBlock* pFollowBlock = m_pModule->m_ControlFlowMgr.CreateBlock ("follow_block");

	CValue StateValue;
	CValue StateCmpValue;
	CValue BindSiteArrayValue;

	Result =
		m_pModule->m_OperatorMgr.GetField (ThisValue, m_FieldArray [EReactorField_State], NULL, &StateValue) &&
		m_pModule->m_ControlFlowMgr.ConditionalJump (StateValue, pUnadviseBlock, pFollowBlock);

	if (!Result)
		return false;

	Result = m_pModule->m_OperatorMgr.GetField (ThisValue, m_FieldArray [EReactorField_BindSiteArray], NULL, &BindSiteArrayValue);
	if (!Result)
		return false;

	for (size_t i = 0; i < m_BindSiteCount; i++)
	{
		CValue IdxValue (i, EType_SizeT);
		CValue BindSiteValue;
		CValue EventValue;
		CValue CookieValue;
		CValue RemoveMethodValue;

		Result =
			m_pModule->m_OperatorMgr.BinaryOperator (EBinOp_Idx, BindSiteArrayValue, IdxValue, &BindSiteValue) &&
			m_pModule->m_OperatorMgr.GetStructField (BindSiteValue, pEventPtrField, NULL, &EventValue) &&
			m_pModule->m_OperatorMgr.GetStructField (BindSiteValue, pCookieField, NULL, &CookieValue) &&
			m_pModule->m_OperatorMgr.MemberOperator (EventValue, "remove", &RemoveMethodValue) &&
			m_pModule->m_OperatorMgr.CallOperator (RemoveMethodValue, CookieValue);

		if (!Result)
			return false;
	}

	Result = m_pModule->m_OperatorMgr.StoreDataRef (StateValue, CValue ((int64_t) 0, EType_Int_p));
	ASSERT (Result);

	m_pModule->m_ControlFlowMgr.Follow (pFollowBlock);

	m_pModule->m_FunctionMgr.InternalEpilogue ();

	return true;
}

//.............................................................................

} // namespace jnc {
