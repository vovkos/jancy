#include "pch.h"
#include "jnc_io_PcapLib.h"
#include "jnc_io_Pcap.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_LIB (
	PcapLib,
	g_pcapLibGuid,
	"PcapLib",
	"Jancy libPcap wrapper extension library"
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE (PcapLib)
	JNC_LIB_FORCED_IMPORT ("io_Pcap.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (PcapLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (Pcap)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

JNC_BEGIN_LIB_FUNCTION_MAP (PcapLib)
	JNC_MAP_TYPE (Pcap)
	JNC_MAP_FUNCTION ("io.createPcapDeviceDescList", &createPcapDeviceDescList)
JNC_END_LIB_FUNCTION_MAP ()

//..............................................................................

} // namespace io
} // namespace jnc

jnc_DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;

JNC_EXTERN_C
JNC_EXPORT
jnc_ExtensionLib*
jncDynamicExtensionLibMain (jnc_DynamicExtensionLibHost* host)
{
	jnc_g_dynamicExtensionLibHost = host;
	return jnc::io::PcapLib_getLib ();
}

//..............................................................................
