#include "pch.h"
#include "jnc_sys_Timer.h"
#include "jnc_sys_SysLib.h"

namespace jnc {
namespace sys {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	Timer,
	"sys.Timer",
	g_sysLibGuid,
	SysLibCacheSlot_Timer,
	Timer,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (Timer)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <Timer>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <Timer>)
	JNC_MAP_FUNCTION ("start", &Timer::start)
	JNC_MAP_FUNCTION ("stop", &Timer::stop)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

bool
JNC_CDECL
Timer::start (
	FunctionPtr ptr,
	uint64_t dueTime,
	uint_t interval
	)
{
	bool result;

	stop ();

	m_timerFuncPtr = ptr;
	m_dueTime = dueTime;
	m_interval = interval;
	m_stopEvent.reset ();

	result = m_thread.start ();
	if (!result)
	{
		m_timerFuncPtr = g_nullFunctionPtr;
		return false;
	}

	return true;
}

void
JNC_CDECL
Timer::stop ()
{
	m_stopEvent.signal ();

	if (m_thread.getThreadId () != axl::sys::getCurrentThreadId ())
	{
		GcHeap* gcHeap = m_runtime->getGcHeap ();
		ASSERT (gcHeap == getCurrentThreadGcHeap ());

		gcHeap->enterWaitRegion ();
		m_thread.waitAndClose ();
		gcHeap->leaveWaitRegion ();
	}

	m_timerFuncPtr = g_nullFunctionPtr;
	m_dueTime = 0;
	m_interval = 0;
}

void
Timer::threadFunc ()
{
	bool result;

	uint64_t timestamp = axl::sys::getTimestamp ();
	if (m_dueTime > timestamp)
	{
		uint_t delay = (uint_t) ((m_dueTime - timestamp) / 10000);
		result = m_stopEvent.wait (delay);
		if (result)
			return;
	}

	callVoidFunctionPtr (m_runtime, m_timerFuncPtr);

	if (!m_interval || m_interval == -1)
		return;

	for (;;)
	{
		result = m_stopEvent.wait (m_interval);
		if (result)
			break;

		callVoidFunctionPtr (m_runtime, m_timerFuncPtr);
	}
}

//..............................................................................

} // namespace sys
} // namespace jnc
