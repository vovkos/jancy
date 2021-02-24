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

namespace jnc {
namespace io {

//..............................................................................

enum IoLibCapability
{
	IoLibCapability_File             = 0x0001,
	IoLibCapability_FileStream       = 0x0002,
	IoLibCapability_Serial           = 0x0004,
	IoLibCapability_Socket           = 0x0008,
	IoLibCapability_Server           = 0x0010,
	IoLibCapability_Tcp              = 0x0020,
	IoLibCapability_Udp              = 0x0040,
	IoLibCapability_NetworkAdapter   = 0x0080,
	IoLibCapability_HostNameResolver = 0x0100,
	IoLibCapability_Mailslot         = 0x0200,
	IoLibCapability_NamedPipe        = 0x0400,
	IoLibCapability_ChildProcess     = 0x0800,
};

AXL_SELECT_ANY uint_t g_capabilities = -1;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum IoLibCacheSlot
{
	IoLibCacheSlot_Serial,
	IoLibCacheSlot_SerialPortDesc,
	IoLibCacheSlot_Address_ip4,
	IoLibCacheSlot_Address_ip6,
	IoLibCacheSlot_SocketAddress_ip4,
	IoLibCacheSlot_SocketAddress_ip6,
	IoLibCacheSlot_SocketAddress,
	IoLibCacheSlot_HostNameResolver,
	IoLibCacheSlot_Socket,
	IoLibCacheSlot_NetworkAdapterAddress,
	IoLibCacheSlot_NetworkAdapterDesc,
	IoLibCacheSlot_File,
	IoLibCacheSlot_MappedFile,
	IoLibCacheSlot_FileStream,
	IoLibCacheSlot_NamedPipe,
	IoLibCacheSlot_Mailslot,
	IoLibCacheSlot_ChildProcess,
};

// {362FF8E2-1BDD-4319-AF8F-AD86C3917AC5}
JNC_DEFINE_GUID(
	g_ioLibGuid,
	0x362ff8e2, 0x1bdd, 0x4319, 0xaf, 0x8f, 0xad, 0x86, 0xc3, 0x91, 0x7a, 0xc5
	);

//..............................................................................

} // namespace io
} // namespace jnc
