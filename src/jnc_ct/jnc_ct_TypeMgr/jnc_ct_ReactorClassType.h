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
	public:
		Reactor() {
			m_functionKind = FunctionKind_Reactor;
			m_flags |= ModuleItemFlag_User;
		}

		virtual
		bool
		compile() {
			return ((ReactorClassType*)m_parentNamespace)->compileReaction(this);
		}
	};

protected:
	ClassType* m_parentType;
	size_t m_parentOffset;
	size_t m_reactionCount;
	Function* m_reactor;
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
		return m_reactor;
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

protected:
	virtual
	bool
	calcLayout();

	virtual
	void
	prepareTypeString() {
		getTypeStringTuple()->m_typeStringPrefix = "reactor";
	}

	virtual
	void
	prepareDoxyLinkedText() {
		getTypeStringTuple()->m_doxyLinkedTextPrefix = "reactor";
	}

	virtual
	bool
	prepareForOperatorNew();

	bool
	compileReaction(Function* function);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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

//..............................................................................

inline
bool
isReactorClassTypeMember(ModuleItemDecl* itemDecl) {
	Namespace* nspace = itemDecl->getParentNamespace();
	return
		nspace->getNamespaceKind() == NamespaceKind_Type &&
		isClassType((ClassType*)nspace, ClassTypeKind_Reactor);
}

//.............................................................................

} // namespace ct
} // namespace jnc
