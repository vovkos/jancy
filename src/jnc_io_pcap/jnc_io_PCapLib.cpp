#include "pch.h"
#include "jnc_io_PCapLib.h"
#include "jnc_io_PCap.h"

namespace jnc {
namespace io {

//.............................................................................

JNC_DEFINE_LIB (PCapLib)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE (PCapLib)
	JNC_LIB_FORCED_IMPORT ("io_PCap.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (PCapLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (PCap)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

JNC_BEGIN_LIB_FUNCTION_MAP (PCapLib)
	JNC_MAP_TYPE (PCap)
	JNC_MAP_FUNCTION ("io.createPCapDeviceDescList", &createPCapDeviceDescList)
JNC_END_LIB_FUNCTION_MAP ()

//.............................................................................

} // namespace io
} // namespace jnc

jnc_DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;

JNC_EXTERN_C
AXL_EXPORT
jnc_ExtensionLib* 
jncDynamicExtensionLibMain (jnc_DynamicExtensionLibHost* host)
{
	jnc_g_dynamicExtensionLibHost = host;
	return jnc::io::PCapLib_getLib ();
}

//.............................................................................
