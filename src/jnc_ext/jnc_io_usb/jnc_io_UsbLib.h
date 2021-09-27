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

#pragma once

namespace jnc {
namespace io {

//..............................................................................

enum UsbLibCacheSlot {
	UsbLibCacheSlot_UsbEndpointDesc,
	UsbLibCacheSlot_UsbInterfaceDesc,
	UsbLibCacheSlot_UsbConfigurationDesc,
	UsbLibCacheSlot_UsbDeviceDesc,
	UsbLibCacheSlot_UsbEndpoint,
	UsbLibCacheSlot_UsbInterface,
	UsbLibCacheSlot_UsbDevice,
};

// {2cc2fe82-652f-4fe4-9573-5da7d5d53b72}
JNC_DEFINE_GUID(
	g_usbLibGuid,
	0x2cc2fe82, 0x652f, 0x4fe4, 0x95, 0x73, 0x5d, 0xa7, 0xd5, 0xd5, 0x3b, 0x72
);

//..............................................................................

} // namespace io
} // namespace jnc
