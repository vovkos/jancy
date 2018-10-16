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
#include "jnc_ct_Scope.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

Scope::Scope ()
{
	m_itemKind = ModuleItemKind_Scope;
	m_namespaceKind = NamespaceKind_Scope;
	m_function = NULL;
	m_breakBlock = NULL;
	m_continueBlock = NULL;
	m_catchBlock = NULL;
	m_finallyBlock = NULL;
	m_tryExpr = NULL;
	m_firstStackVariable = NULL;
	m_disposeLevelVariable = NULL;
	m_gcShadowStackFrameMap = NULL;
	m_sjljFrameIdx = -1;
}

bool
Scope::canStaticThrow ()
{
	return
		m_tryExpr != NULL ||
		(m_flags & ScopeFlag_HasCatch)  ||
		(m_function->getType ()->getFlags () & FunctionTypeFlag_ErrorCode);
}

GcShadowStackFrameMap*
Scope::findGcShadowStackFrameMap ()
{
	if (m_flags & ScopeFlag_FrameMapCached)
		return m_gcShadowStackFrameMap;

	if (!m_gcShadowStackFrameMap)
	{
		Scope* scope = getParentScope ();
		for (; scope; scope = scope->getParentScope ())
			if (scope->m_gcShadowStackFrameMap)
			{
				m_gcShadowStackFrameMap = scope->m_gcShadowStackFrameMap;
				break;
			}
	}

	m_flags |= ScopeFlag_FrameMapCached;
	return m_gcShadowStackFrameMap;
}

//..............................................................................

} // namespace ct
} // namespace jnc
