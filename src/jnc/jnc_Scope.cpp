#include "pch.h"
#include "jnc_Scope.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CScope::CScope ()
{
	m_ItemKind = EModuleItem_Scope;
	m_NamespaceKind = ENamespace_Scope;
	m_pItemDecl = this;
	m_Level = 1; // 0 is global
	m_pFunction = NULL;
	m_pBreakBlock = NULL;
	m_pContinueBlock = NULL;
	m_pCatchBlock = NULL;
	m_pFinallyBlock = NULL;
	m_pFinallyReturnAddress = NULL;
}

//.............................................................................

} // namespace jnc {
