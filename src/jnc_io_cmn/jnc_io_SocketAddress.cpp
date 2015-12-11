#include "pch.h"
#include "jnc_io_SocketAddress.h"

namespace jnc {
namespace io {

//.............................................................................

bool
SocketAddress::parse (
	rt::DataPtr selfPtr,
	rt::DataPtr stringPtr
	)
{
	axl::io::SockAddr sockAddr;
	
	bool result = sockAddr.parse ((const char*) stringPtr.m_p);
	if (!result)
		return false;

	((SocketAddress*) selfPtr.m_p)->setSockAddr (sockAddr);
	return true;
}

axl::io::SockAddr
SocketAddress::getSockAddr () const
{
	axl::io::SockAddr sockAddr;

	ASSERT (sizeof (SocketAddress) == sizeof (sockAddr));
	memcpy (&sockAddr, this, sizeof (sockAddr));
	
	*(uint16_t*) &sockAddr = 0;
	sockAddr.m_addr.sa_family = m_family == AddressFamily_Ip6 ?
		AF_INET6 :
		m_family;

	return sockAddr;
}

void
SocketAddress::setSockAddr (const axl::io::SockAddr& sockAddr)
{
	ASSERT (sizeof (SocketAddress) == sizeof (sockAddr));
	memcpy (this, &sockAddr, sizeof (SocketAddress));

	m_family = sockAddr.m_addr.sa_family == AF_INET6 ?
		AddressFamily_Ip6 :
		sockAddr.m_addr.sa_family;
}

SocketAddress
SocketAddress::fromSockAddr (const axl::io::SockAddr& sockAddr)
{
	SocketAddress socketAddress;
	socketAddress.setSockAddr (sockAddr);
	return socketAddress;
}

//.............................................................................

} // namespace io
} // namespace jnc
