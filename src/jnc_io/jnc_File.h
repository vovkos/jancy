#pragma once

#include "jnc_IoLibSlots.h"

namespace jnc {

//.............................................................................

class File: public IfaceHdr
{
public:
	JNC_BEGIN_TYPE_MAP ("jnc.File", g_ioLibSlot, IoLibTypeSlot_File)
	JNC_END_TYPE_MAP ()

public:
};

//.............................................................................

} // namespace jnc
