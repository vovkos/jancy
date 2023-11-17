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
	String m_description;
	String m_manufacturer;
	String m_driver;
	String m_manufacturerDescriptor;
	String m_productDescriptor;
	String m_serialNumberDescriptor;
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
