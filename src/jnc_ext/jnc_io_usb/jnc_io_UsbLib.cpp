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
#include "jnc_io_UsbDesc.h"
#include "jnc_io_UsbEndpoint.h"
#include "jnc_io_UsbInterface.h"
#include "jnc_io_UsbDevice.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_LIB (
	UsbLib,
	g_usbLibGuid,
	"UsbLib",
	"Jancy libusb wrapper extension library"
	)

JNC_BEGIN_LIB_SOURCE_FILE_TABLE (UsbLib)
	JNC_LIB_FORCED_IMPORT ("io_Usb.jnc")
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (UsbLib)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (UsbEndpoint)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (UsbInterface)
	JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY (UsbDevice)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

JNC_BEGIN_LIB_FUNCTION_MAP (UsbLib)
	JNC_MAP_TYPE (UsbEndpoint)
	JNC_MAP_TYPE (UsbInterface)
	JNC_MAP_TYPE (UsbDevice)
	JNC_MAP_FUNCTION ("io.getUsbClassString", &getUsbClassString)
	JNC_MAP_FUNCTION ("io.getUsbSpeedString", &getUsbSpeedString)
	JNC_MAP_FUNCTION ("io.getUsbTransferTypeString", &getUsbTransferTypeString)
	JNC_MAP_FUNCTION ("io.createUsbDeviceArray", &createUsbDeviceArray)
	JNC_MAP_FUNCTION ("io.openUsbDevice", &openUsbDevice)
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
	g::getModule ()->setTag ("jnc_io_usb");
	jnc_g_dynamicExtensionLibHost = host;

	axl::io::createUsbDefaultContext ();
	axl::io::startUsbDefaultContextEventThread ();

	return jnc::io::UsbLib_getLib ();
}

//..............................................................................
