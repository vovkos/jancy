#include "pch.h"
#include "jnc_StdLib.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

void
initStdLibSlot (ExtensionLibSlotDb* slotDb)
{
	g_stdLibSlot = slotDb->getSlot (g_stdLibGuid);
}

ExtensionLib* 
getStdLib (ExtensionLibSlotDb* slotDb)
{
	static int32_t onceFlag = 0;
	mt::callOnce (initStdLibSlot, slotDb, &onceFlag);
	return rtl::getSimpleSingleton <StdLib> ();
}

//.............................................................................

int
StdLib::strCmp (
	DataPtr ptr1,
	DataPtr ptr2
	)
{
	if (ptr1.m_p == ptr2.m_p)
		return 0;

	return 
		!ptr1.m_p ? -1 :
		!ptr2.m_p ? 1 :
		strcmp ((char*) ptr1.m_p, (char*) ptr2.m_p);
}

int
StdLib::striCmp (
	DataPtr ptr1,
	DataPtr ptr2
	)
{
	if (ptr1.m_p == ptr2.m_p)
		return 0;

	return 
		!ptr1.m_p ? -1 :
		!ptr2.m_p ? 1 :
		_stricmp ((char*) ptr1.m_p, (char*) ptr2.m_p);
}

DataPtr 
StdLib::strChr (
	DataPtr ptr,
	int c
	)
{
	DataPtr resultPtr = { 0 };

	if (!ptr.m_p)
		return resultPtr;

	resultPtr = ptr;
	resultPtr.m_p = strchr ((char*) ptr.m_p, c);
	return resultPtr;
}

DataPtr
StdLib::strCat (
	DataPtr ptr1,
	DataPtr ptr2
	)
{
	size_t length1 = strLen (ptr1);
	size_t length2 = strLen (ptr2);

	return memCat (ptr1, length1, ptr2, length2 + 1);
}

DataPtr
StdLib::strDup (
	DataPtr ptr,
	size_t length
	)
{
	if (length == -1)
		length = strLen (ptr);

	return jnc::strDup ((const char*) ptr.m_p, length);
}

int
StdLib::memCmp (
	DataPtr ptr1,
	DataPtr ptr2,
	size_t size
	)
{
	if (ptr1.m_p == ptr2.m_p)
		return 0;

	return 
		!ptr1.m_p ? -1 :
		!ptr2.m_p ? 1 :
		memcmp (ptr1.m_p, ptr2.m_p, size);
}

DataPtr 
StdLib::memChr (
	DataPtr ptr,
	int c,
	size_t size
	)
{
	DataPtr resultPtr = { 0 };

	if (!ptr.m_p)
		return resultPtr;

	resultPtr = ptr;
	resultPtr.m_p = memchr (ptr.m_p, c, size);
	return resultPtr;
}

void
StdLib::memCpy (
	DataPtr dstPtr,
	DataPtr srcPtr,
	size_t size
	)
{
	if (dstPtr.m_p && srcPtr.m_p)
		memcpy (dstPtr.m_p, srcPtr.m_p, size);
}

void
StdLib::memSet (
	DataPtr ptr,
	int c,
	size_t size
	)
{
	if (ptr.m_p)
		memset (ptr.m_p, c, size);
}

DataPtr
StdLib::memCat (
	DataPtr ptr1,
	size_t size1,
	DataPtr ptr2,
	size_t size2
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	size_t totalSize = size1 + size2;
	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (totalSize);
	if (!resultPtr.m_p)
		return g_nullPtr;

	char* p = (char*) resultPtr.m_p;

	if (ptr1.m_p)
		memcpy (p, ptr1.m_p, size1);
	else
		memset (p, 0, size1);

	if (ptr2.m_p)
		memcpy (p + size1, ptr2.m_p, size2);
	else
		memset (p + size1, 0, size2);

	return resultPtr;
}

DataPtr
StdLib::memDup (
	DataPtr ptr,
	size_t size
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	if (ptr.m_p)
		memcpy (resultPtr.m_p, ptr.m_p, size);
	else
		memset (resultPtr.m_p, 0, size);

	return resultPtr;
}

