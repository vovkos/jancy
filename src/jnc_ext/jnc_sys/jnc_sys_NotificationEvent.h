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

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace sys {

JNC_DECLARE_OPAQUE_CLASS_TYPE(NotificationEvent)

//..............................................................................

class NotificationEvent: public IfaceHdr
{
public:
	axl::sys::NotificationEvent m_event;

public:
	void
	JNC_CDECL
	signal()
	{
		m_event.signal();
	}

	void
	JNC_CDECL
	reset()
	{
		m_event.reset();
	}

	bool
	JNC_CDECL
	wait(uint_t timeout);
};

//..............................................................................

} // namespace sys
} // namespace jnc
