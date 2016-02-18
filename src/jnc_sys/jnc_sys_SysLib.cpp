#include "pch.h"
#include "jnc_sys_SysLib.h"

namespace jnc {
namespace ext {

//.............................................................................

void
initSysLib (ExtensionLibHost* host)
{
	sys::g_sysLibCacheSlot = host->getLibCacheSlot (sys::g_sysLibGuid);
}

ExtensionLib*
getSysLib (ExtensionLibHost* host)
{
	static int32_t onceFlag = 0;
	sl::callOnce (initSysLib, host, &onceFlag);
	return sl::getSimpleSingleton <sys::SysLib> ();
}

//.............................................................................

} // namespace ext

namespace sys {

//.............................................................................


void
SysLib::sleep (uint32_t msCount)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.enterWaitRegion ();
	axl::sys::sleep (msCount);
	runtime->m_gcHeap.leaveWaitRegion ();
}

struct ThreadContext
{
	rt::FunctionPtr m_ptr;
	rt::Runtime* m_runtime;
	axl::sys::Event m_threadStartedEvent;
};

// a small note on thread starting sequence

// we must protect function closure object from being prematurely collected during passing it to the
// newly created thread. to achieve that, createThread waits until thread func registers mutator 
// thread with JNC_BEGIN_CALL_SITE macro. after mutator thread is registered, collection will not start until
// we hit a safepoint within jancy thread, so we can resume host thread right after JNC_BEGIN_CALL_SITE.

#if (_AXL_ENV == AXL_ENV_WIN)

DWORD
WINAPI
SysLib::threadFunc (PVOID context0)
{
	ThreadContext* context = (ThreadContext*) context0;
	ASSERT (context && context->m_runtime && context->m_ptr.m_p);
	rt::FunctionPtr ptr = context->m_ptr;

	JNC_BEGIN_CALL_SITE (context->m_runtime);
	context->m_threadStartedEvent.signal ();
	
	((void (AXL_CDECL*) (rt::IfaceHdr*)) ptr.m_p) (ptr.m_closure);
	
	JNC_END_CALL_SITE ();

	return 0;
}

bool
SysLib::createThread (rt::FunctionPtr ptr)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	ThreadContext context;
	context.m_ptr = ptr;
	context.m_runtime = runtime;

	DWORD threadId;
	HANDLE h = ::CreateThread (NULL, 0, SysLib::threadFunc, &context, 0, &threadId);

	runtime->m_gcHeap.enterWaitRegion ();
	context.m_threadStartedEvent.wait ();
	runtime->m_gcHeap.leaveWaitRegion ();

	return h != NULL;
}

#elif (_AXL_ENV == AXL_ENV_POSIX)

void*
SysLib::threadFunc (void* context0)
{
	ThreadContext* context = (ThreadContext*) context0;
	ASSERT (context && context->m_runtime && context->m_ptr.m_p);
	rt::FunctionPtr ptr = context->m_ptr;

	JNC_BEGIN_CALL_SITE (context->m_runtime);
	context->m_threadStartedEvent.signal ();

	((void (AXL_CDECL*) (rt::IfaceHdr*)) ptr.m_p) (ptr.m_closure);
	
	JNC_END_CALL_SITE ();

	return NULL;
}

bool
SysLib::createThread (rt::FunctionPtr ptr)
{
	rt::Runtime* runtime = rt::getCurrentThreadRuntime ();
	ASSERT (runtime);

	ThreadContext context;
	context.m_ptr = ptr;
	context.m_runtime = runtime;

	pthread_t thread;
	int result = pthread_create (&thread, NULL, SysLib::threadFunc, &context);

	runtime->m_gcHeap.enterWaitRegion ();
	context.m_threadStartedEvent.wait ();
	runtime->m_gcHeap.leaveWaitRegion ();

	return result == 0;
}

#endif

//.............................................................................

} // namespace sys
} // namespace jnc
