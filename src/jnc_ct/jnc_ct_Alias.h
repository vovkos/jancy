// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_ImportType.h"

namespace jnc {
namespace ct {

//.............................................................................

class Alias: 
	public UserModuleItem,
	public ModuleItemInitializer
{
	friend class VariableMgr;

protected:
	Type* m_type;
	ImportType* m_type_i;

public:
	Alias ();
	
	Type* 
	getType ()
	{
		return m_type;
	}

	ImportType*
	getType_i ()
	{
		return m_type_i;
	}

protected:
	virtual
	bool
	calcLayout ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
