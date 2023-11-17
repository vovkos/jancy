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

#include "jnc_io_UsbMonDeviceDescBase.h"

namespace jnc {
namespace io {

JNC_DECLARE_TYPE(HidMonDeviceDesc)

//..............................................................................

struct HidMonDeviceDesc: UsbMonDeviceDescBase {
	JNC_DECLARE_TYPE_STATIC_METHODS(HidMonDeviceDesc)

	DataPtr m_nextPtr;
	String m_hidDeviceName;
	DataPtr m_reportDescriptorPtr;
	size_t m_reportDescriptorSize;

	uint16_t m_usagePage;
	uint16_t m_usage;
	uint16_t m_releaseNumber;
	uint8_t m_interfaceId;
	uint8_t m_inEndpointId;
	uint8_t m_outEndpointId;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
enumerateHidMonDevices(DataPtr countPtr);

//..............................................................................

} // namespace io
} // namespace jnc
