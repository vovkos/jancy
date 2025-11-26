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

#pragma once

#include "jnc_ct_ClassType.h"

namespace jnc {
namespace ct {

//..............................................................................

enum ReactorMethod {
	ReactorMethod_Start,
	ReactorMethod_Stop,
	ReactorMethod_Restart,
	ReactorMethod_AddOnChangedBinding,
	ReactorMethod_AddOnEventBinding,
	ReactorMethod_EnterReactiveStmt,
	ReactorMethod__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

Function*
getReactorMethod(
	Module* module,
	ReactorMethod method
);

//..............................................................................

class ReactorClassType: public ClassType {
	friend class TypeMgr;
	friend class ControlFlowMgr;
	friend class ClassType;
	friend class Parser;

protected:
	class Reactor: public CompilableFunction {
		friend class ReactorClassType;

	public:
		Reactor() {
			m_functionKind = FunctionKind_Reactor;
			m_flags |= ModuleItemFlag_User;
		}

		virtual
		bool
		compile();
	};

protected:
	ClassType* m_parentType;
	size_t m_parentOffset;
	size_t m_reactionCount;
	Reactor* m_reactor;
	ClassType* m_userDataType;
	sl::Array<Function*> m_onEventHandlerMap;

public:
	ReactorClassType();

	ClassType*
	getParentType() {
		return m_parentType;
	}

	size_t
	getParentOffset() {
		return m_parentOffset;
	}

	size_t
	getReactionCount() {
		return m_reactionCount;
	}

	Function*
	getReactor() {
		return m_reactor ? m_reactor : createReactor();
	}

	void
	addOnEventHandler(
		size_t reactionIdx,
		Function* function
	);

	Function*
	getOnEventHandler(size_t reactionIdx) {
		return m_onEventHandlerMap[reactionIdx];
	}

	ClassType*
	getUserDataType() {
		return m_userDataType;
	}

protected:
	virtual
	sl::StringRef
	createItemString(size_t index);

	virtual
	bool
	calcLayout();

	virtual
	bool
	prepareForOperatorNew();

	Function*
	createReactor();
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
ReactorClassType::ReactorClassType() {
	m_classTypeKind = ClassTypeKind_Reactor;
	m_namespaceStatus = NamespaceStatus_Ready;
	m_parentType = NULL;
	m_parentOffset = 0;
	m_reactionCount = 0;
	m_reactor = NULL;
	m_userDataType = NULL;
}

inline
void
ReactorClassType::addOnEventHandler(
	size_t reactionIdx,
	Function* function
) {
	if (m_onEventHandlerMap.getCount() <= reactionIdx)
		m_onEventHandlerMap.setCountZeroConstruct(reactionIdx + 1);

	m_onEventHandlerMap.rwi()[reactionIdx] = function;
}

//.............................................................................

} // namespace ct
} // namespace jnc
