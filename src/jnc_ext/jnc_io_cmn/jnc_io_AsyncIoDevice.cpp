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
	m_activeEvents = 0;

	m_readBuffer.clear ();
	m_writeBuffer.clear ();
	m_freeOverlappedReadList.insertListTail (&m_activeOverlappedReadList);
	m_freeReadWriteMetaDataList.insertListTail (&m_readMetaDataList);
	m_freeReadWriteMetaDataList.insertListTail (&m_writeMetaDataList);

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
	m_freeOverlappedReadList.insertListTail (&m_activeOverlappedReadList);
	m_freeReadWriteMetaDataList.insertListTail (&m_readMetaDataList);
	m_freeReadWriteMetaDataList.insertListTail (&m_writeMetaDataList);

#if (_JNC_OS_POSIX)
	m_selfPipe.close ();
#endif

	m_isOpen = false;
}


bool
AsyncIoDevice::setReadParallelismImpl (
	uint_t* p,
	uint_t count
	)
{
	if (!m_isOpen)
	{
		*p = count;
		return true;
	}

	m_ioLock.lock ();
	*p = count;
	wakeIoThread ();
	m_ioLock.unlock ();
	return true;
}


bool
AsyncIoDevice::setReadBlockSizeImpl (
	size_t* p,
	size_t size
	)
{
	if (!m_isOpen)
	{
		*p = size;
		return true;
	}

	m_ioLock.lock ();
	*p = size;
	wakeIoThread ();
	m_ioLock.unlock ();
	return true;
}

bool
AsyncIoDevice::setReadBufferSizeImpl (
	size_t* p,
	size_t size
	)
{
	m_ioLock.lock ();
	
	bool result = m_readBuffer.setBufferSize (size);
	if (!result)
	{
		m_ioLock.unlock ();
		propagateLastError ();
		return false;
	}

	*p = size;
	m_ioLock.unlock ();
	return true;
}

bool
AsyncIoDevice::setWriteBufferSizeImpl (
	size_t* p,
	size_t size
	)
{
	m_ioLock.lock ();

	bool result = m_writeBuffer.setBufferSize (size);
	if (!result)
	{
		m_ioLock.unlock ();
		propagateLastError ();
		return false;
	}

	*p = size;
	m_ioLock.unlock ();
	return true;
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

	AsyncWait* wait = AXL_MEM_NEW (AsyncWait);
	wait->m_mask = eventMask;
	wait->m_handlerPtr = handlerPtr;
	m_asyncWaitList.insertTail (wait);
	handle_t handle = m_asyncWaitMap.add (wait);
	wait->m_handle = handle;
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
			m_asyncWaitMap.eraseHandle (wait->m_handle);
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
AsyncIoDevice::setEvents (uint_t events)
{
	m_ioLock.lock ();
	if (!(m_activeEvents ^ events))
	{
		m_ioLock.unlock ();
		return;
	}

	m_activeEvents |= events;
	processWaitLists_l ();
}

void
AsyncIoDevice::setErrorEvent (
	uint_t event,
	const err::Error& error
	)
{
	JNC_BEGIN_CALL_SITE (m_runtime)

	DataPtr errorPtr = memDup (error, error->m_size);

	m_ioLock.lock ();
	if (m_activeEvents & event)
	{
		m_ioLock.unlock ();
	}
	else
	{
		m_activeEvents |= event;
		m_ioErrorPtr = errorPtr;
		processWaitLists_l ();
	}

	JNC_END_CALL_SITE ()
}

bool
AsyncIoDevice::isReadBufferValid ()
{
	return 		
		m_readBuffer.isValid () && 
		(m_readOverflowBuffer.isEmpty () || m_readBuffer.isFull ());
}

bool
AsyncIoDevice::addToReadBuffer (
	const void* p, 
	size_t size,
	const uint_t* flags
	)
{
	size_t addedSize = m_readBuffer.write (p, size);
	if (addedSize < size)
	{
		size_t overflowSize = size - addedSize;
		m_readOverflowBuffer.append ((char*) p + addedSize, overflowSize);
	}

	if (*flags & AsyncIoCompatibilityFlag_MaintainReadBlockSize)
	{
		ReadWriteMetaData* meta = createReadWriteMetaData ();
		meta->m_blockSize = size;
		m_readMetaDataList.insertTail (meta);
	}

	ASSERT (isReadBufferValid ());
	return true;
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

	m_ioLock.lock ();

	if (!m_readMetaDataList.isEmpty ())
	{
		ReadWriteMetaData* meta = *m_readMetaDataList.getHead ();
		if (size < meta->m_blockSize)
		{
			meta->m_blockSize -= size;
		}
		else
		{
			size = meta->m_blockSize;
			m_readMetaDataList.remove (meta);
			m_freeReadWriteMetaDataList.insertHead (meta);
		}
	}

	size_t result = m_readBuffer.read (p, size);
	if (!m_readOverflowBuffer.isEmpty ())
	{
		p += result;
		size -= result;

		size_t overflowSize = m_readOverflowBuffer.getCount ();
		size_t extraSize = AXL_MIN (overflowSize, size);

		memcpy (p, m_readOverflowBuffer, extraSize);
		result += extraSize;

		size_t movedSize = m_readBuffer.write (m_readOverflowBuffer + extraSize, overflowSize - extraSize);
		m_readOverflowBuffer.remove (0, movedSize);
	}

	if (result)
	{
		if (m_readBuffer.isEmpty ())
			m_activeEvents &= ~AsyncIoEvent_IncomingData;

		wakeIoThread ();
	}

	ASSERT (isReadBufferValid ());
	m_ioLock.unlock ();

	return result;
}

size_t
AsyncIoDevice::bufferedWrite (
	DataPtr ptr,
	size_t size,
	const uint_t* flags
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
		if (*flags & AsyncIoCompatibilityFlag_MaintainReadBlockSize)
		{
			ReadWriteMetaData* meta = createReadWriteMetaData ();
			meta->m_blockSize = result;
			m_writeMetaDataList.insertTail (meta);
		}

		if (m_writeBuffer.isFull ())
			m_activeEvents &= ~AsyncIoEvent_TransmitBufferReady;

		wakeIoThread ();
	}

	m_ioLock.unlock ();

	return result;
}

//..............................................................................

} // namespace io
} // namespace jnc
