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
#include "jnc_io_HidEnumerator.h"
#include "jnc_io_HidLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE(
	HidDeviceDesc,
	"io.HidDeviceDesc",
	g_hidLibGuid,
	HidLibCacheSlot_HidDeviceDesc
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HidDeviceDesc)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

DataPtr
createHidDeviceDesc(
	Runtime* runtime,
	const hid_device_info* info
) {
	DataPtr descPtr = createData<HidDeviceDesc>(runtime);
	HidDeviceDesc* desc = (HidDeviceDesc*)descPtr.m_p;
	desc->m_nextPtr = jnc::g_nullDataPtr;
	desc->m_path = allocateString(info->path);
	desc->m_manufacturer = allocateString(info->manufacturer_string);
	desc->m_product = allocateString(info->product_string);
	desc->m_serialNumber = allocateString(info->serial_number);
	desc->m_releaseNumber = info->release_number;
	desc->m_vendorId = info->vendor_id;
	desc->m_productId = info->product_id;
	desc->m_usagePage = info->usage_page;
	desc->m_usage = info->usage;
	desc->m_interfaceId = info->interface_number;
	desc->m_busType = info->bus_type;
	return descPtr;
}

DataPtr
enumerateHidDevices(DataPtr countPtr) {
	axl::io::HidDeviceInfoList list;
	bool result = list.enumerate();
	if (!result) {
		if (countPtr.m_p)
			*(size_t*)countPtr.m_p = 0;

		return g_nullDataPtr;
	}

	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime, false);

	size_t count = 0;
	DataPtr resultPtr = g_nullDataPtr;
	HidDeviceDesc* prevDesc = NULL;
	axl::io::HidDeviceInfoIterator it = list.getHead();
	for (; it; it++) {
		DataPtr descPtr = createHidDeviceDesc(runtime, *it);
		if (prevDesc)
			prevDesc->m_nextPtr = descPtr;
		else
			resultPtr = descPtr;

		prevDesc = (HidDeviceDesc*)descPtr.m_p;
		count++;
	}

	if (countPtr.m_p)
		*(size_t*)countPtr.m_p = count;

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
