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

#include "jnc_io_UsbDeviceStrings.h"

namespace jnc {
namespace io {

JNC_DECLARE_TYPE(UsbMonDeviceDesc)

//..............................................................................

struct UsbMonDeviceDesc: UsbDeviceStrings {
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbMonDeviceDesc)

	DataPtr m_nextPtr;
	DataPtr m_captureDeviceNamePtr;
	uint_t m_captureDeviceId;

	uint16_t m_vendorId;
	uint16_t m_productId;

	uint8_t m_address;
	uint8_t m_port;
	uint8_t m_class;
	uint8_t m_subClass;
	uint8_t m_speed;

	uint8_t m_manufacturerDescriptorId;
	uint8_t m_productDescriptorId;
	uint8_t m_serialNumberDescriptorId;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
enumerateUsbMonDevices(
	uint_t flags,
	DataPtr countPtr
);

//..............................................................................

} // namespace io
} // namespace jnc
