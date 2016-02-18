#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_sys_SysLibGlobals.h"

namespace jnc {
namespace sys {
		
//.............................................................................

class Timer: public rt::IfaceHdr
{
public:
	JNC_BEGIN_CLASS_TYPE_MAP ("sys.Timer", g_sysLibCacheSlot, SysLibTypeCacheSlot_Timer)
		JNC_MAP_CONSTRUCTOR (&sl::construct <Timer>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <Timer>)
		JNC_MAP_FUNCTION ("setTimer", &Timer::setTimer)
		JNC_MAP_FUNCTION ("stop", &Timer::stop)
	JNC_END_CLASS_TYPE_MAP ()

public:
//	axl::sys::Timer m_timer;

public:
	bool 
	AXL_CDECL
	setTimer (
		rt::FunctionPtr ptr,
		bool isPeriodic
		)
	{
		return true;
	}

	void
	AXL_CDECL
	stop ()
	{

	}
};

//.............................................................................

} // namespace sys
} // namespace jnc
