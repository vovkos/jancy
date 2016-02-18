#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_sys_SysLibGlobals.h"

namespace jnc {
namespace sys {

//.............................................................................

class Lock: public rt::IfaceHdr
{
public:
	JNC_BEGIN_CLASS_TYPE_MAP ("sys.Lock", g_sysLibCacheSlot, SysLibTypeCacheSlot_Lock)
		JNC_MAP_CONSTRUCTOR (&sl::construct <Lock>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <Lock>)
		JNC_MAP_FUNCTION ("lock", &Lock::lock)
		JNC_MAP_FUNCTION ("unlock", &Lock::unlock)
	JNC_END_CLASS_TYPE_MAP ()

public:
	axl::sys::Lock m_lock;

public:
	void
	AXL_CDECL
	lock ()
	{
		m_lock.lock ();
	}

	void
	AXL_CDECL
	unlock ()
	{
		m_lock.unlock ();
	}
};

//.............................................................................

} // namespace sys
} // namespace jnc
