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
#include "jnc_io_UsbEnumerator.h"
#include "jnc_io_UsbDevice.h"
#include "jnc_io_UsbLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE(
	UsbDeviceEntry,
	"io.UsbDeviceEntry",
	g_usbLibGuid,
	UsbLibCacheSlot_UsbDeviceEntry
)

JNC_BEGIN_TYPE_FUNCTION_MAP(UsbDeviceEntry)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

DataPtr
createUsbDeviceEntry(
	Runtime* runtime,
	const axl::io::UsbDeviceEntry* srcDeviceEntry
) {
	DataPtr resultPtr = createData<UsbDeviceEntry> (runtime);
	UsbDeviceEntry* deviceEntry = (UsbDeviceEntry*)resultPtr.m_p;
	initUsbDeviceDesc(deviceEntry, srcDeviceEntry);
	deviceEntry->m_device = createClass<UsbDevice>(runtime);
	deviceEntry->m_device->setDevice(srcDeviceEntry->m_device);
	return resultPtr;
}

DataPtr
enumerateUsbDevices(
	uint_t flags,
	DataPtr countPtr
) {
	axl::io::UsbDeviceList loDeviceList;
	sl::List<axl::io::UsbDeviceEntry> deviceList;
	size_t count = axl::io::enumerateUsbDevices(&loDeviceList, &deviceList, flags);

	if (deviceList.isEmpty()) {
		if (countPtr.m_p)
			*(size_t*)countPtr.m_p = 0;

		return g_nullDataPtr;
	}

	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime);

	sl::Iterator<axl::io::UsbDeviceEntry> it = deviceList.getHead();
	DataPtr entryPtr = createUsbDeviceEntry(runtime, *it);
	DataPtr resultPtr = entryPtr;

	UsbDeviceEntry* prevEntry = (UsbDeviceEntry*)entryPtr.m_p;
	for (it++; it; it++) {
		entryPtr = createUsbDeviceEntry(runtime, *it);
		prevEntry->m_nextPtr = entryPtr;
		prevEntry = (UsbDeviceEntry*)entryPtr.m_p;
	}

	if (countPtr.m_p)
		*(size_t*)countPtr.m_p = count;

	return resultPtr;
}

DataPtr
enumerateUsbDevicesNoDesc(DataPtr countPtr) {
	axl::io::UsbDeviceList deviceList;
	size_t count = deviceList.enumerateDevices();
	if (count == -1 || count == 0) {
		if (countPtr.m_p)
			*(size_t*)countPtr.m_p = 0;

		return g_nullDataPtr;
	}

	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime);

	Type* classPtrType = (Type*)UsbDevice::getType(runtime->getModule())->getClassPtrType();
	DataPtr arrayPtr = runtime->getGcHeap()->allocateArray(classPtrType, count);
	UsbDevice** dstDeviceArray = (UsbDevice**) arrayPtr.m_p;
	libusb_device** srcDeviceArray = deviceList;

	for (size_t i = 0; i < count; i++) {
		UsbDevice* device = createClass<UsbDevice> (runtime);
		device->setDevice(srcDeviceArray[i]);
		dstDeviceArray[i] = device;
	}

	if (countPtr.m_p)
		*(size_t*)countPtr.m_p = count;

	return arrayPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
