#include "pch.h"
#include "jnc_sys_Thread.h"
#include "jnc_sys_SysLib.h"

namespace jnc {
namespace sys {

//.............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	Thread, 
	"sys.Thread", 
	g_sysLibGuid, 
	SysLibCacheSlot_Thread,
	Thread,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Thread)
	JNC_MAP_CONSTRUCTOR (&sl::construct <Thread>)
	JNC_MAP_DESTRUCTOR (&sl::destruct <Thread>)
	JNC_MAP_FUNCTION ("start", &Thread::start)
	JNC_MAP_FUNCTION ("wait", &Thread::wait)
	JNC_MAP_FUNCTION ("waitAndClose", &Thread::waitAndClose)
	JNC_MAP_FUNCTION ("terminate", &Thread::terminate)
JNC_END_TYPE_FUNCTION_MAP ()

//.............................................................................

bool
AXL_CDECL
Thread::start (FunctionPtr ptr)
{
	bool result;

	if (m_thread.isOpen ())
	{
		err::setError (err::SystemErrorCode_InvalidDeviceState);
		return false;
	}

	if (!ptr.m_p)
	{
		err::setError (err::SystemErrorCode_InvalidParameter);
		return false;
	}

	m_threadFuncPtr = ptr;
	result = m_thread.start ();
	if (!result)
	{
		m_threadFuncPtr = g_nullFunctionPtr;
		return false;
	}

	return true;
}

bool
AXL_CDECL
Thread::wait (uint_t timeout)
{
	bool result;

	GcHeap* gcHeap = m_runtime->getGcHeap ();
	ASSERT (gcHeap == getCurrentThreadGcHeap ());

	gcHeap->enterWaitRegion ();
	result = m_thread.wait (timeout);
	gcHeap->leaveWaitRegion ();

	return result;
}

void
AXL_CDECL
Thread::waitAndClose (uint_t timeout)
{
	GcHeap* gcHeap = m_runtime->getGcHeap ();
	ASSERT (gcHeap == getCurrentThreadGcHeap ());

	gcHeap->enterWaitRegion ();
	m_thread.waitAndClose (timeout);
	gcHeap->leaveWaitRegion ();

	m_threadFuncPtr = g_nullFunctionPtr;
}

//.............................................................................

} // namespace sys
} // namespace jnc
