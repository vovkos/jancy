#pragma once

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace sys {

JNC_DECLARE_OPAQUE_CLASS_TYPE (Event)

//.............................................................................

class Event: public IfaceHdr
{
public:
	axl::sys::Event m_event;

public:
	void
	AXL_CDECL
	signal ()
	{
		m_event.signal ();
	}

	bool
	AXL_CDECL
	wait (uint_t timeout);
};

//.............................................................................

} // namespace sys
} // namespace jnc
