#include "pch.h"
#include "jnc_io_DevMonLib.h"
#include "jnc_io_DeviceMonitor.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_LIB(
	DevMonLib,
	g_devMonLibGuid,
	"DevMonLib",
	"Jancy library for monitoring IO device activity"
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(DevMonLib)
	JNC_LIB_IMPORT("io_DeviceMonitor.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(DevMonLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(jnc::io::DeviceMonitor)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(DevMonLib)
	JNC_MAP_TYPE(jnc::io::DeviceMonitor)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

} // namespace io
} // namespace jnc

//..............................................................................

jnc::DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;

extern "C"
AXL_EXPORT
jnc::ExtensionLib*
jncDynamicExtensionLibMain(jnc::DynamicExtensionLibHost* host)
{
	g::getModule()->setTag("jnc_io_devmon");
	err::getErrorMgr()->setRouter(host->m_errorRouter);
	jnc_g_dynamicExtensionLibHost = host;

	return jnc::requireCapability("org.jancy.io.devmon") ?
		jnc::io::DevMonLib_getLib() :
		NULL;
}

//..............................................................................
