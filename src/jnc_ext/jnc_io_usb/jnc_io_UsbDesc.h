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

JNC_DECLARE_TYPE(UsbEndpointDesc)
JNC_DECLARE_TYPE(UsbInterfaceDesc)
JNC_DECLARE_TYPE(UsbConfigurationDesc)
JNC_DECLARE_TYPE(UsbDeviceDesc)

//..............................................................................

DataPtr
getUsbClassString(uint8_t cls);

DataPtr
getUsbSpeedString(libusb_speed speed);

DataPtr
getUsbTransferTypeString(libusb_transfer_type type);

//..............................................................................

struct UsbEndpointDesc
{
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbEndpointDesc)

	uint8_t m_endpointId;
	uint8_t m_transferType;
	uint8_t m_isoSyncType;
	uint8_t m_isoUsage;
	uint16_t m_maxPacketSize;
	uint8_t m_interval;
	uint8_t m_refresh;
	uint8_t m_synchAddress;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbEndpointDesc(
	UsbEndpointDesc* dstDesc,
	const libusb_endpoint_descriptor* srcDesc
	);

//..............................................................................

struct UsbInterfaceDesc
{
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbInterfaceDesc)

	DataPtr m_nextAltSettingInterfacePtr;
	DataPtr m_endpointTable;
	size_t m_endpointCount;

	uint8_t m_interfaceId;
	uint8_t m_altSettingId;
	uint8_t m_descriptionStringId;
	uint8_t m_class;
	uint8_t m_subClass;
	uint8_t m_protocol;

	UsbEndpointDesc*
	findEndpointDesc(uint8_t endpointId);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbInterfaceDesc(
	Runtime* runtime,
	UsbInterfaceDesc* dstDesc,
	const libusb_interface_descriptor* srcDesc
	);

void
initUsbInterfaceDesc(
	Runtime* runtime,
	UsbInterfaceDesc* dstDesc,
	const libusb_interface* srcDesc
	);

//..............................................................................

struct UsbConfigurationDesc
{
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbConfigurationDesc)

	DataPtr m_interfaceTable;
	size_t m_interfaceCount;

	uint8_t m_configurationId;
	uint8_t m_descriptionStringId;
	uint8_t m_attributes;
	uint8_t m_maxPower;

	UsbInterfaceDesc*
	findInterfaceDesc(
		uint8_t interfaceId,
		uint8_t altSettingId
		);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbConfigurationDesc(
	Runtime* runtime,
	UsbConfigurationDesc* dstDesc,
	const libusb_config_descriptor* srcDesc
	);

DataPtr
createUsbConfigurationDesc(
	Runtime* runtime,
	const libusb_config_descriptor* srcDesc
	);

//..............................................................................

struct UsbDeviceDesc
{
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbDeviceDesc)

	DataPtr m_configurationTable;
	size_t m_configurationCount;

	uint16_t m_usbVersion;
	uint16_t m_deviceVersion;
	uint16_t m_vendorId;
	uint16_t m_productId;
	uint8_t m_vendorStringId;
	uint8_t m_productStringId;
	uint8_t m_serialStringId;
	uint8_t m_class;
	uint8_t m_subClass;
	uint8_t m_protocol;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createUsbDeviceDesc(
	Runtime* runtime,
	const libusb_device_descriptor* srcDesc,
	axl::io::UsbDevice* srcDevice
	);

//..............................................................................

} // namespace io
} // namespace jnc
