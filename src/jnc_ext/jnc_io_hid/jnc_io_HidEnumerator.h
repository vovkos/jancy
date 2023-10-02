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

JNC_DECLARE_TYPE(HidDeviceDesc)

//..............................................................................

struct HidDeviceDesc {
	JNC_DECLARE_TYPE_STATIC_METHODS(HidDeviceDesc)

	DataPtr m_nextPtr;
	DataPtr m_path;
	DataPtr m_manufacturer;
	DataPtr m_product;
	DataPtr m_serialNumber;

	uint16_t m_vendorId;
	uint16_t m_productId;
	uint16_t m_usagePage;
	uint16_t m_usage;
	uint16_t m_releaseNumber;
	uint8_t m_interfaceId;
	uint8_t m_busType;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
enumerateHidDevices(DataPtr countPtr);

DataPtr
createHidDeviceDesc(
	Runtime* runtime,
	const hid_device_info* info
);

//..............................................................................

} // namespace io
} // namespace jnc
