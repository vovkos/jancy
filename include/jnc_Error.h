#pragma once

#include "jnc_DataPtrType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"

namespace jnc {
		
//.............................................................................

struct Error: err::ErrorData
{
	JNC_API_BEGIN_TYPE ("jnc.Error", ApiSlot_jnc_Error)
		JNC_API_CONST_PROPERTY ("m_description", getDescription_s)
	JNC_API_END_TYPE ()

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
