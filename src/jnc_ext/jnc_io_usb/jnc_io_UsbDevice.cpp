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
#include "jnc_io_UsbDevice.h"
#include "jnc_io_UsbInterface.h"
#include "jnc_io_UsbDesc.h"
#include "jnc_io_UsbLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE (
	UsbDevice,
	"io.UsbDevice",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbDevice,
	UsbDevice,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP (UsbDevice)
	JNC_MAP_CONSTRUCTOR (&jnc::construct <UsbDevice>)
	JNC_MAP_DESTRUCTOR (&jnc::destruct <UsbDevice>)
	JNC_MAP_FUNCTION ("open",               &UsbDevice::open)
	JNC_MAP_FUNCTION ("close",              &UsbDevice::close)
	JNC_MAP_FUNCTION ("attachKernelDriver", &UsbDevice::attachKernelDriver)
	JNC_MAP_FUNCTION ("detachKernelDriver", &UsbDevice::detachKernelDriver)
	JNC_MAP_FUNCTION ("claimInterface",     &UsbDevice::claimInterface)
	JNC_MAP_FUNCTION ("getStringDesc",      &UsbDevice::getStringDesc)
	JNC_MAP_AUTOGET_PROPERTY ("m_isAutoDetachKernelDriverEnabled", &UsbDevice::setAutoDetachKernelDriverEnabled)
	JNC_MAP_CONST_PROPERTY ("m_isKernelDriverActive", &UsbDevice::isKernelDriverActive)
	JNC_MAP_CONST_PROPERTY ("m_deviceDesc", &UsbDevice::getDeviceDesc)
	JNC_MAP_CONST_PROPERTY ("m_activeConfigurationDesc", &UsbDevice::getActiveConfigurationDesc)
	JNC_MAP_CONST_PROPERTY ("m_bus",        &UsbDevice::getBus)
	JNC_MAP_CONST_PROPERTY ("m_address",    &UsbDevice::getAddress)
	JNC_MAP_CONST_PROPERTY ("m_speed",      &UsbDevice::getSpeed)
	JNC_MAP_PROPERTY ("m_configurationId",  &UsbDevice::getConfigurationId, &UsbDevice::setConfigurationId)
JNC_END_TYPE_FUNCTION_MAP ()

//..............................................................................

bool
JNC_CDECL
UsbDevice::open ()
{
	bool result = m_device.open ();
	if (!result)
	{
		jnc::propagateLastError ();
		return false;
	}

	m_isOpen = true;
	return true;
}

DataPtr
JNC_CDECL
UsbDevice::getDeviceDesc (UsbDevice* self)
{
	libusb_device_descriptor desc;

	bool result = self->m_device.getDeviceDescriptor (&desc);
	if (!result)
	{
		jnc::propagateLastError ();
		return g_nullPtr;
	}

	return createUsbDeviceDesc (getCurrentThreadRuntime (), &desc, &self->m_device);
}

DataPtr
JNC_CDECL
UsbDevice::getActiveConfigurationDesc (UsbDevice* self)
{
	axl::io::UsbConfigDescriptor desc;

	bool result = self->m_device.getActiveConfigDescriptor (&desc);
	if (!result)
	{
		jnc::propagateLastError ();
		return g_nullPtr;
	}

	return createUsbConfigurationDesc (getCurrentThreadRuntime (), desc);
}

bool
JNC_CDECL
UsbDevice::setAutoDetachKernelDriverEnabled (bool isEnabled)
{
	bool result = m_device.setAutoDetachKernelDriver (isEnabled);
	if (!result)
		jnc::propagateLastError ();

	return result;
}

bool
JNC_CDECL
UsbDevice::attachKernelDriver (uint_t interfaceId)
{
	bool result = m_device.attachKernelDriver (interfaceId);
	if (!result)
		jnc::propagateLastError ();

	return result;
}

bool
JNC_CDECL
UsbDevice::detachKernelDriver (uint_t interfaceId)
{
	bool result = m_device.detachKernelDriver (interfaceId);
	if (!result)
		jnc::propagateLastError ();

	return result;
}

