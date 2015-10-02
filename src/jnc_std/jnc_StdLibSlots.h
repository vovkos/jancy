#pragma once

namespace jnc {

//.............................................................................

// {CBC2E0EE-A7D5-4DE4-96E5-2A403C6B14B5}
AXL_RTL_DEFINE_GUID (
	g_stdLibGuid,
	0xcbc2e0ee, 0xa7d5, 0x4de4, 0x96, 0xe5, 0x2a, 0x40, 0x3c, 0x6b, 0x14, 0xb5
	);


AXL_SELECT_ANY size_t g_stdLibSlot;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum StdLibTypeSlot
{	
	StdLibTypeSlot_Error,
	StdLibTypeSlot_String,
	StdLibTypeSlot_StringRef,
	StdLibTypeSlot_StringBuilder,
	StdLibTypeSlot_StringHashTable,
	StdLibTypeSlot_VariantHashTable,
	StdLibTypeSlot_ListEntry,
	StdLibTypeSlot_List,
	StdLibTypeSlot_ConstBuffer,
	StdLibTypeSlot_ConstBufferRef,
	StdLibTypeSlot_BufferRef,
	StdLibTypeSlot_Buffer,
};

//.............................................................................

} // namespace jnc {
