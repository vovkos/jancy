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
#include "jnc_io_UsbLib.h"
#include "jnc_io_UsbDescriptor.h"
#include "jnc_io_UsbEnumerator.h"
#include "jnc_io_UsbEndpoint.h"
#include "jnc_io_UsbInterface.h"
#include "jnc_io_UsbDevice.h"
#include "jnc_io_UsbDeviceFilter.h"

namespace jnc {
namespace io {

//..............................................................................

void
initializeUsbLibCapabilities() {
	g_canAccessAllUsbDevices = isCapabilityEnabled("org.jancy.io.usb");
	if (g_canAccessAllUsbDevices)
		return;

	size_t size = getCapabilityParamSize("org.jancy.io.usb.devices");
	if (!size)
		return;

	sl::Array<char> buffer;
	buffer.setCount(size);
	readCapabilityParam("org.jancy.io.usb.devices", buffer, size);

	const uint32_t* p = (uint32_t*)buffer.cp();
	const uint32_t* end = p + size / sizeof(uint32_t);
	for (; p < end; p++)
		enableUsbDeviceAccess(*p);
}

//..............................................................................

JNC_DEFINE_TYPE(
	UsbDeviceStrings,
	"io.UsbDeviceStrings",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbDeviceStrings
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbDeviceStrings)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_LIB_EX(
	UsbLib,
	g_usbLibGuid,
	"UsbLib",
	"Jancy libusb wrapper extension library",
	initializeUsbLibCapabilities
)

// we allocate descriptors from C++ so they must be parsed & laid out in advance (even if never used)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE(UsbLib)
	JNC_LIB_IMPORT("io_UsbDescriptors.jnc")
	JNC_LIB_REQUIRE_TYPE(TypeKind_Struct, "io.UsbDeviceDescriptor")
	JNC_LIB_REQUIRE_TYPE(TypeKind_Struct, "io.UsbConfigurationDescriptor")
	JNC_LIB_REQUIRE_TYPE(TypeKind_Struct, "io.UsbInterfaceDescriptor")
	JNC_LIB_REQUIRE_TYPE(TypeKind_Struct, "io.UsbEndpointDescriptor")
JNC_END_LIB_SOURCE_FILE_TABLE()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(UsbLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(UsbEndpoint)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(UsbInterface)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(UsbDevice)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE()

JNC_BEGIN_LIB_FUNCTION_MAP(UsbLib)
	JNC_MAP_TYPE(UsbEndpointDescriptor)
	JNC_MAP_TYPE(UsbEndpoint)
	JNC_MAP_TYPE(UsbInterfaceDescriptor)
	JNC_MAP_TYPE(UsbInterface)
	JNC_MAP_TYPE(UsbDeviceDescriptor)
	JNC_MAP_TYPE(UsbDevice)
	JNC_MAP_TYPE(UsbDeviceStrings)
	JNC_MAP_TYPE(UsbDeviceEntry)
	JNC_MAP_FUNCTION("io.enumerateUsbDevices", enumerateUsbDevices)
	JNC_MAP_FUNCTION("io.enumerateUsbDevicesNoDesc", enumerateUsbDevicesNoDesc)
JNC_END_LIB_FUNCTION_MAP()

//..............................................................................

} // namespace io
} // namespace jnc

jnc_DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;

JNC_EXTERN_C
JNC_EXPORT
jnc_ExtensionLib*
jncDynamicExtensionLibMain(jnc_DynamicExtensionLibHost* host) {
	g::getModule()->setTag("jnc_io_usb");
	err::getErrorMgr()->setRouter(host->m_errorRouter);
	jnc_g_dynamicExtensionLibHost = host;
	jnc::io::initializeUsbLibCapabilities();
	axl::io::registerUsbErrorProvider();
	axl::io::getUsbDefaultContext()->createDefault();
	axl::io::getUsbDefaultContextEventThread()->start();
	return jnc::io::UsbLib_getLib();
}

JNC_EXTERN_C
JNC_EXPORT
bool_t
jncDynamicExtensionLibUnload() {
	axl::io::getUsbDefaultContextEventThread()->stop();
	axl::io::getUsbDefaultContext()->close();
	return true;
}

//..............................................................................
