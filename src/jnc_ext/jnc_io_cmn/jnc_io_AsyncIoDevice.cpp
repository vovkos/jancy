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

namespace jnc {
namespace io {

//..............................................................................

AsyncIoDevice::AsyncIoDevice ()
{
	m_runtime = getCurrentThreadRuntime ();
	ASSERT (m_runtime);

	m_options = 0;
	m_activeEvents = 0;
	m_ioErrorPtr = g_nullPtr;
	m_ioThreadFlags = 0;

	m_isOpen = false;
}

void
AsyncIoDevice::markOpaqueGcRoots (jnc::GcHeap* gcHeap)
{
	if (!m_runtime) // not constructed yet
		return;

	m_lock.lock ();

	sl::Iterator <AsyncWait> it = m_asyncWaitList.getHead ();
	for (; it; it++)
		if (it->m_handlerPtr.m_closure)
			gcHeap->markClass (it->m_handlerPtr.m_closure->m_box);

	it = m_pendingAsyncWaitList.getHead ();
	for (; it; it++)
		if (it->m_handlerPtr.m_closure)
			gcHeap->markClass (it->m_handlerPtr.m_closure->m_box);

	m_lock.unlock ();
}

void
AsyncIoDevice::open ()
{
	m_isOpen = true;
	m_ioThreadFlags = 0;
	m_activeEvents = 0;

	m_readBuffer.clear ();
	m_readOverflowBuffer.clear ();
	m_writeBuffer.clear ();

#if (_JNC_OS_WIN)
	m_ioThreadEvent.reset ();
#elif (_JNC_OS_POSIX)
	m_ioThreadSelfPipe.create ();
#endif
}

void
AsyncIoDevice::close ()
{
	cancelAllWaits ();

#if (_JNC_OS_WIN)
	m_freeReadWriteMetaList.insertListTail (&m_readMetaList);
	m_freeReadWriteMetaList.insertListTail (&m_writeMetaList);
#elif (_JNC_OS_POSIX)
	m_ioThreadSelfPipe.close ();
#endif

	m_isOpen = false;
}

bool
AsyncIoDevice::setReadBufferSize (
	size_t* targetField,
	size_t size
	)
{
	m_lock.lock ();

	if (m_readBuffer.getBufferSize () == size)
	{
		m_lock.unlock ();
		return true;
	}

	// the easiest way to ensure buffer consistency on resize is just to drop everything

	m_readMetaList.clear ();
	m_readBuffer.clear ();
	m_readOverflowBuffer.clear ();

	if (m_activeEvents & (AsyncIoEvent_ReadBufferFull | AsyncIoEvent_IncomingData))
	{
		m_activeEvents &= ~(AsyncIoEvent_ReadBufferFull | AsyncIoEvent_IncomingData);
		wakeIoThread ();
	}

	bool result = m_readBuffer.setBufferSize (size);
	if (!result)
	{
		m_lock.unlock ();
		return false;
	}

	*targetField = size;
	m_lock.unlock ();
	return true;
}

bool
AsyncIoDevice::setWriteBufferSize (
	size_t* targetField,
	size_t size
	)
{
	m_lock.lock ();

	if (m_writeBuffer.getBufferSize () == size)
	{
		m_lock.unlock ();
		return true;
	}

	// the easiest way to ensure buffer consistency on resize is just to drop everything

	m_writeMetaList.clear ();
	m_writeBuffer.clear ();

	if (!(m_activeEvents & AsyncIoEvent_WriteBufferReady)) // set active events in IO thread
		wakeIoThread ();

	bool result = m_writeBuffer.setBufferSize (size);
	if (!result)
	{
		m_lock.unlock ();
		return false;
	}

	*targetField = size;
	m_lock.unlock ();
	return true;
}

uint_t
AsyncIoDevice::blockingWait (
	uint_t eventMask,
	uint_t timeout
	)
{
	m_lock.lock ();

	uint_t triggeredEvents = eventMask & m_activeEvents;
	if (triggeredEvents)
	{
		m_lock.unlock ();
		return triggeredEvents;
	}

	sys::Event event;

	SyncWait wait;
	wait.m_mask = eventMask;
	wait.m_event = &event;
	m_syncWaitList.insertTail (&wait);
	m_lock.unlock ();

	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	gcHeap->enterWaitRegion ();
	event.wait (timeout);
	gcHeap->leaveWaitRegion ();

	m_lock.lock ();
	triggeredEvents = eventMask & m_activeEvents;
	m_syncWaitList.remove (&wait);
	m_lock.unlock ();

	return triggeredEvents;
}

handle_t
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

	m_lock.lock ();

	uint_t triggeredEvents = eventMask & m_activeEvents;
	if (triggeredEvents)
	{
		m_lock.unlock ();
		callVoidFunctionPtr (handlerPtr, triggeredEvents);
		return 0; // not added
	}

