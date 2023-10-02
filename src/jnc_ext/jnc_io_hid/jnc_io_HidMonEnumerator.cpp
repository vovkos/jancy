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
#include "jnc_io_HidMonEnumerator.h"
#include "jnc_io_HidLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE(
	HidMonDeviceDesc,
	"io.HidMonDeviceDesc",
	g_hidLibGuid,
	HidLibCacheSlot_HidMonDeviceDesc
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HidMonDeviceDesc)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

DataPtr
createHidMonDeviceDesc(
	Runtime* runtime,
	const axl::io::HidMonDeviceDesc* srcDesc
) {
	DataPtr descPtr = createData<HidMonDeviceDesc>(runtime);
	HidMonDeviceDesc* dstDesc = (HidMonDeviceDesc*)descPtr.m_p;
	initUsbDeviceStrings(dstDesc, srcDesc);
	dstDesc->m_vendorId = srcDesc->m_vendorId;
	dstDesc->m_productId = srcDesc->m_productId;
	dstDesc->m_address = srcDesc->m_address;
	dstDesc->m_port = srcDesc->m_port;
	dstDesc->m_class = srcDesc->m_class;
	dstDesc->m_subClass = srcDesc->m_subClass;
	dstDesc->m_speed = srcDesc->m_speed;
	dstDesc->m_nextPtr = jnc::g_nullDataPtr;
	dstDesc->m_hidDeviceNamePtr = strDup(srcDesc->m_hidDeviceName);
	dstDesc->m_captureDeviceNamePtr = strDup(srcDesc->m_captureDeviceName);
	dstDesc->m_captureDeviceId = srcDesc->m_captureDeviceId;
	dstDesc->m_releaseNumber = srcDesc->m_releaseNumber;
	dstDesc->m_vendorId = srcDesc->m_vendorId;
	dstDesc->m_productId = srcDesc->m_productId;
	dstDesc->m_usagePage = srcDesc->m_usagePage;
	dstDesc->m_usage = srcDesc->m_usage;
	dstDesc->m_interfaceId = srcDesc->m_interfaceId;
	dstDesc->m_endpointId = srcDesc->m_endpointId;
	dstDesc->m_reportDescriptorSize = srcDesc->m_reportDescriptor.getCount();
	dstDesc->m_reportDescriptorPtr = memDup(srcDesc->m_reportDescriptor, dstDesc->m_reportDescriptorSize);
	return descPtr;
}

DataPtr
enumerateHidMonDevices(DataPtr countPtr) {
	sl::List<axl::io::HidMonDeviceDesc> list;
	axl::io::enumerateHidMonDevices(&list, axl::io::UsbDeviceStringId_All);

	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime, false);

	size_t count = 0;
	DataPtr resultPtr = g_nullDataPtr;
	HidMonDeviceDesc* prevDesc = NULL;
	sl::ConstIterator<axl::io::HidMonDeviceDesc> it = list.getHead();
	for (; it; it++) {
		DataPtr descPtr = createHidMonDeviceDesc(runtime, *it);
		if (prevDesc)
			prevDesc->m_nextPtr = descPtr;
		else
			resultPtr = descPtr;

		prevDesc = (HidMonDeviceDesc*)descPtr.m_p;
		count++;
	}

	if (countPtr.m_p)
		*(size_t*)countPtr.m_p = count;

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
