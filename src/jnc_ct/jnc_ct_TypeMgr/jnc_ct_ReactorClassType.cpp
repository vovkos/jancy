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

Function*
getReactorMethod(
	Module* module,
	ReactorMethod method
	)
{
	static const char* nameTable[ReactorMethod__Count] =
	{
		"start",                   // ReactorMethod_Start,
		"stop",                    // ReactorMethod_Stop,
		"restart",                 // ReactorMethod_Restart,
		"!addOnChangedBinding",    // ReactorMethod_AddOnChangedBinding,
		"!addOnEventBinding",      // ReactorMethod_AddOnEventBinding,
		"!resetOnChangedBindings", // ReactorMethod_ResetOnChangedBindings,
	};

	ASSERT(method < countof(nameTable));

	ClassType* reactorType = (ClassType*)module->m_typeMgr.getStdType(StdType_ReactorBase);
	Function* function = reactorType->getMemberMethodArray() [method];
	ASSERT(function->getName() == nameTable[method]);

	return function;
}

//..............................................................................

ReactorClassType::ReactorClassType()
{
	m_classTypeKind = ClassTypeKind_Reactor;
	m_parentType = NULL;
	m_parentOffset = 0;
	m_reactionCount = 0;
	m_reaction = NULL;
}

Function*
ReactorClassType::createOnEventHandler(
	size_t reactionIdx,
	FunctionType* type
	)
{
	Function* function = createUnnamedMethod(StorageKind_Member, FunctionKind_Internal, type);

	sl::HashTableIterator<size_t, Function*> it = m_onEventMap.visit(reactionIdx);

	ASSERT(!it->m_value);
	it->m_value = function;

	return function;
}

bool
ReactorClassType::setBody(sl::BoxList<Token>* tokenList)
{
	if (!m_body.isEmpty())
	{
		err::setFormatStringError("'%s' already has a body", m_qualifiedName.sz());
		return false;
	}

	sl::takeOver(&m_body, tokenList);
	m_module->markForCompile(this);
	return true;
}

bool
ReactorClassType::calcLayout()
{
	bool result;

	if (m_body.isEmpty())
	{
		err::setFormatStringError("reactor '%s' has no body", m_qualifiedName.sz());
		return false;
	}

	// scan for declarations and count

	Parser parser(m_module);
	parser.m_stage = Parser::Stage_Pass2;
	parser.m_reactorType = this;

	Function* prevFunction = m_module->m_functionMgr.setCurrentFunction(m_reaction); // we need some method for OperatorMgr::getThisValueType to work
	m_module->m_namespaceMgr.openNamespace(this);

	result = parser.parseTokenList(SymbolKind_reactor_body_0, m_body, false);
	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace();
	m_module->m_functionMgr.setCurrentFunction(prevFunction);
	m_reactionCount = parser.m_reactionIdx;

	return ClassType::calcLayout();
}

bool
ReactorClassType::compile()
{
	bool result = ClassType::compile(); // compile default constructor & destructor
	if (!result)
		return false;

	ASSERT(!m_body.isEmpty());
	const Token::Pos* pos = &m_body.getHead()->m_pos;

	if (m_parentUnit)
		m_module->m_unitMgr.setCurrentUnit(m_parentUnit);

	Value argValueArray[2];
	m_module->m_namespaceMgr.openNamespace(this);
	m_module->m_functionMgr.internalPrologue(m_reaction, argValueArray, countof(argValueArray), pos);

	Parser parser(m_module);
	parser.m_stage = Parser::Stage_Reaction;
	parser.m_reactorType = this;
	parser.m_reactionIdxArgValue = argValueArray[1];

	result = parser.parseTokenList(SymbolKind_reactor_body, m_body, true);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	m_module->m_namespaceMgr.closeNamespace();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
