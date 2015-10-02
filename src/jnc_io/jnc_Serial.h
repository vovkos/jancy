#pragma once

#include "jnc_IoLibSlots.h"

namespace jnc {
		
//.............................................................................

class Serial: public IfaceHdr
{
public:
	JNC_BEGIN_TYPE_MAP ("jnc.Serial", g_ioLibSlot, IoLibTypeSlot_Serial)
	JNC_END_TYPE_MAP ()

public:
};

//.............................................................................

} // namespace jnc
