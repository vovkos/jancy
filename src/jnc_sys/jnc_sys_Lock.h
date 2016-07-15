#pragma once

#include "jnc_ExtensionLib.h"
#include "jnc_sys_SysLibGlobals.h"

namespace jnc {
namespace sys {

//.............................................................................

class Lock: public IfaceHdr
{
public:
	axl::sys::Lock m_lock;

public:
	void
	AXL_CDECL
	lock ();

	void
	AXL_CDECL
	unlock ()
	{
		m_lock.unlock ();
	}
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DECLARE_OPAQUE_CLASS_TYPE (
	Lock, 
	"sys.Lock", 
	g_sysLibGuid, 
	SysLibCacheSlot_Lock, 
	Lock, 
	NULL
	)

//.............................................................................

} // namespace sys
} // namespace jnc
