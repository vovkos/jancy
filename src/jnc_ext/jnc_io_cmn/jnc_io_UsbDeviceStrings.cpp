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
	dst->m_descriptionPtr = strDup(src->m_description);
	dst->m_manufacturerPtr = strDup(src->m_manufacturer);
	dst->m_driverPtr = strDup(src->m_driver);
	dst->m_manufacturerDescriptorPtr = strDup(src->m_manufacturerDescriptor);
	dst->m_productDescriptorPtr = strDup(src->m_productDescriptor);
	dst->m_serialNumberDescriptorPtr = strDup(src->m_serialNumberDescriptor);
}

//..............................................................................

} // namespace io
} // namespace jnc
