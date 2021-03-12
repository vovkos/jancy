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
#include "jnc_io_SshLib.h"
#include "jnc_io_Ssh.h"
#include "jnc_io_SocketCapabilities.h"

namespace jnc {
namespace io {

//..............................................................................

void
initializeSshLibCapabilities()
{
	g_sshCapability = jnc::isCapabilityEnabled("org.jancy.io.ssh");
	initializeSocketCapabilities();
}

//..............................................................................

JNC_DEFINE_LIB_EX(
	SshLib,
	g_sshLibGuid,
	"SshLib",
	"Jancy libSSH2 wrapper extension library",
	initializeSshLibCapabilities
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(SshLib)
	JNC_LIB_IMPORT("io_Ssh.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(SshLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(SshChannel)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(SshLib)
	JNC_MAP_TYPE(SshChannel)
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

	g::getModule()->setTag("jnc_io_ssh");
	err::getErrorMgr()->setRouter(host->m_errorRouter);
	jnc_g_dynamicExtensionLibHost = host;
	jnc::io::initializeSshLibCapabilities();
	return jnc::io::SshLib_getLib();
}

//..............................................................................
