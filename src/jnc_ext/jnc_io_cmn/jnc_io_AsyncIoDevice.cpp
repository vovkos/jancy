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
	m_ioThreadFlags = 0;
	m_activeEvents = 0;

	m_readBuffer.clear ();
	m_writeBuffer.clear ();
	m_freeReadWriteMetaList.insertListTail (&m_readMetaList);
	m_freeReadWriteMetaList.insertListTail (&m_writeMetaList);

#if (_JNC_OS_WIN)
	m_freeOverlappedReadList.insertListTail (&m_activeOverlappedReadList);
	m_overlappedWriteBlock.clear ();
	m_ioThreadEvent.reset ();
#elif (_JNC_OS_POSIX)
	m_ioThreadSelfPipe.create ();
#endif
}

void
AsyncIoDevice::close ()
{
	cancelAllWaits ();

	m_ioThreadFlags = 0;
	m_activeEvents = 0;

	m_readBuffer.clear ();
	m_writeBuffer.clear ();
	m_freeReadWriteMetaList.insertListTail (&m_readMetaList);
	m_freeReadWriteMetaList.insertListTail (&m_writeMetaList);

#if (_JNC_OS_WIN)
	m_freeOverlappedReadList.insertListTail (&m_activeOverlappedReadList);
#elif (_JNC_OS_POSIX)
	m_ioThreadSelfPipe.close ();
#endif

	m_isOpen = false;
}

bool
AsyncIoDevice::setReadParallelism (
	uint_t* p,
	uint_t count
	)
{
	if (!m_isOpen)
	{
		*p = count;
		return true;
	}

	m_lock.lock ();
	*p = count;
	wakeIoThread ();
	m_lock.unlock ();
	return true;
}


bool
AsyncIoDevice::setReadBlockSize (
	size_t* p,
	size_t size
	)
{
	if (!m_isOpen)
	{
		*p = size;
		return true;
	}

	m_lock.lock ();
	*p = size;
	wakeIoThread ();
	m_lock.unlock ();
	return true;
}

bool
AsyncIoDevice::setReadBufferSize (
	size_t* p,
	size_t size
	)
{
	m_lock.lock ();

	bool result = m_readBuffer.setBufferSize (size);
	if (!result)
	{
		m_lock.unlock ();
		propagateLastError ();
		return false;
	}

	*p = size;
	m_lock.unlock ();
	return true;
}

bool
AsyncIoDevice::setWriteBufferSize (
	size_t* p,
	size_t size
	)
{
	m_lock.lock ();

	bool result = m_writeBuffer.setBufferSize (size);
	if (!result)
	{
		m_lock.unlock ();
		propagateLastError ();
		return false;
	}

	*p = size;
	m_lock.unlock ();
	return true;
}

uint_t
JNC_CDECL
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

	event.wait (timeout);

	m_lock.lock ();
	triggeredEvents = eventMask & m_activeEvents;
	m_syncWaitList.remove (&wait);
	m_lock.unlock ();

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
	handle_t handle = m_asyncWaitMap.add (wait);
	wait->m_handle = handle;
	m_lock.unlock ();

	return handle;
}

bool
JNC_CDECL
AsyncIoDevice::cancelWait (handle_t handle)
{
	m_lock.lock ();

	sl::HandleTable <AsyncWait*>::MapIterator it = m_asyncWaitMap.find (handle);
	if (!it)
	{
		m_lock.unlock ();
		jnc::setError (err::Error (err::SystemErrorCode_InvalidParameter));
		return false; // not found
	}

	m_asyncWaitList.erase (it->m_value->m_value);
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

	sl::StdList <AsyncWait> asyncWaitList; // will be cleared upon exiting the scope
	asyncWaitList.takeOver (&m_asyncWaitList);
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
	m_lock.lock ();
	if (!(m_activeEvents ^ events))
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

bool
AsyncIoDevice::isReadBufferValid ()
{
	return
		m_readBuffer.isValid () &&
		m_readBuffer.getDataSize () >= m_readMetaList.getCount () &&
		(m_readOverflowBuffer.isEmpty () || m_readBuffer.isFull ());
}

bool
AsyncIoDevice::isWriteBufferValid ()
{
	return
		m_writeBuffer.isValid () &&
		m_writeBuffer.getDataSize () >= m_writeMetaList.getCount ();
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

	m_lock.lock ();
	if (m_readMetaList.isEmpty () ||
		m_readMetaList.getHead ()->m_blockSize <= size ||
		!(m_ioThreadFlags & IoThreadFlag_Datagram))
		return bufferedReadImpl_l (ptr.m_p, size, params);

	size_t datagramSize = m_readMetaList.getHead ()->m_blockSize;
	char datagramBuffer [256];
	sl::Array <char> datagram (ref::BufKind_Stack, datagramBuffer, sizeof (datagramBuffer));
	datagram.setCount (datagramSize);

	size_t result = bufferedReadImpl_l (datagram, datagramSize, params);
	memcpy (ptr.m_p, datagram, size);
	return result;
}

size_t
AsyncIoDevice::bufferedReadImpl_l (
	void* p0,
	size_t size,
	sl::Array <char>* params
	)
{
	char* p = (char*) p0;
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

		if (size < meta->m_blockSize)
		{
			meta->m_blockSize -= size;
		}
		else
		{
			size = meta->m_blockSize;
			m_readMetaList.remove (meta);
			m_freeReadWriteMetaList.insertHead (meta);
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
		m_readOverflowBuffer.remove (0, extraSize + movedSize);
	}

	if (result)
	{
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
	if (result)
	{
		if (m_options & AsyncIoOption_KeepWriteBlockSize)
		{
			ReadWriteMeta* meta = createReadWriteMeta (paramSize);
			meta->m_blockSize = result;
			memcpy (meta + 1, params, paramSize);
			m_writeMetaList.insertTail (meta);
		}

		if (m_writeBuffer.isFull ())
			m_activeEvents &= ~AsyncIoEvent_TransmitBufferReady;

		wakeIoThread ();
	}

	ASSERT (isWriteBufferValid ());
	m_lock.unlock ();

	return result;
}

void
AsyncIoDevice::addToReadBuffer (
	const void* p,
	size_t size,
	const void* params,
	size_t paramSize
	)
{
	if (!size)
		return;

	size_t addedSize = m_readBuffer.write (p, size);
	if (addedSize < size)
	{
		size_t overflowSize = size - addedSize;
		m_readOverflowBuffer.append ((char*) p + addedSize, overflowSize);
	}

	if (m_options & AsyncIoOption_KeepReadBlockSize)
	{
		ReadWriteMeta* meta = createReadWriteMeta ();
		meta->m_blockSize = size;
		m_readMetaList.insertTail (meta);
	}

	ASSERT (isReadBufferValid ());
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
		ASSERT (meta->m_blockSize <= m_writeBuffer.getDataSize ());

		data->setCount (meta->m_blockSize);
		m_writeBuffer.read (*data, meta->m_blockSize);

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
		m_activeEvents |= AsyncIoEvent_ReceiveBufferFull;

	if (!m_writeBuffer.isFull ())
		m_activeEvents |= AsyncIoEvent_TransmitBufferReady;
}

//..............................................................................

} // namespace io
} // namespace jnc
