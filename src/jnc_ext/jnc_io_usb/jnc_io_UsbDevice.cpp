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
#include "jnc_io_UsbAsyncControlEndpoint.h"
#include "jnc_io_UsbInterface.h"
#include "jnc_io_UsbDescriptor.h"
#include "jnc_io_UsbLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	UsbDevice,
	"io.UsbDevice",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbDevice,
	UsbDevice,
	&UsbDevice::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbDevice)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<UsbDevice>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<UsbDevice>)
	JNC_MAP_FUNCTION("open", &UsbDevice::open)
	JNC_MAP_FUNCTION("close", &UsbDevice::close)
	JNC_MAP_FUNCTION("getLangIdTable", &UsbDevice::getLangIdTable)
	JNC_MAP_FUNCTION("getStringDescriptor", &UsbDevice::getStringDescriptor)
	JNC_MAP_FUNCTION("attachKernelDriver", &UsbDevice::attachKernelDriver)
	JNC_MAP_FUNCTION("detachKernelDriver", &UsbDevice::detachKernelDriver)
	JNC_MAP_FUNCTION("claimInterface", &UsbDevice::claimInterface)
	JNC_MAP_FUNCTION("controlTransfer", &UsbDevice::controlTransfer_0)
	JNC_MAP_OVERLOAD(&UsbDevice::controlTransfer_1)
	JNC_MAP_AUTOGET_PROPERTY("m_isAutoDetachKernelDriverEnabled", &UsbDevice::setAutoDetachKernelDriverEnabled)
	JNC_MAP_CONST_PROPERTY("m_isKernelDriverActive", &UsbDevice::isKernelDriverActive)
	JNC_MAP_CONST_PROPERTY("m_deviceDescriptor", &UsbDevice::getDeviceDescriptor)
	JNC_MAP_CONST_PROPERTY("m_activeConfigurationDescriptor", &UsbDevice::getActiveConfigurationDescriptor)
	JNC_MAP_CONST_PROPERTY("m_bus", &UsbDevice::getBus)
	JNC_MAP_CONST_PROPERTY("m_address", &UsbDevice::getAddress)
	JNC_MAP_CONST_PROPERTY("m_port", &UsbDevice::getPort)
	JNC_MAP_CONST_PROPERTY("m_speed", &UsbDevice::getSpeed)
	JNC_MAP_PROPERTY("m_configurationId", &UsbDevice::getConfigurationId, &UsbDevice::setConfigurationId)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

UsbDevice::UsbDevice() {
	m_isOpen = false;
	m_isAutoDetachKernelDriverEnabled = false;
	m_asyncControlEndpoint = NULL;
	m_deviceDescriptorPtr = g_nullDataPtr;
	m_langIdTablePtr = g_nullDataPtr;
}

void
JNC_CDECL
UsbDevice::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	if (m_asyncControlEndpoint)
		m_asyncControlEndpoint->markOpaqueGcRoots(gcHeap);

	sl::Iterator<UsbInterface, UsbInterface::GetParentLink> it = m_interfaceList.getHead();
	for (; it; it++)
		gcHeap->markClassPtr(*it);

	gcHeap->markDataPtr(m_deviceDescriptorPtr);
	gcHeap->markDataPtr(m_langIdTablePtr);
}


void
UsbDevice::removeInterface(UsbInterface* iface) {
	m_lock.lock();
	m_interfaceList.remove(iface);
	m_lock.unlock();
}

void
JNC_CDECL
UsbDevice::close() {
	if (!m_isOpen)
		return;

	m_lock.lock();
	while (!m_interfaceList.isEmpty()) {
		UsbInterface* iface = m_interfaceList.removeHead();
		sl::ListLink* link = UsbInterface::GetParentLink()(iface);
		*link = sl::g_nullListLink;
		m_lock.unlock();

		iface->release();

		m_lock.lock();
	}
	m_lock.unlock();

	if (m_asyncControlEndpoint) {
		delete m_asyncControlEndpoint;
		m_asyncControlEndpoint = NULL;
	}

	m_device.close();
	m_isOpen = false;
}

bool
JNC_CDECL
UsbDevice::open() {
	close();

	bool result =
		checkAccess() &&
		m_device.open();

	if (!result)
		return false;

	m_isOpen = true;
	return true;
}

DataPtr
JNC_CDECL
UsbDevice::getDeviceDescriptor(UsbDevice* self) {
	if (self->m_deviceDescriptorPtr.m_p)
		return self->m_deviceDescriptorPtr;

	libusb_device_descriptor desc;

	bool result = self->m_device.getDeviceDescriptor(&desc);
	if (!result)
		return g_nullDataPtr;

	DataPtr ptr = createUsbDeviceDescriptor(getCurrentThreadRuntime(), &desc, &self->m_device);
	self->m_deviceDescriptorPtr = ptr;
	return ptr;
}

DataPtr
JNC_CDECL
UsbDevice::getActiveConfigurationDescriptor(UsbDevice* self) {
	axl::io::UsbConfigDescriptor desc;

	bool result = self->m_device.getActiveConfigDescriptor(&desc);
	if (!result)
		return g_nullDataPtr;

	return createUsbConfigurationDescriptor(getCurrentThreadRuntime(), desc);
}

