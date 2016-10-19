#pragma once

#include "jnc_ExtensionLib.h"

namespace jnc {
namespace std {

JNC_DECLARE_TYPE (Error)

//..............................................................................

struct Error: err::ErrorHdr
{
	DataPtr
	getDescription ();

	static
	DataPtr
	getDescription_s (DataPtr selfPtr)
	{
		return ((Error*) selfPtr.m_p)->getDescription ();
	}
};

//..............................................................................

} // namespace std
} // namespace jnc
