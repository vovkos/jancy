// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ClassType.h"

namespace jnc {

//.............................................................................

enum EReactorField
{
	EReactorField_Parent,
	EReactorField_Lock,
	EReactorField_State,
	EReactorField_BindSiteArray,

	EReactorField__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum EReactorMethod
{
	EReactorMethod_Start,
	EReactorMethod_Stop,
	EReactorMethod__Count,
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct TReaction: rtl::TListLink
{
	rtl::CBoxListT <CValue> m_BindSiteList;
	CFunction* m_pFunction;
};

//.............................................................................

class CReactorClassType: public CClassType
{
	friend class CTypeMgr;
	friend class CParser;

protected:
	CStructField* m_FieldArray [EReactorField__Count];
	CFunction* m_MethodArray [EReactorMethod__Count];
	rtl::CIteratorT <CStructField> m_FirstArgField;
	size_t m_BindSiteCount;
	rtl::CBoxListT <CToken> m_Body;

public:
	CReactorClassType ();

	CStructField*
	GetField (EReactorField Field)
	{
		ASSERT (Field < EReactorField__Count);
		return m_FieldArray [Field];
	}

	CFunction*
	GetMethod (EReactorMethod Method)
	{
		ASSERT (Method < EReactorMethod__Count);
		return m_MethodArray [Method];
	}

	size_t
	GetBindSiteCount ()
	{
		return m_BindSiteCount;
	}

	bool
	HasBody ()
	{
		return !m_Body.IsEmpty ();
	}

	rtl::CConstBoxListT <CToken>
	GetBody ()
	{
		return m_Body;
	}

	bool
	SetBody (rtl::CBoxListT <CToken>* pTokenList);

	CFunction*
	CreateHandler (const rtl::CArrayT <CFunctionArg*>& ArgArray = rtl::CArrayT <CFunctionArg*> ());

	bool
	BindHandlers (const rtl::CConstListT <TReaction>& HandlerList);

	virtual
	bool
	Compile ()
	{
		// do not call CClass::Compile (it compiles default-constructor and default-destructor)

		return
			CompilePrimer () &&
			CompileStartMethod () &&
			CompileStopMethod () &&
			CompileConstructor () &&
			CompileDestructor ();
	}

protected:
	virtual
	void
	PrepareTypeString ()
	{
		m_TypeString.Format (
			"reactor %s %s",
			m_QualifiedName.cc (),
			m_MethodArray [EReactorMethod_Start]->GetType ()->GetShortType ()->GetArgString ().cc ()
			);
	}

	virtual
	bool
	CalcLayout ();

	bool
	CallStopMethod ();

	bool
	CompileConstructor ();

	bool
	CompileDestructor ();

	bool
	CompileStartMethod ();

	bool
	CompileStopMethod ();
};

//.............................................................................

// structure backing up reactor bind site in reactor class

struct TReactorBindSite
{
	TIfaceHdr* m_pOnChanged;
	intptr_t m_Cookie;
};

//.............................................................................

} // namespace jnc {
