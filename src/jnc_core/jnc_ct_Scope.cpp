#include "pch.h"
#include "jnc_ct_Scope.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

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
	m_finallyBlock = NULL;
	m_firstStackVariable = NULL;
	m_gcShadowStackFrameMap = NULL;
	m_firstStackVariable = NULL;
}

//.............................................................................

} // namespace ct
} // namespace jnc
