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
	Function* function = reactorType->getMethodArray() [method];
	ASSERT(function->getName() == nameTable[method]);

	return function;
}

//..............................................................................

ReactorClassType::ReactorClassType()
{
	m_classTypeKind = ClassTypeKind_Reactor;
	m_namespaceStatus = NamespaceStatus_Ready;
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
	Function* function = createUnnamedMethod(FunctionKind_Internal, type);

	sl::HashTableIterator<size_t, Function*> it = m_onEventMap.visit(reactionIdx);

	ASSERT(!it->m_value);
	it->m_value = function;

	return function;
}

bool
ReactorClassType::calcLayout()
{
	bool result;

	if (m_body.isEmpty())
	{
		err::setFormatStringError("reactor '%s' has no body", getQualifiedName().sz());
		return false;
	}

	// scan for declarations and count reactions

	m_module->m_unitMgr.setCurrentUnit(m_parentUnit);

	Parser parser(m_module, Parser::Mode_Compile);
	parser.m_reactorType = this;

	Function* prevFunction = m_module->m_functionMgr.setCurrentFunction(m_reaction); // we need some method for OperatorMgr::getThisValueType to work
	m_module->m_namespaceMgr.openNamespace(this);

	result = parser.parseBody(SymbolKind_reactor_body_0, m_bodyPos, m_body);
	if (!result)
		return false;

	m_module->m_namespaceMgr.closeNamespace();
	m_module->m_functionMgr.setCurrentFunction(prevFunction);
	m_reactionCount = parser.m_reactionIdx;

	return ClassType::calcLayout();
}

bool
ReactorClassType::prepareForOperatorNew()
{
	bool result = ClassType::prepareForOperatorNew();
	if (!result)
		return false;

	// explicitly mark Reactor.reaction for compilation as we don't call it directly
	// it's called from RTL in rtl::ReactorImpl::reactionLoop

	m_module->markForCompile(m_reaction);
	return true;
}

bool
ReactorClassType::compileReaction(Function* function)
{
	ASSERT(function == m_reaction);
	ASSERT(!m_body.isEmpty());

	if (m_parentUnit)
		m_module->m_unitMgr.setCurrentUnit(m_parentUnit);

	Value argValueArray[2];
	m_module->m_namespaceMgr.openNamespace(this);
	m_module->m_functionMgr.internalPrologue(function, argValueArray, countof(argValueArray), &m_bodyPos);

	Parser parser(m_module, Parser::Mode_Reaction);
	parser.m_reactorType = this;
	parser.m_reactionIdxArgValue = argValueArray[1];

	bool result = parser.parseBody(SymbolKind_reactor_body, m_bodyPos, m_body);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	m_module->m_namespaceMgr.closeNamespace();

	// explicitly mark all event handlers for compilation as we don't call those directly
	// they are called from RTL in rtl::ReactorImpl::reactionLoop

	sl::MapIterator<size_t, Function*> it = m_onEventMap.getHead();
	for (; it; it++)
		m_module->markForCompile(it->m_value);

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
