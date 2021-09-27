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

AXL_SELECT_ANY bool g_sshCapability = true;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
requireSshCapability() {
	return g_sshCapability || jnc::failWithCapabilityError("org.jancy.io.ssh");
}

//..............................................................................

// {5B43440D-8E4A-4EF5-AA23-D613210EB8E9}
JNC_DEFINE_GUID(
	g_sshLibGuid,
	0x5b43440d, 0x8e4a, 0x4ef5, 0xaa, 0x23, 0xd6, 0x13, 0x21, 0xe, 0xb8, 0xe9
);

enum SshLibCacheSlot {
	SshLibCacheSlot_SshChannel,
};

//..............................................................................

} // namespace io
} // namespace jnc
