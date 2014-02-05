#include "pch.h"
#include "jncc.h"

//.............................................................................

int
CStdLib::Printf (
	const char* pFormat,
	...
	)
{
	AXL_VA_DECL (va, pFormat);

	CJnc* pJnc = CJnc::GetCurrentJnc ();
	ASSERT (pJnc);

	return pJnc->GetOutStream ()->Printf_va (pFormat, va);
}

//.............................................................................

bool
CJnc::RunFunction (
	jnc::CFunction* pFunction,
	int* pReturnValue
	)
{
	typedef
	int
	FFunction ();

	FFunction* pf = (FFunction*) pFunction->GetMachineCode ();
	ASSERT (pf);

	bool Result = true;

	try
	{
		int ReturnValue = pf ();
		if (pReturnValue)
			*pReturnValue = ReturnValue;
	}
	catch (err::CError Error)
	{
		err::SetError (Error);
		Result = false;
	}

	return Result;
}

bool
CJnc::RunFunction (int* pReturnValue)
{
	bool Result;

	jnc::CModuleItem* pFunctionItem = m_Module.m_NamespaceMgr.GetGlobalNamespace ()->FindItem (m_pCmdLine->m_FunctionName);
	if (!pFunctionItem || pFunctionItem->GetItemKind () != jnc::EModuleItem_Function)
	{
		err::SetFormatStringError ("'%s' is not found or not a function\n", m_pCmdLine->m_FunctionName.cc ());
		return false;
	}

	jnc::CFunction* pFunction = (jnc::CFunction*) pFunctionItem;

	jnc::CScopeThreadRuntime ScopeRuntime (&m_Runtime);

	m_Runtime.Startup ();

	jnc::CFunction* pConstructor = m_Module.GetConstructor ();
	if (pConstructor)
	{
		Result = RunFunction (pConstructor);
		if (!Result)
			return false;
	}

	Result = RunFunction (pFunction, pReturnValue);
	if (!Result)
		return false;

	jnc::CFunction* pDestructor = m_Module.GetDestructor ();
	if (pDestructor)
	{
		Result = RunFunction (pDestructor);
		if (!Result)
			return false;
	}

	m_Runtime.Shutdown ();

	return true;
}

//.............................................................................
