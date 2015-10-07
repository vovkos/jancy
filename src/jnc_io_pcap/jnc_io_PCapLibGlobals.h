#pragma once

namespace jnc {
namespace ext {

AXL_SELECT_ANY ext::ExtensionLibHost* g_extensionLibHost;

} // namespace ext

namespace io {

//.............................................................................

// {72C7158B-F297-4F88-83A7-96E7FB548B29}
AXL_SL_DEFINE_GUID (
	g_pcapLibGuid,
	0x72c7158b, 0xf297, 0x4f88, 0x83, 0xa7, 0x96, 0xe7, 0xfb, 0x54, 0x8b, 0x29
	);

AXL_SELECT_ANY size_t g_pcapLibCacheSlot;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum PCapLibTypeCacheSlot
{	
	PCapLibTypeCacheSlot_PCap,
	PCapLibTypeCacheSlot_PCapEventParams,
	PCapLibTypeCacheSlot_PCapAddress,
	PCapLibTypeCacheSlot_PCapDeviceDesc,
};

//.............................................................................

} // namespace io
} // namespace jnc
