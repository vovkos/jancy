// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Value.h"

namespace jnc {

class CModule;

//.............................................................................

class CScopeLevelStack
{
	friend class CNamespaceMgr;

	struct TEntry: rtl::TListLink
	{
		CValue m_ScopeLevelValue;
		CValue m_ObjHdrValue;
	};

protected:
	CModule* m_pModule;

	rtl::CStdListT <TEntry> m_List;
	rtl::CArrayT <TEntry*> m_Stack;

public:
	CScopeLevelStack ()
	{
		m_pModule = NULL;
	}

	void
	Clear ()
	{
		m_List.Clear ();
		m_Stack.Clear ();
	}

	void
	TakeOver (CScopeLevelStack* pSrcStack);

	CValue
	GetScopeLevel (size_t Level);

	CValue
	GetObjHdr (size_t Level);

protected:
	TEntry*
	GetEntry (size_t Level);
};

//.............................................................................

} // namespace jnc {
