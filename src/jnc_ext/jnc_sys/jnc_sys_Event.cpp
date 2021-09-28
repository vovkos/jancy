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

#include "pch.h"
#include "jnc_sys_Event.h"
#include "jnc_sys_SysLib.h"
#include "jnc_Runtime.h"

namespace jnc {
namespace sys {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Event,
	"sys.Event",
	g_sysLibGuid,
	SysLibCacheSlot_Event,
	Event,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Event)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<Event>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<Event>)
	JNC_MAP_FUNCTION("signal", &Event::signal)
	JNC_MAP_FUNCTION("reset", &Event::reset)
	JNC_MAP_FUNCTION("wait", &Event::wait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace sys
} // namespace jnc
