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
#include "jnc_io_SerialPortEnumerator.h"
#include "jnc_io_IoLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_TYPE(
	SerialPortDesc,
	"io.SerialPortDesc",
	g_ioLibGuid,
	IoLibCacheSlot_SerialPortDesc
)

JNC_BEGIN_TYPE_FUNCTION_MAP(SerialPortDesc)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

DataPtr
createSerialPortDesc(
	Runtime* runtime,
	axl::io::SerialPortDesc* portDesc
) {
	DataPtr portPtr = createData<SerialPortDesc>(runtime);
	SerialPortDesc* port = (SerialPortDesc*)portPtr.m_p;
	port->m_deviceName = allocateString(portDesc->m_deviceName);
	port->m_description = allocateString(portDesc->m_description);
	port->m_manufacturer = allocateString(portDesc->m_manufacturer);
	port->m_hardwareIds = allocateString(portDesc->m_hardwareIds);
	port->m_driver = allocateString(portDesc->m_driver);
	port->m_location = allocateString(portDesc->m_location);
	return portPtr;
}

DataPtr
enumerateSerialPorts(
	uint_t flags,
	DataPtr countPtr
) {
	sl::List<axl::io::SerialPortDesc> portList;
	axl::io::enumerateSerialPorts(&portList, flags);

	if (portList.isEmpty()) {
		if (countPtr.m_p)
			*(size_t*)countPtr.m_p = 0;

		return g_nullDataPtr;
	}

	Runtime* runtime = getCurrentThreadRuntime();
	NoCollectRegion noCollectRegion(runtime);

	sl::Iterator<axl::io::SerialPortDesc> it = portList.getHead();

	DataPtr portPtr = createSerialPortDesc(runtime, *it);
	DataPtr resultPtr = portPtr;
	size_t count = 1;

	SerialPortDesc* prevPort = (SerialPortDesc*)portPtr.m_p;
	for (it++; it; it++) {
		portPtr = createSerialPortDesc(runtime, *it);
		prevPort->m_nextPtr = portPtr;
		prevPort = (SerialPortDesc*)portPtr.m_p;
		count++;
	}

	if (countPtr.m_p)
		*(size_t*)countPtr.m_p = count;

	return resultPtr;
}

//..............................................................................

} // namespace io
} // namespace jnc
