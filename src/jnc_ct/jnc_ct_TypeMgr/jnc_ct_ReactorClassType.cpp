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
#include "jnc_ct_ParseContext.h"
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
	Function* function = reactorType->getMethodArray()[method];
	ASSERT(function->getName() == nameTable[method]);
	return function;
}

//..............................................................................

bool
ReactorClassType::Reactor::compile() {
	ReactorClassType* reactorType = (ReactorClassType*)m_parentNamespace;
	const sl::StringRef& body = reactorType->getBody();
	const lex::LineColOffset& bodyPos = reactorType->getBodyPos();

	ASSERT(!body.isEmpty());

	ParseContext parseContext(ParseContextKind_Body, m_module, m_parentUnit, reactorType);
	Parser parser(m_module, m_pragmaConfig, Parser::Mode_Compile);

	Value argValueArray[2];
	m_module->m_functionMgr.internalPrologue(this, argValueArray, countof(argValueArray), &bodyPos);
	m_module->m_controlFlowMgr.enterReactor(reactorType, argValueArray[1]);

	bool result =
		parser.parseBody(SymbolKind_compound_stmt, bodyPos, body) &&
		m_module->m_controlFlowMgr.leaveReactor();

	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue();
	return true;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

sl::StringRef
ReactorClassType::createItemString(size_t index) {
	switch (index) {
	case TypeStringKind_Prefix:
	case TypeStringKind_DoxyLinkedTextPrefix:
		return "reactor";

	default:
		return ClassType::createItemString(index);
	}
}

bool
ReactorClassType::calcLayout() {
	if (m_body.isEmpty()) {
		err::setFormatStringError("reactor '%s' has no body", getItemName().sz());
		return false;
	}

	return ClassType::calcLayout();
}

bool
ReactorClassType::prepareForOperatorNew() {
	bool result = ClassType::prepareForOperatorNew();
	if (!result)
		return false;

	// explicitly mark react() for compilation as we don't call it directly
	// it's called from RTL in rtl::ReactorImpl::reactionLoop

	m_module->markForCompile(getReactor());

	// explicitly mark all event handlers for compilation as we don't call those directly
	// they are called from RTL in rtl::ReactorImpl::reactionLoop

	size_t count = m_onEventHandlerMap.getCount();
	for (size_t i = 0; i < count; i++) {
		Function* handler = m_onEventHandlerMap[i];
		if (handler)
			m_module->markForCompile(handler);
	}

	// ensure closure type is created -- it's used in rtl::ReactorImpl::reactionLoop
	m_module->m_typeMgr.getStdType(StdType_ReactorClosure);
	return true;
}

Function*
ReactorClassType::createReactor() {
	ASSERT(!m_reactor);

	Type* voidType = m_module->m_typeMgr.getPrimitiveType(TypeKind_Void);
	Type* sizeType = m_module->m_typeMgr.getPrimitiveType(TypeKind_SizeT);
	FunctionType* functionType = m_module->m_typeMgr.getFunctionType(voidType, (Type**)&sizeType, 1);
	m_reactor = createMethod<Reactor>("!react", functionType);
	m_reactor->m_parentUnit = m_parentUnit;
	return m_reactor;
}

//..............................................................................

} // namespace ct
} // namespace jnc
