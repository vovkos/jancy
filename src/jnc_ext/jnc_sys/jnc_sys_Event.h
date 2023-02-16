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
#include "jnc_ExtensionLib.h"
#include "jnc_rt_Event.h"

namespace jnc {
namespace sys {

JNC_DECLARE_OPAQUE_CLASS_TYPE(Event)
JNC_DECLARE_OPAQUE_CLASS_TYPE(NotificationEvent)

//..............................................................................

template <typename T>
class EventBase: public IfaceHdr {
protected:
	rt::EventBase<T> m_event;

public:
	void
	JNC_CDECL
	signal() {
		m_event.signal();
	}

	void
	JNC_CDECL
	reset() {
		m_event.reset();
	}

	bool
	JNC_CDECL
	wait(uint_t timeout) {
		return m_event.wait(timeout);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef EventBase<axl::sys::Event> Event;
typedef EventBase<axl::sys::NotificationEvent> NotificationEvent;

//..............................................................................

} // namespace sys
} // namespace jnc
