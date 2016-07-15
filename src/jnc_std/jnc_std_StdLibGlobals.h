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

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum StdLibCacheSlot
{	
	StdLibCacheSlot_Error,
	StdLibCacheSlot_String,
	StdLibCacheSlot_StringRef,
	StdLibCacheSlot_StringBuilder,
	StdLibCacheSlot_StringHashTable,
	StdLibCacheSlot_VariantHashTable,
	StdLibCacheSlot_ListEntry,
	StdLibCacheSlot_List,
	StdLibCacheSlot_ConstBuffer,
	StdLibCacheSlot_ConstBufferRef,
	StdLibCacheSlot_BufferRef,
	StdLibCacheSlot_Buffer,
};

//.............................................................................

} // namespace std
} // namespace jnc
