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

enum ReactorFieldKind
{
	ReactorFieldKind_Parent,
	ReactorFieldKind_Lock,
	ReactorFieldKind_State,
	ReactorFieldKind_BindSiteArray,
	ReactorFieldKind_ReactionStateArray,

	ReactorFieldKind__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ReactorMethodKind
{
	ReactorMethodKind_Start,
	ReactorMethodKind_Stop,
	ReactorMethodKind__Count,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Reaction: sl::ListLink
{
	sl::BoxList <Value> m_bindSiteList;
	Function* m_function;
};

//..............................................................................

class ReactorClassType: public ClassType
{
	friend class TypeMgr;
	friend class Parser;

protected:
	StructField* m_fieldArray [ReactorFieldKind__Count];
	Function* m_methodArray [ReactorMethodKind__Count];
	sl::Iterator <StructField> m_firstArgField;
	size_t m_bindSiteCount;
	size_t m_reactionCount;
	sl::BoxList <Token> m_body;

public:
	ReactorClassType ();

	StructField*
	getField (ReactorFieldKind field)
	{
		ASSERT (field < ReactorFieldKind__Count);
		return m_fieldArray [field];
	}

	Function*
	getMethod (ReactorMethodKind method)
	{
		ASSERT (method < ReactorMethodKind__Count);
		return m_methodArray [method];
	}

	size_t
	getBindSiteCount ()
	{
		return m_bindSiteCount;
	}

	size_t
	getReactionCount ()
	{
		return m_reactionCount;
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

	bool
	subscribe (const sl::ConstList <Reaction>& reactionList);

	virtual
	bool
	compile ()
	{
		// do not call CClass::Compile (it compiles default-constructor and default-destructor)

		return
			compileStartMethod () &&
			compileStopMethod () &&
			compileConstructor () &&
			compileDestructor ();
	}

protected:
	virtual
	void
	prepareTypeString ();

	virtual
	void
	prepareDoxyLinkedText ();

	virtual
	bool
	calcLayout ();

	bool
	callStopMethod ();

	bool
	compileConstructor ();

	bool
	compileDestructor ();

	bool
	compileStartMethod ();

	bool
	compileStopMethod ();
};

//..............................................................................

} // namespace ct
} // namespace jnc
