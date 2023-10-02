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

JNC_DECLARE_TYPE(HidMonDeviceDesc)

//..............................................................................

struct HidMonDeviceDesc: UsbDeviceStrings {
	JNC_DECLARE_TYPE_STATIC_METHODS(HidMonDeviceDesc)

	DataPtr m_nextPtr;
	DataPtr m_hidDeviceNamePtr;
	DataPtr m_captureDeviceNamePtr;
	uint_t m_captureDeviceId;

	uint16_t m_vendorId;
	uint16_t m_productId;
	uint16_t m_usagePage;
	uint16_t m_usage;
	uint16_t m_releaseNumber;

	uint8_t m_address;
	uint8_t m_port;
	uint8_t m_class;
	uint8_t m_subClass;
	uint8_t m_speed;
	uint8_t m_interfaceId;
	uint8_t m_endpointId;

	DataPtr m_reportDescriptorPtr;
	size_t m_reportDescriptorSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
enumerateHidMonDevices(DataPtr countPtr);

//..............................................................................

} // namespace io
} // namespace jnc
