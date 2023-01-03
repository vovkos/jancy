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

#pragma once

#include "jnc_io_UsbDeviceDesc.h"

namespace jnc {
namespace io {

JNC_DECLARE_TYPE(UsbDeviceEntry)

class UsbDevice;

//..............................................................................

struct UsbDeviceEntry: UsbDeviceDesc {
	JNC_DECLARE_TYPE_STATIC_METHODS(UsbDeviceEntry)

	DataPtr m_nextPtr;
	UsbDevice* m_device;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
enumerateUsbDevices(
	uint_t flags,
	DataPtr countPtr
);

DataPtr
enumerateUsbDevicesNoDesc(DataPtr countPtr);

//..............................................................................

} // namespace io
} // namespace jnc
