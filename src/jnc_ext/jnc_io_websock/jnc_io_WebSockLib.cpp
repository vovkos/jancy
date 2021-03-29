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
#include "jnc_io_WebSockLib.h"
#include "jnc_io_WebSocket.h"

namespace jnc {
namespace io {

//..............................................................................

void
initializeWebSockLibCapabilities()
{
	g_webSockCapability = jnc::isCapabilityEnabled("org.jancy.io.websocket");
	initializeSocketCapabilities();
}

//..............................................................................

JNC_DEFINE_LIB_EX(
	WebSockLib,
	g_webSockLibGuid,
	"WebSockLib",
	"Jancy libSsl2 wrapper extension library",
	initializeWebSockLibCapabilities
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(WebSockLib)
	JNC_LIB_IMPORT("io_WebSocket.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(WebSockLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(WebSocket)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(WebSockLib)
	JNC_MAP_TYPE(WebSocket)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

} // namespace io
} // namespace jnc

jnc_DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;

JNC_EXTERN_C
JNC_EXPORT
jnc_ExtensionLib*
jncDynamicExtensionLibMain(jnc_DynamicExtensionLibHost* host)
{
#if (_JNC_OS_WIN)
	WSADATA WsaData;
	WSAStartup(0x0202, &WsaData);
#endif

	::SSL_library_init();
	g::getModule()->setTag("jnc_io_websock");
	err::getErrorMgr()->setRouter(host->m_errorRouter);
	jnc_g_dynamicExtensionLibHost = host;
	jnc::io::initializeWebSockLibCapabilities();
	return jnc::io::WebSockLib_getLib();
}

//..............................................................................