	AsyncWait* wait = AXL_MEM_NEW (AsyncWait);
	wait->m_mask = eventMask;
	wait->m_handlerPtr = handlerPtr;
	m_asyncWaitList.insertTail (wait);
	handle_t handle = (handle_t) m_asyncWaitMap.add (wait);
	wait->m_handle = handle;
	m_lock.unlock ();

	return handle;
}

bool
AsyncIoDevice::cancelWait (handle_t handle)
{
	m_lock.lock ();

	sl::HandleTableIterator <AsyncWait*> it = m_asyncWaitMap.find ((uintptr_t) handle);
	if (!it)
	{
		m_lock.unlock ();
		jnc::setError (err::Error (err::SystemErrorCode_InvalidParameter));
		return false; // not found
	}

	m_asyncWaitList.erase (it->m_value);
	m_asyncWaitMap.erase (it);
	m_lock.unlock ();

	return true;
}

void
AsyncIoDevice::wakeIoThread ()
{
#if (_JNC_OS_WIN)
	m_ioThreadEvent.signal ();
#else
	m_ioThreadSelfPipe.write (" ", 1);
#endif
}

void
AsyncIoDevice::sleepIoThread ()
{
#if (_JNC_OS_WIN)
	m_ioThreadEvent.wait ();
#elif (_JNC_OS_POSIX)
	char buffer [256];
	m_ioThreadSelfPipe.read (buffer, sizeof (buffer));
#endif
}

void
AsyncIoDevice::cancelAllWaits ()
{
	m_lock.lock ();

	sl::List <AsyncWait> asyncWaitList; // will be cleared upon exiting the scope
	sl::takeOver (&asyncWaitList, &m_asyncWaitList);
	m_asyncWaitMap.clear ();

	sl::Iterator <SyncWait> it = m_syncWaitList.getHead ();
	for (; it; it++)
		it->m_event->signal ();

	m_lock.unlock ();
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
			m_asyncWaitMap.eraseKey ((uintptr_t) wait->m_handle);
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
		m_lock.unlock ();

		callVoidFunctionPtr (m_runtime, wait->m_handlerPtr, wait->m_mask);

		m_lock.lock ();
		m_pendingAsyncWaitList.erase (wait);
	}

	m_lock.unlock ();
	return count;
}

