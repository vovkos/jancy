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

#include "jnc_Runtime.h"

namespace jnc {
namespace rt {

//..............................................................................

template <typename T>
class EventBase: public T {
#if (_JNC_OS_POSIX)
protected:
	enum {
		WaitGranularity = 500,
	};
#endif

public:
	// on POSIX, we currently use pthread condition variables for events.
	// unfortunately, there is no cross-platform way to interrupt pthread_cond_wait.
	// as such, we use pthread_cond_timedwait and check for the abort flag in between

	bool
	wait(uint_t timeout) {
		bool result;

		uint64_t endTimestamp = timeout != -1 ?	axl::sys::getTimestamp() + (uint64_t)timeout * 10000 : -1;
		Runtime* runtime = getCurrentThreadRuntime();
		ASSERT(runtime);

		GcHeap* gcHeap = runtime->getGcHeap();
		gcHeap->enterWaitRegion();

		for (;;) {
#if (_JNC_OS_WIN)
			axl::sys::win::WaitResult waitResult;

			if (endTimestamp == -1)
				waitResult = m_event.wait(-1, true);
			else {
				uint64_t timestamp = axl::sys::getTimestamp();
				timeout = timestamp >= endTimestamp ? 0 : (uint_t)((endTimestamp - timestamp) / 10000);
				waitResult = m_event.wait(timeout, true);
			}

			if (waitResult == axl::sys::win::WaitResult_IoCompletion &&	!runtime->isAborted())
				continue;

			result = waitResult == axl::sys::win::WaitResult_Object0;
			break;
#else
			uint64_t timestamp = axl::sys::getTimestamp();
			if (timestamp >= endTimestamp)
				timeout = 0;
			else {
				timeout = (uint_t)((endTimestamp - timestamp) / 10000);
				if (timeout > WaitGranularity)
					timeout = WaitGranularity;
			}

			result = T::wait(timeout);
			if (result || !timeout || runtime->isAborted())
				break;
#endif
		}

		gcHeap->leaveWaitRegion();
		return result;
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef EventBase<axl::sys::Event> Event;
typedef EventBase<axl::sys::NotificationEvent> NotificationEvent;

//..............................................................................

} // namespace rt
} // namespace jnc
