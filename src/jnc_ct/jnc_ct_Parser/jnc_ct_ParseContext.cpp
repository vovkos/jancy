//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_ct_ParseContext.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

void
ParseContext::set(
	ParseContextKind contextKind,
	Module* module,
	Unit* unit,
	Namespace* nspace
) {
	m_module = module;
	m_prevUnit = module->m_unitMgr.setCurrentUnit(unit);
	m_prevReactorBody = module->m_controlFlowMgr.setCurrentReactor(NULL);

	if (contextKind != ParseContextKind_Expression)
		m_isNamespaceOpened = module->m_namespaceMgr.openNamespaceIf(nspace);
	else { // preserve scope for expressions
		Scope* scope = module->m_namespaceMgr.getCurrentScope();
		m_isNamespaceOpened = module->m_namespaceMgr.openNamespaceIf(nspace);
		module->m_namespaceMgr.m_currentScope = scope;
	}
}

void
ParseContext::restore() {
	m_module->m_unitMgr.setCurrentUnit(m_prevUnit);
	m_module->m_controlFlowMgr.setCurrentReactor(m_prevReactorBody);
	if (m_isNamespaceOpened)
		m_module->m_namespaceMgr.closeNamespace();
}

//..............................................................................

} // namespace ct
} // namespace jnc
