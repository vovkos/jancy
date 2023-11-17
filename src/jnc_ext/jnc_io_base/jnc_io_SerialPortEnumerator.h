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
	String m_deviceName;
	String m_description;
	String m_manufacturer;
	String m_hardwareIds;
	String m_driver;
	String m_location;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
enumerateSerialPorts(
	uint_t flags,
	DataPtr countPtr
);

//..............................................................................

} // namespace io
} // namespace jnc
