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

#include "pch.h"
#include "jnc_io_UsbDescriptor.h"
#include "jnc_io_UsbLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE(
	UsbEndpointDescriptor,
	"io.UsbEndpointDescriptor",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbEndpointDescriptor
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbEndpointDescriptor)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(
	UsbInterfaceDescriptor,
	"io.UsbInterfaceDescriptor",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbInterfaceDescriptor
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbInterfaceDescriptor)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(
	UsbConfigurationDescriptor,
	"io.UsbConfigurationDescriptor",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbConfigurationDescriptor
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbConfigurationDescriptor)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(
	UsbDeviceDescriptor,
	"io.UsbDeviceDescriptor",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbDeviceDescriptor
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbDeviceDescriptor)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
initUsbEndpointDescriptor(
	UsbEndpointDescriptor* dstDescriptor,
	const libusb_endpoint_descriptor* srcDescriptor
) {
	dstDescriptor->m_endpointId = srcDescriptor->bEndpointAddress;
	dstDescriptor->m_transferType = (srcDescriptor->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK);
	dstDescriptor->m_isoSyncType = (srcDescriptor->bmAttributes & LIBUSB_ISO_SYNC_TYPE_MASK);
	dstDescriptor->m_isoUsage = (srcDescriptor->bmAttributes & LIBUSB_ISO_USAGE_TYPE_MASK);
	dstDescriptor->m_maxPacketSize = srcDescriptor->wMaxPacketSize;
	dstDescriptor->m_interval = srcDescriptor->bInterval;
	dstDescriptor->m_refresh = srcDescriptor->bRefresh;
	dstDescriptor->m_synchAddress = srcDescriptor->bSynchAddress;
}

//..............................................................................

