#include "pch.h"
#include "jnc_sys_Event.h"
#include "jnc_Runtime.h"

namespace jnc {
namespace sys {

//.............................................................................
	
JNC_BEGIN_TYPE_FUNCTION_MAP (Event)
	JNC_MAP_CONSTRUCTOR (&sl::construct <Event>)
	JNC_MAP_DESTRUCTOR (&sl::destruct <Event>)
	JNC_MAP_FUNCTION ("signal", &Event::signal)
	JNC_MAP_FUNCTION ("wait", &Event::wait)
JNC_END_TYPE_FUNCTION_MAP ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
AXL_CDECL
Event::wait (uint_t timeout)
{
	bool result;

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);
	
	gcHeap->enterWaitRegion ();
	result = m_event.wait (timeout);
	gcHeap->leaveWaitRegion ();

	return result;
}

//.............................................................................

} // namespace sys
} // namespace jnc
