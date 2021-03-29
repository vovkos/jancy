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

AXL_SELECT_ANY bool g_sslCapability = true;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
requireSslCapability()
{
	return g_sslCapability || jnc::failWithCapabilityError("org.jancy.io.ssl");
}

//..............................................................................

// {275366FE-EA5D-48C9-9414-640E1D43339B}
JNC_DEFINE_GUID(
	g_sslLibGuid,
	0x275366fe, 0xea5d, 0x48c9, 0x94, 0x14, 0x64, 0xe, 0x1d, 0x43, 0x33, 0x9b
	);

enum SslLibCacheSlot
{
	SslLibCacheSlot_SslCertNameEntry,
	SslLibCacheSlot_SslCertName,
	SslLibCacheSlot_SslCipher,
	SslLibCacheSlot_SslCertificate,
	SslLibCacheSlot_SslState,
	SslLibCacheSlot_SslSocket,
};

//..............................................................................

} // namespace io
} // namespace jnc
