#include "pch.h"
#include "jnc_io_UsbMonLib.h"
#include "jnc_io_UsbMonitor.h"
#include "jnc_io_UsbMonEnumerator.h"

namespace jnc {
namespace io {

//..............................................................................

void
initializeUsbMonLibCapabilities() {
	g_devMonCapability = jnc::isCapabilityEnabled("org.jancy.io.devmon");
}

//..............................................................................

JNC_DEFINE_LIB_EX(
	UsbMonLib,
	g_usbMonLibGuid,
	"UsbMonLib",
	"Jancy library for monitoring IO device activity",
	initializeUsbMonLibCapabilities
)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(UsbMonLib)
	JNC_LIB_IMPORT("io_UsbMonitor.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(UsbMonLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(UsbMonitor)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(UsbMonLib)
	JNC_MAP_TYPE(UsbMonitor)
	JNC_MAP_FUNCTION("io.enumerateUsbMonDevices", enumerateUsbMonDevices)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

} // namespace io
} // namespace jnc

//..............................................................................

jnc::DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;

extern "C"
AXL_EXPORT
jnc::ExtensionLib*
jncDynamicExtensionLibMain(jnc::DynamicExtensionLibHost* host) {
	g::getModule()->setTag("jnc_io_devmon");
	err::getErrorMgr()->setRouter(host->m_errorRouter);
	jnc_g_dynamicExtensionLibHost = host;
	jnc::io::initializeUsbMonLibCapabilities();
#if (_AXL_OS_LINUX)
	axl::io::getUsbDefaultContext()->createDefault();
#endif
	return jnc::io::UsbMonLib_getLib();
}

#if (_AXL_OS_LINUX)

JNC_EXTERN_C
JNC_EXPORT
bool_t
jncDynamicExtensionLibUnload() {
	axl::io::getUsbDefaultContext()->close();
	return true;
}

#endif

//..............................................................................
