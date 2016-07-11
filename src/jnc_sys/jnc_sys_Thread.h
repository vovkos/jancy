#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_sys_SysLibGlobals.h"

namespace jnc {
namespace sys {

//.............................................................................
	
class Thread: public IfaceHdr
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (Thread, NULL)

	JNC_BEGIN_CLASS_TYPE_MAP ("sys.Thread", g_sysLibCacheSlot, SysLibTypeCacheSlot_Thread)
		JNC_MAP_CONSTRUCTOR (&sl::construct <Thread>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <Thread>)
		JNC_MAP_FUNCTION ("start", &Thread::start)
		JNC_MAP_FUNCTION ("wait", &Thread::wait)
		JNC_MAP_FUNCTION ("waitAndClose", &Thread::waitAndClose)
		JNC_MAP_FUNCTION ("terminate", &Thread::terminate)
	JNC_END_CLASS_TYPE_MAP ()

protected:
	class ThreadImpl: public axl::sys::ThreadImpl <ThreadImpl>
	{
	public:
		void
		threadFunc ()
		{
			AXL_CONTAINING_RECORD (this, Thread, m_thread)->threadFunc ();
		}
	};

public:
	FunctionPtr m_threadFuncPtr;
	uintptr_t m_threadId;

protected:
	jnc::rt::Runtime* m_runtime;
	ThreadImpl m_thread;

public:
	Thread ()
	{
		m_runtime = rt::getCurrentThreadRuntime ();
		ASSERT (m_runtime);
	}

	~Thread ()
	{
		waitAndClose (-1);
	}

	bool
	AXL_CDECL
	start (FunctionPtr ptr);

	bool
	AXL_CDECL
	wait (uint_t timeout);

	void
	AXL_CDECL
	waitAndClose (uint_t timeout);

	bool
	AXL_CDECL
	terminate ()
	{
		return m_thread.terminate ();
	}

protected:
	void
	threadFunc ()
	{
		rt::callVoidFunctionPtr (m_runtime, m_threadFuncPtr);
	}
};

//.............................................................................

} // namespace sys
} // namespace jnc
