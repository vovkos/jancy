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
#include "jnc_io_SslLib.h"
#include "jnc_io_SslCertificate.h"
#include "jnc_io_SslCipher.h"
#include "jnc_io_SslState.h"
#include "jnc_io_SslSocket.h"

namespace jnc {
namespace io {

//..............................................................................

void
initializeSslLibCapabilities()
{
	g_sslCapability = jnc::isCapabilityEnabled("org.jancy.io.ssl");
	initializeSocketCapabilities();
}

//..............................................................................

JNC_DEFINE_LIB_EX(
	SslLib,
	g_sslLibGuid,
	"SslLib",
	"Jancy libSsl2 wrapper extension library",
	initializeSslLibCapabilities
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(SslLib)
	JNC_LIB_IMPORT("io_SslSocket.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(SslLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(SslCertName)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(SslCertificate)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(SslCipher)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(SslState)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(SslSocket)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(SslLib)
	JNC_MAP_FUNCTION("io.getSslNidShortName", getSslNidShortName)
	JNC_MAP_FUNCTION("io.getSslNidLongName", getSslNidLongName)

	JNC_MAP_TYPE(SslCertNameEntry)
	JNC_MAP_TYPE(SslCertName)
	JNC_MAP_TYPE(SslCertificate)
	JNC_MAP_TYPE(SslCipher)
	JNC_MAP_TYPE(SslState)
	JNC_MAP_TYPE(SslSocket)
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
	g::getModule()->setTag("jnc_io_ssl");
	err::getErrorMgr()->setRouter(host->m_errorRouter);
	jnc_g_dynamicExtensionLibHost = host;
	jnc::io::initializeSslLibCapabilities();
	jnc::io::SslState::initAppData();
	return jnc::io::SslLib_getLib();
}

//..............................................................................
