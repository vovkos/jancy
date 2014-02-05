#include "pch.h"
#include "jnc_PropertyTemplate.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CPropertyTemplate::CPropertyTemplate ()
{
	m_ItemKind = EModuleItem_PropertyTemplate;
	m_NamespaceKind = ENamespace_PropertyTemplate;
	m_pItemDecl = this;
	m_pGetterType = NULL;
	m_TypeFlags = 0;
}

bool
CPropertyTemplate::AddMethod (
	EFunction FunctionKind,
	CFunctionType* pFunctionType
	)
{
	bool Result;

	if (FunctionKind != EFunction_Getter && FunctionKind != EFunction_Setter)
	{
		err::SetFormatStringError ("property templates can only have accessors");
		return false;
	}

	if (FunctionKind == EFunction_Getter)
	{
		Result = m_Verifier.CheckGetter (pFunctionType);
		if (!Result)
			return false;

		if (m_pGetterType)
		{
			err::SetFormatStringError ("property template already has a getter");
			return false;
		}

		m_pGetterType = pFunctionType;
	}
	else
	{
		Result = 
			m_Verifier.CheckSetter (pFunctionType) &&
			m_SetterType.AddOverload (pFunctionType);

		if (!Result)
			return false;
	}

	return true;
}

CPropertyType*
CPropertyTemplate::CalcType ()
{
	if (!m_pGetterType)
	{
		err::SetFormatStringError ("incomplete property template: no 'get' method");
		return NULL;
	}

	return m_pModule->m_TypeMgr.GetPropertyType (m_pGetterType, m_SetterType, m_TypeFlags);
}

//.............................................................................

} // namespace jnc {
