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
		"!enterReactiveStmt",      // ReactorMethod_EnterReactiveStmt,
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
	m_reactor = NULL;
}

bool
ReactorClassType::calcLayout() {
	if (m_body.isEmpty()) {
		err::setFormatStringError("reactor '%s' has no body", getQualifiedName().sz());
		return false;
	}
/*
	// scan for declarations and count reactions

	m_module->m_unitMgr.setCurrentUnit(m_parentUnit);

	Parser parser(m_module, m_pragmaConfig, Parser::Mode_Compile);
	parser.m_reactorType = this;

	Function* prevFunction = m_module->m_functionMgr.setCurrentFunction(m_reactorFunction); // we need some method for OperatorMgr::getThisValueType to work

	ParseContext parseContext(m_module, m_parentUnit, this);
	bool result =
		parser.parseBody(SymbolKind_reactor_body_0, m_bodyPos, m_body) &&
		ClassType::calcLayout();

	m_module->m_functionMgr.setCurrentFunction(prevFunction);
	m_reactionCount = parser.getReactionCount();
	return result; */

	return ClassType::calcLayout();
}

bool
ReactorClassType::prepareForOperatorNew() {
	bool result = ClassType::prepareForOperatorNew();
	if (!result)
		return false;

	// explicitly mark Reactor.reaction for compilation as we don't call it directly
	// it's called from RTL in rtl::ReactorImpl::reactionLoop

	m_module->markForCompile(m_reactor);
	return true;
}

bool
ReactorClassType::compileReaction(Function* function) {
	ASSERT(function == m_reactor);
	ASSERT(!m_body.isEmpty());

	ParseContext parseContext(m_module, m_parentUnit, this);

	Value argValueArray[3];
	size_t argCount = m_parentType ? 3 : 2;
	m_module->m_functionMgr.internalPrologue(function, argValueArray, argCount, &m_bodyPos);

	Parser parser(m_module, m_pragmaConfig, Parser::Mode_Compile);

	m_module->m_controlFlowMgr.enterReactor(this, argValueArray[argCount - 1]);
	bool result = parser.parseBody(SymbolKind_compound_stmt, m_bodyPos, m_body);
	m_module->m_controlFlowMgr.leaveReactor();

	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();

	// explicitly mark all event handlers for compilation as we don't call those directly
	// they are called from RTL in rtl::ReactorImpl::reactionLoop

	size_t count = m_onEventHandlerMap.getCount();
	for (size_t i = 0; i < count; i++) {
		Function* handler = m_onEventHandlerMap[i];
		if (handler)
			m_module->markForCompile(handler);
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
