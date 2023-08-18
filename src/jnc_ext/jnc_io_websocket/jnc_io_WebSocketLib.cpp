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
#include "jnc_io_WebSocketLib.h"
#include "jnc_io_WebSocketLibDep.h"
#include "jnc_io_WebSocket.h"
#include "jnc_io_WebSocketHandshake.h"
#include "jnc_io_WebSocketHandshakeHeaders.h"

namespace jnc {
namespace io {

//..............................................................................

void
initializeWebSocketLibCapabilities() {
	g_webSocketCapability = jnc::isCapabilityEnabled("org.jancy.io.websocket");
	initializeSocketCapabilities();
}

//..............................................................................

JNC_DEFINE_LIB_EX(
	WebSocketLib,
	g_webSocketLibGuid,
	"WebSocketLib",
	"Jancy libSsl2 wrapper extension library",
	initializeWebSocketLibCapabilities
)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(WebSocketLib)
	JNC_LIB_IMPORT("io_WebSocket.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(WebSocketLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(WebSocket)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(WebSocketHandshake)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(WebSocketHandshakeHeaders)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(WebSocketLib)
	JNC_MAP_TYPE(WebSocket)
	JNC_MAP_TYPE(WebSocketHandshake)
	JNC_MAP_TYPE(WebSocketHandshakeHeaders)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

} // namespace io
} // namespace jnc

jnc_DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;

JNC_EXTERN_C
JNC_EXPORT
jnc_ExtensionLib*
jncDynamicExtensionLibMain(jnc_DynamicExtensionLibHost* host) {
#if (_JNC_OS_WIN)
	WSADATA WsaData;
	WSAStartup(0x0202, &WsaData);
#endif

	::SSL_library_init();
	err::getErrorMgr()->setRouter(host->m_errorRouter);
	jnc_g_dynamicExtensionLibHost = host;
	jnc::io::initializeWebSocketLibCapabilities();
	return jnc::io::WebSocketLib_getLib();
}

//..............................................................................
