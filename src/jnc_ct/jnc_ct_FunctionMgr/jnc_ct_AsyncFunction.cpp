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
#include "jnc_ct_AsyncFunction.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
AsyncFunction::compile ()
{
	ASSERT (m_parentUnit && m_parentNamespace);

	bool result;

	m_module->m_unitMgr.setCurrentUnit (m_parentUnit);
	m_module->m_namespaceMgr.openNamespace (m_parentNamespace);

	Value promiseValue;
	m_module->m_functionMgr.internalPrologue (this, &promiseValue, 1, &m_body.getHead ()->m_pos);
	m_module->m_functionMgr.m_promiseValue = promiseValue;

	// extract 'this' pointer and adjust m_thisValue
	// m_module->m_functionMgr.m_thisValue = promiseValue;

	m_module->m_namespaceMgr.openScope (
		m_body.getHead ()->m_pos,
		ScopeFlag_CatchAhead | ScopeFlag_HasCatch
		);

	// extract state and then jump accordingly

	BasicBlock* initialBlock = m_module->m_controlFlowMgr.createAsyncBlock ();
	m_module->m_controlFlowMgr.follow (initialBlock);

	Parser parser (m_module);
	parser.m_stage = Parser::Stage_Pass2;

	result =
		parser.parseTokenList (SymbolKind_compound_stmt, m_body, true) &&
		m_module->m_controlFlowMgr.checkReturn () &&
		m_module->m_controlFlowMgr.catchLabel (m_body.getTail ()->m_pos);

	if (!result)
		return false;

	Function* throwFunc = m_module->m_functionMgr.getStdFunction (StdFunc_AsyncThrow);
	m_module->m_llvmIrBuilder.createCall (throwFunc, throwFunc->getType (), promiseValue, NULL);
	m_module->m_controlFlowMgr.asyncRet (NULL);
	m_module->m_namespaceMgr.closeScope ();
	m_module->m_functionMgr.internalEpilogue ();
	m_module->m_namespaceMgr.closeNamespace ();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
