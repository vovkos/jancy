#include "pch.h"
#include "jnc_sys_Lock.h"
#include "jnc_sys_SysLib.h"
#include "jnc_Runtime.h"

namespace jnc {
namespace sys {

//.............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	Lock, 
	"sys.Lock", 
	g_sysLibGuid, 
	SysLibCacheSlot_Lock, 
	Lock, 
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Lock)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <Lock>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <Lock>)
	JNC_MAP_FUNCTION ("lock", &Lock::lock)
	JNC_MAP_FUNCTION ("unlock", &Lock::unlock)
JNC_END_TYPE_FUNCTION_MAP ()

//.............................................................................

void
JNC_CDECL
Lock::lock ()
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	gcHeap->enterWaitRegion ();
	m_lock.lock ();
	gcHeap->leaveWaitRegion ();
}

//.............................................................................

} // namespace sys
} // namespace jnc
