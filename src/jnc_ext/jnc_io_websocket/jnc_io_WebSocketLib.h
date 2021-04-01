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

AXL_SELECT_ANY bool g_webSocketCapability = true;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
bool
requireWebSocketCapability()
{
	return g_webSocketCapability || jnc::failWithCapabilityError("org.jancy.io.websocket");
}

//..............................................................................

// {012e10bc-5a2e-412e-aeaa-bbc04463fa7f}
JNC_DEFINE_GUID(
	g_webSocketLibGuid,
	0x012e10bc, 0x5a2e, 0x412e, 0xae, 0xaa, 0xbb, 0xc0, 0x44, 0x63, 0xfa, 0x7f
	);

enum WebSocketLibCacheSlot
{
	WebSocketLibCacheSlot_WebSocket,
};

//..............................................................................

} // namespace io
} // namespace jnc
