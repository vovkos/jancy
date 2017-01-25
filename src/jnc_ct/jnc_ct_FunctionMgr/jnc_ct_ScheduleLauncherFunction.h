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

//..............................................................................

} // namespace ct
} // namespace jnc