DataPtr
JNC_CDECL
UsbDevice::getLangIdTable(UsbDevice* self) {
	if (!self->m_langIdTablePtr.m_p)
		return self->m_langIdTablePtr;

	sl::String_utf16 string = self->m_device.getStringDescriptor(0, 0); // special descriptor
	DataPtr ptr = memDup(string.cp(), string.getLength() * sizeof(utf16_t));
	self->m_langIdTablePtr = ptr;
	return ptr;
}

String
JNC_CDECL
UsbDevice::getStringDescriptor(
	UsbDevice* self,
	uint_t stringId,
	uint_t langId
) {
	if (!self->m_isOpen) {
		err::setError(err::SystemErrorCode_InvalidDeviceState);
		return g_nullString;
	}

	char buffer[256];

	if (!langId) {
		sl::String string(rc::BufKind_Stack, buffer, sizeof(buffer));
		self->m_device.getStringDescriptorAscii(&string, stringId);
		return allocateString(string);
	}

	sl::String_utf16 string(rc::BufKind_Stack, buffer, sizeof(buffer));
	self->m_device.getStringDescriptor(&string, stringId, langId);
	return allocateString(string);
}

UsbInterface*
JNC_CDECL
UsbDevice::claimInterface(
	uint_t interfaceId,
	uint_t altSettingId
) {
	if (!m_isOpen) {
		jnc::setError(err::Error(err::SystemErrorCode_InvalidDeviceState));
		return NULL;
	}

	bool result = m_device.claimInterface(interfaceId);
	if (!result)
		return NULL;

	UsbInterface* iface = NULL;

	Runtime* runtime = getCurrentThreadRuntime();
	GcHeap* gcHeap = runtime->getGcHeap();
	gcHeap->enterNoCollectRegion();

	DataPtr configDescPtr = getActiveConfigurationDescriptor(this);
	UsbConfigurationDescriptor* configDescriptor = (UsbConfigurationDescriptor*)configDescPtr.m_p;
	UsbInterfaceDescriptor* ifaceDescriptor = configDescriptor->findInterfaceDescriptor(interfaceId, altSettingId);

	if (!ifaceDescriptor)
		err::setError(err::SystemErrorCode_ObjectNameNotFound);
	else {
		iface = createClass<UsbInterface> (runtime);
		iface->m_device = this;
		iface->m_interfaceDescriptorPtr.m_p = ifaceDescriptor;

		iface->m_interfaceDescriptorPtr.m_validator = runtime->getGcHeap()->createDataPtrValidator(
			configDescPtr.m_validator->m_targetBox,
			ifaceDescriptor,
			sizeof(UsbInterfaceDescriptor)
		);

		iface->m_isClaimed = true;

		m_lock.lock();
		m_interfaceList.insertTail(iface);
		m_lock.unlock();
	}

	gcHeap->leaveNoCollectRegion(false);

	return iface;
}

size_t
JNC_CDECL
UsbDevice::controlTransfer_0(
	uint_t requestType,
	uint_t requestCode,
	uint_t value,
	uint_t index,
	DataPtr ptr,
	size_t size,
	uint_t timeout
) {
	if (!m_isOpen) {
		jnc::setError(err::Error(err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	return m_device.controlTransfer(requestType, requestCode, value, index, ptr.m_p, size, timeout);
}

bool
JNC_CDECL
UsbDevice::controlTransfer_1(
	uint_t requestType,
	uint_t requestCode,
	uint_t value,
	uint_t index,
	DataPtr ptr,
	size_t size,
	uint_t timeout,
	FunctionPtr completionFuncPtr
) {
	if (!m_isOpen) {
		jnc::setError(err::Error(err::SystemErrorCode_InvalidDeviceState));
		return false;
	}

	if (!m_asyncControlEndpoint) {
		UsbAsyncControlEndpoint* endpoint = new UsbAsyncControlEndpoint(&m_device);
		bool result = endpoint->start();
		if (!result) {
			delete endpoint;
			return false;
		}

		m_asyncControlEndpoint = endpoint;
	}

	return m_asyncControlEndpoint->transfer(
		requestType,
		requestCode,
		value,
		index,
		ptr,
		size,
		timeout,
		completionFuncPtr
	);
}

void
JNC_CDECL
UsbDevice::cancelControlTransfers() {
	if (m_asyncControlEndpoint)
		m_asyncControlEndpoint->cancelTransfers();
}

bool
UsbDevice::checkAccessByVidPid() {
	libusb_device_descriptor desc;

	return
		m_device.getDeviceDescriptor(&desc) &&
		checkUsbDeviceAccess(desc.idVendor, desc.idProduct);
}

//..............................................................................

UsbDevice*
openUsbDevice(
	uint_t vendorId,
	uint_t productId
) {
	axl::io::UsbDevice srcDevice;

	bool result =
		checkUsbDeviceAccess(vendorId, productId) &&
		srcDevice.open(vendorId, productId);

	if (!result)
		return NULL;

	UsbDevice* device = NULL;
	Runtime* runtime = getCurrentThreadRuntime();
	JNC_BEGIN_CALL_SITE(runtime)
		device = createClass<UsbDevice> (runtime);
		device->takeOver(&srcDevice);
	JNC_END_CALL_SITE()

	return device;
}

//..............................................................................

} // namespace io
} // namespace jnc
