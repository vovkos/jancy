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

JNC_DECLARE_TYPE(NetworkAdapterAddress)
JNC_DECLARE_TYPE(NetworkAdapterDesc)

//..............................................................................

struct NetworkAdapterAddress
{
	JNC_DECLARE_TYPE_STATIC_METHODS(NetworkAdapterAddress)

	DataPtr m_nextPtr;
	axl::io::SockAddr m_address;
	size_t m_netMaskBitCount;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct NetworkAdapterDesc
{
	JNC_DECLARE_TYPE_STATIC_METHODS(NetworkAdapterDesc)

	DataPtr m_nextPtr;
	DataPtr m_namePtr;
	DataPtr m_descriptionPtr;
	uint_t m_type;
	uint_t m_flags;
	uint8_t m_macAddress[6];
	DataPtr m_addressPtr;
	size_t m_addressCount;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createNetworkAdapterDescList(
	DataPtr adapterCountPtr,
	DataPtr addressCountPtr
	);

//..............................................................................

} // namespace io
} // namespace jnc
