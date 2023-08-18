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
#include "jnc_io_UsbMonEnumerator.h"
#include "jnc_io_UsbMonLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE(
	UsbMonDeviceDesc,
	"io.UsbMonDeviceDesc",
	g_usbMonLibGuid,
	UsbMonLibCacheSlot_UsbMonDeviceDesc
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbMonDeviceDesc)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

DataPtr
createUsbMonDeviceDesc(
	Runtime* runtime,
	axl::io::UsbMonDeviceDesc* srcDesc,
	uint_t mask
) {
	DataPtr descPtr = createData<UsbMonDeviceDesc>(runtime);
	UsbMonDeviceDesc* dstDesc = (UsbMonDeviceDesc*)descPtr.m_p;
	initUsbDeviceStrings(dstDesc, srcDesc);
	dstDesc->m_captureDeviceNamePtr = strDup(srcDesc->m_captureDeviceName);
	dstDesc->m_captureDeviceId = srcDesc->m_captureDeviceId;
	dstDesc->m_vendorId = srcDesc->m_vendorId;
	dstDesc->m_productId = srcDesc->m_productId;
	dstDesc->m_address = srcDesc->m_address;
	dstDesc->m_port = srcDesc->m_port;
	dstDesc->m_class = srcDesc->m_class;
	dstDesc->m_subClass = srcDesc->m_subClass;
	dstDesc->m_speed = srcDesc->m_speed;
	dstDesc->m_manufacturerDescriptorId = srcDesc->m_manufacturerDescriptorId;
	dstDesc->m_productDescriptorId = srcDesc->m_productDescriptorId;
	dstDesc->m_serialNumberDescriptorId = srcDesc->m_serialNumberDescriptorId;
	return descPtr;
}

DataPtr
enumerateUsbMonDevices(
	uint_t flags,
	DataPtr countPtr
) {
	sl::List<axl::io::UsbMonDeviceDesc> deviceList;
	axl::io::enumerateUsbMonDevices(&deviceList, flags);

	if (deviceList.isEmpty()) {
		if (countPtr.m_p)
			*(size_t*)countPtr.m_p = 0;

		return g_nullDataPtr;
	}

	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime, false);

	sl::Iterator<axl::io::UsbMonDeviceDesc> it = deviceList.getHead();

	DataPtr descPtr = createUsbMonDeviceDesc(runtime, *it, flags);
	DataPtr resultPtr = descPtr;
	size_t count = 1;

	UsbMonDeviceDesc* prevDesc = (UsbMonDeviceDesc*)descPtr.m_p;
	for (it++; it; it++) {
		descPtr = createUsbMonDeviceDesc(runtime, *it, flags);
		prevDesc->m_nextPtr = descPtr;
		prevDesc = (UsbMonDeviceDesc*)descPtr.m_p;
		count++;
	}

	if (countPtr.m_p)
		*(size_t*)countPtr.m_p = count;

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
