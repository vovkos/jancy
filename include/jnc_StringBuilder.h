#pragma once

#include "jnc_ClassType.h"
#include "jnc_Api.h"
#include "jnc_StdLibApiSlots.h"

namespace jnc {
		
//.............................................................................

class StringBuilder: public IfaceHdr
{
public:
	JNC_API_BEGIN_CLASS ("jnc.StringBuilder", ApiSlot_jnc_StringBuilder)
	JNC_API_END_CLASS ()
};

//.............................................................................

} // namespace jnc
