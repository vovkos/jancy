#pragma once

#include "jnc_DataPtrType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"

namespace jnc {
		
//.............................................................................

struct Error: err::ErrorData
{
	JNC_BEGIN_TYPE ("jnc.Error", StdApiSlot_Error)
		JNC_CONST_PROPERTY ("m_description", getDescription_s)
	JNC_END_TYPE ()

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
