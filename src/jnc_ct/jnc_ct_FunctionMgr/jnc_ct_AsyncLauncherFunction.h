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

class AsyncLauncherFunction: public Function { // requires a body to be compiled
	friend class FunctionMgr;

public:
	virtual
	bool
	compile();
};

//..............................................................................

} // namespace ct
} // namespace jnc
