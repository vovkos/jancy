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

JNC_DECLARE_TYPE(UsbDeviceDesc)

//..............................................................................

struct UsbDeviceDesc {
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

	DataPtr m_descriptionPtr;
	DataPtr m_manufacturerPtr;
	DataPtr m_driverPtr;
	DataPtr m_manufacturerDescriptorPtr;
	DataPtr m_productDescriptorPtr;
	DataPtr m_serialNumberDescriptorPtr;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbDeviceDesc(
	UsbDeviceDesc* dstDesc,
	const axl::io::UsbDeviceDesc* srcDesc
);

//..............................................................................

} // namespace io
} // namespace jnc
