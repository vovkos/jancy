// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_Function.h"

namespace jnc {

//.............................................................................

class CScheduleLauncherFunction: public CFunction
{
	friend class CFunctionMgr;

protected:
	rtl::CString m_Signature;

public:
	CScheduleLauncherFunction ()
	{
		m_FunctionKind = EFunction_ScheduleLauncher;
	}

	virtual
	bool 
	Compile ();
};

//.............................................................................

} // namespace jnc {
