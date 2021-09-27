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

enum SocketCapability {
	SocketCapability_Server = 0x01,
	SocketCapability_Ip4    = 0x02,
	SocketCapability_Ip6    = 0x04,
	SocketCapability_Icmp   = 0x08,
	SocketCapability_Tcp    = 0x10,
	SocketCapability_Udp    = 0x20,
	SocketCapability_Raw    = 0x40,
};

AXL_SELECT_ANY uint_t g_socketCapabilities = -1;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initializeSocketCapabilities();

bool
failWithSocketCapabilityError(SocketCapability capability);

inline
bool
requireSocketCapability(SocketCapability capability) {
	return (g_socketCapabilities & capability) || failWithSocketCapabilityError(capability);
}

//..............................................................................

} // namespace io
} // namespace jnc
