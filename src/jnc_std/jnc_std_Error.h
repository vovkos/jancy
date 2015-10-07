#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_std_StdLibGlobals.h"

namespace jnc {
namespace std {
		
//.............................................................................

struct Error: err::ErrorHdr
{
	JNC_BEGIN_TYPE_MAP ("std.Error", g_stdLibCacheSlot, StdLibTypeCacheSlot_Error)
		JNC_MAP_CONST_PROPERTY ("m_description", getDescription_s)
	JNC_END_TYPE_MAP ()

public:
	rt::DataPtr
	getDescription ();

protected:
	static
	rt::DataPtr
	getDescription_s (rt::DataPtr selfPtr)
	{
		return ((Error*) selfPtr.m_p)->getDescription ();
	}
};

//.............................................................................

} // namespace std
} // namespace jnc
