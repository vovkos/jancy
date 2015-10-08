#pragma once

namespace jnc {
namespace ext {

AXL_SELECT_ANY ExtensionLibHost* g_extensionLibHost;

} // namespace ext

namespace io {

//.............................................................................

// {362FF8E2-1BDD-4319-AF8F-AD86C3917AC5}
AXL_SL_DEFINE_GUID (
	g_ioLibGuid,
	0x362ff8e2, 0x1bdd, 0x4319, 0xaf, 0x8f, 0xad, 0x86, 0xc3, 0x91, 0x7a, 0xc5
	);

AXL_SELECT_ANY size_t g_ioLibCacheSlot;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum IoLibTypeCacheSlot
{	
	IoLibTypeCacheSlot_Serial,
	IoLibTypeCacheSlot_SerialEventParams,
	IoLibTypeCacheSlot_SerialPortDesc,
	IoLibTypeCacheSlot_Address_ip4,
	IoLibTypeCacheSlot_Address_ip6,
	IoLibTypeCacheSlot_SocketAddress_ip4,
	IoLibTypeCacheSlot_SocketAddress_ip6,
	IoLibTypeCacheSlot_SocketAddress,
	IoLibTypeCacheSlot_SocketAddressResolver,
	IoLibTypeCacheSlot_SocketAddressResolverEventParams,
	IoLibTypeCacheSlot_Socket,
	IoLibTypeCacheSlot_SocketEventParams,
	IoLibTypeCacheSlot_NetworkAdapterAddress,
	IoLibTypeCacheSlot_NetworkAdapterDesc,
	IoLibTypeCacheSlot_MappedFile,
	IoLibTypeCacheSlot_FileStream,
	IoLibTypeCacheSlot_FileStreamEventParams,
	IoLibTypeCacheSlot_NamedPipe,
};

//.............................................................................

} // namespace io
} // namespace jnc
