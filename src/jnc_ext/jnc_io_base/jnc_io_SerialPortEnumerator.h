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

JNC_DECLARE_TYPE(SerialPortDesc)

//..............................................................................

struct SerialPortDesc {
	JNC_DECLARE_TYPE_STATIC_METHODS(SerialPortDesc)

	DataPtr m_nextPtr;
	DataPtr m_deviceNamePtr;
	DataPtr m_descriptionPtr;
	DataPtr m_manufacturerPtr;
	DataPtr m_hardwareIdsPtr;
	DataPtr m_driverPtr;
	DataPtr m_locationPtr;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
enumerateSerialPorts(
	uint_t mask,
	DataPtr countPtr
);

//..............................................................................

} // namespace io
} // namespace jnc
