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
) {
	static const char* nameTable[ReactorMethod__Count] = {
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

ReactorClassType::ReactorClassType() {
	m_classTypeKind = ClassTypeKind_Reactor;
	m_namespaceStatus = NamespaceStatus_Ready;
	m_parentType = NULL;
	m_parentOffset = 0;
	m_reactionCount = 0;
	m_reaction = NULL;
}

bool
ReactorClassType::calcLayout() {
	if (m_body.isEmpty()) {
		err::setFormatStringError("reactor '%s' has no body", getQualifiedName().sz());
		return false;
	}

	// scan for declarations and count reactions

	m_module->m_unitMgr.setCurrentUnit(m_parentUnit);

	Parser parser(m_module, m_pragmaConfig, Parser::Mode_Compile);
	parser.m_reactorType = this;

	Function* prevFunction = m_module->m_functionMgr.setCurrentFunction(m_reaction); // we need some method for OperatorMgr::getThisValueType to work

	ParseContext parseContext(m_module, m_parentUnit, this);
	bool result =
		parser.parseBody(SymbolKind_reactor_body_0, m_bodyPos, m_body) &&
		ClassType::calcLayout();

	m_module->m_functionMgr.setCurrentFunction(prevFunction);
	m_reactionCount = parser.getReactionCount();
	return result;
}

bool
ReactorClassType::prepareForOperatorNew() {
	bool result = ClassType::prepareForOperatorNew();
	if (!result)
		return false;

	// explicitly mark Reactor.reaction for compilation as we don't call it directly
	// it's called from RTL in rtl::ReactorImpl::reactionLoop

	m_module->markForCompile(m_reaction);
	return true;
}

bool
ReactorClassType::compileReaction(Function* function) {
	ASSERT(function == m_reaction);
	ASSERT(!m_body.isEmpty());

	ParseContext parseContext(m_module, m_parentUnit, this);

	Value argValueArray[2];
	m_module->m_functionMgr.internalPrologue(function, argValueArray, countof(argValueArray), &m_bodyPos);

	Parser parser(m_module, m_pragmaConfig, Parser::Mode_Reaction);
	parser.m_reactorType = this;
	parser.m_reactionIdxArgValue = argValueArray[1];

	bool result = parser.parseBody(SymbolKind_reactor_body, m_bodyPos, m_body);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();

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
