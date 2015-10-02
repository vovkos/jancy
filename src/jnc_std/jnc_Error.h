#pragma once

#include "jnc_StdLibSlots.h"

namespace jnc {
		
//.............................................................................

struct Error: err::ErrorData
{
	JNC_BEGIN_TYPE_MAP ("jnc.Error", g_stdLibSlot, StdLibTypeSlot_Error)
		JNC_MAP_CONST_PROPERTY ("m_description", getDescription_s)
	JNC_END_TYPE_MAP ()

public:
	DataPtr
	getDescription ();

protected:
	static
	DataPtr
	getDescription_s (DataPtr selfPtr)
	{
		return ((Error*) selfPtr.m_p)->getDescription ();
	}
};

//.............................................................................

} // namespace jnc