UsbEndpointDescriptor*
UsbInterfaceDescriptor::findEndpointDescriptor(uint8_t endpointId) {
	for (size_t i = 0; i < m_endpointCount; i++) {
		UsbEndpointDescriptor* endpointDescriptor = &((UsbEndpointDescriptor*)m_endpointTable.m_p) [i];
		if (endpointDescriptor->m_endpointId == endpointId)
			return endpointDescriptor;
	}

	return NULL;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbInterfaceDescriptor(
	Runtime* runtime,
	UsbInterfaceDescriptor* dstDescriptor,
	const libusb_interface_descriptor* srcDescriptor
) {
	Type* endpointDescriptorType = UsbEndpointDescriptor::getType(runtime->getModule());

	dstDescriptor->m_nextAltSettingInterfacePtr = g_nullDataPtr;
	dstDescriptor->m_endpointTable = runtime->getGcHeap()->allocateArray(endpointDescriptorType, srcDescriptor->bNumEndpoints);
	dstDescriptor->m_endpointCount = srcDescriptor->bNumEndpoints;

	UsbEndpointDescriptor* dstEndpointDescriptorTable = (UsbEndpointDescriptor*)dstDescriptor->m_endpointTable.m_p;
	for (size_t i = 0; i < srcDescriptor->bNumEndpoints; i++)
		initUsbEndpointDescriptor(&dstEndpointDescriptorTable[i], &srcDescriptor->endpoint[i]);

	dstDescriptor->m_interfaceId = srcDescriptor->bInterfaceNumber;
	dstDescriptor->m_altSettingId = srcDescriptor->bAlternateSetting;
	dstDescriptor->m_descriptionStringId = srcDescriptor->iInterface;
	dstDescriptor->m_class = srcDescriptor->bInterfaceClass;
	dstDescriptor->m_subClass = srcDescriptor->bInterfaceSubClass;
	dstDescriptor->m_protocol = srcDescriptor->bInterfaceProtocol;
}

void
initUsbInterfaceDescriptor(
	Runtime* runtime,
	UsbInterfaceDescriptor* dstDescriptor,
	const libusb_interface* iface
) {
	if (!iface->num_altsetting)
		return;

	JNC_BEGIN_CALL_SITE(runtime)
		initUsbInterfaceDescriptor(runtime, dstDescriptor, &iface->altsetting[0]);

		UsbInterfaceDescriptor* prevDescriptor = dstDescriptor;
		for (size_t i = 1; i < (size_t)iface->num_altsetting; i++) {
			DataPtr descriptorPtr = createData<UsbInterfaceDescriptor> (runtime);
			dstDescriptor = (UsbInterfaceDescriptor*)descriptorPtr.m_p;
			initUsbInterfaceDescriptor(runtime, dstDescriptor, &iface->altsetting[i]);

			prevDescriptor->m_nextAltSettingInterfacePtr = descriptorPtr;
			prevDescriptor = dstDescriptor;
		}
	JNC_END_CALL_SITE()
}

//..............................................................................

UsbInterfaceDescriptor*
UsbConfigurationDescriptor::findInterfaceDescriptor(
	uint8_t interfaceId,
	uint8_t altSettingId
) {
	if (interfaceId >= m_interfaceCount)
		return NULL;

	UsbInterfaceDescriptor* ifaceDescriptor = &((UsbInterfaceDescriptor*)m_interfaceDescriptorTable.m_p) [interfaceId];
	for (size_t i = 0; i < altSettingId; i++) {
		if (!ifaceDescriptor->m_nextAltSettingInterfacePtr.m_p)
			return NULL;

		ifaceDescriptor = (UsbInterfaceDescriptor*)ifaceDescriptor->m_nextAltSettingInterfacePtr.m_p;
	}

	return ifaceDescriptor;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbConfigurationDescriptor(
	Runtime* runtime,
	UsbConfigurationDescriptor* dstDescriptor,
	const libusb_config_descriptor* srcDescriptor
) {
	JNC_BEGIN_CALL_SITE(runtime)
		Type* ifaceDescriptorType = UsbInterfaceDescriptor::getType(runtime->getModule());
		dstDescriptor->m_interfaceDescriptorTable = runtime->getGcHeap()->allocateArray(ifaceDescriptorType, srcDescriptor->bNumInterfaces);
		dstDescriptor->m_interfaceCount = srcDescriptor->bNumInterfaces;

		UsbInterfaceDescriptor* dstInterfaceDescriptorTable = (UsbInterfaceDescriptor*)dstDescriptor->m_interfaceDescriptorTable.m_p;
		for (size_t i = 0; i < srcDescriptor->bNumInterfaces; i++)
			initUsbInterfaceDescriptor(runtime, &dstInterfaceDescriptorTable[i], &srcDescriptor->interface[i]);

		dstDescriptor->m_configurationId = srcDescriptor->bConfigurationValue;
		dstDescriptor->m_descriptionStringId = srcDescriptor->iConfiguration;
		dstDescriptor->m_attributes = srcDescriptor->bmAttributes;
		dstDescriptor->m_maxPower = srcDescriptor->MaxPower;
	JNC_END_CALL_SITE()
}

DataPtr
createUsbConfigurationDescriptor(
	Runtime* runtime,
	const libusb_config_descriptor* srcDescriptor
) {
	DataPtr resultPtr = g_nullDataPtr;

	JNC_BEGIN_CALL_SITE(runtime)
		resultPtr = createData<UsbConfigurationDescriptor> (runtime);
		UsbConfigurationDescriptor* dstDescriptor = (UsbConfigurationDescriptor*)resultPtr.m_p;
		initUsbConfigurationDescriptor(runtime, dstDescriptor, srcDescriptor);
	JNC_END_CALL_SITE()

	return resultPtr;
}

//..............................................................................

DataPtr
createUsbDeviceDescriptor(
	Runtime* runtime,
	const libusb_device_descriptor* srcDescriptor,
	axl::io::UsbDevice* srcDevice
) {
	DataPtr resultPtr = g_nullDataPtr;

	JNC_BEGIN_CALL_SITE(runtime)
		resultPtr = createData<UsbDeviceDescriptor> (runtime);
		UsbDeviceDescriptor* deviceDescriptor = (UsbDeviceDescriptor*)resultPtr.m_p;

		Type* configDescriptorType = UsbConfigurationDescriptor::getType(runtime->getModule());
		deviceDescriptor->m_configurationDescriptorTable = runtime->getGcHeap()->allocateArray(configDescriptorType, srcDescriptor->bNumConfigurations);
		deviceDescriptor->m_configurationCount = srcDescriptor->bNumConfigurations;

		UsbConfigurationDescriptor* dstConfigDescriptorTable = (UsbConfigurationDescriptor*)deviceDescriptor->m_configurationDescriptorTable.m_p;
		for (size_t i = 0; i < srcDescriptor->bNumConfigurations; i++) {
			axl::io::UsbConfigDescriptor srcConfigDescriptor;
			bool result = srcDevice->getConfigDescriptor(&srcConfigDescriptor, i);
			if (result) // LIBUSB_ERROR_NOT_FOUND may happen with libusb-incompatible drivers
				initUsbConfigurationDescriptor(runtime, &dstConfigDescriptorTable[i], srcConfigDescriptor);
		}

		deviceDescriptor->m_usbVersion = srcDescriptor->bcdUSB;
		deviceDescriptor->m_deviceVersion = srcDescriptor->bcdDevice;
		deviceDescriptor->m_vendorId = srcDescriptor->idVendor;
		deviceDescriptor->m_productId = srcDescriptor->idProduct;
		deviceDescriptor->m_class = srcDescriptor->bDeviceClass;
		deviceDescriptor->m_subClass = srcDescriptor->bDeviceSubClass;
		deviceDescriptor->m_protocol = srcDescriptor->bDeviceProtocol;
		deviceDescriptor->m_vendorStringId = srcDescriptor->iManufacturer;
		deviceDescriptor->m_productStringId = srcDescriptor->iProduct;
		deviceDescriptor->m_serialStringId = srcDescriptor->iSerialNumber;
	JNC_END_CALL_SITE()

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
