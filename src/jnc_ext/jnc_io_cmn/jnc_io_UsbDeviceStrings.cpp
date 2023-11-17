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
#include "jnc_io_UsbDeviceStrings.h"

namespace jnc {
namespace io {

//..............................................................................

void
initUsbDeviceStrings(
	UsbDeviceStrings* dst,
	const axl::io::UsbDeviceStrings* src
) {
	dst->m_description = allocateString(src->m_description);
	dst->m_manufacturer = allocateString(src->m_manufacturer);
	dst->m_driver = allocateString(src->m_driver);
	dst->m_manufacturerDescriptor = allocateString(src->m_manufacturerDescriptor);
	dst->m_productDescriptor = allocateString(src->m_productDescriptor);
	dst->m_serialNumberDescriptor = allocateString(src->m_serialNumberDescriptor);
}

//..............................................................................

} // namespace io
} // namespace jnc
