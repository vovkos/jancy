#pragma once

#include "jnc_ExtensionLib.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace sys {

JNC_DECLARE_OPAQUE_CLASS_TYPE (Thread)

//.............................................................................
	
class Thread: public IfaceHdr
{
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
	Runtime* m_runtime;
	ThreadImpl m_thread;

public:
	Thread ()
	{
		m_runtime = getCurrentThreadRuntime ();
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
		callVoidFunctionPtr (m_runtime, m_threadFuncPtr);
	}
};

//.............................................................................

} // namespace sys
} // namespace jnc
