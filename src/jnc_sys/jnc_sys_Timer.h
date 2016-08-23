#pragma once

#include "jnc_ExtensionLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace sys {

JNC_DECLARE_OPAQUE_CLASS_TYPE (Timer)

//.............................................................................

class Timer: public IfaceHdr
{
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
	Runtime* m_runtime;
	ThreadImpl m_thread;
	axl::sys::Event m_stopEvent;
	uint64_t m_dueTime;
	uint_t m_interval;

public:
	Timer ()
	{
		m_runtime = getCurrentThreadRuntime ();
		ASSERT (m_runtime);
	}

	~Timer ()
	{
		stop ();
	}

	bool 
	JNC_CDECL
	start (
		FunctionPtr ptr,
		uint64_t dueTime,
		uint_t interval
		);

	void
	JNC_CDECL
	stop ();

protected:
	void
	threadFunc ();
};


//.............................................................................

} // namespace sys
} // namespace jnc
