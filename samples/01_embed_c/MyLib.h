#pragma once

// Allocation on GC heap requires type pointer but looking up type by name
// during every allocation request is unnecessary expensive. Much better
// approach is to cache types in a table after name lookup succeseeded.

// The enum below lists all the named types in API exported to jancy script.

//..............................................................................

// {1D64EF25-4DA8-41A2-B6E7-007405D3AC39}
JNC_DEFINE_GUID (
	g_myLibGuid,
	0x1d64ef25, 0x4da8, 0x41a2, 0xb6, 0xe7, 0x0, 0x74, 0x5, 0xd3, 0xac, 0x39
	);

enum MyLibCacheSlot
{
	MyLibCacheSlot_TestStruct,
	MyLibCacheSlot_TestClass,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DECLARE_LIB (MyLib)

//..............................................................................