void
StdLib::sleep (uint32_t msCount)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.enterWaitRegion ();
	g::sleep (msCount);
	runtime->m_gcHeap.leaveWaitRegion ();
}

struct ThreadContext
{
	FunctionPtr m_ptr;
	Runtime* m_runtime;
	mt::Event m_threadStartedEvent;
};

// a small note on thread starting sequence

// we must protect function closure object from being prematurely collected during passing it to the
// newly created thread. to achieve that, createThread waits until thread func registers mutator 
// thread with JNC_BEGIN_CALL_SITE macro. after mutator thread is registered, collection will not start until
// we hit a safepoint within jancy thread, so we can resume host thread right after JNC_BEGIN_CALL_SITE.

#if (_AXL_ENV == AXL_ENV_WIN)

DWORD
WINAPI
StdLib::threadFunc (PVOID context0)
{
	ThreadContext* context = (ThreadContext*) context0;
	ASSERT (context && context->m_runtime && context->m_ptr.m_p);
	FunctionPtr ptr = context->m_ptr;

	JNC_BEGIN_CALL_SITE (context->m_runtime);
	context->m_threadStartedEvent.signal ();
	
	((void (AXL_CDECL*) (IfaceHdr*)) ptr.m_p) (ptr.m_closure);
	
	JNC_END_CALL_SITE ();

	return 0;
}

bool
StdLib::createThread (FunctionPtr ptr)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ThreadContext context;
	context.m_ptr = ptr;
	context.m_runtime = runtime;

	DWORD threadId;
	HANDLE h = ::CreateThread (NULL, 0, StdLib::threadFunc, &context, 0, &threadId);

	runtime->m_gcHeap.enterWaitRegion ();
	context.m_threadStartedEvent.wait ();
	runtime->m_gcHeap.leaveWaitRegion ();

	return h != NULL;
}

#elif (_AXL_ENV == AXL_ENV_POSIX)

void*
StdLib::threadFunc (void* context0)
{
	ThreadContext* context = (ThreadContext*) context0;
	ASSERT (context && context->m_runtime && context->m_ptr.m_p);
	FunctionPtr ptr = context->m_ptr;

	JNC_BEGIN_CALL_SITE (context->m_runtime);
	context->m_threadStartedEvent.signal ();

	((void (AXL_CDECL*) (IfaceHdr*)) ptr.m_p) (ptr.m_closure);
	
	JNC_END_CALL_SITE ();

	return NULL;
}

bool
StdLib::createThread (FunctionPtr ptr)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ThreadContext context;
	context.m_ptr = ptr;
	context.m_runtime = runtime;

	pthread_t thread;
	int result = pthread_create (&thread, NULL, StdLib::threadFunc, &context);

	runtime->m_gcHeap.enterWaitRegion ();
	context.m_threadStartedEvent.wait ();
	runtime->m_gcHeap.leaveWaitRegion ();

	return result == 0;
}

#endif

DataPtr
StdLib::getErrorPtr (const err::ErrorData* errorData)
{
	size_t size = errorData->m_size;

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (size);
	if (!resultPtr.m_p)
		return g_nullPtr;

	memcpy (resultPtr.m_p, errorData, size);
	return resultPtr;
}

void
StdLib::collectGarbage ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->m_gcHeap.collect ();
}

DataPtr
StdLib::format (
	DataPtr formatStringPtr,
	...
	)
{
	AXL_VA_DECL (va, formatStringPtr);

	char buffer [256];
	rtl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	string.format_va ((const char*) formatStringPtr.m_p, va);
	size_t length = string.getLength ();

	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	DataPtr resultPtr = runtime->m_gcHeap.tryAllocateBuffer (length + 1);
	if (!resultPtr.m_p)
		return g_nullPtr;

	memcpy (resultPtr.m_p, string.cc (), length);
	return resultPtr;
}

//.............................................................................

} // namespace jnc {
