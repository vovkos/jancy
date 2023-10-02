#include "pch.h"
#include "jnc_io_HidLib.h"
#include "jnc_io_HidLibDep.h"
#include "jnc_io_HidDevice.h"
#include "jnc_io_HidEnumerator.h"
#if (_JNC_IO_USBMON)
#	include "jnc_io_HidMonEnumerator.h"
#endif
#include "jnc_io_HidRd.h"
#include "jnc_io_HidDb.h"

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
	JNC_LIB_REQUIRE_TYPE(TypeKind_Class, "io.HidUsagePage") // HidReportField: HidUsagePage const* const m_usagePage;
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(HidLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(HidDevice)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(HidUsagePage)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(HidDb)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(HidReportField)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(HidReport)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(HidStandaloneReport)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(HidRdCollection)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(HidRd)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(HidLib)
	JNC_MAP_TYPE(HidDevice)
	JNC_MAP_TYPE(HidUsagePage)
	JNC_MAP_TYPE(HidDb)
	JNC_MAP_TYPE(HidReportField)
	JNC_MAP_TYPE(HidReport)
	JNC_MAP_TYPE(HidStandaloneReport)
	JNC_MAP_TYPE(HidRdCollection)
	JNC_MAP_TYPE(HidRd)
	JNC_MAP_FUNCTION_Q("io.enumerateHidDevices", enumerateHidDevices)
#if (_JNC_IO_USBMON)
	JNC_MAP_FUNCTION_Q("io.enumerateHidMonDevices", enumerateHidMonDevices)
#endif
	JNC_MAP_FUNCTION_Q("io.getHidRdUnit", getHidRdUnit)
	JNC_MAP_FUNCTION_Q("io.getHidRdComplexUnitString", getHidRdComplexUnitString)
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
