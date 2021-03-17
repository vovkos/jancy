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

#include "jnc_sys_EventBase.h"

namespace jnc {
namespace sys {

JNC_DECLARE_OPAQUE_CLASS_TYPE(NotificationEvent)

//..............................................................................

typedef EventBase<axl::sys::NotificationEvent> NotificationEvent;

//..............................................................................

} // namespace sys
} // namespace jnc
