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

enum ReactorMethod
{
	ReactorMethod_Start,
	ReactorMethod_Stop,
	ReactorMethod_Restart,
	ReactorMethod_AddOnChangedBinding,
	ReactorMethod_AddOnEventBinding,
	ReactorMethod_ResetOnChangedBindings,
	ReactorMethod__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

Function*
getReactorMethod (
	Module* module,
	ReactorMethod method
	);

//..............................................................................

class ReactorClassType: public ClassType
{
	friend class TypeMgr;
	friend class ClassType;
	friend class Parser;

protected:
	ClassType* m_parentType;
	size_t m_parentOffset;
	size_t m_reactionCount;
	Function* m_reaction;
	sl::HashTable <size_t, Function*, sl::HashId <size_t> > m_onEventMap;
	sl::BoxList <Token> m_body;

public:
	ReactorClassType ();

	ClassType*
	getParentType ()
	{
		return m_parentType;
	}

	size_t
	getParentOffset ()
	{
		return m_parentOffset;
	}

	size_t
	getReactionCount ()
	{
		return m_reactionCount;
	}

	Function*
	getReaction ()
	{
		return m_reaction;
	}

	Function*
	createOnEventHandler (
		size_t reactionIdx,
		FunctionType* type
		);

	Function*
	findOnEventHandler (size_t reactionIdx)
	{
		return m_onEventMap.findValue (reactionIdx, NULL);
	}

	bool
	hasBody ()
	{
		return !m_body.isEmpty ();
	}

	sl::ConstBoxList <Token>
	getBody ()
	{
		return m_body;
	}

	bool
	setBody (sl::BoxList <Token>* tokenList);

protected:
	virtual
	bool
	calcLayout ();

	virtual
	bool
	compile ();
};

//..............................................................................

inline
bool
isReactorClassTypeMember (ModuleItemDecl* itemDecl)
{
	Namespace* nspace = itemDecl->getParentNamespace ();
	return
		nspace->getNamespaceKind () == NamespaceKind_Type &&
		isClassType ((ClassType*) nspace, ClassTypeKind_Reactor);
}

//.............................................................................

} // namespace ct
} // namespace jnc
