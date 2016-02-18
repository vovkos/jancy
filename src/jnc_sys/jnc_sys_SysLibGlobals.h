#pragma once

namespace jnc {
namespace sys {

//.............................................................................

// {D8C0847C-93D5-4146-B795-5DB1A111855A}
AXL_SL_DEFINE_GUID (
	g_sysLibGuid,
	0xd8c0847c, 0x93d5, 0x4146, 0xb7, 0x95, 0x5d, 0xb1, 0xa1, 0x11, 0x85, 0x5a
	);

AXL_SELECT_ANY size_t g_sysLibCacheSlot;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum SysLibTypeCacheSlot
{	
	SysLibTypeCacheSlot_Lock,
	SysLibTypeCacheSlot_Event,
	SysLibTypeCacheSlot_Thread,
	SysLibTypeCacheSlot_Timer,
};

//.............................................................................

} // namespace sys
} // namespace jnc
