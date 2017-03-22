//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_Def.h"

namespace jnc {
namespace std {

//..............................................................................

// {CBC2E0EE-A7D5-4DE4-96E5-2A403C6B14B5}
JNC_DEFINE_GUID (
	g_stdLibGuid,
	0xcbc2e0ee, 0xa7d5, 0x4de4, 0x96, 0xe5, 0x2a, 0x40, 0x3c, 0x6b, 0x14, 0xb5
	);

enum StdLibCacheSlot
{
	StdLibCacheSlot_Error,
	StdLibCacheSlot_String,
	StdLibCacheSlot_StringRef,
	StdLibCacheSlot_StringBuilder,
	StdLibCacheSlot_MapEntry,
	StdLibCacheSlot_HashTable,
	StdLibCacheSlot_StringHashTable,
	StdLibCacheSlot_VariantHashTable,
	StdLibCacheSlot_ListEntry,
	StdLibCacheSlot_List,
	StdLibCacheSlot_Array,
	StdLibCacheSlot_ConstBuffer,
	StdLibCacheSlot_ConstBufferRef,
	StdLibCacheSlot_BufferRef,
	StdLibCacheSlot_Buffer,
};

//..............................................................................

} // namespace std
} // namespace jnc
