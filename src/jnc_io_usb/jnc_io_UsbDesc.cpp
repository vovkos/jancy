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
#include "jnc_io_UsbDesc.h"
#include "jnc_io_UsbLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE (
	UsbEndpointDesc,
	"io.UsbEndpointDesc",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbEndpointDesc
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbEndpointDesc)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	UsbInterfaceDesc,
	"io.UsbInterfaceDesc",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbInterfaceDesc
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbInterfaceDesc)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	UsbConfigurationDesc,
	"io.UsbConfigurationDesc",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbConfigurationDesc
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbConfigurationDesc)
JNC_END_TYPE_FUNCTION_MAP ()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE (
	UsbDeviceDesc,
	"io.UsbDeviceDesc",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbDeviceDesc
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbDeviceDesc)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

DataPtr
createUsbEndpointDesc (
	Runtime* runtime,
	libusb_endpoint_descriptor* srcDesc
	)
{
	DataPtr resultPtr = g_nullPtr;

	JNC_BEGIN_CALL_SITE (runtime)

	resultPtr = createData <UsbEndpointDesc> (runtime);
	UsbEndpointDesc* endpointDesc = (UsbEndpointDesc*) resultPtr.m_p;

	endpointDesc->m_endpointId = srcDesc->bEndpointAddress;
	endpointDesc->m_transferType = (srcDesc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK);
	endpointDesc->m_isoSyncType = (srcDesc->bmAttributes & LIBUSB_ISO_SYNC_TYPE_MASK);
	endpointDesc->m_isoUsage = (srcDesc->bmAttributes & LIBUSB_ISO_USAGE_TYPE_MASK);
	endpointDesc->m_maxPacketSize = srcDesc->wMaxPacketSize;
	endpointDesc->m_interval = srcDesc->bInterval;
	endpointDesc->m_refresh = srcDesc->bRefresh;
	endpointDesc->m_synchAddress = srcDesc->bSynchAddress;

	JNC_END_CALL_SITE ()

	return resultPtr;
}

//..............................................................................

UsbEndpointDesc*
UsbInterfaceDesc::findEndpointDesc (uint8_t endpointId)
{
	DataPtr* ptrArray = (DataPtr*) m_endpointTable.m_p;
	for (size_t i = 0; i < m_endpointCount; i++)
	{
		UsbEndpointDesc* endpointDesc = (UsbEndpointDesc*) ptrArray [i].m_p;
		if (endpointDesc->m_endpointId == endpointId)
			return endpointDesc;
	}

	return NULL;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createUsbInterfaceDesc (
	Runtime* runtime,
	libusb_interface_descriptor* srcDesc
	)
{
	DataPtr resultPtr = g_nullPtr;

	JNC_BEGIN_CALL_SITE (runtime)

	resultPtr = createData <UsbInterfaceDesc> (runtime);
	UsbInterfaceDesc* ifaceDesc = (UsbInterfaceDesc*) resultPtr.m_p;

	ifaceDesc->m_nextAlternatePtr;
	ifaceDesc->m_endpointTable;
	ifaceDesc->m_endpointCount;
	ifaceDesc->m_interfaceId;
	ifaceDesc->m_altSettingId;
	ifaceDesc->m_descriptionStringId;
	ifaceDesc->m_class;
	ifaceDesc->m_subClass;
	ifaceDesc->m_protocol;

	JNC_END_CALL_SITE ()

	return resultPtr;
}

//..............................................................................

UsbInterfaceDesc*
UsbConfigurationDesc::findInterfaceDesc (
	uint8_t interfaceId,
	uint8_t altSettingId
	)
{
	if (interfaceId >= m_interfaceCount)
		return NULL;

	DataPtr ifaceDescPtr = ((DataPtr*) m_interfaceTable.m_p) [interfaceId];
	UsbInterfaceDesc* ifaceDesc = (UsbInterfaceDesc*) ifaceDescPtr.m_p;
	for (size_t i = 0; i < altSettingId; i++)
	{
		if (!ifaceDesc->m_nextAlternatePtr.m_p)
			return NULL;

		ifaceDesc = (UsbInterfaceDesc*) ifaceDesc->m_nextAlternatePtr.m_p;
	}

	return ifaceDesc;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createUsbConfigurationDesc (
	Runtime* runtime,
	libusb_config_descriptor* srcDesc
	)
{
	DataPtr resultPtr = g_nullPtr;

	JNC_BEGIN_CALL_SITE (runtime)

	resultPtr = createData <UsbConfigurationDesc> (runtime);
	UsbConfigurationDesc* configDesc = (UsbConfigurationDesc*) resultPtr.m_p;

	configDesc->m_interfaceTable;
	configDesc->m_interfaceCount;
	configDesc->m_configurationId;
	configDesc->m_descriptionStringId;
	configDesc->m_attributes;
	configDesc->m_maxPower;

	JNC_END_CALL_SITE ()

	return resultPtr;
}

//..............................................................................

DataPtr
createUsbDeviceDesc (
	Runtime* runtime,
	libusb_device_descriptor* srcDesc
	)
{
	DataPtr resultPtr = g_nullPtr;

	JNC_BEGIN_CALL_SITE (runtime)

	resultPtr = createData <UsbDeviceDesc> (runtime);
	UsbDeviceDesc* deviceDesc = (UsbDeviceDesc*) resultPtr.m_p;

	deviceDesc->m_configurationTable;
	deviceDesc->m_configurationCount;

	deviceDesc->m_usbVersion;
	deviceDesc->m_deviceVersion;
	deviceDesc->m_vendorId;
	deviceDesc->m_productId;
	deviceDesc->m_vendorStringId;
	deviceDesc->m_productStringId;
	deviceDesc->m_serialStringId;
	deviceDesc->m_class;
	deviceDesc->m_subClass;
	deviceDesc->m_protocol;

	JNC_END_CALL_SITE ()

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