UsbInterface*
JNC_CDECL
UsbDevice::claimInterface (
	uint8_t interfaceId,
	uint8_t altSettingId
	)
{
	if (!m_isOpen)
	{
		jnc::setError (err::Error (err::SystemErrorCode_InvalidDeviceState));
		return NULL;
	}

	bool result = m_device.claimInterface (interfaceId);
	if (!result)
	{
		jnc::propagateLastError ();
		return NULL;
	}

	Runtime* runtime = getCurrentThreadRuntime ();
	UsbInterface* iface = NULL;

	JNC_BEGIN_NESTED_CALL_SITE (runtime)

	DataPtr configDescPtr = getActiveConfigurationDesc (this);
	UsbConfigurationDesc* configDesc = (UsbConfigurationDesc*) configDescPtr.m_p;
	UsbInterfaceDesc* ifaceDesc = configDesc->findInterfaceDesc (interfaceId, altSettingId);

	if (!ifaceDesc)
	{
		err::setError (err::SystemErrorCode_ObjectNameNotFound);
	}
	else
	{
		iface = createClass <UsbInterface> (runtime);
		iface->m_parentDevice = this;
		iface->m_interfaceDescPtr.m_p = ifaceDesc;

		iface->m_interfaceDescPtr.m_validator = runtime->getGcHeap ()->createDataPtrValidator (
			configDescPtr.m_validator->m_targetBox,
			ifaceDesc,
			sizeof (UsbInterfaceDesc)
			);

		iface->m_isClaimed = true;
		iface->m_syncId = 0;
	}

	JNC_END_NESTED_CALL_SITE ()

	if (!iface)
		jnc::propagateLastError ();

	return iface;
}

DataPtr
JNC_CDECL
UsbDevice::getStringDesc (
	UsbDevice* self,
	uint8_t stringId
	)
{
	if (!self->m_isOpen)
	{
		err::setError (err::SystemErrorCode_InvalidDeviceState);
		jnc::propagateLastError ();
		return g_nullPtr;
	}

	sl::String string = self->m_device.getStringDesrciptor (stringId);
	return strDup (string, string.getLength ());
}

//..............................................................................

DataPtr
createUsbDeviceArray (DataPtr countPtr)
{
	axl::io::UsbDeviceList deviceList;
	size_t count = deviceList.enumerateDevices ();

	DataPtr arrayPtr = g_nullPtr;
	Runtime* runtime = getCurrentThreadRuntime ();
	GcHeap* gcHeap = runtime->getGcHeap ();
	Type* classPtrType = (Type*) UsbDevice::getType (runtime->getModule ())->getClassPtrType ();

	JNC_BEGIN_NESTED_CALL_SITE (runtime)

	arrayPtr = gcHeap->allocateArray (classPtrType, count);
	UsbDevice** dstDeviceArray = (UsbDevice**) arrayPtr.m_p;
	libusb_device** srcDeviceArray = deviceList;

	for (size_t i = 0; i < count; i++)
	{
		UsbDevice* device = createClass <UsbDevice> (runtime);
		device->setDevice (srcDeviceArray [i]);
		dstDeviceArray [i] = device;
	}

	JNC_END_NESTED_CALL_SITE ()

	if (countPtr.m_p)
		*(size_t*) countPtr.m_p = count;

	return arrayPtr;
}

UsbDevice*
openUsbDevice (
	uint_t vendorId,
	uint_t productId
	)
{
	axl::io::UsbDevice srcDevice;
	bool result = srcDevice.open (vendorId, productId);
	if (!result)
	{
		jnc::propagateLastError ();
		return NULL;
	}

	UsbDevice* device = NULL;
	Runtime* runtime = getCurrentThreadRuntime ();
	JNC_BEGIN_NESTED_CALL_SITE (runtime)
	device = createClass <UsbDevice> (runtime);
	device->takeOver (&srcDevice);
	JNC_END_NESTED_CALL_SITE ()

	return device;
}

//..............................................................................

} // namespace io
} // namespace jnc
