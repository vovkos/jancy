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
#include "jnc_io_IoLib.h"

#include "jnc_io_File.h"
#include "jnc_io_MappedFile.h"
#include "jnc_io_FileStream.h"
#include "jnc_io_Serial.h"
#include "jnc_io_Socket.h"
#include "jnc_io_SocketAddress.h"
#include "jnc_io_HostNameResolver.h"
#include "jnc_io_NetworkAdapter.h"
#include "jnc_io_ChildProcess.h"

#if (_JNC_OS_WIN)
#	include "jnc_io_NamedPipe.h"
#	include "jnc_io_Mailslot.h"
#endif

namespace jnc {
namespace io {

//..............................................................................

DataPtr getSymbolicLinkTarget(DataPtr namePtr)
{
	sl::String linkTarget;
	bool result = axl::io::getSymbolicLinkTarget(&linkTarget, (const char*) namePtr.m_p);
	if (!result)
		return g_nullDataPtr;

	return strDup(linkTarget);
}

DataPtr getTempDir()
{
	return strDup(axl::io::getTempDir());
}

DataPtr getHomeDir()
{
	return strDup(axl::io::getHomeDir());
}

//..............................................................................

JNC_DEFINE_LIB(
	IoLib,
	g_ioLibGuid,
	"IoLib",
	"Jancy standard IO extension library"
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(IoLib)
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(IoLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(File)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(MappedFile)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(FileStream)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Serial)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Socket)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(HostNameResolver)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(ChildProcess)
#if (_JNC_OS_WIN)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(NamedPipe)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Mailslot)
#endif
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(IoLib)
	JNC_MAP_TYPE(File)
	JNC_MAP_TYPE(FileStream)
	JNC_MAP_TYPE(MappedFile)
	JNC_MAP_TYPE(Serial)
	JNC_MAP_TYPE(Socket)
	JNC_MAP_TYPE(Address_ip4)
	JNC_MAP_TYPE(Address_ip6)
	JNC_MAP_TYPE(SocketAddress_ip4)
	JNC_MAP_TYPE(SocketAddress_ip6)
	JNC_MAP_TYPE(SocketAddress)
	JNC_MAP_TYPE(HostNameResolver)
	JNC_MAP_TYPE(ChildProcess)
#if (_JNC_OS_WIN)
	JNC_MAP_TYPE(NamedPipe)
	JNC_MAP_TYPE(Mailslot)
#endif
	JNC_MAP_FUNCTION("io.createNetworkAdapterDescList", createNetworkAdapterDescList)
	JNC_MAP_FUNCTION("io.createSerialPortDescList", createSerialPortDescList)
	JNC_MAP_FUNCTION("io.getSymbolicLinkTarget", getSymbolicLinkTarget)
	JNC_MAP_FUNCTION("io.getTempDir", getTempDir)
	JNC_MAP_FUNCTION("io.getHomeDir", getHomeDir)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

void
initializeCapabilities()
{
	if (isEveryCapabilityEnabled())
		return;

	g_capabilities = 0;

	if (isCapabilityEnabled("org.jancy.io.file"))
		g_capabilities |= IoLibCapability_File;

	if (isCapabilityEnabled("org.jancy.io.file-stream"))
		g_capabilities |= IoLibCapability_FileStream;

	if (isCapabilityEnabled("org.jancy.io.serial"))
		g_capabilities |= IoLibCapability_Serial;

	if (isCapabilityEnabled("org.jancy.io.net"))
		g_capabilities |=
			IoLibCapability_Socket |
			IoLibCapability_NetworkAdapter |
			IoLibCapability_HostNameResolver;

	if (isCapabilityEnabled("org.jancy.io.server"))
		g_capabilities |= IoLibCapability_Server;

	if (isCapabilityEnabled("org.jancy.io.tcp"))
		g_capabilities |= IoLibCapability_Tcp;

	if (isCapabilityEnabled("org.jancy.io.udp"))
		g_capabilities |= IoLibCapability_Udp;

	if (isCapabilityEnabled("org.jancy.io.mailslot"))
		g_capabilities |= IoLibCapability_Mailslot;

	if (isCapabilityEnabled("org.jancy.io.named-pipe"))
		g_capabilities |= IoLibCapability_NamedPipe;

	if (isCapabilityEnabled("org.jancy.io.child-process"))
		g_capabilities |= IoLibCapability_ChildProcess;
}

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

	g::getModule()->setTag("jnc_io_base");
	err::getErrorMgr()->setRouter(host->m_errorRouter);
	jnc_g_dynamicExtensionLibHost = host;
	jnc::io::initializeCapabilities();
	return jnc::io::IoLib_getLib();
}

//..............................................................................
