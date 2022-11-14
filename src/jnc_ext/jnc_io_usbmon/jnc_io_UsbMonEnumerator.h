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

JNC_DECLARE_TYPE(UsbMonDeviceDesc)

//..............................................................................

struct UsbMonDeviceDesc {
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbMonDeviceDesc)

	DataPtr m_nextPtr;
	DataPtr m_captureDeviceNamePtr;
	DataPtr m_descriptionPtr;
	DataPtr m_manufacturerPtr;
	DataPtr m_driverPtr;
	DataPtr m_manufacturerDescriptorPtr;
	DataPtr m_productDescriptorPtr;
	DataPtr m_serialNumberDescriptorPtr;

	uint_t m_speed;
	uint_t m_vendorId;
	uint_t m_productId;
	uint_t m_address;
	uint_t m_class;
	uint_t m_subClass;
	uint_t m_manufacturerDescriptorId;
	uint_t m_productDescriptorId;
	uint_t m_serialNumberDescriptorId;
	uint_t m_flags;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
enumerateUsbMonDevices(
	uint_t mask,
	DataPtr countPtr
);

//..............................................................................

} // namespace io
} // namespace jnc
