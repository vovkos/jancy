#pragma once

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace sys {

JNC_DECLARE_OPAQUE_CLASS_TYPE (Lock)

//.............................................................................

class Lock: public IfaceHdr
{
public:
	axl::sys::Lock m_lock;

public:
	void
	JNC_CDECL
	lock ();

	void
	JNC_CDECL
	unlock ()
	{
		m_lock.unlock ();
	}
};

//.............................................................................

} // namespace sys
} // namespace jnc
