#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_sys_SysLibGlobals.h"

namespace jnc {
namespace sys {
		
//.............................................................................

class Timer: public IfaceHdr
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (Timer, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("sys.Timer", g_sysLibCacheSlot, SysLibTypeCacheSlot_Timer)
		JNC_MAP_CONSTRUCTOR (&sl::construct <Timer>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <Timer>)
		JNC_MAP_FUNCTION ("start", &Timer::start)
		JNC_MAP_FUNCTION ("stop", &Timer::stop)
	JNC_END_CLASS_TYPE_MAP ()

protected:
	class ThreadImpl: public axl::sys::ThreadImpl <ThreadImpl>
	{
	public:
		void
		threadFunc ()
		{
			AXL_CONTAINING_RECORD (this, Timer, m_thread)->threadFunc ();
		}
	};

public:
	FunctionPtr m_timerFuncPtr;

protected:
	jnc::rt::Runtime* m_runtime;
	ThreadImpl m_thread;
	axl::sys::Event m_stopEvent;
	uint64_t m_dueTime;
	uint_t m_interval;

public:
	Timer ()
	{
		m_runtime = rt::getCurrentThreadRuntime ();
		ASSERT (m_runtime);
	}

	~Timer ()
	{
		stop ();
	}

	bool 
	AXL_CDECL
	start (
		FunctionPtr ptr,
		uint64_t dueTime,
		uint_t interval
		);

	void
	AXL_CDECL
	stop ();

protected:
	void
	threadFunc ();
};

//.............................................................................

} // namespace sys
} // namespace jnc
