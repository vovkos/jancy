#pragma once

namespace jnc {

//.............................................................................

// {362FF8E2-1BDD-4319-AF8F-AD86C3917AC5}
AXL_RTL_DEFINE_GUID (
	g_ioLibGuid,
	0x362ff8e2, 0x1bdd, 0x4319, 0xaf, 0x8f, 0xad, 0x86, 0xc3, 0x91, 0x7a, 0xc5
	);

AXL_SELECT_ANY size_t g_ioLibSlot;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum IoLibTypeSlot
{	
	IoLibTypeSlot_Socket,
	IoLibTypeSlot_Serial,
	IoLibTypeSlot_File,
};

//.............................................................................

} // namespace jnc {
