#include "pch.h"
#include "jnc_io_HidLib.h"
#include "jnc_io_HidDevice.h"
#include "jnc_io_HidEnumerator.h"

namespace jnc {
namespace io {

//..............................................................................

void
initializeHidLibCapabilities() {
	g_hidCapability = jnc::isCapabilityEnabled("org.jancy.io.usb");
}

//..............................................................................

JNC_DEFINE_LIB_EX(
	HidLib,
	g_hidLibGuid,
	"HidLib",
	"Jancy library for monitoring IO device activity",
	initializeHidLibCapabilities
)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(HidLib)
	JNC_LIB_IMPORT("io_HidDevice.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(HidLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(HidDevice)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(HidLib)
	JNC_MAP_TYPE(HidDevice)
	JNC_MAP_FUNCTION_Q("io.enumerateHidDevices", enumerateHidDevices)
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
	err::getErrorMgr()->setRouter(host->m_errorRouter);
	jnc_g_dynamicExtensionLibHost = host;
	jnc::io::initializeHidLibCapabilities();
	axl::io::hidInit();
	return jnc::io::HidLib_getLib();
}

JNC_EXTERN_C
JNC_EXPORT
bool_t
jncDynamicExtensionLibUnload() {
	axl::io::hidExit();
	return true;
}

//..............................................................................
