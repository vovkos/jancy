#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_sys_SysLibGlobals.h"

namespace jnc {
namespace sys {

//.............................................................................

class Event: public rt::IfaceHdr
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (Event, NULL)
	
	JNC_BEGIN_CLASS_TYPE_MAP ("sys.Event", g_sysLibCacheSlot, SysLibTypeCacheSlot_Event)
		JNC_MAP_CONSTRUCTOR (&sl::construct <Event>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <Event>)
		JNC_MAP_FUNCTION ("signal", &Event::signal)
		JNC_MAP_FUNCTION ("wait", &Event::wait)
	JNC_END_CLASS_TYPE_MAP ()

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
	wait (uint_t timeout)
	{
		return m_event.signal ();
	}
};

//.............................................................................

} // namespace sys
} // namespace jnc
