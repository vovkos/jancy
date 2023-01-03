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

#include "jnc_io_UsbDeviceDesc.h"

namespace jnc {
namespace io {

JNC_DECLARE_TYPE(UsbMonDeviceDesc)

//..............................................................................

struct UsbMonDeviceDesc: UsbDeviceDesc {
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbMonDeviceDesc)

	DataPtr m_nextPtr;
	DataPtr m_captureDeviceNamePtr;
	uint8_t m_captureDeviceId;
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
