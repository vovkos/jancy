// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

//.............................................................................

class ScheduleLauncherFunction: public Function
{
	friend class FunctionMgr;

protected:
	sl::String m_signature;

public:
	ScheduleLauncherFunction ()
	{
		m_functionKind = FunctionKind_ScheduleLauncher;
	}

	virtual
	bool 
	compile ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
