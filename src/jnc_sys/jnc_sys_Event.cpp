#include "pch.h"
#include "jnc_sys_Event.h"

namespace jnc {
namespace sys {

//.............................................................................

bool
AXL_CDECL
Event::wait (uint_t timeout)
{
	bool result;

	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	rt::enterWaitRegion (runtime);
	result = m_event.wait (timeout);
	rt::leaveWaitRegion (runtime);

	return result;
}

//.............................................................................

} // namespace sys
} // namespace jnc
