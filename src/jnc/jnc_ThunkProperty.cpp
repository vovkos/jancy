#include "pch.h"
#include "jnc_ThunkProperty.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CThunkProperty::CThunkProperty ()
{
	m_PropertyKind = EProperty_Thunk;
	m_pTargetProperty = NULL;
}

bool
CThunkProperty::Create (
	CProperty* pTargetProperty,
	CPropertyType* pThunkPropertyType,
	bool HasUnusedClosure
	)
{
	bool Result;

	m_pTargetProperty = pTargetProperty;
	m_pType = HasUnusedClosure ? 
		pThunkPropertyType->GetStdObjectMemberPropertyType () : 
		pThunkPropertyType;

	m_pGetter = m_pModule->m_FunctionMgr.GetDirectThunkFunction (
		pTargetProperty->GetGetter (), 
		pThunkPropertyType->GetGetterType (),
		HasUnusedClosure
		);

	CFunction* pTargetSetter = pTargetProperty->GetSetter ();
	CFunctionTypeOverload* pThunkSetterType = pThunkPropertyType->GetSetterType ();

	size_t SetterCount = pThunkSetterType->GetOverloadCount ();
	if (SetterCount && !pTargetSetter)
	{
		SetCastError (pTargetProperty, pThunkPropertyType);
		return false;
	}

	for (size_t i = 0; i < SetterCount; i++)
	{
		CFunctionType* pThunkFunctionType = pThunkSetterType->GetOverload (i);

		CFunction* pOverload = pTargetSetter->ChooseSetterOverload (pThunkFunctionType);
		if (!pOverload)
			return false;

		CFunction* pThunkFunction = m_pModule->m_FunctionMgr.GetDirectThunkFunction (
			pOverload, 
			pThunkFunctionType,
			HasUnusedClosure
			);

		if (!m_pSetter)
		{
			m_pSetter = pThunkFunction;
		}
		else
		{
			Result = m_pSetter->AddOverload (pThunkFunction);
			if (!Result)
				return false;
		}
	}

	return true;
}

//.............................................................................

CDataThunkProperty::CDataThunkProperty ()
{
	m_PropertyKind = EProperty_DataThunk;
	m_pTargetVariable = NULL;
}

bool 
CDataThunkProperty::Compile ()
{
	bool Result = CompileGetter ();
	if (!Result)
		return false;

	if (m_pSetter)
	{
		size_t Count = m_pSetter->GetOverloadCount ();
		for (size_t i = 0; i < Count; i++)
		{
			CFunction* pOverload = m_pSetter->GetOverload (i);
			Result = CompileSetter (pOverload);
			if (!Result)
				return false;
		}
	}

	return true;
}

bool 
CDataThunkProperty::CompileGetter ()
{
	m_pModule->m_FunctionMgr.InternalPrologue (m_pGetter);

	bool Result = m_pModule->m_ControlFlowMgr.Return (m_pTargetVariable);
	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

bool 
CDataThunkProperty::CompileSetter (CFunction* pSetter)
{
	CValue SrcValue;
	
	size_t ArgCount = pSetter->GetType ()->GetArgArray ().GetCount ();
	ASSERT (ArgCount == 1 || ArgCount == 2);

	CValue ArgValueArray [2];
	m_pModule->m_FunctionMgr.InternalPrologue (pSetter, ArgValueArray, ArgCount);
	
	bool Result = m_pModule->m_OperatorMgr.StoreDataRef (m_pTargetVariable, ArgValueArray [ArgCount - 1]);
	if (!Result)
		return false;

	m_pModule->m_FunctionMgr.InternalEpilogue ();
	return true;
}

//.............................................................................

} // namespace jnc {
