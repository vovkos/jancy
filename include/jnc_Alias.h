// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ImportType.h"

namespace jnc {

//.............................................................................

class CAlias: 
	public CUserModuleItem,
	public CModuleItemInitializer
{
	friend class CVariableMgr;

protected:
	CType* m_pType;
	CImportType* m_pType_i;

public:
	CAlias ();
	
	CType* 
	GetType ()
	{
		return m_pType;
	}

	CImportType*
	GetType_i ()
	{
		return m_pType_i;
	}

protected:
	virtual
	bool
	CalcLayout ();
};

//.............................................................................

} // namespace jnc {
