#include "pch.h"
#include "jnc_sys_Lock.h"

namespace jnc {
namespace sys {

//.............................................................................

void
AXL_CDECL
Lock::lock ()
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	rt::enterWaitRegion (runtime);
	m_lock.lock ();
	rt::leaveWaitRegion (runtime);
}

//.............................................................................

} // namespace sys
} // namespace jnc
