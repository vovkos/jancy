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
	m_finallyBlock = NULL;
}

BasicBlock* 
Scope::getOrCreateCatchBlock ()
{
	ASSERT (m_flags & ScopeFlag_CatchAhead);

	if (m_catchBlock)
		return m_catchBlock;

	m_catchBlock = m_module->m_controlFlowMgr.createBlock ("catch_block", BasicBlockFlag_Catch);
	return m_catchBlock;
}

BasicBlock* 
Scope::getOrCreateFinallyBlock ()
{
	ASSERT (m_flags & ScopeFlag_FinallyAhead);

	if (m_finallyBlock)
		return m_finallyBlock;

	m_finallyBlock = m_module->m_controlFlowMgr.createBlock ("finally_block", BasicBlockFlag_Finally);
	return m_finallyBlock;
}


//.............................................................................

} // namespace jnc {
