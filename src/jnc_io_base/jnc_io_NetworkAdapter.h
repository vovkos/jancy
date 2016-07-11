#pragma once

#include "jnc_io_IoLibGlobals.h"

namespace jnc {
namespace io {

//.............................................................................

struct NetworkAdapterAddress
{
	JNC_BEGIN_TYPE_MAP ("io.NetworkAdapterAddress", g_ioLibCacheSlot, IoLibTypeCacheSlot_NetworkAdapterAddress)
	JNC_END_TYPE_MAP ()

	DataPtr m_nextPtr;
	axl::io::SockAddr m_address;
	size_t m_netMaskBitCount;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct NetworkAdapterDesc
{
	JNC_BEGIN_TYPE_MAP ("io.NetworkAdapterDesc", g_ioLibCacheSlot, IoLibTypeCacheSlot_NetworkAdapterDesc)
	JNC_END_TYPE_MAP ()

	DataPtr m_nextPtr;
	DataPtr m_namePtr;
	DataPtr m_descriptionPtr;
	uint_t m_type;
	uint_t m_flags;
	uint8_t m_mac [6];
	DataPtr m_addressPtr;
	size_t m_addressCount;
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

DataPtr
createNetworkAdapterDescList (
	DataPtr adapterCountPtr,
	DataPtr addressCountPtr	
	);

//.............................................................................

} // namespace io
} // namespace jnc
