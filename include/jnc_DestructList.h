// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

//.............................................................................

class CDestructList
{
protected:
	struct TEntry: rtl::TListLink 
	{
		CFunction* m_pDestructor;
		CValue m_ArgValue; // could be null for static destructors
		CVariable* m_pFlagVariable; // could be null for unconditional destructors
	};

protected:
	CModule* m_pModule;
	rtl::CStdListT <TEntry> m_List;

public:
	CDestructList ();

	void
	Clear ()
	{
		m_List.Clear ();
	}

	bool
	IsEmpty ()
	{
		return m_List.IsEmpty ();
	}

	void 
	AddDestructor (
		CFunction* pDestructor,
		const CValue& ArgValue
		)
	{
		AddDestructor (pDestructor, ArgValue, NULL);
	}

	void 
	AddDestructor (
		CFunction* pDestructor,
		const CValue& ArgValue,
		CVariable* pFlagVariable
		);

	void 
	AddStaticDestructor (
		CFunction* pDestructor,
		CVariable* pFlagVariable
		)
	{
		AddDestructor (pDestructor, CValue (), pFlagVariable);
	}

	void
	RunDestructors ();
};

//.............................................................................

} // namespace jnc {
