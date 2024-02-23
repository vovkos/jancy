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
#include "jnc_io_SocketAddress.h"
#include "../jnc_io_base/jnc_io_IoLib.h"

namespace jnc {
namespace io {

//..............................................................................

bool
SocketAddress::isEqual(
	DataPtr selfPtr,
	DataPtr addressPtr
) {
	axl::io::SockAddr sockAddr1 = ((SocketAddress*)selfPtr.m_p)->getSockAddr();
	axl::io::SockAddr sockAddr2 = ((SocketAddress*)addressPtr.m_p)->getSockAddr();
	return sockAddr1.isEqual(sockAddr2);
}

bool
SocketAddress::isMatch(
	DataPtr selfPtr,
	DataPtr addressPtr
) {
	axl::io::SockAddr sockAddr1 = ((SocketAddress*)selfPtr.m_p)->getSockAddr();
	axl::io::SockAddr sockAddr2 = ((SocketAddress*)addressPtr.m_p)->getSockAddr();
	return sockAddr1.isMatch(sockAddr2);
}

bool
SocketAddress::parse(
	DataPtr selfPtr,
	String string
) {
	axl::io::SockAddr sockAddr;
	bool result = sockAddr.parse(string >> toAxl);
	if (!result)
		return false;

	((SocketAddress*)selfPtr.m_p)->setSockAddr(sockAddr);
	return true;
}

axl::io::SockAddr
SocketAddress::getSockAddr() const {
	axl::io::SockAddr sockAddr;

	ASSERT(sizeof(SocketAddress) == sizeof(sockAddr));
	memcpy(&sockAddr, this, sizeof(sockAddr));

	*(uint16_t*)&sockAddr = 0;
	sockAddr.m_addr.sa_family = m_family == AddressFamily_Ip6 ?
		AF_INET6 :
		m_family;

	return sockAddr;
}

void
SocketAddress::setSockAddr(const axl::io::SockAddr& sockAddr) {
	ASSERT(sizeof(SocketAddress) == sizeof(sockAddr));
	memcpy(this, &sockAddr, sizeof(SocketAddress));

	m_family = sockAddr.m_addr.sa_family == AF_INET6 ?
		AddressFamily_Ip6 :
		sockAddr.m_addr.sa_family;
}

SocketAddress
SocketAddress::fromSockAddr(const axl::io::SockAddr& sockAddr) {
	SocketAddress socketAddress;
	socketAddress.setSockAddr(sockAddr);
	return socketAddress;
}

//..............................................................................

JNC_DEFINE_TYPE(
	Address_ip4,
	"io.Address_ip4",
	g_ioLibGuid,
	IoLibCacheSlot_Address_ip4
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Address_ip4)
	JNC_MAP_FUNCTION("parse",     &Address_ip4::parse)
	JNC_MAP_FUNCTION("getString", &Address_ip4::getString)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(
	Address_ip6,
	"io.Address_ip6",
	g_ioLibGuid,
	IoLibCacheSlot_Address_ip6
)

JNC_BEGIN_TYPE_FUNCTION_MAP(Address_ip6)
	JNC_MAP_FUNCTION("parse",     &Address_ip6::parse)
	JNC_MAP_FUNCTION("getString", &Address_ip6::getString)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(
	SocketAddress_ip4,
	"io.SocketAddress_ip4",
	g_ioLibGuid,
	IoLibCacheSlot_SocketAddress_ip4
)

JNC_BEGIN_TYPE_FUNCTION_MAP(SocketAddress_ip4)
	JNC_MAP_FUNCTION("isEqual",   &SocketAddress_ip4::isEqual)
	JNC_MAP_FUNCTION("isMatch",   &SocketAddress_ip4::isMatch)
	JNC_MAP_FUNCTION("parse",     &SocketAddress_ip4::parse)
	JNC_MAP_FUNCTION("getString", &SocketAddress_ip4::getString)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(
	SocketAddress_ip6,
	"io.SocketAddress_ip6",
	g_ioLibGuid,
	IoLibCacheSlot_SocketAddress_ip6
)

JNC_BEGIN_TYPE_FUNCTION_MAP(SocketAddress_ip6)
	JNC_MAP_FUNCTION("isEqual",   &SocketAddress_ip6::isEqual)
	JNC_MAP_FUNCTION("isMatch",   &SocketAddress_ip6::isMatch)
	JNC_MAP_FUNCTION("parse",     &SocketAddress_ip6::parse)
	JNC_MAP_FUNCTION("getString", &SocketAddress_ip6::getString)
JNC_END_TYPE_FUNCTION_MAP()

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_DEFINE_TYPE(
	SocketAddress,
	"io.SocketAddress",
	g_ioLibGuid,
	IoLibCacheSlot_SocketAddress
)

JNC_BEGIN_TYPE_FUNCTION_MAP(SocketAddress)
	JNC_MAP_FUNCTION("isEqual",   &SocketAddress::isEqual)
	JNC_MAP_FUNCTION("isMatch",   &SocketAddress::isMatch)
	JNC_MAP_FUNCTION("parse",     &SocketAddress::parse)
	JNC_MAP_FUNCTION("getString", &SocketAddress::getString)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

} // namespace io
} // namespace jnc
