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

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace rtl {

class Promise;

JNC_DECLARE_CLASS_TYPE(SchedulerImpl)

//..............................................................................

class SchedulerImpl: public Scheduler
{
public:
	Promise*
	JNC_CDECL
	asyncWait();
};

//..............................................................................

} // namespace rtl
} // namespace jnc
