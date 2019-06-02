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
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbInterface)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<UsbInterface>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<UsbInterface>)
	JNC_MAP_FUNCTION("release", &UsbInterface::release)
	JNC_MAP_FUNCTION  ("openEndpoint", &UsbInterface::openEndpoint)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

UsbInterface::UsbInterface()
{
	m_parentDevice = NULL;
	m_interfaceDescPtr = g_nullDataPtr;
	m_isClaimed = false;
}

void
JNC_CDECL
UsbInterface::release()
{
	if (!m_isClaimed)
		return;

	UsbInterfaceDesc* interfaceDesc = (UsbInterfaceDesc*)m_interfaceDescPtr.m_p;
	bool result = m_parentDevice->m_device.releaseInterface(interfaceDesc->m_interfaceId);
	if (result)
		m_isClaimed = false;
}

UsbEndpoint*
JNC_CDECL
UsbInterface::openEndpoint(
	uint8_t endpointId,
	bool isSuspended
	)
{
	UsbInterfaceDesc* interfaceDesc = (UsbInterfaceDesc*)m_interfaceDescPtr.m_p;
	UsbEndpointDesc* endpointDesc = interfaceDesc->findEndpointDesc(endpointId);
	if (!endpointDesc)
	{
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

	endpoint->open(isSuspended);

	return endpoint;
}

//..............................................................................

} // namespace io
} // namespace jnc
