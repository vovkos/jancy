#pragma once

namespace jnc {
namespace ext {

AXL_SELECT_ANY ExtensionLibHost* g_extensionLibHost;

} // namespace ext

namespace io {

//.............................................................................

// {5B43440D-8E4A-4EF5-AA23-D613210EB8E9}
AXL_SL_DEFINE_GUID (
	g_sshLibGuid,
	0x5b43440d, 0x8e4a, 0x4ef5, 0xaa, 0x23, 0xd6, 0x13, 0x21, 0xe, 0xb8, 0xe9
	);

AXL_SELECT_ANY size_t g_sshLibCacheSlot;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum SshLibCacheSlot
{	
	SshLibCacheSlot_SshChannel,
	SshLibCacheSlot_SshEventParams,
};

//.............................................................................

} // namespace io
} // namespace jnc
