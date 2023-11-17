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
#include "jnc_io_UsbMonDeviceDescBase.h"

namespace jnc {
namespace io {

//..............................................................................

void
initUsbMonDeviceDescBase(
	UsbMonDeviceDescBase* dst,
	const axl::io::UsbMonDeviceDesc* src
) {
	initUsbDeviceStrings(dst, src);

	dst->m_captureDeviceName = allocateString(src->m_captureDeviceName);
	dst->m_captureDeviceId = src->m_captureDeviceId;
	dst->m_vendorId = src->m_vendorId;
	dst->m_productId = src->m_productId;
	dst->m_address = src->m_address;
	dst->m_port = src->m_port;
	dst->m_class = src->m_class;
	dst->m_subClass = src->m_subClass;
	dst->m_speed = src->m_speed;
}

//..............................................................................

} // namespace io
} // namespace jnc
