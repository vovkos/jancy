#pragma once

//.............................................................................

// Allocation on GC heap requires type pointer but looking up type by name 
// during every allocation request is unnecessary expensive. Much better 
// approach is to cache types in a table after name lookup succeseeded. 

// This enum lists all the named types in API exported to jancy script.

enum ApiSlot
{
	ApiSlot_TestStruct,
	ApiSlot_TestClass,
};

//.............................................................................
