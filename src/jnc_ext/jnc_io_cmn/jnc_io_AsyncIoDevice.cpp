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
#include "jnc_io_AsyncIoDevice.h"
#include "../jnc_io_base/jnc_io_IoLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	AsyncIoDevice,
	"io.AsyncIoDevice",
	g_ioLibGuid,
	IoLibCacheSlot_AsyncIoDevice,
	AsyncIoDevice,
	&AsyncIoDevice::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (AsyncIoDevice)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <AsyncIoDevice>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <AsyncIoDevice>)
	
	JNC_MAP_FUNCTION ("wait", &AsyncIoDevice::wait)
	JNC_MAP_FUNCTION ("cancelWait", &AsyncIoDevice::cancelWait)
	JNC_MAP_FUNCTION ("blockingWait", &AsyncIoDevice::blockingWait)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

AsyncIoDevice::AsyncIoDevice ()
{
	m_runtime = getCurrentThreadRuntime ();
	ASSERT (m_runtime);
}

void
JNC_CDECL
AsyncIoDevice::markOpaqueGcRoots (jnc::GcHeap* gcHeap)
{
	sl::Iterator <AsyncWait> it = m_asyncWaitList.getHead ();
	for (; it; it++)
		if (it->m_handlerPtr.m_closure)
			gcHeap->markClass (it->m_handlerPtr.m_closure->m_box);

	it = m_pendingAsyncWaitList.getHead ();
	for (; it; it++)
		if (it->m_handlerPtr.m_closure)
			gcHeap->markClass (it->m_handlerPtr.m_closure->m_box);
}

void
AsyncIoDevice::open ()
{
	m_isOpen = true;
	m_ioFlags = 0;
	m_activeEvents = AsyncIoEvent_TransmitBufferReady; // status line events will be updated in io thread

	m_readBuffer.clear ();
	m_writeBuffer.clear ();
	m_freeReadBlockList.insertListTail (&m_activeReadBlockList);

#if (_JNC_OS_WIN)
	m_ioThreadEvent.reset ();
#elif (_JNC_OS_POSIX)
	m_selfPipe.create ();
#endif
}

void
AsyncIoDevice::close ()
{
	cancelAllWaits ();

	m_ioFlags = 0;
	m_activeEvents = 0;

	m_readBuffer.clear ();
	m_writeBuffer.clear ();
	m_freeReadBlockList.insertListTail (&m_activeReadBlockList);

#if (_JNC_OS_POSIX)
	m_selfPipe.close ();
#endif

	m_isOpen = false;
}

uint_t
JNC_CDECL
AsyncIoDevice::blockingWait (
	uint_t eventMask,
	uint_t timeout
	)
{
	m_ioLock.lock ();

	uint_t triggeredEvents = eventMask & m_activeEvents;
	if (triggeredEvents)
	{
		m_ioLock.unlock ();
		return triggeredEvents;
	}

	sys::Event event;
	
	SyncWait wait;
	wait.m_mask = eventMask;
	wait.m_event = &event;
	m_syncWaitList.insertTail (&wait);
	m_ioLock.unlock ();

	event.wait (timeout);

	m_ioLock.lock ();
	triggeredEvents = eventMask & m_activeEvents;
	m_syncWaitList.remove (&wait);
	m_ioLock.unlock ();

	return triggeredEvents;
}

handle_t 
JNC_CDECL
AsyncIoDevice::wait (
	uint_t eventMask,
	FunctionPtr handlerPtr
	)
{
	if (!m_isOpen)
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return (handle_t) (intptr_t) -1;
	}

	m_ioLock.lock ();

	uint_t triggeredEvents = eventMask & m_activeEvents;
	if (triggeredEvents)
	{
		m_ioLock.unlock ();
		callVoidFunctionPtr (handlerPtr, triggeredEvents);
		return 0; // not added
	}

	sys::Event event;
	
	AsyncWait* wait = AXL_MEM_NEW (AsyncWait);
	wait->m_mask = eventMask;
	wait->m_handlerPtr = handlerPtr;
	m_asyncWaitList.insertTail (wait);
	handle_t handle = m_asyncWaitMap.add (wait);
	m_ioLock.unlock ();

	return handle;
}

bool
JNC_CDECL
AsyncIoDevice::cancelWait (handle_t handle)
{
	m_ioLock.lock ();

	sl::HandleTable <AsyncWait*>::MapIterator it = m_asyncWaitMap.find (handle);
	if (!it)
	{
		m_ioLock.unlock ();
		jnc::setError (err::Error (err::SystemErrorCode_InvalidParameter));
		return false; // not found
	}

	m_asyncWaitList.erase (it->m_value->m_value);
	m_asyncWaitMap.erase (it);
	m_ioLock.unlock ();

	return true;
}

