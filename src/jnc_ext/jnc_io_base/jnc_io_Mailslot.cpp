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
#include "jnc_io_Mailslot.h"
#include "jnc_io_IoLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	Mailslot,
	"io.Mailslot",
	g_ioLibGuid,
	IoLibCacheSlot_Mailslot,
	Mailslot,
	&Mailslot::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(Mailslot)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<Mailslot>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<Mailslot>)

	JNC_MAP_AUTOGET_PROPERTY("m_readParallelism", &Mailslot::setReadParallelism)
	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize",   &Mailslot::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize",  &Mailslot::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options",         &Mailslot::setOptions)
	JNC_MAP_CONST_PROPERTY("m_osHandle",          &Mailslot::getOsHandle)

	JNC_MAP_FUNCTION("open",         &Mailslot::open)
	JNC_MAP_FUNCTION("close",        &Mailslot::close)
	JNC_MAP_FUNCTION("read",         &Mailslot::read)
	JNC_MAP_FUNCTION("wait",         &Mailslot::wait)
	JNC_MAP_FUNCTION("cancelWait",   &Mailslot::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &Mailslot::blockingWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

Mailslot::Mailslot()
{
	m_readParallelism = Def_ReadParallelism;
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_options = Def_Options;

	m_readBuffer.setBufferSize(Def_ReadBufferSize);

#if (_AXL_OS_WIN)
	m_overlappedIo = NULL;
#endif
}

bool
JNC_CDECL
Mailslot::open(DataPtr namePtr)
{
	close();

	if (!requireIoLibCapability(IoLibCapability_Mailslot))
		return false;

	char buffer[256];
	sl::String_w deviceName(ref::BufKind_Stack, buffer, sizeof(buffer));
	deviceName = L"\\\\.\\mailslot\\";

	sl::StringRef name((const char*) namePtr.m_p);
	if (name.isPrefix("\\\\.\\mailslot\\"))
		deviceName += name.getSubString(deviceName.getLength());
	else
		deviceName += name;

	HANDLE h = ::CreateMailslotW(deviceName, 0, -1, NULL);
	if (h == INVALID_HANDLE_VALUE)
	{
		err::setLastSystemError();
		return false;
	}

	m_file.m_file.attach(h);

	ASSERT(!m_overlappedIo);
	m_overlappedIo = AXL_MEM_NEW(OverlappedIo);

	AsyncIoDevice::open();
	m_ioThread.start();
	return true;
}

void
JNC_CDECL
Mailslot::close()
{
	if (!m_file.isOpen())
		return;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread();
	m_lock.unlock();

	GcHeap* gcHeap = m_runtime->getGcHeap();
	gcHeap->enterWaitRegion();
	m_ioThread.waitAndClose();
	gcHeap->leaveWaitRegion();

	m_file.close();
	AsyncIoDevice::close();

#if (_AXL_OS_WIN)
	if (m_overlappedIo)
	{
		AXL_MEM_DELETE(m_overlappedIo);
		m_overlappedIo = NULL;
	}
#endif
}

void
Mailslot::ioThreadFunc()
{
	ASSERT(m_file.isOpen() && m_overlappedIo);

	bool result;

	HANDLE waitTable[2] =
	{
		m_ioThreadEvent.m_event,
		NULL, // placeholder for read completion event
	};

	size_t waitCount = 1; // always 2 or 3

	m_ioThreadEvent.signal(); // do initial update of active events

	for (;;)
	{
		DWORD waitResult = ::WaitForMultipleObjects(waitCount, waitTable, false, INFINITE);
		if (waitResult == WAIT_FAILED)
		{
			setIoErrorEvent(err::getLastSystemErrorCode());
			return;
		}

		// do as much as we can without lock

		while (!m_overlappedIo->m_activeOverlappedReadList.isEmpty())
		{
			OverlappedRead* read = *m_overlappedIo->m_activeOverlappedReadList.getHead();
			result = read->m_overlapped.m_completionEvent.wait(0);
			if (!result)
				break;

			dword_t actualSize;
			result = m_file.m_file.getOverlappedResult(&read->m_overlapped, &actualSize);
			if (!result)
			{
				setIoErrorEvent();
				return;
			}

			m_overlappedIo->m_activeOverlappedReadList.remove(read);

			// only the main read buffer must be lock-protected

			m_lock.lock();
			addToReadBuffer(read->m_buffer, actualSize);
			m_lock.unlock();

			read->m_overlapped.m_completionEvent.reset();
			m_overlappedIo->m_overlappedReadPool.put(read);
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing)
		{
			m_lock.unlock();
			break;
		}

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		updateReadWriteBufferEvents();

		// take snapshots before releasing the lock

		bool isReadBufferFull = m_readBuffer.isFull();
		size_t readParallelism = m_readParallelism;
		size_t readBlockSize = m_readBlockSize;

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();

		size_t activeReadCount = m_overlappedIo->m_activeOverlappedReadList.getCount();
		if (!isReadBufferFull && activeReadCount < readParallelism)
		{
			size_t newReadCount = readParallelism - activeReadCount;
			for (size_t i = 0; i < newReadCount; i++)
			{
				OverlappedRead* read = m_overlappedIo->m_overlappedReadPool.get();

				result =
					read->m_buffer.setCount(readBlockSize) &&
					m_file.m_file.overlappedRead(read->m_buffer, readBlockSize, &read->m_overlapped);

				if (!result)
				{
					m_overlappedIo->m_overlappedReadPool.put(read);
					setIoErrorEvent();
					return;
				}

				m_overlappedIo->m_activeOverlappedReadList.insertTail(read);
			}
		}

		if (m_overlappedIo->m_activeOverlappedReadList.isEmpty())
		{
			waitCount = 1;
		}
		else
		{
			OverlappedRead* read = *m_overlappedIo->m_activeOverlappedReadList.getHead();
			waitTable[1] = read->m_overlapped.m_completionEvent.m_event;
			waitCount = 2;
		}
	}
}

//..............................................................................

} // namespace io
} // namespace jnc