void
AsyncIoDevice::setEvents_l (uint_t events)
{
	if ((m_activeEvents & events) == events) // was set already
	{
		m_lock.unlock ();
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

	m_lock.lock ();
	if (m_activeEvents & event)
	{
		m_lock.unlock ();
	}
	else
	{
		m_activeEvents |= event;
		m_ioErrorPtr = errorPtr;
		processWaitLists_l ();
	}

	JNC_END_CALL_SITE ()
}

size_t
AsyncIoDevice::getMetaListDataSize (const sl::ConstList <ReadWriteMeta>& metaList)
{
	size_t size = 0;

	sl::ConstIterator <ReadWriteMeta> it = metaList.getHead ();
	for (; it; it++)
		size += it->m_dataSize;

	return size;
}

bool
AsyncIoDevice::isReadBufferValid ()
{
	return
		m_readBuffer.isValid () &&
		(m_readBuffer.isFull () || m_readOverflowBuffer.isEmpty ()) &&
		(m_readMetaList.isEmpty () ||
		m_readBuffer.getDataSize () + m_readOverflowBuffer.getCount () == getMetaListDataSize (m_readMetaList));
}

bool
AsyncIoDevice::isWriteBufferValid ()
{
	return
		m_writeBuffer.isValid () &&
		(m_writeMetaList.isEmpty () ||
		m_writeBuffer.getDataSize () == getMetaListDataSize (m_writeMetaList));
}

size_t
AsyncIoDevice::bufferedRead (
	DataPtr ptr,
	size_t size,
	sl::Array <char>* params
	)
{
	if (!m_isOpen)
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	size_t result;
	char* p = (char*) ptr.m_p;

	m_lock.lock ();
	if (m_readMetaList.isEmpty ())
	{
		if (params)
			params->clear ();
	}
	else
	{
		ReadWriteMeta* meta = *m_readMetaList.getHead ();
		if (params)
			params->copy ((char*) (meta + 1), meta->m_paramSize);

		if (size >= meta->m_dataSize)
		{
			size = meta->m_dataSize;
			m_readMetaList.remove (meta);
			m_freeReadWriteMetaList.insertHead (meta);
		}
		else if (m_ioThreadFlags & IoThreadFlag_Datagram)
		{
			m_readMetaList.remove (meta);
			m_freeReadWriteMetaList.insertHead (meta);
		}
		else
		{
			meta->m_dataSize -= size;
		}
	}

	result = m_readBuffer.read (p, size);

	if (result)
	{
		if (!m_readOverflowBuffer.isEmpty ())
		{
			size_t overflowSize = m_readOverflowBuffer.getCount ();

			if (!m_readBuffer.isEmpty ()) // refill the main buffer first
			{
				size_t movedSize = m_readBuffer.write (m_readOverflowBuffer, overflowSize);
				m_readOverflowBuffer.remove (0, movedSize);
			}
			else // we can read some extra data directly from the overflow buffer
			{
				p += result;
				size -= result;

				size_t extraSize = AXL_MIN (overflowSize, size);
				memcpy (p, m_readOverflowBuffer, extraSize);
				result += extraSize;

				// pump the remainder into the main buffer

				size_t movedSize = m_readBuffer.write (m_readOverflowBuffer + extraSize, overflowSize - extraSize);
				m_readOverflowBuffer.remove (0, extraSize + movedSize);
			}
		}

		if (!m_readBuffer.isFull ())
			m_activeEvents &= ~AsyncIoEvent_ReadBufferFull;

		if (m_readBuffer.isEmpty ())
			m_activeEvents &= ~AsyncIoEvent_IncomingData;

		wakeIoThread ();
	}

	ASSERT (isReadBufferValid ());
	m_lock.unlock ();

	return result;
}

size_t
AsyncIoDevice::bufferedWrite (
	DataPtr dataPtr,
	size_t dataSize,
	const void* params,
	size_t paramSize
	)
{
	ASSERT ((m_options & AsyncIoOption_KeepWriteBlockSize) || paramSize == 0);

	if (!m_isOpen)
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	m_lock.lock ();

	size_t result = m_writeBuffer.write (dataPtr.m_p, dataSize);
	if (result || (m_ioThreadFlags & IoThreadFlag_Datagram))
	{
		if (m_options & AsyncIoOption_KeepWriteBlockSize)
		{
			ReadWriteMeta* meta = createReadWriteMeta (result, params, paramSize);
			m_writeMetaList.insertTail (meta);
		}

		if (m_writeBuffer.isFull ())
			m_activeEvents &= ~AsyncIoEvent_WriteBufferReady;

		wakeIoThread ();
	}

	ASSERT (isWriteBufferValid ());
	isWriteBufferValid ();
	m_lock.unlock ();

	return result;
}

void
AsyncIoDevice::addToReadBuffer (
	const void* p,
	size_t dataSize,
	const void* params,
	size_t paramSize
	)
{
	ASSERT ((m_options & AsyncIoOption_KeepReadBlockSize) || paramSize == 0);

	if (!dataSize)
		return;

	size_t addedSize = m_readBuffer.write (p, dataSize);
	if (addedSize < dataSize)
	{
		size_t overflowSize = dataSize - addedSize;
		m_readOverflowBuffer.append ((char*) p + addedSize, overflowSize);
	}

	if (m_options & AsyncIoOption_KeepReadBlockSize)
	{
		ReadWriteMeta* meta = createReadWriteMeta (dataSize, params, paramSize);
		m_readMetaList.insertTail (meta);
	}

	ASSERT (isReadBufferValid ());
}

AsyncIoDevice::ReadWriteMeta*
AsyncIoDevice::createReadWriteMeta (
	size_t dataSize,
	const void* params,
	size_t paramSize
	)
{
	ReadWriteMeta* meta = !m_freeReadWriteMetaList.isEmpty () &&
		m_freeReadWriteMetaList.getHead ()->m_paramSize >= paramSize ?
		m_freeReadWriteMetaList.removeHead () :
		AXL_MEM_NEW_EXTRA (ReadWriteMeta, paramSize);

	meta->m_dataSize = dataSize;
	meta->m_paramSize = paramSize;
	memcpy (meta + 1, params, paramSize);
	return meta;
}

void
AsyncIoDevice::getNextWriteBlock (
	sl::Array <char>* data,
	sl::Array <char>* params
	)
{
	if (!data->isEmpty ())
		return;

	if (m_writeMetaList.isEmpty ())
	{
		m_writeBuffer.readAll (data);
		if (params)
			params->clear ();
	}
	else
	{
		ReadWriteMeta* meta = m_writeMetaList.removeHead ();
		ASSERT (meta->m_dataSize <= m_writeBuffer.getDataSize ());

		data->setCount (meta->m_dataSize);
		m_writeBuffer.read (*data, meta->m_dataSize);

		if (params)
			params->copy ((char*) (meta + 1), meta->m_paramSize);

		m_freeReadWriteMetaList.insertHead (meta);
	}

	ASSERT (isWriteBufferValid ());
}

void
AsyncIoDevice::updateReadWriteBufferEvents ()
{
	if (!m_readBuffer.isEmpty ())
		m_activeEvents |= AsyncIoEvent_IncomingData;

	if (m_readBuffer.isFull ())
		m_activeEvents |= AsyncIoEvent_ReadBufferFull;

	if (!m_writeBuffer.isFull ())
		m_activeEvents |= AsyncIoEvent_WriteBufferReady;
}

//..............................................................................

} // namespace io
} // namespace jnc
