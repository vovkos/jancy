//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_ct_Function.h"

namespace jnc {
namespace ct {

//..............................................................................

class AsyncSchedLauncherFunction: public Function
{
	friend class FunctionMgr;

public:
	AsyncSchedLauncherFunction()
	{
		m_functionKind = FunctionKind_AsyncSchedLauncher;
	}

	virtual
	bool
	compile();
};

//..............................................................................

} // namespace ct
} // namespace jnc