void
AsyncIoDevice::wakeIoThread ()
{
#if (_JNC_OS_WIN)
	m_ioThreadEvent.signal ();
#else
	m_selfPipe.write (" ", 1);
#endif
}

void
AsyncIoDevice::cancelAllWaits ()
{
	m_ioLock.lock ();

	sl::StdList <AsyncWait> asyncWaitList; // will be cleared upon exiting the scope
	asyncWaitList.takeOver (&m_asyncWaitList);
	m_asyncWaitMap.clear ();

	sl::Iterator <SyncWait> it = m_syncWaitList.getHead ();
	for (; it; it++)
		it->m_event->signal ();

	m_ioLock.unlock ();
}

size_t
AsyncIoDevice::processWaitLists_l ()
{
	sl::Iterator <AsyncWait> asyncIt = m_asyncWaitList.getHead ();
	while (asyncIt)
	{	
		uint_t triggeredEvents = asyncIt->m_mask & m_activeEvents;
		if (!triggeredEvents)
		{
			asyncIt++;
		}
		else
		{
			sl::Iterator <AsyncWait> nextIt = asyncIt.getNext ();
			AsyncWait* wait = *asyncIt;
			wait->m_mask = triggeredEvents;
			m_asyncWaitList.remove (wait);
			m_pendingAsyncWaitList.insertTail (wait);
			m_asyncWaitMap.eraseHandle (wait);
			asyncIt = nextIt;
		}
	}

	sl::Iterator <SyncWait> syncIt = m_syncWaitList.getHead ();
	for (; syncIt; syncIt++)
	{	
		if (syncIt->m_mask & m_activeEvents)
			syncIt->m_event->signal ();
	}

	size_t count = m_pendingAsyncWaitList.getCount ();

	while (!m_pendingAsyncWaitList.isEmpty ())
	{	
		AsyncWait* wait = *m_pendingAsyncWaitList.getHead ();
		m_ioLock.unlock ();

		callVoidFunctionPtr (m_runtime, wait->m_handlerPtr, wait->m_mask);
		
		m_ioLock.lock ();
		m_pendingAsyncWaitList.erase (wait);
	}

	m_ioLock.unlock ();
	return count;
}

void
AsyncIoDevice::setIoErrorEvent (const err::Error& error)
{
	DataPtr errorPtr = memDup (error, error->m_size);

	m_ioLock.lock ();
	m_activeEvents |= AsyncIoEvent_IoError;
	m_ioErrorPtr = errorPtr;
	processWaitLists_l ();
}

size_t
AsyncIoDevice::bufferedRead (
	DataPtr ptr,
	size_t size
	)
{
	if (!m_isOpen)
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	char* p = (char*) ptr.m_p;
	size_t result = 0;

	m_ioLock.lock ();

	if (!m_readOverflowBuffer.isEmpty ())
	{
		result = m_readOverflowBuffer.read (p, size);
		if (!m_readOverflowBuffer.isEmpty ()) // not yet
		{
			m_ioLock.unlock ();
			return result;			
		}

		p += result;
		size -= result;
	}

	result += m_readBuffer.read (p, size);

	if (result)
	{
		if (m_readBuffer.isEmpty ())
			m_activeEvents &= ~AsyncIoEvent_IncomingData;

		wakeIoThread ();
	}

	m_ioLock.unlock ();

	return result;
}

size_t
AsyncIoDevice::bufferedWrite (
	DataPtr ptr,
	size_t size
	)
{
	if (!m_isOpen)
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	m_ioLock.lock ();

	size_t result = m_writeBuffer.write (ptr.m_p, size);

	if (result)
	{
		if (m_writeBuffer.isFull ())
			m_activeEvents &= ~AsyncIoEvent_TransmitBufferReady;

		wakeIoThread ();
	}

	m_ioLock.unlock ();

	return result;
}

bool
AsyncIoDevice::addToReadBuffer (
	const void* p, 
	size_t size
	)
{
	size_t addedSize = m_readBuffer.write (p, size);
	if (addedSize == size)
		return true;

	ASSERT (addedSize < size);
	
	size_t overflowSize = size - addedSize;
	size_t oldDataSize = m_readOverflowBuffer.getDataSize ();
	size_t bufferSize = m_readOverflowBuffer.getBufferSize ();
	
	if (oldDataSize + overflowSize > bufferSize)
	{
		// grow overflow buffer

		bool result = m_readOverflowBuffer.setBufferSize (oldDataSize + overflowSize);
		if (!result)
			return false;
	}

	m_readOverflowBuffer.write ((char*) p + addedSize, overflowSize);
	return true;
}

//..............................................................................

} // namespace io
} // namespace jnc
