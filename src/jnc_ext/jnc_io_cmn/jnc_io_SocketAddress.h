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

JNC_DECLARE_TYPE(Address_ip4)
JNC_DECLARE_TYPE(Address_ip6)
JNC_DECLARE_TYPE(SocketAddress_ip4)
JNC_DECLARE_TYPE(SocketAddress_ip6)
JNC_DECLARE_TYPE(SocketAddress)

//..............................................................................

enum AddressFamily
{
	AddressFamily_Undefined = 0,
	AddressFamily_Unused    = 1,
	AddressFamily_Ip4       = 2, // AF_INET
	AddressFamily_Ip6,           // actual value is platform-specific
	AddressFamily__Count,
};

//..............................................................................

struct Address_ip4: public in_addr
{
	JNC_DECLARE_TYPE_STATIC_METHODS(Address_ip4)

	static
	bool
	parse(
		DataPtr selfPtr,
		DataPtr stringPtr
		)
	{
		return axl::io::parseAddr_ip4(
			(in_addr*)selfPtr.m_p,
			(const char*) stringPtr.m_p
			);
	}

	static
	DataPtr
	getString(DataPtr selfPtr)
	{
		sl::String string = axl::io::getAddrString_ip4((const in_addr*) selfPtr.m_p);
		return strDup(string, string.getLength());
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct Address_ip6: public in6_addr
{
	JNC_DECLARE_TYPE_STATIC_METHODS(Address_ip6)

	static
	bool
	parse(
		DataPtr selfPtr,
		DataPtr stringPtr
		)
	{
		return axl::io::parseAddr_ip6(
			(in6_addr*)selfPtr.m_p,
			(const char*) stringPtr.m_p
			);
	}

	static
	DataPtr
	getString(DataPtr selfPtr)
	{
		sl::String string = axl::io::getAddrString_ip6((const in6_addr*) selfPtr.m_p);
		return strDup(string, string.getLength());
	}
};

//..............................................................................

struct SocketAddress_ip4: public sockaddr_in
{
	JNC_DECLARE_TYPE_STATIC_METHODS(SocketAddress_ip4)

	static
	bool
	isEqual(
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return axl::io::isSockAddrEqual_ip4(
			(const sockaddr_in*) selfPtr.m_p,
			(const sockaddr_in*) addressPtr.m_p
			);
	}

	static
	bool
	isMatch(
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return axl::io::isSockAddrMatch_ip4(
			(const sockaddr_in*) selfPtr.m_p,
			(const sockaddr_in*) addressPtr.m_p
			);
	}

	static
	bool
	parse(
		DataPtr selfPtr,
		DataPtr stringPtr
		)
	{
		return axl::io::parseSockAddr_ip4(
			(sockaddr_in*)selfPtr.m_p,
			(const char*) stringPtr.m_p
			);
	}

	static
	DataPtr
	getString(DataPtr selfPtr)
	{
		sl::String string = axl::io::getSockAddrString_ip4((const sockaddr_in*) selfPtr.m_p);
		return strDup(string, string.getLength());
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketAddress_ip6: public sockaddr_in6
{
	JNC_DECLARE_TYPE_STATIC_METHODS(SocketAddress_ip6)

	static
	bool
	isEqual(
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return axl::io::isSockAddrEqual_ip6(
			(const sockaddr_in6*) selfPtr.m_p,
			(const sockaddr_in6*) addressPtr.m_p
			);
	}

	static
	bool
	isMatch(
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return axl::io::isSockAddrMatch_ip6(
			(const sockaddr_in6*) selfPtr.m_p,
			(const sockaddr_in6*) addressPtr.m_p
			);
	}

	static
	bool
	parse(
		DataPtr selfPtr,
		DataPtr stringPtr
		)
	{
		return axl::io::parseSockAddr_ip6(
			(sockaddr_in6*)selfPtr.m_p,
			(const char*) stringPtr.m_p
			);
	}

	static
	DataPtr
	getString(DataPtr selfPtr)
	{
		sl::String string = axl::io::getSockAddrString_ip6((const sockaddr_in6*) selfPtr.m_p);
		return strDup(string, string.getLength());
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct SocketAddress
{
	JNC_DECLARE_TYPE_STATIC_METHODS(SocketAddress)

	union
	{
		uint16_t m_family;
		sockaddr_in m_addr_ip4;
		sockaddr_in6 m_addr_ip6;
	};

	static
	bool
	isEqual(
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return ((SocketAddress*)selfPtr.m_p)->getSockAddr().isEqual((const sockaddr*) addressPtr.m_p);
	}

	static
	bool
	isMatch(
		DataPtr selfPtr,
		DataPtr addressPtr
		)
	{
		return ((SocketAddress*)selfPtr.m_p)->getSockAddr().isMatch((const sockaddr*) addressPtr.m_p);
	}

	static
	bool
	parse(
		DataPtr selfPtr,
		DataPtr stringPtr
		);

	static
	DataPtr
	getString(DataPtr selfPtr)
	{
		sl::String string = ((SocketAddress*)selfPtr.m_p)->getSockAddr().getString();
		return strDup(string, string.getLength());
	}

	axl::io::SockAddr
	getSockAddr() const;

	void
	setSockAddr(const axl::io::SockAddr& sockAddr);

	static
	SocketAddress
	fromSockAddr(const axl::io::SockAddr& sockAddr);
};

//..............................................................................

} // namespace io
} // namespace jnc
