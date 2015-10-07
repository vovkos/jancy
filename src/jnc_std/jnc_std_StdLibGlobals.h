#pragma once

namespace jnc {
namespace std {

//.............................................................................

// {CBC2E0EE-A7D5-4DE4-96E5-2A403C6B14B5}
AXL_SL_DEFINE_GUID (
	g_stdLibGuid,
	0xcbc2e0ee, 0xa7d5, 0x4de4, 0x96, 0xe5, 0x2a, 0x40, 0x3c, 0x6b, 0x14, 0xb5
	);

AXL_SELECT_ANY size_t g_stdLibCacheSlot;
AXL_SELECT_ANY ext::ExtensionLibHost* g_stdLibHost;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum StdLibTypeCacheSlot
{	
	StdLibTypeCacheSlot_Error,
	StdLibTypeCacheSlot_String,
	StdLibTypeCacheSlot_StringRef,
	StdLibTypeCacheSlot_StringBuilder,
	StdLibTypeCacheSlot_StringHashTable,
	StdLibTypeCacheSlot_VariantHashTable,
	StdLibTypeCacheSlot_ListEntry,
	StdLibTypeCacheSlot_List,
	StdLibTypeCacheSlot_ConstBuffer,
	StdLibTypeCacheSlot_ConstBufferRef,
	StdLibTypeCacheSlot_BufferRef,
	StdLibTypeCacheSlot_Buffer,
};

//.............................................................................

} // namespace std
} // namespace jnc
