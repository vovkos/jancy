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
	UsbMonLibTypeCacheSlot_UsbMonDeviceDesc
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbMonDeviceDesc)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

DataPtr
createUsbMonDeviceDesc(
	Runtime* runtime,
	axl::io::UsbMonDeviceDesc* srcDevice,
	uint_t mask
) {
	DataPtr devicePtr = createData<UsbMonDeviceDesc> (runtime);
	UsbMonDeviceDesc* dstDevice = (UsbMonDeviceDesc*)devicePtr.m_p;

	dstDevice->m_captureDeviceNamePtr = strDup(srcDevice->m_captureDeviceName);
	dstDevice->m_descriptionPtr = strDup(srcDevice->m_description);
	dstDevice->m_manufacturerPtr = strDup(srcDevice->m_manufacturer);
	dstDevice->m_driverPtr = strDup(srcDevice->m_driver);
	dstDevice->m_manufacturerDescriptorPtr = strDup(srcDevice->m_manufacturerDescriptor);
	dstDevice->m_productDescriptorPtr = strDup(srcDevice->m_productDescriptor);
	dstDevice->m_serialNumberDescriptorPtr = strDup(srcDevice->m_serialNumberDescriptor);
	dstDevice->m_speed = srcDevice->m_speed;
	dstDevice->m_vendorId = srcDevice->m_vendorId;
	dstDevice->m_productId = srcDevice->m_productId;
	dstDevice->m_address = srcDevice->m_address;
	dstDevice->m_class = srcDevice->m_class;
	dstDevice->m_subClass = srcDevice->m_subClass;
	dstDevice->m_manufacturerDescriptorId = srcDevice->m_manufacturerDescriptorId;
	dstDevice->m_productDescriptorId = srcDevice->m_productDescriptorId;
	dstDevice->m_serialNumberDescriptorId = srcDevice->m_serialNumberDescriptorId;
	dstDevice->m_flags = srcDevice->m_flags;

	return devicePtr;
}

DataPtr
enumerateUsbMonDevices(
	uint_t mask,
	DataPtr countPtr
) {
	sl::List<axl::io::UsbMonDeviceDesc> deviceList;
	axl::io::enumerateUsbMonDevices(&deviceList, mask);

	if (deviceList.isEmpty()) {
		if (countPtr.m_p)
			*(size_t*)countPtr.m_p = 0;

		return g_nullDataPtr;
	}

	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime, false);

	sl::Iterator<axl::io::UsbMonDeviceDesc> it = deviceList.getHead();

	DataPtr devicePtr = createUsbMonDeviceDesc(runtime, *it, mask);
	DataPtr resultPtr = devicePtr;
	size_t count = 1;

	UsbMonDeviceDesc* prevDevice = (UsbMonDeviceDesc*)devicePtr.m_p;
	for (it++; it; it++) {
		devicePtr = createUsbMonDeviceDesc(runtime, *it, mask);
		prevDevice->m_nextPtr = devicePtr;
		prevDevice = (UsbMonDeviceDesc*)devicePtr.m_p;
		count++;
	}

	if (countPtr.m_p)
		*(size_t*)countPtr.m_p = count;

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
