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

//..............................................................................

struct UsbMonDeviceDescBase: UsbDeviceStrings {
	String m_captureDeviceName;
	uint_t m_captureDeviceId;
	uint16_t m_vendorId;
	uint16_t m_productId;
	uint8_t m_address;
	uint8_t m_port;
	uint8_t m_class;
	uint8_t m_subClass;
	uint8_t m_speed;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbMonDeviceDescBase(
	UsbMonDeviceDescBase* dst,
	const axl::io::UsbMonDeviceDesc* src
);

//..............................................................................

} // namespace io
} // namespace jnc
