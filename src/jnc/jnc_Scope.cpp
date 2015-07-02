#include "pch.h"
#include "jnc_Scope.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

Scope::Scope ()
{
	m_itemKind = ModuleItemKind_Scope;
	m_namespaceKind = NamespaceKind_Scope;
	m_itemDecl = this;
	m_function = NULL;
	m_breakBlock = NULL;
	m_continueBlock = NULL;
	m_catchBlock = NULL;
	m_catchFollowBlock = NULL;
	m_finallyBlock = NULL;
	m_finallyReturnAddress = NULL;
}

//.............................................................................

} // namespace jnc {
