#pragma once

#include "jnc_IoLibSlots.h"

namespace jnc {

//.............................................................................

class Socket: public IfaceHdr
{
public:
	JNC_BEGIN_CLASS_TYPE_MAP ("jnc.Socket", g_ioLibSlot, IoLibTypeSlot_Socket)
	JNC_END_CLASS_TYPE_MAP ()

public:
};

//.............................................................................

} // namespace jnc
