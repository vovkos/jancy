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
#include "jnc_io_UsbInterface.h"
#include "jnc_io_UsbDevice.h"
#include "jnc_io_UsbEndpoint.h"
#include "jnc_io_UsbDescriptor.h"
#include "jnc_io_UsbLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	UsbInterface,
	"io.UsbInterface",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbInterface,
	UsbInterface,
	&UsbInterface::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbInterface)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<UsbInterface>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<UsbInterface>)
	JNC_MAP_FUNCTION("release", &UsbInterface::release)
	JNC_MAP_FUNCTION("openEndpoint", &UsbInterface::openEndpoint)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

UsbInterface::UsbInterface() {
	m_device = NULL;
	m_interfaceDescriptorPtr = g_nullDataPtr;
	m_isClaimed = false;
}

void
JNC_CDECL
UsbInterface::markOpaqueGcRoots(jnc::GcHeap* gcHeap) {
	sl::Iterator<UsbEndpoint, UsbEndpoint::GetParentLink> it = m_endpointList.getHead();
	for (; it; it++)
		gcHeap->markClassPtr(*it);
}

void
UsbInterface::removeEndpoint(UsbEndpoint* endpoint) {
	m_lock.lock();
	m_endpointList.remove(endpoint);
	m_lock.unlock();
}

void
JNC_CDECL
UsbInterface::release() {
	if (!m_isClaimed)
		return;

	m_lock.lock();
	while (!m_endpointList.isEmpty()) {
		UsbEndpoint* endpoint = m_endpointList.removeHead();
		sl::ListLink* link = UsbEndpoint::GetParentLink()(endpoint);
		*link = sl::g_nullListLink;
		m_lock.unlock();

		endpoint->close();

		m_lock.lock();
	}
	m_lock.unlock();

	UsbInterfaceDescriptor* interfaceDescriptor = (UsbInterfaceDescriptor*)m_interfaceDescriptorPtr.m_p;
	m_device->m_device.releaseInterface(interfaceDescriptor->m_interfaceId);
	m_device->removeInterface(this);
	m_isClaimed = false;
}

UsbEndpoint*
JNC_CDECL
UsbInterface::openEndpoint(
	uint8_t endpointId,
	bool isSuspended
) {
	UsbInterfaceDescriptor* interfaceDescriptor = (UsbInterfaceDescriptor*)m_interfaceDescriptorPtr.m_p;
	UsbEndpointDescriptor* endpointDescriptor = interfaceDescriptor->findEndpointDescriptor(endpointId);
	if (!endpointDescriptor) {
		err::setError(err::SystemErrorCode_ObjectNameNotFound);
		return NULL;
	}

	Runtime* runtime = getCurrentThreadRuntime();
	GcHeap* gcHeap = runtime->getGcHeap();
	gcHeap->enterNoCollectRegion();

	UsbEndpoint* endpoint = createClass<UsbEndpoint>(runtime);
	endpoint->m_interface = this;
	endpoint->m_endpointDescriptorPtr.m_p = endpointDescriptor;

	endpoint->m_endpointDescriptorPtr.m_validator = runtime->getGcHeap()->createDataPtrValidator(
		m_interfaceDescriptorPtr.m_validator->m_targetBox,
		endpointDescriptor,
		sizeof(UsbEndpointDescriptor)
	);

	m_lock.lock();
	m_endpointList.insertTail(endpoint);
	m_lock.unlock();

	gcHeap->leaveNoCollectRegion(false);

	endpoint->open(isSuspended);
	return endpoint;
}

//..............................................................................

} // namespace io
} // namespace jnc
