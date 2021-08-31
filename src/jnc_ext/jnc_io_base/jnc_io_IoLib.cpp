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

void
initializeIoLibCapabilities()
{
	if (isEveryCapabilityEnabled())
	{
		g_ioLibCapabilities = -1;
		initializeSocketCapabilities();
		return;
	}

	g_ioLibCapabilities = 0;

	if (isCapabilityEnabled("org.jancy.io.file"))
		g_ioLibCapabilities |= IoLibCapability_File;

	if (isCapabilityEnabled("org.jancy.io.file-stream"))
		g_ioLibCapabilities |= IoLibCapability_FileStream;

	if (isCapabilityEnabled("org.jancy.io.serial"))
		g_ioLibCapabilities |= IoLibCapability_Serial;

#if (_JNC_OS_WIN)
	if (isCapabilityEnabled("org.jancy.io.win.named-pipe"))
		g_ioLibCapabilities |= IoLibCapability_NamedPipe;

	if (isCapabilityEnabled("org.jancy.io.win.mailslot"))
		g_ioLibCapabilities |= IoLibCapability_Mailslot;
#endif

	if (isCapabilityEnabled("org.jancy.io.child-process"))
		g_ioLibCapabilities |= IoLibCapability_ChildProcess;

	if (isCapabilityEnabled("org.jancy.io.net"))
		g_ioLibCapabilities |=
			IoLibCapability_NetworkAdapter |
			IoLibCapability_HostNameResolver;

	initializeSocketCapabilities();
}

bool
failWithIoLibCapabilityError(IoLibCapability capability)
{
	const char* stringTable[] =
	{
		"org.jancy.io.file",            // IoLibCapability_File
		"org.jancy.io.file-stream",     // IoLibCapability_FileStream
		"org.jancy.io.serial",          // IoLibCapability_Serial
		"org.jancy.io.named-pipe",      // IoLibCapability_NamedPipe
		"org.jancy.io.mailslot",        // IoLibCapability_Mailslot
		"org.jancy.io.child-process",   // IoLibCapability_ChildProcess
		"org.jancy.io.network-adapter", // IoLibCapability_NetworkAdapter
		"org.jancy.io.dns",             // IoLibCapability_HostNameResolver
		"org.jancy.io.server",          // IoLibCapability_Server
		"org.jancy.io.ip4",             // IoLibCapability_Ip4
		"org.jancy.io.ip6",             // IoLibCapability_Ip6
		"org.jancy.io.icmp",            // IoLibCapability_Icmp
		"org.jancy.io.tcp",             // IoLibCapability_Tcp
		"org.jancy.io.udp",             // IoLibCapability_Udp

	};

	size_t i = sl::getLoBitIdx(capability);

	return failWithCapabilityError(
		i < countof(stringTable) ?
			stringTable[i] :
			"org.jancy.io.?"
		);
}

//..............................................................................

JNC_DEFINE_LIB_EX(
	IoLib,
	g_ioLibGuid,
	"IoLib",
	"Jancy standard IO extension library",
	initializeIoLibCapabilities
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
	jnc::io::initializeIoLibCapabilities();
	return jnc::io::IoLib_getLib();
}

//..............................................................................
