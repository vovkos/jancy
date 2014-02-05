// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_AttributeBlock.h"

namespace jnc {

class CModule;

//.............................................................................

class CAttributeMgr: public rtl::TListLink
{
	friend class CModule;

protected:
	CModule* m_pModule;

	rtl::CStdListT <CAttributeBlock> m_AttributeBlockList;

public:
	CAttributeMgr ();

	CModule* 
	GetModule ()
	{
		return m_pModule;
	}

	void
	Clear ()
	{
		m_AttributeBlockList.Clear ();
	}

	CAttributeBlock*
	CreateAttributeBlock ();

	CAttribute*
	CreateAttribute (
		const rtl::CString& Name,
		CValue* pValue = NULL
		);
};

//.............................................................................

} // namespace jnc {
