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

#include "pch.h"
#include "jnc_io_SocketCapabilities.h"

namespace jnc {
namespace io {

//..............................................................................

void
initializeSocketCapabilities() {
	if (isEveryCapabilityEnabled()) {
		g_socketCapabilities = -1;
		return;
	}

	g_socketCapabilities = 0;

	if (isCapabilityEnabled("org.jancy.io.server"))
		g_socketCapabilities |= SocketCapability_Server;

	if (isCapabilityEnabled("org.jancy.io.ip4"))
		g_socketCapabilities |= SocketCapability_Ip4;

	if (isCapabilityEnabled("org.jancy.io.ip6"))
		g_socketCapabilities |= SocketCapability_Ip6;

	if (isCapabilityEnabled("org.jancy.io.icmp"))
		g_socketCapabilities |= SocketCapability_Icmp;

	if (isCapabilityEnabled("org.jancy.io.tcp"))
		g_socketCapabilities |= SocketCapability_Tcp;

	if (isCapabilityEnabled("org.jancy.io.udp"))
		g_socketCapabilities |= SocketCapability_Udp;

	if (isCapabilityEnabled("org.jancy.io.raw"))
		g_socketCapabilities |= SocketCapability_Raw;
}

bool
failWithSocketCapabilityError(SocketCapability capability) {
	const char* stringTable[] = {
		"org.jancy.io.server", // SocketCapability_Server
		"org.jancy.io.ip4",    // SocketCapability_Ip4
		"org.jancy.io.ip6",    // SocketCapability_Ip6
		"org.jancy.io.icmp",   // SocketCapability_Icmp
		"org.jancy.io.tcp",    // SocketCapability_Tcp
		"org.jancy.io.udp",    // SocketCapability_Udp
		"org.jancy.io.raw",    // SocketCapability_Raw
	};

	size_t i = sl::getLoBitIdx(capability);
	return failWithCapabilityError(
		i < countof(stringTable) ?
			stringTable[i] :
			"org.jancy.io.?"
	);
}

//..............................................................................

} // namespace io
} // namespace jnc
