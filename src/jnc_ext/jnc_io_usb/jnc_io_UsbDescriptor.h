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

JNC_DECLARE_TYPE(UsbEndpointDescriptor)
JNC_DECLARE_TYPE(UsbInterfaceDescriptor)
JNC_DECLARE_TYPE(UsbConfigurationDescriptor)
JNC_DECLARE_TYPE(UsbDeviceDescriptor)

//..............................................................................

struct UsbEndpointDescriptor {
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbEndpointDescriptor)

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
initUsbEndpointDescriptor(
	UsbEndpointDescriptor* dstDescriptor,
	const libusb_endpoint_descriptor* srcDescriptor
);

//..............................................................................

struct UsbInterfaceDescriptor {
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbInterfaceDescriptor)

	DataPtr m_nextAltSettingInterfacePtr;
	DataPtr m_endpointTable;
	size_t m_endpointCount;

	uint8_t m_interfaceId;
	uint8_t m_altSettingId;
	uint8_t m_descriptionStringId;
	uint8_t m_class;
	uint8_t m_subClass;
	uint8_t m_protocol;

	UsbEndpointDescriptor*
	findEndpointDescriptor(uint8_t endpointId);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbInterfaceDescriptor(
	Runtime* runtime,
	UsbInterfaceDescriptor* dstDescriptor,
	const libusb_interface_descriptor* srcDescriptor
);

void
initUsbInterfaceDescriptor(
	Runtime* runtime,
	UsbInterfaceDescriptor* dstDescriptor,
	const libusb_interface* iface
);

//..............................................................................

struct UsbConfigurationDescriptor {
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbConfigurationDescriptor)

	DataPtr m_interfaceDescriptorTable;
	size_t m_interfaceCount;

	uint8_t m_configurationId;
	uint8_t m_descriptionStringId;
	uint8_t m_attributes;
	uint8_t m_maxPower;

	UsbInterfaceDescriptor*
	findInterfaceDescriptor(
		uint8_t interfaceId,
		uint8_t altSettingId
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbConfigurationDescriptor(
	Runtime* runtime,
	UsbConfigurationDescriptor* dstDescriptor,
	const libusb_config_descriptor* srcDescriptor
);

DataPtr
createUsbConfigurationDescriptor(
	Runtime* runtime,
	const libusb_config_descriptor* srcDescriptor
);

//..............................................................................

struct UsbDeviceDescriptor {
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbDeviceDescriptor)

	DataPtr m_configurationDescriptorTable;
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
createUsbDeviceDescriptor(
	Runtime* runtime,
	const libusb_device_descriptor* srcDescriptor,
	axl::io::UsbDevice* device
);

//..............................................................................

} // namespace io
} // namespace jnc
