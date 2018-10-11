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
#include "jnc_ct_ReactorClassType.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_Parser.llk.h"

namespace jnc {
namespace ct {

//..............................................................................

ReactorClassType::ReactorClassType ()
{
	m_classTypeKind = ClassTypeKind_Reactor;
	m_parentType = NULL;
	m_parentOffset = 0;
	m_reactionCount = 0;
	m_reaction = NULL;
}

Function*
ReactorClassType::createOnEventHandler (
	size_t reactionIdx,
	FunctionType* type
	)
{
	Function* function = createUnnamedMethod (StorageKind_Member, FunctionKind_Internal, type);

	sl::HashTableIterator <size_t, Function*> it = m_onEventMap.visit (reactionIdx);

	ASSERT (!it->m_value);
	it->m_value = function;

	return function;
}

bool
ReactorClassType::setBody (sl::BoxList <Token>* tokenList)
{
	if (!m_body.isEmpty ())
	{
		err::setFormatStringError ("'%s' already has a body", m_tag.sz ());
		return false;
	}

	sl::takeOver (&m_body, tokenList);
	m_module->markForCompile (this);
	return true;
}

bool
ReactorClassType::calcLayout ()
{
	bool result;

	if (m_body.isEmpty ())
	{
		err::setFormatStringError ("reactor '%s' has no body", m_tag.sz ());
		return false;
	}

	// scan for declarations and count

	Parser parser (m_module);
	parser.m_stage = Parser::Stage_Pass2;
	parser.m_reactorType = this;

	m_module->m_namespaceMgr.openNamespace (this);

	result = parser.parseTokenList (SymbolKind_reactor_body_0, m_body, false);
	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace ();

	if (!parser.m_reactionIdx)
	{
		err::setFormatStringError ("reactor '%s' has no reactions", m_tag.sz ());
		return false;
	}

	m_reactionCount = parser.m_reactionIdx;

	return ClassType::calcLayout ();
}

bool
ReactorClassType::compile ()
{
	bool result;

	Value argValue;
	m_module->m_functionMgr.internalPrologue (m_reaction, &argValue, 1);

	Value thisValue = m_module->m_functionMgr.getThisValue ();
	ASSERT (thisValue);

	Parser parser (m_module);
	parser.m_stage = Parser::Stage_Reaction;
	parser.m_reactorType = this;
	parser.m_reactionIdxArgValue = argValue;

	result = parser.parseTokenList (SymbolKind_reactor_body, m_body, true);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
