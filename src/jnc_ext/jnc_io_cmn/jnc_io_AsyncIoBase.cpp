//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_io_AsyncIoBase.h"

namespace jnc {
namespace io {

//..............................................................................

AsyncIoBase::AsyncIoBase()
{
	m_runtime = getCurrentThreadRuntime();
	ASSERT(m_runtime);

	m_options = 0;
	m_activeEvents = 0;
	m_ioErrorPtr = g_nullDataPtr;
	m_pendingIoErrorPtr = g_nullDataPtr;
	m_ioThreadFlags = 0;

	m_isOpen = false;
}

void
AsyncIoBase::markOpaqueGcRoots(jnc::GcHeap* gcHeap)
{
	if (!m_runtime) // not constructed yet
		return;

	m_lock.lock(); // we touch wait lists outside of a call site (in IO thread)

	sl::Iterator<AsyncWait> it = m_asyncWaitList.getHead();
	for (; it; it++)
		if (it->m_handlerPtr.m_closure)
			gcHeap->markClass(it->m_handlerPtr.m_closure->m_box);

	it = m_pendingAsyncWaitList.getHead();
	for (; it; it++)
		if (it->m_handlerPtr.m_closure)
			gcHeap->markClass(it->m_handlerPtr.m_closure->m_box);

	m_lock.unlock();

	gcHeap->markDataPtr(m_pendingIoErrorPtr);
}

void
AsyncIoBase::open()
{
	m_ioErrorPtr = g_nullDataPtr;
	m_pendingIoErrorPtr = g_nullDataPtr;
	m_isOpen = true;
	m_ioThreadFlags = 0;
	m_activeEvents = 0;

#if (_JNC_OS_WIN)
	m_ioThreadEvent.reset();
#elif (_JNC_OS_POSIX)
	m_ioThreadSelfPipe.create();
#endif
}

void
AsyncIoBase::close()
{
	cancelAllWaits();

#if (_JNC_OS_POSIX)
	m_ioThreadSelfPipe.close();
#endif

	m_isOpen = false;
}

handle_t
AsyncIoBase::wait(
	uint_t eventMask,
	FunctionPtr handlerPtr
	)
{
	if (!m_isOpen)
	{
		jnc::setError(err::Error(err::SystemErrorCode_InvalidDeviceState));
		return (handle_t)(intptr_t) -1;
	}

	m_lock.lock();

	uint_t triggeredEvents = eventMask & m_activeEvents;
	if (triggeredEvents)
	{
		m_lock.unlock();
		callVoidFunctionPtr(handlerPtr, triggeredEvents);
		return 0; // not added
	}

	AsyncWait* wait = AXL_MEM_NEW(AsyncWait);
	wait->m_mask = eventMask;
	wait->m_handlerPtr = handlerPtr;
	m_asyncWaitList.insertTail(wait);
	handle_t handle = (handle_t)m_asyncWaitMap.add(wait);
	wait->m_handle = handle;
	m_lock.unlock();

	return handle;
}

bool
AsyncIoBase::cancelWait(handle_t handle)
{
	m_lock.lock();

	sl::HandleTableIterator<AsyncWait*> it = m_asyncWaitMap.find((uintptr_t)handle);
	if (!it)
	{
		m_lock.unlock();
		jnc::setError(err::Error(err::SystemErrorCode_InvalidParameter));
		return false; // not found
	}

	m_asyncWaitList.erase(it->m_value);
	m_asyncWaitMap.erase(it);
	m_lock.unlock();

	return true;
}

uint_t
AsyncIoBase::blockingWait(
	uint_t eventMask,
	uint_t timeout
	)
{
	m_lock.lock();

	uint_t triggeredEvents = eventMask & m_activeEvents;
	if (triggeredEvents)
	{
		m_lock.unlock();
		return triggeredEvents;
	}

	sys::Event event;

	SyncWait wait;
	wait.m_mask = eventMask;
	wait.m_event = &event;
	m_syncWaitList.insertTail(&wait);
	m_lock.unlock();

	GcHeap* gcHeap = getCurrentThreadGcHeap();
	ASSERT(gcHeap);

	gcHeap->enterWaitRegion();
	event.wait(timeout);
	gcHeap->leaveWaitRegion();

	m_lock.lock();
	triggeredEvents = eventMask & m_activeEvents;
	m_syncWaitList.remove(&wait);
	m_lock.unlock();

	return triggeredEvents;
}

Promise*
AsyncIoBase::asyncWait(uint_t eventMask)
{
	ClassType* type = (ClassType*)m_runtime->getModule()->getStdType(StdType_Promise);
	Promise* promise = (Promise*)m_runtime->getGcHeap()->allocateClass(type);

	FunctionPtr ptr;
	ptr.m_p = (void*)&onAsyncWaitCompleted;
	ptr.m_closure = &promise->m_ifaceHdr;
	wait(eventMask, ptr);
	return promise;
}

void
AsyncIoBase::onAsyncWaitCompleted(
	IfaceHdr* closure,
	uint_t triggeredEvents
	)
{
	Runtime* runtime = getCurrentThreadRuntime();
	Promise* promise = (Promise*)closure;

	Variant result;
	result.m_type = runtime->getModule()->getPrimitiveType(TypeKind_Int32_u);
	result.m_uint32 = triggeredEvents;

	promise->complete(result, g_nullDataPtr);
}

void
AsyncIoBase::wakeIoThread()
{
#if (_JNC_OS_WIN)
	m_ioThreadEvent.signal();
#else
	m_ioThreadSelfPipe.write(" ", 1);
#endif
}

void
AsyncIoBase::sleepIoThread()
{
#if (_JNC_OS_WIN)
	m_ioThreadEvent.wait();
#elif (_JNC_OS_POSIX)
	char buffer[256];
	m_ioThreadSelfPipe.read(buffer, sizeof(buffer));
#endif
}

void
AsyncIoBase::suspendIoThread(bool isSuspended)
{
	m_lock.lock();

	if (isSuspended)
		m_ioThreadFlags |= IoThreadFlag_Suspended;
	else
		m_ioThreadFlags &= ~IoThreadFlag_Suspended;

	wakeIoThread();
	m_lock.unlock();
}

void
AsyncIoBase::cancelAllWaits()
{
	m_lock.lock();

	sl::List<AsyncWait> asyncWaitList; // will be cleared upon exiting the scope
	sl::takeOver(&asyncWaitList, &m_asyncWaitList);
	m_asyncWaitMap.clear();

	sl::Iterator<SyncWait> it = m_syncWaitList.getHead();
	for (; it; it++)
		it->m_event->signal();

	m_lock.unlock();
}

size_t
AsyncIoBase::processWaitLists_l()
{
	sl::Iterator<AsyncWait> asyncIt = m_asyncWaitList.getHead();
	while (asyncIt)
	{
		uint_t triggeredEvents = asyncIt->m_mask & m_activeEvents;
		if (!triggeredEvents)
		{
			asyncIt++;
		}
		else
		{
			sl::Iterator<AsyncWait> nextIt = asyncIt.getNext();
			AsyncWait* wait = *asyncIt;
			wait->m_mask = triggeredEvents;
			m_asyncWaitList.remove(wait);
			m_pendingAsyncWaitList.insertTail(wait);
			m_asyncWaitMap.eraseKey((uintptr_t)wait->m_handle);
			asyncIt = nextIt;
		}
	}

	sl::Iterator<SyncWait> syncIt = m_syncWaitList.getHead();
	for (; syncIt; syncIt++)
	{
		if (syncIt->m_mask & m_activeEvents)
			syncIt->m_event->signal();
	}

	size_t count = m_pendingAsyncWaitList.getCount();

	while (!m_pendingAsyncWaitList.isEmpty())
	{
		AsyncWait* wait = *m_pendingAsyncWaitList.getHead();
		m_lock.unlock();

		callVoidFunctionPtr(m_runtime, wait->m_handlerPtr, wait->m_mask);

		m_lock.lock();
		m_pendingAsyncWaitList.erase(wait);
	}

	m_lock.unlock();
	return count;
}

void
AsyncIoBase::setEvents_l(uint_t events)
{
	if ((m_activeEvents & events) == events) // was set already
	{
		m_lock.unlock();
		return;
	}

	m_activeEvents |= events;
	processWaitLists_l();
}

void
AsyncIoBase::setIoErrorEvent_l(
	uint_t event,
	const err::Error& error
	)
{
	m_lock.unlock();

	JNC_BEGIN_CALL_SITE(m_runtime)
	m_pendingIoErrorPtr = memDup(error, error->m_size);
 	JNC_END_CALL_SITE()

	m_lock.lock();
	m_ioErrorPtr = m_pendingIoErrorPtr;
	m_pendingIoErrorPtr = g_nullDataPtr;
	setEvents_l(event);
}

//..............................................................................

} // namespace io
} // namespace jnc
