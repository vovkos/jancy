#pragma once

#include "jnc_Def.h"

namespace jnc {
namespace sys {

//..............................................................................

// {D8C0847C-93D5-4146-B795-5DB1A111855A}
JNC_DEFINE_GUID (
	g_sysLibGuid,
	0xd8c0847c, 0x93d5, 0x4146, 0xb7, 0x95, 0x5d, 0xb1, 0xa1, 0x11, 0x85, 0x5a
	);

enum SysLibCacheSlot
{
	SysLibCacheSlot_Lock,
	SysLibCacheSlot_Event,
	SysLibCacheSlot_Thread,
	SysLibCacheSlot_Timer,
};

//..............................................................................

} // namespace sys
} // namespace jnc
