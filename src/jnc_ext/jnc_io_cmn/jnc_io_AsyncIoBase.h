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

#pragma once

namespace jnc {
namespace io {

//..............................................................................

enum AsyncIoBaseEvent
{
	AsyncIoBaseEvent_IoError = 0x0001,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class AsyncIoBase
{
protected:
	enum IoThreadFlag
	{
		IoThreadFlag_Closing   = 0x0001,
		IoThreadFlag_Suspended = 0x0002,
	};

	struct Wait: sl::ListLink
	{
		uint_t m_mask;
	};

	struct SyncWait: Wait
	{
		sys::Event* m_event;
	};

	struct AsyncWait: Wait
	{
		FunctionPtr m_handlerPtr;
		handle_t m_handle;
	};

public:
	uint_t m_options;
	uint_t m_activeEvents;
	DataPtr m_ioErrorPtr;

	bool m_isOpen;

protected:
	Runtime* m_runtime;
	sys::Lock m_lock;
	volatile uint_t m_ioThreadFlags;
	sl::AuxList<SyncWait> m_syncWaitList;
	sl::List<AsyncWait> m_asyncWaitList;
	sl::List<AsyncWait> m_pendingAsyncWaitList;
	sl::HandleTable<AsyncWait*> m_asyncWaitMap;

#if (_JNC_OS_WIN)
	sys::Event m_ioThreadEvent;
#else
	axl::io::psx::Pipe m_ioThreadSelfPipe; // for self-pipe trick
#endif

public:
	AsyncIoBase();

protected:
	void
	markOpaqueGcRoots(jnc::GcHeap* gcHeap);

	handle_t
	wait(
		uint_t eventMask,
		FunctionPtr handlerPtr
		);

	bool
	cancelWait(handle_t handle);

	uint_t
	blockingWait(
		uint_t eventMask,
		uint_t timeout
		);

	void
	open();

	void
	close();

	void
	wakeIoThread();

	void
	sleepIoThread();

	void
	suspendIoThread(bool isSuspended);

	template <typename T>
	void
	setSetting(
		T* p,
		T value
		)
	{
		if (!m_isOpen)
		{
			*p = value;
			return;
		}

		m_lock.lock();
		*p = value;
		wakeIoThread();
		m_lock.unlock();
	}

	void
	cancelAllWaits();

	size_t
	processWaitLists_l();

	void
	setEvents_l(uint_t events);

	void
	setEvents(uint_t events)
	{
		m_lock.lock();
		setEvents_l(events);
	}

	void
	setIoErrorEvent_l(
		uint_t event,
		const err::Error& error
		);

	void
	setIoErrorEvent_l(const err::Error& error)
	{
		setIoErrorEvent_l(AsyncIoBaseEvent_IoError, error);
	}

	void
	setIoErrorEvent_l()
	{
		setIoErrorEvent_l(AsyncIoBaseEvent_IoError, err::getLastError());
	}

	void
	setIoErrorEvent(
		uint_t event,
		const err::Error& error
		)
	{
		m_lock.lock();
		setIoErrorEvent_l(event, error);
	}

	void
	setIoErrorEvent(const err::Error& error)
	{
		setIoErrorEvent(AsyncIoBaseEvent_IoError, error);
	}

	void
	setIoErrorEvent()
	{
		setIoErrorEvent(AsyncIoBaseEvent_IoError, err::getLastError());
	}
};

//..............................................................................

} // namespace io
} // namespace jnc
