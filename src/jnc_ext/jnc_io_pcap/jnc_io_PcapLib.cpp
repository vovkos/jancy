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
#include "jnc_io_PcapLib.h"
#include "jnc_io_PcapLibDep.h"
#include "jnc_io_Pcap.h"
#include "jnc_io_PcapFilter.h"

#if (_JNC_OS_POSIX)
#	include "jnc_io_PcapSignalMgr.h"
#endif

namespace jnc {
namespace io {

//..............................................................................

void
initializePcapLibCapabilities() {
	g_pcapCapability = jnc::isCapabilityEnabled("org.jancy.io.pcap");
}

//..............................................................................

JNC_DEFINE_LIB_EX(
	PcapLib,
	g_pcapLibGuid,
	"PcapLib",
	"Jancy libPcap wrapper extension library",
	initializePcapLibCapabilities
)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(PcapLib)
	JNC_LIB_IMPORT("io_Pcap.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(PcapLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(Pcap)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(PcapFilter)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(PcapLib)
	JNC_MAP_TYPE(Pcap)
	JNC_MAP_TYPE(PcapFilter)
	JNC_MAP_FUNCTION_Q("io.createPcapDeviceDescList", &createPcapDeviceDescList)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

} // namespace io
} // namespace jnc

jnc_DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;

#if (_JNC_OS_WIN)
const char*
getPcapVersion() {
	const char* version;

	__try {
		version = ::pcap_lib_version();
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		version = NULL;
	}

	return version;
}
#endif

JNC_EXTERN_C
JNC_EXPORT
jnc_ExtensionLib*
jncDynamicExtensionLibMain(jnc_DynamicExtensionLibHost* host) {
	err::getErrorMgr()->setRouter(host->m_errorRouter);
	jnc_g_dynamicExtensionLibHost = host;
	jnc::io::initializePcapLibCapabilities();

#if (!_JNC_OS_WIN)
	const char* pcapVersion = ::pcap_lib_version();
#else
	sl::StringRef isNpcapDisable = ::getenv("JNC_IO_PCAP_DISABLE_NPCAP");
	if (isNpcapDisable.isEmpty())
		::SetDllDirectoryW(io::win::getSystemDir() + L"\\npcap");

	const char* pcapVersion = getPcapVersion();
	if (!pcapVersion) {
		err::setError("can't delay-load pcap (wpcap.dll is missing or invalid)");
		return NULL;
	}
#endif

	TRACE("jnc_io_pcap: Pcap version: %s\n", pcapVersion);
	return jnc::io::PcapLib_getLib();
}

//..............................................................................
