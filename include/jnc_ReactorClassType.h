// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ClassType.h"

namespace jnc {

//.............................................................................

enum ReactorFieldKind
{
	ReactorFieldKind_Parent,
	ReactorFieldKind_Lock,
	ReactorFieldKind_State,
	ReactorFieldKind_BindSiteArray,
	ReactorFieldKind_ReactionStateArray,

	ReactorFieldKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ReactorMethodKind
{
	ReactorMethodKind_Start,
	ReactorMethodKind_Stop,
	ReactorMethodKind__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Reaction: rtl::ListLink
{
	rtl::BoxList <Value> m_bindSiteList;
	Function* m_function;
};

//.............................................................................

class ReactorClassType: public ClassType
{
	friend class TypeMgr;
	friend class Parser;

protected:
	StructField* m_fieldArray [ReactorFieldKind__Count];
	Function* m_methodArray [ReactorMethodKind__Count];
	rtl::Iterator <StructField> m_firstArgField;
	size_t m_bindSiteCount;
	size_t m_reactionCount;
	rtl::BoxList <Token> m_body;

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

	rtl::ConstBoxList <Token>
	getBody ()
	{
		return m_body;
	}

	bool
	setBody (rtl::BoxList <Token>* tokenList);

	bool
	subscribe (const rtl::ConstList <Reaction>& reactionList);

	virtual
	bool
	compile ()
	{
		// do not call CClass::Compile (it compiles default-constructor and default-destructor)

		return
			compilePrimer () &&
			compileStartMethod () &&
			compileStopMethod () &&
			compileConstructor () &&
			compileDestructor ();
	}

protected:
	virtual
	void
	prepareTypeString ()
	{
		m_typeString.format (
			"reactor %s %s",
			m_qualifiedName.cc (),
			m_methodArray [ReactorMethodKind_Start]->getType ()->getShortType ()->getArgString ().cc ()
			);
	}

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

//.............................................................................

// structure backing up reactor bind site in reactor class

struct ReactorBindSite
{
	IfaceHdr* m_onChanged;
	intptr_t m_cookie;
};

//.............................................................................

} // namespace jnc {
