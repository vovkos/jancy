#pragma once

#include "jnc_ExtensionLib.h"
#include "jnc_sys_SysLibGlobals.h"

namespace jnc {
namespace sys {

//.............................................................................

class Event: public IfaceHdr
{
public:
	axl::sys::Event m_event;

public:
	void
	AXL_CDECL
	signal ()
	{
		m_event.signal ();
	}

	bool
	AXL_CDECL
	wait (uint_t timeout);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DECLARE_OPAQUE_CLASS_TYPE (
	Event, 
	"sys.Event", 
	g_sysLibGuid, 
	SysLibCacheSlot_Event, 
	Event, 
	NULL
	)

//.............................................................................

} // namespace sys
} // namespace jnc
