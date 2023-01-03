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
#include "jnc_io_UsbDeviceDesc.h"

namespace jnc {
namespace io {

//..............................................................................

void
initUsbDeviceDesc(
	UsbDeviceDesc* dstDesc,
	const axl::io::UsbDeviceDesc* srcDesc
) {
	dstDesc->m_vendorId = srcDesc->m_vendorId;
	dstDesc->m_productId = srcDesc->m_productId;
	dstDesc->m_address = srcDesc->m_address;
	dstDesc->m_port = srcDesc->m_port;
	dstDesc->m_class = srcDesc->m_class;
	dstDesc->m_subClass = srcDesc->m_subClass;
	dstDesc->m_speed = srcDesc->m_speed;
	dstDesc->m_manufacturerDescriptorId = srcDesc->m_manufacturerDescriptorId;
	dstDesc->m_productDescriptorId = srcDesc->m_productDescriptorId;
	dstDesc->m_serialNumberDescriptorId = srcDesc->m_serialNumberDescriptorId;

	dstDesc->m_descriptionPtr = strDup(srcDesc->m_description);
	dstDesc->m_manufacturerPtr = strDup(srcDesc->m_manufacturer);
	dstDesc->m_driverPtr = strDup(srcDesc->m_driver);
	dstDesc->m_manufacturerDescriptorPtr = strDup(srcDesc->m_manufacturerDescriptor);
	dstDesc->m_productDescriptorPtr = strDup(srcDesc->m_productDescriptor);
	dstDesc->m_serialNumberDescriptorPtr = strDup(srcDesc->m_serialNumberDescriptor);
}

//..............................................................................

} // namespace io
} // namespace jnc
