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

namespace jnc {
namespace io {

//..............................................................................

struct UsbDeviceStrings {
	DataPtr m_descriptionPtr;
	DataPtr m_manufacturerPtr;
	DataPtr m_driverPtr;
	DataPtr m_manufacturerDescriptorPtr;
	DataPtr m_productDescriptorPtr;
	DataPtr m_serialNumberDescriptorPtr;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
initUsbDeviceStrings(
	UsbDeviceStrings* dst,
	const axl::io::UsbDeviceStrings* src
);

//..............................................................................

} // namespace io
} // namespace jnc
