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
#include "jnc_io_UsbDesc.h"
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
	JNC_MAP_FUNCTION  ("openEndpoint", &UsbInterface::openEndpoint)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

UsbInterface::UsbInterface() {
	m_parentDevice = NULL;
	m_interfaceDescPtr = g_nullDataPtr;
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
	// we do force-release ifaces from UsbDevice::~UsbDevice
	// therefore, we need to prevent multi-release

	m_lock.lock();
	if (!m_isClaimed) {
		m_lock.unlock();
		return;
	}

	m_isClaimed = false;

	while (!m_endpointList.isEmpty()) {
		UsbEndpoint* endpoint = m_endpointList.removeHead();
		sl::ListLink* link = UsbEndpoint::GetParentLink()(endpoint);
		*link = sl::g_nullListLink;
		m_lock.unlock();

		endpoint->close();

		m_lock.lock();
	}
	m_lock.unlock();

	UsbInterfaceDesc* interfaceDesc = (UsbInterfaceDesc*)m_interfaceDescPtr.m_p;
	m_parentDevice->m_device.releaseInterface(interfaceDesc->m_interfaceId);
	m_parentDevice->removeInterface(this);
}

UsbEndpoint*
JNC_CDECL
UsbInterface::openEndpoint(
	uint8_t endpointId,
	bool isSuspended
) {
	UsbInterfaceDesc* interfaceDesc = (UsbInterfaceDesc*)m_interfaceDescPtr.m_p;
	UsbEndpointDesc* endpointDesc = interfaceDesc->findEndpointDesc(endpointId);
	if (!endpointDesc) {
		err::setError(err::SystemErrorCode_ObjectNameNotFound);
		return NULL;
	}

	Runtime* runtime = getCurrentThreadRuntime();
	GcHeap* gcHeap = runtime->getGcHeap();
	gcHeap->enterNoCollectRegion();

	UsbEndpoint* endpoint = createClass<UsbEndpoint> (runtime);
	endpoint->m_parentInterface = this;
	endpoint->m_endpointDescPtr.m_p = endpointDesc;

	endpoint->m_endpointDescPtr.m_validator = runtime->getGcHeap()->createDataPtrValidator(
		m_interfaceDescPtr.m_validator->m_targetBox,
		endpointDesc,
		sizeof(UsbEndpointDesc)
	);

	gcHeap->leaveNoCollectRegion(false);

	m_lock.lock();
	m_endpointList.insertTail(endpoint);
	m_lock.unlock();

	endpoint->open(isSuspended);
	return endpoint;
}

//..............................................................................

} // namespace io
} // namespace jnc
