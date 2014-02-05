// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Type.h"

namespace jnc {

//.............................................................................

class CAlias: public CUserModuleItem
{
	friend class CVariableMgr;

protected:
	CType* m_pType;
	rtl::CBoxListT <CToken> m_Initializer;
	rtl::CString m_InitializerString;

public:
	CAlias ()
	{
		m_ItemKind = EModuleItem_Alias;
		m_pType = NULL;
	}
	
	CType* 
	GetType ()
	{
		return m_pType;
	}

	rtl::CConstBoxListT <CToken> 
	GetInitializer ()
	{
		return m_Initializer;
	}

	rtl::CString 
	GetInitializerString ()
	{
		if (m_InitializerString.IsEmpty ())
			m_InitializerString = CToken::GetTokenListString (m_Initializer);

		return m_InitializerString;
	}
};

//.............................................................................

} // namespace jnc {
