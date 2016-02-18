#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_sys_SysLibGlobals.h"

namespace jnc {
namespace sys {

//.............................................................................

class Thread: public rt::IfaceHdr
{
public:
	JNC_BEGIN_CLASS_TYPE_MAP ("sys.Thread", g_sysLibCacheSlot, SysLibTypeCacheSlot_Thread)
		JNC_MAP_CONSTRUCTOR (&sl::construct <Thread>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <Thread>)
		JNC_MAP_FUNCTION ("start", &Thread::start)
		JNC_MAP_FUNCTION ("wait", &Thread::wait)
		JNC_MAP_FUNCTION ("kill", &Thread::kill)
	JNC_END_CLASS_TYPE_MAP ()

public:
	uintptr_t m_threadId;

public:
//	axl::sys::Thread m_thread;

public:
	bool
	AXL_CDECL
	start (rt::FunctionPtr ptr)
	{
		return true;
	}

	bool
	AXL_CDECL
	wait (uint_t timeout)
	{
		return true;
	}

	bool
	AXL_CDECL
	kill ()
	{
		return true;
	}
};

//.............................................................................

} // namespace sys
} // namespace jnc
